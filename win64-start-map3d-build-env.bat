
rem 2013 qt 5.4 opengl 64 bit
rem After QT5.4, they stopped (by default) supporting OpenGL that we use, that is a little obsolete
rem The last binaries that produced this version of Qt were compiled by VS 2013

set QTDIR=C:\Qt\Qt5.4.0\5.4\msvc2013_64_opengl\

call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64

call %QTDIR%\bin\qtenv2.bat

C:\Windows\System32\cmd.exe
