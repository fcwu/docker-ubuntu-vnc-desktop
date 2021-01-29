from __future__ import (
    absolute_import, division, print_function, with_statement
)
import re
import os
from flask import (
    Flask,
    request,
    Response,
    jsonify,
    abort,
)
from gevent import subprocess as gsp, spawn, sleep
from geventwebsocket.exceptions import WebSocketError
from .response import httperror
from .util import ignored
from .state import state
from .log import log


# Flask app
app = Flask('novnc2')
app.config.from_object('config.Default')
app.config.from_object(os.environ.get('CONFIG') or 'config.Development')


@app.route('/api/state')
@httperror
def apistate():
    state.wait(int(request.args.get('id', -1)), 30)
    state.switch_video(request.args.get('video', 'false') == 'true')
    mystate = state.to_dict()
    return jsonify({
        'code': 200,
        'data': mystate,
    })


@app.route('/api/health')
def apihealth():
    if state.health:
        return 'success'
    abort(503, 'unhealthy')


@app.route('/api/reset')
def reset():
    if 'w' in request.args and 'h' in request.args:
        args = {
            'w': int(request.args.get('w')),
            'h': int(request.args.get('h')),
        }
        state.set_size(args['w'], args['h'])

    state.apply_and_restart()

    # check all running
    for i in range(40):
        if state.health:
            break
        sleep(1)
        log.info('wait services is ready...')
    else:
        return jsonify({
            'code': 500,
            'errorMessage': 'service is not ready, please restart container'
        })
    return jsonify({'code': 200})


@app.route('/resize')
@httperror
def apiresize():
    state.reset_size()
    return '<html><head><script type = "text/javascript">var h=window.location.href;window.location.href=h.substring(0,h.length-6);</script></head></html>'


@app.route('/api/live.flv')
@httperror
def liveflv():
    def generate():
        xenvs = {
            'DISPLAY': ':1',
        }
        bufsize = 1024 * 1
        framerate = 20

        # sound
        sound_cmd_input = []
        sound_cmd_parameters = []
        zero_latency_make_sound_not_good = [
            '-tune', 'zerolatency',
        ]

        xenvs['X_WIDTH'] = state.w
        xenvs['X_HEIGHT'] = state.h
        xenvs['X_WIDTH'] -= state.w % 2
        xenvs['X_HEIGHT'] -= state.h % 2

        pixels_count = xenvs['X_WIDTH'] * xenvs['X_HEIGHT']
        # factor (720p)
        #    383: 2400k
        #    300: 3000k
        #    230: 4000k
        factor = 265
        maxbitrate_cmd = [
            '-maxrate', str(int(pixels_count / factor)) + 'k',
            '-bufsize', str(int(pixels_count / factor / 3)) + 'k'
        ]

        # TODO move to global
        # get default source
        sound_cmd_input = [
            '-f', 'alsa',
            '-i', 'hw:2,1',
        ]
        sound_cmd_parameters = [
            '-ar', '44100',
            '-c:a', 'mp3',
        ]
        # flv.js report error if enabling hw acceleration
        # hwaccel_dev = ['-vaapi_device', '/dev/dri/renderD128']
        # hwaccel_if = ['-vf', 'format=nv12,hwupload']
        # vcodec = 'h264_vaapi'
        hwaccel_dev = []
        hwaccel_if = []
        vcodec = 'libx264'
        # zero_latency_make_sound_not_good = []
        # sound_cmd_parameters = []
        # sound_cmd_input = []
        cmd = ['/usr/local/ffmpeg/ffmpeg'] + sound_cmd_input + hwaccel_dev + [
            '-video_size', '{X_WIDTH}x{X_HEIGHT}'.format(**xenvs),
            '-framerate', '{}'.format(framerate),
            '-f', 'x11grab', '-draw_mouse', '1',
            '-i', '{DISPLAY}'.format(**xenvs),
        ] + hwaccel_if + [
            '-r', '{}'.format(framerate),
            '-g', '{}'.format(framerate),
            '-flags:v', '+global_header',
            '-vcodec', vcodec,
            '-preset', 'ultrafast',
            '-b_strategy', '0',
            '-pix_fmt', 'yuv420p',
            '-bsf:v', 'dump_extra=freq=e',
        ] + maxbitrate_cmd \
            + sound_cmd_parameters + zero_latency_make_sound_not_good + [
            '-f', 'flv', 'pipe:1',
        ]
        log.info('command: ' + ' '.join(cmd))
        pobj = gsp.Popen(
            cmd,
            stdout=gsp.PIPE,
            stderr=gsp.PIPE,
            env={k: str(v) for k, v in xenvs.items()},
        )

        def readerr(f):
            reobj = re.compile(r'bitrate=(\S+)')
            global av_bitrate
            try:
                while True:
                    buf = f.read(bufsize)
                    if len(buf) == 0:
                        break
                    patterns = reobj.findall(buf.decode('utf-8', 'ignore'))
                    if len(patterns) > 0:
                        av_bitrate = patterns[-1]
                    # log.info(str(buf))
            except Exception as e:
                log.exception(e)

        preaderr = None
        try:
            preaderr = spawn(readerr, pobj.stderr)
            try:
                while True:
                    buf = pobj.stdout.read(bufsize)
                    if len(buf) == 0:
                        break
                    # ws.send(buf)
                    yield buf
            except WebSocketError:
                pass
            except Exception as e:
                log.exception(e)
            finally:
                with ignored(Exception):
                    pobj.kill()
            preaderr.join()
        except Exception as e:
            log.exception(e)
        finally:
            log.info('exiting')
            with ignored(Exception):
                pobj.kill()
            with ignored(Exception):
                preaderr.kill()
            log.info('exited')
    return Response(generate(), mimetype='video/x-flv')


if __name__ == '__main__':
    app.run(host=app.config['ADDRESS'], port=app.config['PORT'])
