# Get code

```
git clone --recursive https://github.com/fcwu/docker-ubuntu-vnc-desktop
```

# Run in local
```
make clean
FLAVOR=lxqt ARCH=amd64 IMAGE=ubuntu:18.04 make build
make run
```

## develop backend
```
make shell
supervisorctl stop web
cd /src/image/usr/local/lib/web/backend
./run.py --debug
```

## develop frontend
```
cd web
yarn add
BACKEND=http://127.0.0.1:6080 npm run dev
```
