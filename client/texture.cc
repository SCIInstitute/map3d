/* texture.cxx */

#include "texture.h"
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <OpenGl/gl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <stdio.h>

void UseTexture(Texture * t)
{
  glBindTexture(t->tex_dim, t->id);
  glTexParameteri(t->tex_dim, GL_TEXTURE_MAG_FILTER, t->mag_filter);
  glTexParameteri(t->tex_dim, GL_TEXTURE_MIN_FILTER, t->min_filter);
  glTexParameteri(t->tex_dim, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(t->tex_dim, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, t->drawtype);
  //if (gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA, t->p2width, t->p2height, GL_ALPHA, GL_UNSIGNED_BYTE, t->pixels))
    //printf("texture OpenGL Error: gluBuild2DMipmaps() failed\n");
}

Texture *CreateTexture(int id, int tex_dim, int cpp, int width, int height, int pixelFormat, 
                       int min_filter, int mag_filter, int drawtype, unsigned char *data, bool copy/*=false*/)
{
  Texture *t = new Texture;

  t->id = id;
  t->mag_filter = mag_filter;
  t->min_filter = min_filter;
  t->drawtype = drawtype;
  t->tex_dim = tex_dim;
  t->copied = copy;

  if (copy) {
    t->pixels = new unsigned char[width * height * cpp];

    for (int loop1 = 0; loop1 < width; loop1++) {
      for (int loop2 = 0; loop2 < height; loop2++) {
        for (int loop3 = 0; loop3 < cpp; loop3++) {
          t->pixels[(loop2 * width + loop1)*cpp + loop3] = data[(loop2 * width + loop1)*cpp + loop3];
        }
      }
    }
  }
  else
    t->pixels = data;
  glBindTexture(tex_dim, t->id);
  glTexParameteri(tex_dim, GL_TEXTURE_MAG_FILTER, mag_filter);
  glTexParameteri(tex_dim, GL_TEXTURE_MIN_FILTER, min_filter);
  glTexParameteri(tex_dim, GL_TEXTURE_WRAP_S, GL_CLAMP);
  if (tex_dim == GL_TEXTURE_2D) {
    glTexParameteri(tex_dim, GL_TEXTURE_WRAP_T, GL_CLAMP);
    if (gluBuild2DMipmaps(tex_dim, pixelFormat, width, height, pixelFormat, GL_UNSIGNED_BYTE, t->pixels))
      printf("texture OpenGL Error: gluBuild2DMipmaps() failed\n");
  }
  else { //if (tex_dim == GL_TEXTURE_1D) {
    glTexImage1D(tex_dim, 0, pixelFormat, width, 0, pixelFormat, GL_UNSIGNED_BYTE, t->pixels);
  }
  //else
    //printf("Texture type not supported\n");
  return t;
}

void DestroyTexture(Texture* tex)
{
  if (tex->copied)
    delete tex->pixels;
  delete tex;
}
