#!/bin/bash

# run from map3d root directory
rm -rf client/map3d.app/Contents/
rm -f map3d.dmg
# commented out for now, as we brought this back into the standard build
# ./updateThirdparty.sh
qmake -spec macx-g++ -r
make clean
make -j5
macdeployqt client/map3d.app
hdiutil create map3d.dmg -srcfolder client/map3d.app

