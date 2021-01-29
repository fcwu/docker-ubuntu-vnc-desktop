#!/usr/bin/env python3
from __future__ import (
    absolute_import, division, print_function, with_statement
)
import os
import time
import sys
import subprocess
from vnc.util import ignored


def main():
    def run_with_reloader(main_func, extra_files=None, interval=3):
        """Run the given function in an independent python interpreter."""
        def find_files(directory="./"):
            for root, dirs, files in os.walk(directory):
                for basename in files:
                    if basename.endswith('.py'):
                        filename = os.path.join(root, basename)
                        yield filename

        if os.environ.get('WERKZEUG_RUN_MAIN') == 'true':
            try:
                main_func()
            except KeyboardInterrupt:
                pass
            return

        proc = None
        try:
            while True:
                log.info('Restarting with reloader {} {}'.format(
                    sys.executable,
                    ' '.join(sys.argv))
                )
                args = [sys.executable] + sys.argv
                new_environ = os.environ.copy()
                new_environ['WERKZEUG_RUN_MAIN'] = 'true'

                proc = subprocess.Popen(
                    args,
                    env=new_environ,
                    close_fds=True,
                    preexec_fn=os.setsid
                )
                mtimes = {}
                restart = False
                while not restart:
                    for filename in find_files():
                        try:
                            mtime = os.stat(filename).st_mtime
                        except OSError:
                            continue

                        old_time = mtimes.get(filename)
                        if old_time is None:
                            mtimes[filename] = mtime
                            continue
                        elif mtime > old_time:
                            log.info(
                                'Detected change in {}, reloading'.format(
                                    filename
                                )
                            )
                            restart = True
                            proc.terminate()
                            break
                    time.sleep(interval)
        except KeyboardInterrupt:
            pass
        finally:
            with ignored(Exception):
                proc.terminate()

    def run_server():
        import socket
        from gevent.pywsgi import WSGIServer
        from vnc.app import app

        # websocket conflict: WebSocketHandler
        if DEBUG:
            # from werkzeug.debug import DebuggedApplication
            app.debug = True
            # app = DebuggedApplication(app, evalex=True)

        try:
            log.info('Listening on http://localhost:{}'.format(PORT))
            http_server = WSGIServer(('localhost', PORT), app)
            http_server.serve_forever()
            # app.run(host='localhost', port=PORT)
        except socket.error as e:
            log.exception(e)
        except KeyboardInterrupt:
            pass
        finally:
            http_server.stop(timeout=10)
            log.info('shutdown gracefully')

    PORT = 6079
    DEBUG = False
    os.environ['CONFIG'] = 'config.Production'
    entrypoint = run_server
    if '--debug' in sys.argv:
        DEBUG = True
        os.environ['CONFIG'] = 'config.Development'
        entrypoint = lambda: run_with_reloader(run_server)

    # logging
    import logging
    from log.config import LoggingConfiguration
    LoggingConfiguration.set(
        logging.DEBUG if DEBUG else logging.INFO,
        '/var/log/web.log'
    )
    logging.getLogger("werkzeug").setLevel(logging.WARNING)
    log = logging.getLogger('novnc2')

    entrypoint()


if __name__ == "__main__":
    main()
