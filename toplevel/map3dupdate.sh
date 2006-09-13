#!/bin/sh

if [ $# -ne 1 ]
then
    NAME=$USER
else
    NAME=$1
fi
    
svn co svn+ssh://$NAME@svn.cvrti.utah.edu/Home/svn/glibs/cutil/trunk cutil
svn co svn+ssh://$NAME@svn.cvrti.utah.edu/Home/svn/glibs/gfilelib/trunk gfilelib
svn co svn+ssh://$NAME@svn.cvrti.utah.edu/Home/svn/glibs/fids/trunk fids
svn co svn+ssh://$NAME@svn.cvrti.utah.edu/Home/svn/glibs/fi/trunk fi
svn co svn+ssh://$NAME@svn.cvrti.utah.edu/Home/svn/glibs/gdbmp/trunk gdbmp
svn co svn+ssh://$NAME@svn.cvrti.utah.edu/Home/svn/glibs/numseq/trunk numseq
svn co svn+ssh://$NAME@svn.cvrti.utah.edu/Home/svn/glibs/tsdflib/trunk tsdflib
svn co svn+ssh://$NAME@svn.cvrti.utah.edu/Home/svn/dataio/graphicsio/trunk graphicsio
svn co svn+ssh://$NAME@svn.cvrti.utah.edu/Home/svn/dataio/MatlabIO/trunk MatlabIO
svn co https://code.sci.utah.edu/svn/map3d/trunk map3d
