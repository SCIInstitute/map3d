TEMPLATE = lib

CONFIG += staticlib create_prl
CONFIG -= qt

include (../../options.pri)

INCLUDEPATH += ../graphicsio

SOURCES += casesubs.c \
           checkexist.c \
           checkextension.c \
           checkwrite.c \
           clobberfile.c \
           confirmfile.c \
           distance.c \
           getfilelength.c \
           getfilename.c \
           killfile.c \
           mallocsubs.c \
           nint.c \
           nodedistance.c \
           parsefilename.c \
           pointdistance.c \
           readdouble.c \
           readfilename.c \
           readfloat.c \
           readint.c \
           readline.c \
           readlong.c \
           readnextline.c \
           readstring.c \
           reporterror.c \
           stripextension.c \
           textfilesubs.c \
           trulen.c

!unix {
    SOURCES += getline.c
} 
