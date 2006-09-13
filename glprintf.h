/* glprintf.h */

#ifndef GLPRINTF_H
#define GLPRINTF_H

#ifdef __cplusplus
extern "C"
{
#endif

  //int glprintf(float* pos, float* norm, float* up, float width, float height, const char* format, ...);
  //void init_glprintf();

  /*  use GTK fonts.  fonts to choose from are:
  */
  void initFontsAndTextures();

  //size takes an int from 1-10, or one of the defines below
  //it is a float here because most of the code references are floats - see incrSize in dialogs.cc
  void renderString3f(float x, float y, float z, float size, char *string, ...);
  float getFontWidth(float font);
  int getFontHeight(float font);

#define MAP3D_FONT_SIZE_6 1
#define MAP3D_FONT_SIZE_8 2
#define MAP3D_FONT_SIZE_10 3
#define MAP3D_FONT_SIZE_12 4
#define MAP3D_FONT_SIZE_14 5
#define MAP3D_FONT_SIZE_18 6
#define MAP3D_FONT_SIZE_24 7
#define MAP3D_FONT_SIZE_36 8
#define MAP3D_FONT_SIZE_48 9
#define MAP3D_FONT_SIZE_72 10

#define DOT_TEXTURE 2
#define LOCK_TEXTURE 3
#define BGIMAGE_TEXTURE 12

#ifdef __cplusplus
}
#endif

#endif
