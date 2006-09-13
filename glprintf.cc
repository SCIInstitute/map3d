/* glprintf.cxx */

#include <stdio.h>
#include <stdarg.h>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4505)
#define vsnprintf _vsnprintf
#endif
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#ifdef OSX
#include <OpenGL/glu.h>
#include <OpenGl/gl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <stdlib.h>
#include "GenericWindow.h"
#include "map3d-struct.h"
#include "glprintf.h"
#include "colormaps.h"
#include "texture.h"
#include "lock.h"
#include "dot.h"
#include <string.h>             /* for strlen() */

#ifndef _WIN32
static char font_name[][36] = {"helvetica 6",
  "helvetica 8",
  "helvetica 10",
  "helvetica 12",
  "helvetica 14",
  "helvetica 18",
  "helvetica 24",
  "helvetica 36",
  "helvetica 48",
  "helvetica 72"};
#else
static char font_name[][36] = {"Sans 6",
  "Sans 8",
  "Sans 10",
  "Sans 12",
  "Sans 14",
  "Sans 18",
  "Sans 24",
  "Sans 36",
  "Sans 48",
  "Sans 72"};
#endif
// pixels per character
int font_width[] = { 4,5,7,8,10,12,15,24,32,46};
static int font_height[10];
static int font_width_new[10];

float new_font_width[] ={3.63f,4.45f,6.27f,7.27f,8.36f,10.63f,14.36f,22.27f,29.63f,45.0f};
float new_new_font_width[] ={4.025f,4.935f,6.7073f,7.6839f,8.9198f,11.34165f,15.2532f,
  23.70775f,31.5086f,48.0238f};
static int font_head[10];

extern Map3d_Info map3d_info;

void initFontsAndTextures() {
  PangoFontDescription *font_description;
  PangoFont *font;
  PangoFontMetrics *font_metrics;
  
  /*
   * Generate font display lists.
   */
  
  int fontnum = 0;
  
  for (int i = 0; i < 10; i++) {
    
    font_head[fontnum] = glGenLists (128);
    
    font_description = pango_font_description_from_string (font_name[fontnum]);
    
    font = gdk_gl_font_use_pango_font (font_description, 0, 128, 
                                       font_head[fontnum]);
    if (!font) {
      printf("could not load font blah,blah,blah\n");
      continue;
    }
    
    font_metrics = pango_font_get_metrics (font, NULL);
    
    font_height[fontnum] = pango_font_metrics_get_ascent (font_metrics) + pango_font_metrics_get_descent (font_metrics);
    //printf("font_height[%d] = %d\n",fontnum,font_height[fontnum]);
    font_height[fontnum] = PANGO_PIXELS (font_height[fontnum]);
    font_width_new[fontnum] = pango_font_metrics_get_approximate_char_width(font_metrics);
    font_width_new[fontnum] = PANGO_PIXELS (font_width_new[fontnum]);
    //new_font_width[fontnum]= (pango_font_metrics_get_approximate_char_width(font_metrics) + pango_font_metrics_get_approximate_char_width(font_metrics))/2.0;
    //printf("new_font_width[%d] = %d\n",fontnum,new_font_width[fontnum]);
    //new_font_width[fontnum]= (PANGO_PIXELS (new_font_width[fontnum]))*0.55;
    //printf("new_font_width[%d] = %d\n",fontnum,new_font_width[fontnum]);
    
    
    pango_font_description_free (font_description);
    pango_font_metrics_unref (font_metrics);
    fontnum++;
  }
  if (fontnum == 0) {
    g_print ("*** Can't load font '%s'\n", font_name[0]);
    exit (1);
  }
  
  // Lock Texture
  map3d_info.lock_texture = CreateTexture(LOCK_TEXTURE, GL_TEXTURE_2D, 4, 64, 64, GL_RGBA, 
    GL_LINEAR, GL_LINEAR, GL_MODULATE, padlock);

  // Dot Texture   
  map3d_info.dot_texture = CreateTexture(DOT_TEXTURE, GL_TEXTURE_2D, 1, 128, 128, GL_ALPHA,
    GL_LINEAR, GL_LINEAR, GL_REPLACE, dot);

  // Rainbow colormap Texture
  map3d_info.rainbow_texture = CreateTexture(RAINBOW_CMAP, GL_TEXTURE_1D, 3, Rainbow.max+1, 1, GL_RGB,
    GL_NEAREST, GL_NEAREST, GL_MODULATE, Rainbow.map);
  
  // Jet colormap Texture
  map3d_info.jet_texture = CreateTexture(JET_CMAP, GL_TEXTURE_1D, 3, Jet.max+1, 1, GL_RGB,
    GL_NEAREST, GL_NEAREST, GL_MODULATE, Jet.map);
#if 0
  glBindTexture(GL_TEXTURE_1D, JET_CMAP);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, Jet.max+1, 0, GL_RGB, GL_UNSIGNED_BYTE, Jet.map);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
}

// size should be a float/int from 1-10. 
void renderString3f(float x, float y, float z, float size, char *format, ...)
{
  int sizeindex = (int) size;
  // if we have a string longer than this, then we have a bigger problem
  char string1[256]; 
  // char* string1 = new char[length];
  int length1 = 256;
  int length2 = length1;
  
  va_list args;
  va_start(args, format);
  
  memset(string1, 0, length1);
  
  while (vsnprintf(string1, length2, format, args) == -1) {
    memset(string1, 0, length2);
  }

  // just to make sure
  length2 = strlen(string1);

  glRasterPos3f(x, y, z);
  glListBase (font_head[sizeindex-1]);
  glCallLists (strlen (string1), GL_UNSIGNED_BYTE, string1);
}


float getFontWidth(float size) 
{
  // ratio of height to width based on experimentation
  // printf("font_height[%d] = %d\n",size-1,font_height[size-1]);
  // return (int)(font_height[size-1]/2.5);
  // return (int)(font_width[size -1]);
  //printf("size = %d\n", size);
  //  int sizeindex = (int)size;
  //  return (float)(new_new_font_width[sizeindex -1]);
	int sizeindex = (int)size;
	return (float)(font_width_new[sizeindex -1]);
  
}

int getFontHeight(float size) 
{
  int sizeindex = (int)size;
  return font_height[sizeindex-1];
}
