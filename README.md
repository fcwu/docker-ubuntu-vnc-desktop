docker-ubuntu-vnc-desktop
=========================

From Docker Index
```
docker pull dorowu/ubuntu-desktop-lxde-vnc
```

Build yourself
```
git clone https://github.com/fcwu/docker-ubuntu-vnc-desktop.git
docker build --rm -t doro/ubuntu-vnc-desktop docker-ubuntu-vnc-desktop
```

Run
```
docker run -i -t -p 6080:6080 doro/ubuntu-vnc-desktop
```

Browse http://127.0.0.1:6080/vnc.html

<img src="https://raw.github.com/fcwu/docker-ubuntu-vnc-desktop/master/screenshots/lxde.png" width=400/>
