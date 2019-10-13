rem VS Command Prompt needs to be opened
rem This assumes that QTDIR is set 
rem see README for info

set PATH=%PATH%;%QTDIR%\bin;C:\Program Files (x86)\NSIS

qmake -r
nmake
del client\*.exe
makensis client\installer.nsi
copy client\*.exe .

