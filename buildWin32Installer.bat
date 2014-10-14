rem This assumes that Qt is in C:\Qt\4.8.6

set QTDIR=C:\Qt\4.8.6
set PATH=%QTDIR%\path;C:\Program Files (x86)\NSIS;%PATH%;

qmake -r
nmake
del client\*.exe
makensis client\installer.nsi
copy client\*.exe .

