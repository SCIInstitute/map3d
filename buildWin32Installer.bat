rem VS 2015 Command Prompt needs to be opened
rem This assumes that Qt is in C:\Qt\Qt5.4.0\5.4\msvc2012_opengl

rem set QTDIR=C:\Qt\5.7\msvc2015_64
set QTDIR=C:\Qt2\5.7\msvc2015_64
set PATH=%PATH%;%QTDIR%\bin;C:\Program Files (x86)\NSIS

qmake -r
nmake
del client\*.exe
makensis client\installer.nsi
copy client\*.exe .

