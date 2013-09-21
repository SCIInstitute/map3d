#!/bin/bash

# run from map3d root, after map3d build completes
rm -rf client/map3d.app/Contents/
rm -f map3d.dmg
qmake -spec macx-g++ -r
make clean
make -j5
macdeployqt client/map3d.app
hdiutil create map3d.dmg -srcfolder client/map3d.app

