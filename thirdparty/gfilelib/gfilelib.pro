TEMPLATE = lib

CONFIG += staticlib create_prl

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
           opennewoutfile.c \
           readclinks.c \
           readfacfile.c \
           readgeomfile.c \
           readglinks.c \
           readlinksfile.c \
           readlmarkfile.c \
           readptsfile.c \
           writechannelsfile.c \
           writeelementsfile.c \
           writegeomfile.c \
           writelandmarks.c \
           writeptsfile.c
