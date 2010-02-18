TEMPLATE = lib

CONFIG += staticlib

include (../../options.pri)

INCLUDEPATH += ../graphicsio
INCLUDEPATH += ../fi
INCLUDEPATH += ../cutil

SOURCES += dfilefids.c \
           fidfilesubs.c \
           fidsubs.c
           