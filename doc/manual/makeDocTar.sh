#!/bin/sh

# pass in the version number as the only argument
if test "$#" != 1; then
  echo "Please pass the version number of map3d as an argument"
  echo "  i.e., makeDocTar.sh 6.2"
  exit 1
fi

VERSION=$1
TARFILE="map3d-"$VERSION"_docs.tar"

echo "Creating file $TARFILE.gz"
tar cf $TARFILE manual/*.{html,css,gif,png} figures/*.gif manual.pdf
gzip $TARFILE
rm -f $TARFILE