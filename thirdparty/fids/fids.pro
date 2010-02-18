TEMPLATE = lib

CONFIG += staticlib create_prl

include (../../options.pri)

INCLUDEPATH += ../graphicsio
INCLUDEPATH += ../fi
INCLUDEPATH += ../cutil

SOURCES += dfilefids.c \
           fidfilesubs.c \
           fidsubs.c
