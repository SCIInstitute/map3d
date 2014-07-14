#!/bin/bash

if ! test -d thirdparty; then
  echo "map3d thirdparty not checked out.  Run ./updateThirdparty.sh"
  exit 2
fi

FILES=`ls`
mkdir map3d-src
cp -r $FILES map3d-src
cd map3d-src

find . -name "*.svn" | xargs rm -rf
rm -rf data geom
cd ..
tar cfz map3d-src.tar.gz map3d-src
#rm -rf map3d-src
