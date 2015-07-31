TEMPLATE = lib

CONFIG += staticlib create_prl
CONFIG -= qt

include (../../options.pri)

INCLUDEPATH += ../graphicsio
SOURCES += fi.c

