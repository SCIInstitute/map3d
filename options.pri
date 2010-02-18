win32 {
  !no_cpp {
    # instructs the compiler to use C++ instead of C, which we want in most cases
    QMAKE_CXXFLAGS += /TP
    QMAKE_CFLAGS += /TP
  }
  # MT = don't use CRT DLL
  QMAKE_CXXFLAGS += /D_CRT_SECURE_NO_WARNINGS
  QMAKE_CFLAGS += /D_CRT_SECURE_NO_WARNINGS
  #QMAKE_CXXFLAGS_DEBUG -= -MDd
  #QMAKE_CFLAGS_DEBUG -= -MDd
  #QMAKE_CXXFLAGS_RELEASE -= -MD
  #QMAKE_CFLAGS_RELEASE -= -MD
}

CONFIG -= release
CONFIG += debug
