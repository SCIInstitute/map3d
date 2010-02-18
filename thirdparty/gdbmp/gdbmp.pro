TEMPLATE = lib

CONFIG += staticlib
CONFIG += no_cpp
CONFIG += create_prl
CONFIG -= qt

include (../../options.pri)

SOURCES += bucket.c \
           close.c \
           dbmclose.c \
           dbmdelete.c \
           dbmdirfno.c \
           dbmfetch.c \
           dbminit.c \
           dbmopen.c \
           dbmpagfno.c \
           dbmrdonly.c \
           dbmseq.c \
           dbmstore.c \
           delete.c \
           falloc.c \
           fetch.c \
           findkey.c \
           gdbmclose.c \
           gdbmdelete.c \
           gdbmerrno.c \
           gdbmexists.c \
           gdbmfdesc.c \
           gdbmfetch.c \
           gdbmopen.c \
           gdbmpatch.c \
           gdbmreorg.c \
           gdbmseq.c \
           gdbmsetopt.c \
           gdbmstore.c \
           gdbmsync.c \
           global.c \
           hash.c \
           seq.c \
           store.c \
           update.c \
           version.c
