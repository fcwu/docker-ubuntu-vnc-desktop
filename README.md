docker-ubuntu-vnc-desktop
=========================

```
git clone https://github.com/fcwu/docker-ubuntu-vnc-desktop.git
docker build --rm -t doro/ubuntu-vnc-desktop docker-ubuntu-vnc-desktop
docker run -i -t --entrypoint=/bin/bash -p 5900:5900 doro/ubuntu-vnc-desktop
vncview 127.0.0.1:5900
```
