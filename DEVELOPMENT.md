# Get code

```
git clone --recursive https://github.com/fcwu/docker-ubuntu-vnc-desktop
```

or, if you have already cloned it, get submodules contents :
```
git submodule init; git submodule update
```

# Test local code

## Test-run in container rebuilt from local repo

You may edit the code in your local copy of the repo, rebuild the
container, and test the changes:

```
make clean
FLAVOR=lxqt ARCH=amd64 IMAGE=ubuntu:18.04 make build
make run
```

## develop backend

You may wish to work on the backend app. As the "make run" makes sure
to mount the current dir contents under /src in the container, you can
proceed as such (no compilation of the Python code):
```
make shell
supervisorctl -c /etc/supervisor/supervisord.conf stop web
cd /src/image/usr/local/lib/web/backend
./run.py --debug
```

## develop frontend

```
cd web
yarn add
BACKEND=http://127.0.0.1:6080 npm run dev
```
