#!/bin/sh

appname=`basename $0 | sed s,\.sh$,,`

dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi

core=$dirname/lib/core
coredep=$dirname/lib/coredep
export QTTOOLDIR=/opt/Qt5.12.4/5.12.4/gcc_64/bin
export QTLIBDIR=/opt/Qt5.12.4/5.12.4/gcc_64/lib
LD_LIBRARY_PATH=$dirname:$core:$coredep:$QTLIBDIR:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
#export QT_DEBUG_PLUGINS=1
#echo "$0 = "$0
#echo "appname = "$appname
#echo "dirname = "$dirname
#echo "tmp = "$tmp
echo "LD_LIBRARY_PATH = "$LD_LIBRARY_PATH

$dirname/$appname "$@"
