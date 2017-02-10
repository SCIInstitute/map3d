#!/bin/bash

# run from map3d root directory
rm -rf client/map3d.app/Contents/
rm -f map3d.dmg
# commented out for now, as we brought this back into the standard build
# ./updateThirdparty.sh

## This build uses Qt 5.7.
## Either set PATH to the Qt 5.7 installation bin directory in your terminal,
## or bash configuration, or uncomment and modify the following lines
## to match your installation:
# export QTDIR=/Users/ayla/Qt5.7.1/5.7/clang_64
# export PATH=$QTDIR/bin:$PATH

qmake -spec macx-clang -r
make clean
make -j5
macdeployqt client/map3d.app
hdiutil create map3d.dmg -srcfolder client/map3d.app

