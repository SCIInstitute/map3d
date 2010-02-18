/* texture.h */

#ifndef TEXTURE_H
#define TEXTURE_H

class Texture
{
public:
  int min_filter;
  int mag_filter;
  int tex_dim;
  int drawtype;
  int id;
  unsigned char *pixels;
  bool copied;
};

#define DOT_TEXTURE 2
#define LOCK_TEXTURE 3
#define BGIMAGE_TEXTURE 12


Texture *CreateTexture(int id, int tex_dim, int cpp, int width, int height, int pixelFormat, 
                       int min_filter, int mag_filter, int drawtype, unsigned char *data, bool copy = false);
void UseTexture(Texture * tex);
void DestroyTexture(Texture* tex);

#endif
