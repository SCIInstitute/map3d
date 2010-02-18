TEMPLATE = lib

CONFIG += staticlib create_prl
CONFIG -= qt

include (../../options.pri)

DEFINES += BYTE_ORDER=1
DEFINES += LITTLE_ENDIAN=1

SOURCES += gi_elements.c \
           gi_graphicsio.c \
           gi_leadfiducials.c \
           gi_nodes.c \
           gi_rewrites.c \
           gi_surfaces.c \
           gi_timeseries.c \
           gi_utilities.c \
