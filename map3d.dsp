# Microsoft Developer Studio Project File - Name="map3d" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=map3d - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "map3d.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "map3d.mak" CFG="map3d - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "map3d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "map3d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "map3d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "map3d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "map3d___Win32_Debug"
# PROP BASE Intermediate_Dir "map3d___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "."
# PROP Intermediate_Dir "."
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MT /W4 /Zi /Od /I ".\winfix" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WIN32"  /D BYTE_ORDER=1 /D LITTLE_ENDIAN=1 /YX /FD -I../cutil -I../fgile -I../gfile/lib -I../fids -I../graphicsio -I../tedutils/fi -Zm200 /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libcmt.lib glut32.lib Glu32.lib OpenGL32.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"map3d.exe" -nodefaultlib:libc,libcmt -LIBPATH:../cutil -LIBPATH:../gfile/lib -LIBPATH:../fids -LIBPATH:../graphicsio -LIBPATH:../tedutils/fi
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "map3d - Win32 Release"
# Name "map3d - Win32 Debug"
# Begin Source File

SOURCE=.\Arcball.h
# End Source File
# Begin Source File

SOURCE=.\Ball.cpp
# End Source File
# Begin Source File

SOURCE=.\Ball.h
# End Source File
# Begin Source File

SOURCE=.\BallAux.cpp
# End Source File
# Begin Source File

SOURCE=.\BallAux.h
# End Source File
# Begin Source File

SOURCE=.\BallMath.cpp
# End Source File
# Begin Source File

SOURCE=.\BallMath.h
# End Source File
# Begin Source File

SOURCE=.\colormaps.cpp
# End Source File
# Begin Source File

SOURCE=.\colormaps.h
# End Source File
# Begin Source File

SOURCE=.\ColorPicker.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorPicker.h
# End Source File
# Begin Source File

SOURCE=.\contsubs.cpp
# End Source File
# Begin Source File

SOURCE=.\contsubs.h
# End Source File
# Begin Source File

SOURCE=.\datasubs.cpp
# End Source File
# Begin Source File

SOURCE=.\drawlandmarks.cpp
# End Source File
# Begin Source File

SOURCE=.\drawlandmarks.h
# End Source File
# Begin Source File

SOURCE=.\eventdata.h
# End Source File
# Begin Source File

SOURCE=.\genbandshades.cpp
# End Source File
# Begin Source File

SOURCE=.\gencontband.cpp
# End Source File
# Begin Source File

SOURCE=.\gencontband.h
# End Source File
# Begin Source File

SOURCE=.\GenericWindow.h
# End Source File
# Begin Source File

SOURCE=.\geomsubs.cpp
# End Source File
# Begin Source File

SOURCE=.\geomsubs.h
# End Source File
# Begin Source File

SOURCE=.\GeomWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\GeomWindow.h
# End Source File
# Begin Source File

SOURCE=.\GeomWindowHandle.cpp
# End Source File
# Begin Source File

SOURCE=.\GeomWindowMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\GeomWindowMenu.h
# End Source File
# Begin Source File

SOURCE=.\GeomWindowRepaint.cpp
# End Source File
# Begin Source File

SOURCE=.\getcontvals.cpp
# End Source File
# Begin Source File

SOURCE=.\getcontvals.h
# End Source File
# Begin Source File

SOURCE=.\glprintf.cpp
# End Source File
# Begin Source File

SOURCE=.\glprintf.h
# End Source File
# Begin Source File

SOURCE=.\LegendWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\LegendWindow.h
# End Source File
# Begin Source File

SOURCE=.\LinearMapping.cpp
# End Source File
# Begin Source File

SOURCE=.\LinearMapping.h
# End Source File
# Begin Source File

SOURCE=.\LinkArray.h
# End Source File
# Begin Source File

SOURCE=.\linksubs.cpp
# End Source File
# Begin Source File

