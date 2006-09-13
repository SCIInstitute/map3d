/* colormaps.cxx */


#include "colormaps.h"
#include <stdio.h>

/* number of cells in the colormaps (length of the array) */
#define SOLIDLEN 1
#define GREEN2REDLEN 523
#define RAINBOWLEN 1024
#define WHITE2BLACKLEN 766
#define JETLEN 1024

bool ColorMap::initialized = false;

/* the actual colormaps (the array) */
unsigned char solid[SOLIDLEN * 3];
unsigned char rainbow[RAINBOWLEN * 3];
unsigned char white2black[WHITE2BLACKLEN * 3];
unsigned char green2red[GREEN2REDLEN * 3];
unsigned char jet[JETLEN * 3];

/* container classes for the colormaps, one for each map */
ColorMap Solid(SOLID_CMAP);
ColorMap Green2Red(GREEN2RED_CMAP);
ColorMap Rainbow(RAINBOW_CMAP);
ColorMap Grayscale(WHITE2BLACK_CMAP);
ColorMap Jet(JET_CMAP);

void compute_solid()
{
  solid[0] = 75;
  solid[1] = 145;
  solid[2] = 145;
}

void compute_rainbow()
{
  int loop;

  for (loop = 0; loop < 256; loop++) {
    rainbow[(loop + 0) * 3 + 0] = 0;
    rainbow[(loop + 0) * 3 + 1] = (unsigned char)loop;
    rainbow[(loop + 0) * 3 + 2] = 255;
    //printf("entry %d = %d %d %d\n",loop+765,
    //       rainbow[(loop+765)*3+0],rainbow[(loop+765)*3+1],rainbow[(loop+765)*3+2]);
  }

  for (loop = 0; loop < 256; loop++) {
    rainbow[(loop + 256) * 3 + 0] = 0;
    rainbow[(loop + 256) * 3 + 1] = 255;
    rainbow[(loop + 256) * 3 + 2] = (unsigned char)(255 - loop);
    //printf("entry %d = %d %d %d\n",loop+510,
    //       rainbow[(loop+510)*3+0],rainbow[(loop+510)*3+1],rainbow[(loop+510)*3+2]);
  }

  for (loop = 0; loop < 256; loop++) {
    rainbow[(loop + 512) * 3 + 0] = (unsigned char)loop;
    rainbow[(loop + 512) * 3 + 1] = 255;
    rainbow[(loop + 512) * 3 + 2] = 0;
    //printf("entry %d = %d %d %d\n",loop+255,
    //       rainbow[(loop+255)*3+0],rainbow[(loop+255)*3+1],rainbow[(loop+255)*3+2]);
  }

  for (loop = 0; loop < 256; loop++) {
    rainbow[(loop + 768) * 3 + 0] = 255;
    rainbow[(loop + 768) * 3 + 1] = (unsigned char)(255 - loop);
    rainbow[(loop + 768) * 3 + 2] = 0;
    //printf("entry %d = %d %d %d\n",loop,
    //       rainbow[(loop+0)*3+0],rainbow[(loop+0)*3+1],rainbow[(loop+0)*3+2]);
  }
}

void compute_white2black()
{
  int loop = 0;

  white2black[0] = 255;
  white2black[1] = 255;
  white2black[2] = 255;
  //printf("entry %d = %d %d %d\n",loop,
  //       white2black[(loop+0)*3+0],white2black[(loop+0)*3+1],white2black[(loop+0)*3+2]);

  for (loop = 1; loop < 766; loop += 3) {
    white2black[(loop + 0) * 3 + 0] = (unsigned char)(white2black[(loop - 1) * 3 + 0] - 1);
    white2black[(loop + 0) * 3 + 1] = white2black[(loop - 1) * 3 + 1];
    white2black[(loop + 0) * 3 + 2] = white2black[(loop - 1) * 3 + 2];
    //printf("entry %d = %d %d %d\n",loop,
    //       white2black[(loop+0)*3+0],white2black[(loop+0)*3+1],white2black[(loop+0)*3+2]);
    white2black[(loop + 1) * 3 + 0] = (unsigned char)(white2black[(loop - 1) * 3 + 0] - 1);
    white2black[(loop + 1) * 3 + 1] = (unsigned char)(white2black[(loop - 1) * 3 + 1] - 1);
    white2black[(loop + 1) * 3 + 2] = white2black[(loop - 1) * 3 + 2];
    //printf("entry %d = %d %d %d\n",loop+1,
    //       white2black[(loop+1)*3+0],white2black[(loop+1)*3+1],white2black[(loop+1)*3+2]);
    white2black[(loop + 2) * 3 + 0] = (unsigned char)(white2black[(loop - 1) * 3 + 0] - 1);
    white2black[(loop + 2) * 3 + 1] = (unsigned char)(white2black[(loop - 1) * 3 + 1] - 1);
    white2black[(loop + 2) * 3 + 2] = (unsigned char)(white2black[(loop - 1) * 3 + 2] - 1);
    //printf("entry %d = %d %d %d\n",loop+2,
    //       white2black[(loop+2)*3+0],white2black[(loop+2)*3+1],white2black[(loop+2)*3+2]);
  }
}

