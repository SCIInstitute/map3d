/* colormaps.h */

#ifndef COLORMAPS_H
#define COLORMAPS_H

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

class ColorMap
{
public:
  ColorMap(int type);
  unsigned char *map;
  int max;
  int type;
  static bool initialized;
};

/* colormap type - store this way to generate textures with*/
#define SOLID_CMAP 4
#define GREEN2RED_CMAP 5
#define RAINBOW_CMAP 6
#define WHITE2BLACK_CMAP 7
// Matlab Jet colormap
#define JET_CMAP 8 



EXTERN ColorMap Solid;
EXTERN ColorMap Rainbow;
EXTERN ColorMap Grayscale;
EXTERN ColorMap Green2Red;
EXTERN ColorMap Jet;

void init_colormaps();

#endif
