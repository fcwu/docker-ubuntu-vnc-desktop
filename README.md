docker-ubuntu-vnc-desktop
=========================

[![Docker Pulls](https://img.shields.io/docker/pulls/dorowu/ubuntu-desktop-lxde-vnc.svg)](https://hub.docker.com/r/dorowu/ubuntu-desktop-lxde-vnc/)
[![Docker Stars](https://img.shields.io/docker/stars/dorowu/ubuntu-desktop-lxde-vnc.svg)](https://hub.docker.com/r/dorowu/ubuntu-desktop-lxde-vnc/)

From Docker Index
```
docker pull dorowu/ubuntu-desktop-lxde-vnc
```

Build yourself
```
git clone https://github.com/fcwu/docker-ubuntu-vnc-desktop.git
docker build --rm -t dorowu/ubuntu-desktop-lxde-vnc docker-ubuntu-vnc-desktop
```

Run
```
docker run -it --rm -p 6080:80 dorowu/ubuntu-desktop-lxde-vnc
```

Browse http://127.0.0.1:6080/

<img src="https://raw.github.com/fcwu/docker-ubuntu-vnc-desktop/master/screenshots/lxde.png" width=400/>


Connect with VNC Viewer and protect by VNC Password
==================

Forward VNC service port 5900 to host by

```
docker run -it --rm -p 6080:80 -p 5900:5900 dorowu/ubuntu-desktop-lxde-vnc
```

Now, open the vnc viewer and connect to port 5900. If you would like to protect vnc service by password, set environment variable `VNC_PASSWORD`, for example

```
docker run -it --rm -p 6080:80 -p 5900:5900 -e VNC_PASSWORD=mypassword dorowu/ubuntu-desktop-lxde-vnc
```

A prompt will ask password either in the browser or vnc viewer.


Troubleshooting
==================

1. boot2docker connection issue, https://github.com/fcwu/docker-ubuntu-vnc-desktop/issues/2


License
==================

See the LICENSE file for details.
