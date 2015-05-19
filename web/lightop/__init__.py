from flask import (Flask,
                   request,
                   render_template,
                   abort,
                   Response,
                   redirect,
                   )
import os


# Flask app
app = Flask(__name__,
            static_folder='static', static_url_path='',
            instance_relative_config=True)
CONFIG = os.environ.get('CONFIG') or 'config.Development'
app.config.from_object('config.Default')
app.config.from_object(CONFIG)
app.config.from_pyfile('application.cfg')

# logging
import logging
from log.config import LoggingConfiguration
LoggingConfiguration.set(
    logging.DEBUG if os.getenv('DEBUG') else logging.INFO,
    'lightop.log', name='Web')

from auth import auth
auth.init_app(app, app.config['PHASE'])


from gevent import spawn, sleep
from geventwebsocket import WebSocketError
import requests
import websocket
import docker
import json
from functools import wraps
import subprocess
import datetime
import sha
import re

from db.sql import User as DbUser


CHUNK_SIZE = 1024
CID2IMAGE = {'ubuntu-trusty-ttyjs': 'dorowu/lightop-ubuntu-trusty-ttyjs',
             'ubuntu-trusty-lxde': 'dorowu/lightop-ubuntu-trusty-lxde'}
RE_OWNER_CNAME = re.compile('^/(.*)_({})$'.format('|'.join(CID2IMAGE.keys())))


