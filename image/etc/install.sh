#!/bin/bash
echo 'Building Node.js project...'
cd /src/web \
    && yarn \
    && npm run build

cp -R /src/web/dist/. /usr/local/lib/web/frontend/
echo 'Build finished.'