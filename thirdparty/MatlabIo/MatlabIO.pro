TEMPLATE = lib

CONFIG += staticlib

include (../../options.pri)

INCLUDEPATH += $$QMAKE_INCDIR_QT/../src/3rdparty/zlib

SOURCES += matfile.cc \
           matfiledata.cc \
           matlabarray.cc \
           matlabfile.cc \