def exception_to_json(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        try:
            result = func(*args, **kwargs)
            return result
        except (BadRequest,
                KeyError,
                ValueError,
                ) as e:
            result = {'error': {'code': 400,
                                'message': str(e)}}
        except PermissionDenied as e:
            result = {'error': {'code': 403,
                                'message': ', '.join(e.args)}}
        except (NotImplementedError, RuntimeError, AttributeError) as e:
            result = {'error': {'code': 500,
                                'message': ', '.join(e.args)}}
        return json.dumps(result)
    return wrapper


class PermissionDenied(Exception):
    pass


class BadRequest(Exception):
    pass


@app.route('/')
def index():
    return redirect("index.html")


@app.route('/<path:url>')
@auth.login_required
def root(url):
    logging.info("Root route, path: %s", url)
    # If referred from a proxy request, then redirect to a URL with the proxy prefix.
    # This allows server-relative and protocol-relative URLs to work.
    proxy_ref = proxy_ref_info(request)
    if proxy_ref:
        redirect_url = "/p/%s/%s%s" % (proxy_ref[0], url, ("?" + request.query_string if request.query_string else ""))
        logging.info("Redirecting referred URL to: %s", redirect_url)
        return redirect(redirect_url)
    # Otherwise, default behavior
    return render_template('hello.html', name=url, request=request)


@app.route('/u/<cid>/')
@auth.login_required
def proxy_user_root(cid):
    return proxy_user(cid, '')


def container_create_and_network(cid):
    user = auth.current_user.username()
    cname = user + '_' + cid
    dc = docker.Client()
    # create container
    for c in dc.containers(all=True):
        #logging.info(str(c['Names']))
        if '/' + cname in c['Names']:
            break
    else:
        if cid not in CID2IMAGE:
            raise BadRequest(cid, 'not exist')
        try:
            os.makedirs('mnt/home/' + user)
        except OSError:
            pass
        try:
            os.makedirs('mnt/public')
        except OSError:
            pass
        env = ['USER=' + user, 'PASS=' + user]
        if 'width' in request.args:
            env.append('WIDTH=' + str(request.args['width']))
        if 'height' in request.args:
            env.append('HEIGHT=' + str(request.args['height']))
        logging.info('create container')
        dc.create_container(CID2IMAGE[cid], name=cname,
                            volumes=['/home/' + user, '/mnt/public'],
                            environment=env)
    cinfo = dc.inspect_container(user + '_' + cid)
    # start container
    logging.info(cinfo['State']['Running'])
    if not cinfo['State']['Running']:
        binds = {}
        binds[os.path.join(os.getcwd(), 'mnt', 'home', user)] = {'bind': '/home/' + user, 'ro': False}
        binds[os.path.join(os.getcwd(), 'mnt', 'public')] = {'bind': '/mnt/public', 'ro': False}
        logging.info('start container')
        dc.start(cname, binds=binds)
        cinfo = dc.inspect_container(user + '_' + cid)
    # get ip and port
    ipaddr = cinfo['NetworkSettings']['IPAddress']
    for p in cinfo['NetworkSettings']['Ports'].keys():
        port, proto = p.split('/')
        if port != '22':
            port = int(port)
            break
    else:
        raise RuntimeError('port', cinfo['NetworkSettings']['Ports'])

    start = datetime.datetime.now()
    while True:
        now = datetime.datetime.now()
        if (now - start).total_seconds() > 10:
            logging.error('probe failed')
            raise RuntimeError('probe failed')
        try:
            r = requests.get('http://{}:{}/'.format(ipaddr, port))
            if 200 <= r.status_code < 400:
                break
        except Exception:
            pass
        sleep(1)
    return ipaddr, port


# hijact LXDE VNC
@app.route('/u/ubuntu-trusty-lxde/<path:path>')
@auth.login_required
def proxy_user_vnc(path):
    logging.info('vnc ' + path)
    # auth pass
    if path.endswith('/websockify'):
        logging.info('vnc done')
        return ''
    if path != 'vnc_auto.html':
        return proxy_user('ubuntu-trusty-lxde', path)
    if len(request.args.get('hijact', '')) >= 1:
        return proxy_user('ubuntu-trusty-lxde', path)
    ipaddr, port = container_create_and_network('ubuntu-trusty-lxde')
    user = auth.current_user.username()
    #TODO remove when user deleted
    subprocess.check_call(r"sed -i '/^location .*ubuntu-trusty-lxde\/{user}\//,/}}/d' nginx/ws-login.conf".format(user=user),
                          shell=True)
    with open('nginx/ws-login.conf', 'a+') as f:
        f.write('\nlocation /u/ubuntu-trusty-lxde/{user}/websockify\n'
                '{{\n'
                '    auth_request /login_refresh_code;\n'
                '    proxy_pass http://{ipaddr}:{port}/websockify;\n'
                '    proxy_redirect off;\n'
                '    proxy_buffering off;\n'
                '    proxy_set_header Host $host;\n'
                '    proxy_set_header X-Real-IP $remote_addr;\n'
                '    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n'
                '    proxy_http_version 1.1;\n'
                '    proxy_set_header Upgrade $http_upgrade;\n'
                '    proxy_set_header Connection "Upgrade";\n'
                '}}'.format(user=user, ipaddr=ipaddr, port=port)
                )
    subprocess.check_call('sudo nginx -c ' + os.getcwd() +
                          '/nginx.conf -s reload', shell=True)

    geometry = ''
    geometry += '&width=' + request.args.get('width', '1024')
    geometry += '&height=' + request.args.get('height', '768')
    return redirect('/u/ubuntu-trusty-lxde/' +
                    'vnc.html' +
                    ('?host={host}&port={port}&path={path}'
                     '&hijact=1&autoconnect=1{geometry}').format(
                         host=re.findall('https?://([^/:]+)([:0-9]*)/', request.url_root)[0][0],
                         port=6051,
                         path='u/ubuntu-trusty-lxde/' + user + '/websockify',
                         geometry=geometry)
                    )


@app.route('/u/<cid>/<path:path>')
@auth.login_required
def proxy_user(cid, path):
    try:
        ipaddr, port = container_create_and_network(cid)
    except docker.errors.APIError as e:
        return json.dumps({'status': 400, 'message': str(e)})
    except RuntimeError as e:
        return json.dumps({'status': 500, 'message': str(e)})

    # websocket
    if request.environ.get('wsgi.websocket'):
        return proxy_user_websocket(ipaddr, port, path)

    # page
    url = '%s:%d' % (ipaddr, port)
    if len(path) > 0:
        url += '/' + path
    r = get_source_rsp(url)
    logging.info("Got %s response from %s", r.status_code, url)
    headers = dict(r.headers)

    def generate():
        for chunk in r.iter_content(CHUNK_SIZE):
            yield chunk
    return Response(generate(), headers=headers)


@app.route("/session", methods=["GET"])
@exception_to_json
def sessions():
    return json.dumps(CID2IMAGE.keys())


@app.route("/user/", methods=["GET"])
@exception_to_json
@auth.login_required
def users():
    user = auth.current_user.username()
    if user != 'admin':
        raise PermissionDenied('admin only')
    result = []
    for u in DbUser.select():
        result.append({'name': u.user,
                       'id': u.id,
                       'volume': ['/mnt/public', '/home/' + u.user]})
    return json.dumps(result)


@app.route("/user/", methods=["POST"])
@exception_to_json
@auth.login_required
def user_create():
    user = auth.current_user.username()
    if user != 'admin':
        raise PermissionDenied('admin only')
    try:
        username = request.form['username']
        password = request.form['password']
    except KeyError:
        raise BadRequest('username or password')
    u = DbUser.create(user=username, password=sha.new(password).hexdigest())
    return user_detail(u.id)


@app.route("/user/<int:uid>", methods=["GET"])
@exception_to_json
@auth.login_required
def user_detail(uid):
    user = auth.current_user.username()
    if user != 'admin':
        raise PermissionDenied('admin only')
    try:
        u = DbUser.get(DbUser.id == uid)
    except:
        return '{}'
    return json.dumps({'name': u.user,
                       'id': u.id,
                       'volume': ['/mnt/public', '/home/' + u.user]})


@app.route("/user/<int:uid>", methods=["DELETE"])
@exception_to_json
@auth.login_required
def user_delete(uid):
    user = auth.current_user.username()
    if user != 'admin':
        raise PermissionDenied('admin only')
    try:
        u = DbUser.get(DbUser.id == uid)
    except:
        return json.dumps({'num': 0})
    return json.dumps({'num': u.delete_instance()})


@app.route("/login", methods=["POST", "PUT"])
@exception_to_json
def login():
    """Login
    """
    try:
        kargs = dict()
        kargs['username'] = request.form['username']
        kargs['password'] = request.form['password']
        kargs['remember'] = request.form['remember'] \
            if 'remember' in request.form else False
    except KeyError:
        raise BadRequest('username, password or sid')
    user = auth.login(**kargs)
    if user is not None:
        if user.is_anonymous():
            return json.dumps({'username': 'nobody',
                               'isAdmin': False,
                               'anonymous': True})
        return json.dumps({'username': user.username(),
                           'isAdmin': user.is_admin()})
    raise PermissionDenied('Wrong user name or password')


@app.route("/container/", methods=["GET"])
@exception_to_json
@auth.login_required
def containers():
    user = auth.current_user.username()
    if user != 'admin':
        raise PermissionDenied('admin only')
    result = []
    dc = docker.Client()
    for c in dc.containers():
        r = RE_OWNER_CNAME.match(c['Names'][0])
        if r is None:
            continue
        result.append({'id': c['Id'],
                       'session': r.group(2),
                       'owner': r.group(1)})
    return json.dumps(result)


@app.route("/container/<string:cid>", methods=["DELETE"])
@exception_to_json
@auth.login_required
def container_delete(cid):
    user = auth.current_user.username()
    if user != 'admin':
        raise PermissionDenied('admin only')
    try:
        dc = docker.Client()
        logging.info(cid)
        c = dc.inspect_container(cid)
        r = RE_OWNER_CNAME.match(c['Name'])
        if r is None:
            raise RuntimeError()
        dc.kill(cid)
        dc.remove_container(cid)
    except Exception as e:
        logging.error(str(e))
        return json.dumps({'num': 0})
    return json.dumps({'num': 1})


@app.route("/login_refresh", methods=["GET"])
@exception_to_json
@auth.login_required
def login_refresh():
    """Refresh token
    """
    user = auth.current_user
    #raise PermissionDenied('Not a valid user')
    return json.dumps({'username': user.username(), 'isAdmin': False})


@app.route("/login_refresh_code", methods=["GET"])
@exception_to_json
@auth.login_required
def login_refresh_code():
    """Refresh token
    """
    logging.info('!!!!!!!!!!!!!!!!!  refresh code')
    user = auth.current_user
    #raise PermissionDenied('Not a valid user')
    return json.dumps({'username': user.username(), 'isAdmin': False})


@app.route("/logout", methods=["PUT"])
@exception_to_json
@auth.login_required
def logout():
    """Logout
    """
    try:
        username = auth.current_user.username()
    except KeyError:
        return json.dumps({'error': {'code': 400}})
    if auth.logout(username):
        return json.dumps({'username': username})
    return json.dumps({'error': {'code': 403}})


def proxy_user_websocket(ipaddr, port, path):
    def c2s(client, server):
        while True:
            inp = client.receive()
            if inp is None:
                raise WebSocketError()
            server.send(inp)

    def get_headers():
        headers = []
        #for header in request.environ:
        #    if not header.startswith('HTTP_'):
        #        continue
        #    if not header.startswith('HTTP_SEC_') \
        #            and not header.startswith('HTTP_ACCEPT_') \
        #            and not header.startswith('HTTP_USER_AGENT'):
        #        continue
        #    upper = True
        #    k = ''
        #    for c in header[5:].replace('_', '-').lower():
        #        if upper:
        #            k += c.upper()
        #            upper = False
        #        else:
        #            k += c
        #        if c == '-':
        #            upper = True
        #    headers.append('%s: %s' % (k, request.environ[header]))
        return headers

    #https://stackoverflow.com/questions/18240358/html5-websocket-connecting-to-python
    client = request.environ['wsgi.websocket']
    url = '%s:%d' % (ipaddr, port)
    if len(path) > 0:
        url += '/' + path
    logging.info('websocket: ' + url)
    headers = []
    #headers = get_headers()
    #logging.info('headers: ' + str(headers))
    server = websocket.create_connection("ws://" + url, header=headers)
    try:
        spawn(c2s, client, server)
        while True:
            inp = server.recv()
            if inp is None:
                raise WebSocketError()
            client.send(inp)
    except WebSocketError as e:
        logging.error(e)
    except client.WebSocketConnectionClosedException:
        pass
    return json.dumps({'status': 200})


def get_source_rsp(url):
    url = 'http://%s' % url
    logging.info("Fetching %s", url)
    # Ensure the URL is approved, else abort
    if not is_approved(url):
        logging.warn("URL is not approved: %s", url)
        abort(403)
    # Pass original Referer for subsequent resource requests
    proxy_ref = proxy_ref_info(request)
    headers = {"Referer": "http://%s/%s" % (proxy_ref[0], proxy_ref[1])} if proxy_ref else {}
    # Fetch the URL, and stream it back
    logging.info("Fetching with headers: %s, %s", url, headers)
    return requests.get(url, stream=True, params=request.args, headers=headers)


def is_approved(url):
    return True


def split_url(url):
    """Splits the given URL into a tuple of (protocol, host, uri)"""
    proto, rest = url.split(':', 1)
    rest = rest[2:].split('/', 1)
    host, uri = (rest[0], rest[1]) if len(rest) == 2 else (rest[0], "")
    return (proto, host, uri)


def proxy_ref_info(request):
    """Parses out Referer info indicating the request is from a previously proxied page.

    For example, if:
        Referer: http://localhost:8080/p/google.com/search?q=foo
    then the result is:
        ("google.com", "search?q=foo")
    """
    ref = request.headers.get('referer')
    if ref:
        _, _, uri = split_url(ref)
        if uri.find("/") < 0:
            return None
        first, rest = uri.split("/", 1)
        if first in "pd":
            parts = rest.split("/", 1)
            r = (parts[0], parts[1]) if len(parts) == 2 else (parts[0], "")
            logging.info("Referred by proxy host, uri: %s, %s", r[0], r[1])
            return r
    return None


for image in CID2IMAGE.values():
    image = image.split(':')[0]
    cmd = 'docker images | grep -q "^{0} " || docker pull {0}'.format(image)
    logging.info(cmd)
    subprocess.check_call(cmd, shell=True)


try:
    os.makedirs('nginx')
except:
    pass
with open('nginx/ws-login.conf', 'w+') as f:
    f.truncate()


if __name__ == '__main__':
    app.run(host=app.config['ADDRESS'], port=app.config['PORT'])
