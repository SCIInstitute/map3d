TEMPLATE = app

TARGET = map3d

QT += opengl

INCLUDEPATH += ../thirdparty/cutil
INCLUDEPATH += ../thirdparty/fi
INCLUDEPATH += ../thirdparty/fids
INCLUDEPATH += ../thirdparty/gdbmp
INCLUDEPATH += ../thirdparty/gfilelib
INCLUDEPATH += ../thirdparty/graphicsio
INCLUDEPATH += ../thirdparty/MatlabIO
INCLUDEPATH += ../thirdparty/tsdflib
INCLUDEPATH += $$QMAKE_INCDIR_QT/../src/3rdparty/zlib

CONFIG += console link_prl

include (../options.pri)

LIBS += -lMatlabIO
LIBS += -lfids -lfi -ltsdflib -lgfilelib -lgraphicsio -lgdbmp -lcutil
win32 {
    LIB_DIR=/release
    debug {
        LIB_DIR=/debug
    }
    PRE_TARGETDEPS *= ../thirdparty/fids$$LIB_DIR/fids.lib
    PRE_TARGETDEPS *= ../thirdparty/fi$$LIB_DIR/fi.lib
    PRE_TARGETDEPS *= ../thirdparty/tsdflib$$LIB_DIR/tsdflib.lib
    PRE_TARGETDEPS *= ../thirdparty/gfilelib$$LIB_DIR/gfilelib.lib
    PRE_TARGETDEPS *= ../thirdparty/graphicsio$$LIB_DIR/graphicsio.lib
    PRE_TARGETDEPS *= ../thirdparty/gdbmp$$LIB_DIR/gdbmp.lib
    PRE_TARGETDEPS *= ../thirdparty/cutil$$LIB_DIR/cutil.lib
} else {
    PRE_TARGETDEPS *= ../thirdparty/fids/libfids.a
    PRE_TARGETDEPS *= ../thirdparty/fi/libfi.a
    PRE_TARGETDEPS *= ../thirdparty/tsdflib/libtsdflib.a
    PRE_TARGETDEPS *= ../thirdparty/gfilelib/libgfilelib.a
    PRE_TARGETDEPS *= ../thirdparty/graphicsio/libgraphicsio.a
    PRE_TARGETDEPS *= ../thirdparty/gdbmp/libgdbmp.a
    PRE_TARGETDEPS *= ../thirdparty/cutil/libcutil.a
}


LIBPATH += ../thirdparty/cutil$$LIB_DIR
LIBPATH += ../thirdparty/fi$$LIB_DIR
LIBPATH += ../thirdparty/fids$$LIB_DIR
LIBPATH += ../thirdparty/gdbmp$$LIB_DIR
LIBPATH += ../thirdparty/gfilelib$$LIB_DIR
LIBPATH += ../thirdparty/graphicsio$$LIB_DIR
LIBPATH += ../thirdparty/MatlabIO$$LIB_DIR
LIBPATH += ../thirdparty/tsdflib$$LIB_DIR
LIBPATH += $$QMAKE_INCDIR_QT/../src/3rdparty/zlib

include (../options.pri)

FORMS += forms/FileDialog.ui \
         forms/FileDialogWidget.ui \
         forms/FidDialog.ui \
         forms/ContourDialog.ui \
         forms/ImageControlDialog.ui \
         forms/ScaleDialog.ui \


macx {
  ICON = ../map3dapp/map3d-icon.icns
}

# files that need to be moc'ed
HEADERS += GeomWindow.h \
           ContourDialog.h \
           FileDialog.h \
           FidDialog.h \
           ImageControlDialog.h \
           ScaleDialog.h \
           regressiontest.h \
           dialogs.h \
           

SOURCES += Ball.cc \
           BallAux.cc \
           BallMath.cc \
           colormaps.cc \
           Contour_Info.cc \
           ContourDialog.cc \
           dialogs.cc \
           dot.cc \
           drawlandmarks.cc \
           FidDialog.cc \
           FileDialog.cc \
           GenericWindow.cc \
           GeomWindow.cc \
           GeomWindowHandle.cc \
           GeomWindowMenu.cc \
           GeomWindowRepaint.cc \
           LegendWindow.cc \
           lock.cc \
           MainWindow.cc \
           map3d-struct.cc \
           map3d.cc \
           Map3d_Geom.cc \
           map3dmath.cc \
           MeshList.cc \
           ParseCommandLineOptions.cc \
           ProcessCommandLineOptions.cc \
           PickWindow.cc \
           readfiles.cc \
           regressiontest.cc \
           reportstate.cc \
           ImageControlDialog.cc \
           savescreen.cc \
           savestate.cc \
           ScaleDialog.cc \
           scalesubs.cc \
           Surf_Data.cc \
           texture.cc \
           Transforms.cc \
           usage.cc \
           WindowManager.cc
           