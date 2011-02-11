TEMPLATE = lib

CONFIG += staticlib create_prl
CONFIG -= qt

include (../../options.pri)

INCLUDEPATH += ../graphicsio
INCLUDEPATH += ../cutil
INCLUDEPATH += ../numseq

SOURCES += checkgeom.c \
           findnoderange.c \
           geomlib.c \
           geomutilsubs.c \
           getsurflist.c \
           landmarks.c \
           landmarksubs.c \
           readclinks.c \
           readfacfile.c \
           readglinks.c \
           readlinksfile.c \
           readlmarkfile.c \
           readptsfile.c \
           writechannelsfile.c \
           writeelementsfile.c \
           writelandmarks.c \
           writeptsfile.c
