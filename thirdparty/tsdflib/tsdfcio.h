#ifndef __TSDFCIO_H__
#define __TSDFCIO_H__

#if DYNAMIC

#ifdef _WIN32
  #ifdef BUILD_tsdfcio
    #define TSDFCIOSHARE __declspec(dllexport)
  #else
    #define TSDFCIOSHARE __declspec(dllimport)
  #endif
#else
  #define TSDFCIOSHARE
#endif

#else
#define TSDFCIOSHARE
#endif

#include "container.h"
#include "pslist.h"
#include "fslist.h"
#include "fidset.h"
#include "giofidset.h"
#include "gdbmkeys.h"

#endif
