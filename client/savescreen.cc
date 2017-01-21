/* savescreen.cxx */

#include "WindowManager.h"
#include "GeomWindow.h"
#include "MainWindow.h"
#include "Map3d_Geom.h"
#include "MeshList.h"
#include "savescreen.h"
#include "dialogs.h"
#include "texture.h"
#include "dialogs.h"
#include "glprintf.h"
#include "ProcessCommandLineOptions.h"
#include "cutil.h"

#include <QPixmap>
#include <QImage>
#include <QApplication>
#include <QDesktopWidget>

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

int progresswin;
static bool save_lock = false;
extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;
extern int key_pressed;

void SaveScreen()
{
  // saveScreen will flush the event queue in order to ensure all the windows 
  // are drawn up-to-date, which will often result in 
  // calling other events which will call SaveScreen.  We don't want this to
  // happen - otherwise we will get a million identical images.
  if (save_lock)
    return;  
  int x=INT_MAX, y=INT_MAX, x_max=INT_MIN, y_max=INT_MIN, height, width;
  unsigned i;
  char filename[280];

  save_lock = true;
  qApp->processEvents();
  save_lock = false;

  // make the serialized filename
  int suffix = map3d_info.imagesuffix;
  const char* fill = "";
  for (; suffix <= INT_MAX; suffix++) {
    strcpy(filename, map3d_info.imagefile);
    StripExtension(filename);
    if (suffix > 99 && suffix < 1000)
      fill = "0";
    else if (suffix > 9 && suffix < 100)
      fill = "00";
    else if (suffix < 10)
      fill = "000";
    sprintf(filename,"%s%s%d%s", filename, fill, suffix, GetExtension(map3d_info.imagefile));
    FILE* test = fopen(filename, "r");
    if (test) {
      fclose(test);
    }
    else {
      map3d_info.imagesuffix = suffix+1;
      break; // pick this filename
    }
  }


  FILE* test = fopen(filename, "r");
  if (test) {
    fclose(test);
    if (!prompt_overwrite(0, filename))
      return;
  }

  QPixmap imageData;

  if (masterWindow) {
    QRect frameGeom = masterWindow->frameGeometry();
    x = frameGeom.x();
    y = frameGeom.y();
    width = frameGeom.width();
    height = frameGeom.height();
  }
  else
  {
    for (i = 0; i < numWindows(); i++) {
      Map3dGLWidget* win = GetWindow(i);
      if (win) {
        QRect frameGeom = win->frameGeometry();
        int tmpx = frameGeom.x();
        int tmpy = frameGeom.y();
        int tmpwidth = frameGeom.width();
        int tmpheight = frameGeom.height();

        x = MIN(x, tmpx);
        y = MIN(y, tmpy);
        x_max = MAX(x_max, tmpx+tmpwidth);
        y_max = MAX(y_max, tmpy+tmpheight);
      }
    }
    width = x_max - x;
    height = y_max - y;
  }
  // WM still doesn't compensate for borders...
  imageData = QPixmap::grabWindow(((QApplication*)qApp)->desktop()->winId(), x, y, width, height);

#if 0 
  FIX - should be able to do better
  QPixmap entireImage(width, height);

  // set all to bg color
  float* bgcolor = GetGeomWindow(0)->bgcolor;
  entireImage.fill(QColor::fromRgbF(bgcolor[0], bgcolor[1], bgcolor[2]));

  // draw the windows in order of their reverse pop order (between 1 and apx. numWindows)
  int highestVal = 99999;
  for (i = 0; i < numWindows(); i++) {
    int winid = -1;
    int bestVal = 0;
    for (unsigned j = 0; j < numWindows(); j++) {
      Map3dGLWidget* tmpwin = GetWindow(j);
      if (tmpwin && tmpwin->poplevel < highestVal && tmpwin->poplevel > bestVal) {
        bestVal = tmpwin->poplevel;
        winid = j;
      }
    }
    highestVal = bestVal;
    if (winid == -1)
      continue;
    Map3dGLWidget* win = (Map3dGLWidget*) GetWindow(winid);
    if (!win)
      continue;
    int tmpx, tmpy, tmpheight, tmpwidth;
    if (masterWindow) {
      tmpx = win->x;
      tmpy = win->y;
      tmpheight = win->height;
      tmpwidth = win->width;
    }
    else {
      gtk_window_get_position(GTK_WINDOW(win->window), &tmpx, &tmpy);
      gtk_window_get_size(GTK_WINDOW(win->window), &tmpwidth, &tmpheight);
    }
    
    float* tmppixels = new float[tmpheight * tmpwidth * 3];
    gdk_gl_drawable_gl_begin(win->gldrawable, win->glcontext);
    
    glFlush();
    glFinish();
    glReadPixels(0, 0, tmpwidth, tmpheight, GL_RGB, GL_FLOAT, tmppixels);
    gdk_gl_drawable_gl_end(win->gldrawable);
    for (int j = 0; j < tmpheight; j++) {
      // for cases where the specified image size is smaller than the set of map3d windows
      if (y_max-tmpy-tmpheight+j >= height)
        break;
       if (y_max-tmpy-tmpheight+j < 0)
         continue; // don't break here, because it could become valid in later iterations...
      for (int k = 0; k < tmpwidth; k++) {
        if (tmpx+k-x >= width)
          break;
        int index = (y_max-tmpy-tmpheight+j)*width+tmpx+k-x;
        int tmpindex = j*tmpwidth+k;

        pixels[index*3+0] = tmppixels[tmpindex*3+0];
        pixels[index*3+1] = tmppixels[tmpindex*3+1];
        pixels[index*3+2] = tmppixels[tmpindex*3+2];
        if (index > width*height || index < 0 || tmpindex > tmpwidth*tmpheight || tmpindex < 0) {
          printf("Potential error in screen coordinates\n");
        }
      }
    }
    delete tmppixels;
  }

  unsigned char *char_pixels = 0;
  char_pixels = (unsigned char *)malloc(height * width * 3);

  for (i = 0; i < (unsigned) height * width * 3; i++) {
    float tmp = (float)*(pixels + i);
  //    tmp = (tmp < 0) ? 0 : tmp;
  //    tmp = (tmp > 1) ? 1 : tmp;
  //    tmp = sqrt(tmp);
    char_pixels[i] = (unsigned char)(tmp * 255);
  }

#endif

  const char* extension = GetExtension(filename);
  if (strcmp(extension,".png") && strcmp(extension,".ppm") && strcmp(extension,".jpg"))
  {  
    printf("Warning: %s file type not supported", extension);
    return;
  }

  imageData.save(filename);
  printf(" %s saved to disk\n", filename);
}

Texture* readImage(const char* filename)
{
  QImage pixmap(filename);

  if (pixmap.isNull())
  {
    printf("Could not load %s", filename);
    return 0;
  }

  unsigned int width = pixmap.width();
  unsigned int height = pixmap.height();

  unsigned char* image_data = new unsigned char[width*height*3];
  for (unsigned i = 0; i < height; i++) 
  {
    for (unsigned j = 0; j < width; j++)
    {
      QRgb pixel = pixmap.pixel(j, i);
      image_data[((height-1-i)*width*3)+(j*3)+0] = qRed(pixel);
      image_data[((height-1-i)*width*3)+(j*3)+1] = qBlue(pixel);
      image_data[((height-1-i)*width*3)+(j*3)+2] = qGreen(pixel);
    }
  }

  Texture* t = CreateTexture(BGIMAGE_TEXTURE, GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_LINEAR, GL_LINEAR, GL_REPLACE, image_data, true);
  return t;
}