SOURCE=.\linksubs.h
# End Source File
# Begin Source File

SOURCE=.\lock.cpp
# End Source File
# Begin Source File

SOURCE=.\lock.h
# End Source File
# Begin Source File

SOURCE=.\MainWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\MainWindow.h
# End Source File
# Begin Source File

SOURCE=".\map3d-struct.cpp"
# End Source File
# Begin Source File

SOURCE=".\map3d-struct.h"
# End Source File
# Begin Source File

SOURCE=.\map3d.cpp
# End Source File
# Begin Source File

SOURCE=.\map3d.h
# End Source File
# Begin Source File

SOURCE=.\map3dmath.cpp
# End Source File
# Begin Source File

SOURCE=.\map3dmath.h
# End Source File
# Begin Source File

SOURCE=.\map3dsubs.cpp
# End Source File
# Begin Source File

SOURCE=.\map3dsubs.h
# End Source File
# Begin Source File

SOURCE=.\matrixstuff.cpp
# End Source File
# Begin Source File

SOURCE=.\matrixstuff.h
# End Source File
# Begin Source File

SOURCE=.\MeshList.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshList.h
# End Source File
# Begin Source File

SOURCE=.\minmax.cpp
# End Source File
# Begin Source File

SOURCE=.\minmax.h
# End Source File
# Begin Source File

SOURCE=.\ParseCommandLineOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ParseCommandLineOptions.h
# End Source File
# Begin Source File

SOURCE=.\pickinfo.h
# End Source File
# Begin Source File

SOURCE=.\PickWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\PickWindow.h
# End Source File
# Begin Source File

SOURCE=.\ProcessCommandLineOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcessCommandLineOptions.h
# End Source File
# Begin Source File

SOURCE=.\readdatafile.cpp
# End Source File
# Begin Source File

SOURCE=.\readdatafile.h
# End Source File
# Begin Source File

SOURCE=.\readgeom.cpp
# End Source File
# Begin Source File

SOURCE=.\readgeom.h
# End Source File
# Begin Source File

SOURCE=.\readgradfiles.cpp
# End Source File
# Begin Source File

SOURCE=.\readmap3dfidfile.cpp
# End Source File
# Begin Source File

SOURCE=.\readmap3dgeomfile.cpp
# End Source File
# Begin Source File

SOURCE=.\readmap3dgeomfile.h
# End Source File
# Begin Source File

SOURCE=.\readpotfiles.cpp
# End Source File
# Begin Source File

SOURCE=.\readpotfiles.h
# End Source File
# Begin Source File

SOURCE=.\reportstate.cpp
# End Source File
# Begin Source File

SOURCE=.\reportstate.h
# End Source File
# Begin Source File

SOURCE=.\scalarsubs.cpp
# End Source File
# Begin Source File

SOURCE=.\scalesubs.cpp
# End Source File
# Begin Source File

SOURCE=.\scalesubs.h
# End Source File
# Begin Source File

SOURCE=.\SizePicker.cpp
# End Source File
# Begin Source File

SOURCE=.\SizePicker.h
# End Source File
# Begin Source File

SOURCE=.\Transforms.cpp
# End Source File
# Begin Source File

SOURCE=.\Transforms.h
# End Source File
# Begin Source File

SOURCE=.\usage.cpp
# End Source File
# Begin Source File

SOURCE=.\usage.h
# End Source File
# Begin Source File

SOURCE=.\WindowManager.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowManager.h
# End Source File
# Begin Source File

SOURCE=..\cutil\libcutil.lib
# End Source File
# Begin Source File

SOURCE=..\fids\libfids.lib
# End Source File
# Begin Source File

SOURCE=..\gfile\lib\libgfile.lib
# End Source File
# Begin Source File

SOURCE=..\graphicsio\libgraphicsio.lib
# End Source File
# Begin Source File

SOURCE=..\tedutils\fi\libfi.lib
# End Source File
# End Target
# End Project