void compute_green2red()
{
  int i, blacklen, index;

  blacklen = GREEN2REDLEN - 2 * 256;
  index = 0;

  /* green intensity reduces to dark */
  for (i = 255; i >= 0; i--) {
    green2red[index * 3 + 0] = 0; /* red component */
    green2red[index * 3 + 1] = (unsigned char)i;  /* green component */
    green2red[index * 3 + 2] = 0; /* blue component */
    index++;                    /* move to the next item */
  }

  /* middle black zone */
  for (i = 0; i < blacklen; i++) {
    green2red[index * 3 + 0] = 0;
    green2red[index * 3 + 1] = 0;
    green2red[index * 3 + 2] = 0;
    index++;
  }

  for (i = 0; i <= 255; i++) {
    green2red[index * 3 + 0] = (unsigned char)i;  /*red */
    green2red[index * 3 + 1] = 0; /*green */
    green2red[index * 3 + 2] = 0; /*blue */
    index++;
  }
}

void compute_jet()
{
  // Matlab jet colormap -- size must be a power of 2 for texturing purposes
  int index;
  // go from (.5,1] in blue
  for (int i = 0; i < JETLEN/8; i++) {
    index = i;
    jet[index*3 + 0] = 0;
    jet[index*3 + 1] = 0;
    jet[index*3 + 2] = (unsigned char)(255*(0.5 + ((i+1)/(JETLEN/4.0))));
  }
  // blue is 1, green goes from (0, 1]
  for (int i = 0; i < JETLEN/4; i++) {
    index = JETLEN/8+i;
    jet[index*3 + 0] = 0;
    jet[index*3 + 1] = (unsigned char)(255*((i+1)/(JETLEN/4.0)));
    jet[index*3 + 2] = 255;
  }

  // blue goes from (1, 0], green is 1, red goes from (0, 1]
  for (int i = 0; i < JETLEN/4; i++) {
    index = JETLEN*3/8+i;
    jet[index*3 + 0] = (unsigned char)(255*((i+1)/(JETLEN/4.0)));
    jet[index*3 + 1] = 255;
    jet[index*3 + 2] = (unsigned char)(255*(1-((i+1)/(JETLEN/4.0))));
  }
  // green goes from (1,0], red is 1
  for (int i = 0; i < JETLEN/4; i++) {
    index = JETLEN*5/8+i;
    jet[index*3 + 0] = 255;
    jet[index*3 + 1] = (unsigned char)(255*(1-((i+1)/(JETLEN/4.0))));
    jet[index*3 + 2] = 0;
  }
  // red goes from (1,.5)
  for (int i = 0; i < JETLEN/8; i++) {
    index = JETLEN*7/8+i;
    jet[index*3 + 0] = (unsigned char)(255*(1-((i+1)/(JETLEN/4.0))));
    jet[index*3 + 1] = 0;
    jet[index*3 + 2] = 0;
  }


}


ColorMap::ColorMap(int type) : type(type)
{
  if (!initialized) {
    compute_solid();
    compute_rainbow();
    compute_white2black();
    compute_green2red();
    compute_jet();
    initialized = true;
  }

  switch(type) {
  /* set max to the index of the last cell 
     in the colormap i.e. length-1 */
  case SOLID_CMAP: 
    map = solid; 
    max = SOLIDLEN - 1; 
    break;
  case RAINBOW_CMAP: 
    map = rainbow;
    max = RAINBOWLEN - 1;
    break;
  case WHITE2BLACK_CMAP: 
    map = white2black;
    max = WHITE2BLACKLEN - 1; 
    break;
  case GREEN2RED_CMAP: 
    map = green2red;
    max = GREEN2REDLEN - 1;
    break;
  case JET_CMAP: 
    map = jet;
    max = JETLEN - 1;
    break;
  }
}
