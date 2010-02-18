TEMPLATE = lib

CONFIG += staticlib create_prl

include (../../options.pri)

INCLUDEPATH += $$QMAKE_INCDIR_QT/../src/3rdparty/zlib

SOURCES += matfile.cc \
           matfiledata.cc \
           matlabarray.cc \
           matlabfile.cc \
