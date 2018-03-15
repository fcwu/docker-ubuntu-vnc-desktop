# Run in local
```
make build
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
