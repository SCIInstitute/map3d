rem - 2012 qt 5.4 opengl 32 bit
rem set QTDIR=C:\Qt\Qt5.4.0\5.4\msvc2012_opengl\
rem call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"

rem works 2013 qt 5.4 opengl 32 bit
rem set QTDIR=C:\Qt\Qt5.4.0\5.4\msvc2013_opengl\
rem call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"

rem 2013 qt 5.4 opengl 64 bit
set QTDIR=C:\Qt\Qt5.4.0\5.4\msvc2013_64_opengl\
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64

call %QTDIR%\bin\qtenv2.bat

C:\Windows\System32\cmd.exe