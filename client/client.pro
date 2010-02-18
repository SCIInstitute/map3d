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

CONFIG += console

include (../options.pri)

win32 {
    LIB_DIR=/release
    debug {
        LIB_DIR=/debug
        }
    LIBS += -lfids -lfi -ltsdflib -lgraphicsio -lgfilelib -lcutil -lgdbmp -lMatlabIO
} else { 	 
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
         forms/ImageControlDialog.ui \

# fies that need to be moc'ed
HEADERS += GeomWindow.h \
           FileDialog.h \
           ImageControlDialog.h \
           regressiontest.h \
           dialogs.h \
           

SOURCES += Ball.cc \
           BallAux.cc \
           BallMath.cc \
           colormaps.cc \
           Contour_Info.cc \
#           ContourDialog.cc \
           dialogs.cc \
           dot.cc \
           drawlandmarks.cc \
#           FidDialog.cc \
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
#           ScaleDialog.cc \
           scalesubs.cc \
           Surf_Data.cc \
           texture.cc \
           Transforms.cc \
           usage.cc \
           WindowManager.cc
           
           
           
