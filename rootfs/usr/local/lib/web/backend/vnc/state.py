from __future__ import (
    absolute_import, division, print_function, with_statement
)
from os import environ
from gevent.event import Event
from gevent import subprocess as gsp
from re import search as research
from .log import log


class State(object):
    def __init__(self):
        self._eid = 0
        self._event = Event()
        self._w = self._h = self._health = None
        self.size_changed_count = 0

    def wait(self, eid, timeout=5):
        if eid < self._eid:
            return
        self._event.clear()
        self._event.wait(timeout)
        return self._eid

    def notify(self):
        self._eid += 1
        self._event.set()

    def _update_health(self):
        health = True
        output = gsp.check_output([
            'supervisorctl', '-c', '/etc/supervisor/supervisord.conf',
            'status'
        ], encoding='UTF-8')
        for line in output.strip().split('\n'):
            if not line.startswith('web') and line.find('RUNNING') < 0:
                health = False
                break
        if self._health != health:
            self._health = health
            self.notify()
        return self._health

    def to_dict(self):
        self._update_health()

        state = {
            'id': self._eid,
            'config': {
                'fixedResolution': 'RESOLUTION' in environ,
                'sizeChangedCount': self.size_changed_count
            }
        }

        self._update_size()
        state.update({
            'width': self.w,
            'height': self.h,
        })

        return state

    def set_size(self, w, h):
        gsp.check_call((
            'sed -i \'s#'
            '^exec /usr/bin/Xvfb.*$'
            '#'
            'exec /usr/bin/Xvfb :1 -screen 0 {}x{}x24'
            '#\' /usr/local/bin/xvfb.sh'
        ).format(w, h), shell=True)
        self.size_changed_count += 1

    def apply_and_restart(self):
        gsp.check_call([
            'supervisorctl', '-c', '/etc/supervisor/supervisord.conf',
            'restart', 'x:'
        ])
        self._w = self._h = self._health = None
        self.notify()

    def switch_video(self, onoff):
        xenvs = {
            'DISPLAY': ':1',
        }
        try:
            cmd = 'nofb' if onoff else 'fb'
            gsp.check_output(['x11vnc', '-remote', cmd], env=xenvs)
        except gsp.CalledProcessError as e:
            log.warn('failed to set x11vnc fb: ' + str(e))

    def _update_size(self):
        if self._w is not None and self._h is not None:
            return
        xenvs = {
            'DISPLAY': ':1',
        }
        try:
            output = gsp.check_output([
                'x11vnc', '-query', 'dpy_x,dpy_y'
            ], env=xenvs).decode('utf-8')
            mobj = research(r'dpy_x:(\d+).*dpy_y:(\d+)', output)
            if mobj is not None:
                w, h = int(mobj.group(1)), int(mobj.group(2))
                changed = False
                if self._w != w:
                    changed = True
                    self._w = w
                if self._h != h:
                    changed = True
                    self._h = h
                if changed:
                    self.notify()
        except gsp.CalledProcessError as e:
            log.warn('failed to get dispaly size: ' + str(e))

    def reset_size(self):
        self.size_changed_count = 0

    @property
    def w(self):
        return self._w

    @property
    def h(self):
        return self._h

    @property
    def health(self):
        self._update_health()
        return self._health


state = State()
