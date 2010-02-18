#ifndef __GRAPHICSIO_H__
#define __GRAPHICSIO_H__

#if DYNAMIC

#ifdef _WIN32
  #ifdef BUILD_graphicsio
    #define GRAPHICSIOSHARE __declspec(dllexport)
  #else
    #define GRAPHICSIOSHARE __declspec(dllimport)
  #endif
#else
  #define GRAPHICSIOSHARE
#endif

#else
#define GRAPHICSIOSHARE
#endif

#include "gi_graphicsio.h"
#include "gi_elements.h"
#include "gi_nodes.h"
#include "gi_timeseries.h"
#include "gi_surfaces.h"
#include "gi_utilities.h"
#include "gi_leadfiducials.h"
#include "gi_rewrites.h"

#endif


