docker-ubuntu-vnc-desktop
=========================

```
git clone https://github.com/fcwu/docker-ubuntu-vnc-desktop.git
docker build --rm -t doro/ubuntu-vnc-desktop docker-ubuntu-vnc-desktop
docker run -i -t --entrypoint=/bin/bash -p 6080:6080 doro/ubuntu-vnc-desktop
```

Browse http://127.0.0.1:6080/vnc.html

<img src="https://raw.github.com/fcwu/docker-ubuntu-vnc-desktop/master/screenshots/lxde.png" width=400/>
