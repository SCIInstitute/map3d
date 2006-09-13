/* savescreen.cxx */

#ifdef _WIN32
#include <windows.h>
#include <limits.h>
#pragma warning(disable:4505)
#undef TRACE
#endif
//#else
//#include <values.h>

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

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include <png.h>

//some versions of the jpeg include do not have extern "C"
#ifndef _WIN32
extern "C" {
#else
//jpeg wants these to be defined for windows
#define _WIN32_
#define __WIN32__
// compile to link against a static jpeg (we need dynamic jpeg for X-based systems)
#define JPEG_STATIC
#endif

#include <jpeglib.h>

#ifndef _WIN32
}
#endif
#ifdef OSX
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

int progresswin;
static bool save_lock = false;
extern Map3d_Info map3d_info;
extern MainWindow *masterWindow;
extern SaveDialog *savedialog;
extern int key_pressed;

void write_row_callback(png_structp /*png_ptr */ , png_uint_32 /*row */ , int /*pass */ )
{
}


void SaveScreen()
{
  // saveScreen will flush the event queue in order to ensure all the windows 
  // are drawn up-to-date, which will often result in 
  // calling other events which will call SaveScreen.  We don't want this to
  // happen - otherwise we will get a million identical images.
  if (save_lock)
    return;  
  int x=INT_MAX, y=INT_MAX, x_max=INT_MIN, y_max=INT_MIN, height, width;
  unsigned i, j;
  float *pixels;
  char filename[280];

  // make sure we finish any GTK redraws before we continue
  // turn off the animation idle loop to avoid conflicts in gtk_main
  if (map3d_info.saving_animations) 
    gtk_idle_remove(savedialog->animation_idle_loop);

  // turn off frame advance idle loop
  if (key_pressed) {
    for (unsigned i = 0; i < numGeomWindows(); i++) {
      GeomWindow* win = GetGeomWindow(i);
      if (win && win->idle_id != 0) {
        gtk_idle_remove(win->idle_id);
        win->idle_id = -1;
      }
    }
  }

  save_lock = true;
  while (gtk_events_pending())
    gtk_main_iteration ();
  save_lock = false;

  // turn it back on. 
  if (map3d_info.saving_animations)
    savedialog->animation_idle_loop = 
      gtk_idle_add_priority(GTK_PRIORITY_REDRAW,(GtkFunction) AnimationIdleLoop, savedialog);

  if (key_pressed) {
    for (unsigned i = 0; i < numGeomWindows(); i++) {
      GeomWindow* win = GetGeomWindow(i);
      if (win && win->idle_id == -1) { // we just turned it off
        win->idle_id = gtk_idle_add_priority(GTK_PRIORITY_REDRAW,(GtkFunction) GeomWindowHandleFrameAdvances, win);
      }
    }
  }

  if (savedialog && savedialog->image_filename) {
    if (strcmp(map3d_info.imagefile, gtk_entry_get_text(GTK_ENTRY(savedialog->image_filename))) != 0) {
      strcpy(map3d_info.imagefile, gtk_entry_get_text(GTK_ENTRY(savedialog->image_filename)));
      map3d_info.imagesuffix = 0;
    }
  }
  // make the serialized filename
  int suffix = map3d_info.imagesuffix;
  char* fill = "";
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
    char message[300];
    sprintf(message, "Overwrite %s?", filename);
    fclose(test);
    if (savedialog) {
      GtkWidget* overwrite = gtk_message_dialog_new(GTK_WINDOW(savedialog->window), GTK_DIALOG_DESTROY_WITH_PARENT, 
                               GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, message);
      int response = gtk_dialog_run(GTK_DIALOG(overwrite));
      gtk_widget_destroy(overwrite);
      if (response != GTK_RESPONSE_OK)
        return;
    }
  }

  FILE* fout;
  if (masterWindow) {
    //gtk_window_get_position(GTK_WINDOW(masterWindow->window), &x, &y);
    gtk_window_get_size(GTK_WINDOW(masterWindow->window), &width, &height);

    x = y = 0;
    y_max = height;
    x_max = width;
    //width += 2*map3d_info.globalBorderWidth;
    //height += 2*map3d_info.globalBorderWidth+map3d_info.globalTitleHeight;
  }
  else {
    for (i = 0; i < numWindows(); i++) {
      GenericWindow* win = GetWindow(i);
      if (win) {
        int tmpx, tmpy, tmpwidth, tmpheight;
        gtk_window_get_position(GTK_WINDOW(win->window), &tmpx, &tmpy);
        gtk_window_get_size(GTK_WINDOW(win->window), &tmpwidth, &tmpheight);

        x = MIN(x, tmpx);
        y = MIN(y, tmpy);
        x_max = MAX(x_max, tmpx+tmpwidth);
        y_max = MAX(y_max, tmpy+tmpheight);
      }
    }
    // gtk still doesn't compensate for borders...
    width = x_max-x;//+2*map3d_info.globalBorderWidth;
    height = y_max-y;//+2*map3d_info.globalBorderWidth+map3d_info.globalTitleHeight;
  }

  if (savedialog && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(savedialog->image_default_res))) {
    width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(savedialog->image_res_width));
    height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(savedialog->image_res_height));
    y_max = height+y;
    x_max = width+x;
  }

  pixels = (float *)malloc(sizeof(float) * height * width * 3);
  if (!pixels)
    printf("Not enough memory to save image: we need %d bytes of memory\n", height*width*3*sizeof(char));

  // set all to bg color
  float* bgcolor = GetGeomWindow(0)->bgcolor;
  for (i = 0; i < (unsigned)width*height; i++) {
    pixels[i*3+0] = bgcolor[0];
    pixels[i*3+1] = bgcolor[1];
    pixels[i*3+2] = bgcolor[2];
  }

  // draw the windows in order of their reverse pop order (between 1 and apx. numWindows)
  int highestVal = 99999;
  for (i = 0; i < numWindows(); i++) {
    int winid = -1;
    int bestVal = 0;
    for (unsigned j = 0; j < numWindows(); j++) {
      GLWindow* tmpwin = GetWindow(j);
      if (tmpwin && tmpwin->poplevel < highestVal && tmpwin->poplevel > bestVal) {
        bestVal = tmpwin->poplevel;
        winid = j;
      }
    }
    highestVal = bestVal;
    if (winid == -1)
      continue;
    GLWindow* win = (GLWindow*) GetWindow(winid);
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

  
  // do png file
  if (strcmp(GetExtension(filename),".png") == 0) {
    fout = fopen(filename, "wb");

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
      return;
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
      return;
    }
    png_init_io(png_ptr, fout);
    png_set_write_status_fn(png_ptr, write_row_callback);
    png_textp text_ptr = (png_textp) png_malloc(png_ptr, (png_uint_32) sizeof(png_text));

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    //png_set_gAMA(png_ptr, info_ptr, 2.0);

    text_ptr[0].key = "Software";
    text_ptr[0].text = "Map3D";
    text_ptr[0].text_length = 5;
    text_ptr[0].compression = PNG_TEXT_COMPRESSION_zTXt;

    //png_set_text(png_ptr, info_ptr, text_ptr, 1);

    png_bytep *row_pointers = (png_bytep *) png_malloc(png_ptr, height * sizeof(png_bytep));
    for (y = height - 1, j = 0; y >= 0; y--, j++)
      row_pointers[y] = &char_pixels[j * (width) * 3];
    //png_set_rows(png_ptr, info_ptr, row_pointers);

    //png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);

    png_free(png_ptr, row_pointers);
    png_free(png_ptr, text_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);//(png_infopp) NULL);
    //delete gammacorrected;
    fclose(fout);
  }

  // save a ppm file
  else if (strcmp(GetExtension(filename),".ppm") == 0) {
    fout = fopen(filename, "wb");
    fprintf(fout, "P6\n");
    fprintf(fout, "%i %i\n", width, height);
    fprintf(fout, "255\n");
    for (y = height - 1; y >= 0; y--)
      for (x = 0; x < width; x++) {
        for (int i = 0; i < 3; i++) {
          unsigned char tmp = char_pixels[i+x*3+y*width*3];
          fprintf(fout, "%c", tmp);
        }
      }
    fclose(fout);
  }

  // save a jpg file
  else if (strcmp(GetExtension(filename),".jpg") == 0) {
    fout = fopen(filename, "wb");

    jpeg_compress_struct jpg_info;
    jpeg_error_mgr jpg_err;
    jpg_info.err = jpeg_std_error(&jpg_err);
    jpeg_create_compress(&jpg_info);

    jpeg_stdio_dest(&jpg_info, fout);

    jpg_info.image_width = width;
    jpg_info.image_height = height;
    jpg_info.input_components = 3; // RGB
    jpg_info.in_color_space = JCS_RGB;
    jpeg_set_defaults(&jpg_info);

    jpeg_start_compress(&jpg_info, TRUE);

    while (jpg_info.next_scanline < jpg_info.image_height) {
      JSAMPROW row = &char_pixels[(height-jpg_info.next_scanline-1)*width*3];
      jpeg_write_scanlines(&jpg_info, &row, 1);
    }
    jpeg_finish_compress(&jpg_info);
    jpeg_destroy_compress(&jpg_info);
    fclose(fout);
  }
  else {
    printf("Extension %s not supported.  Please use .ppm, .png, or .jpg",
      GetExtension(filename));
    delete pixels;
    delete char_pixels;
    return;
  }
  printf(" %s saved to disk\n", filename);
  delete pixels;
  delete char_pixels;
  unsigned long temp;
  if (map3d_info.saving_animations)
    savedialog->animation_last_save_time = g_timer_elapsed(savedialog->timer, &temp);
  //printf("%ul\n", savedialog->animation_last_save_time);
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;


METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}


Texture* readImage(const char* filename)
{
  const char* ext = GetExtension((char*) filename);
  if (strcmp(ext,".png") == 0 || strcmp(ext,".PNG") == 0) {
    // more or less copied from http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-3.4
    FILE * infile;		/* source file */
    if ((infile = fopen(filename, "rb")) == NULL) {
      fprintf(stderr, "can't open %s\n", filename);
      return 0;
    }
    // check for valid png file, and read ahead 8 bytes
    png_byte check_buffer[8];
    fread(check_buffer, 1, 8, infile);
    if (png_sig_cmp(check_buffer, 0, 8)) {
      fprintf(stderr, "%s is not a valid png file\n", filename);
      return 0; 
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if (!png_ptr) {
      fprintf(infile, "Cannot initialize png struct\n");
      return 0;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      png_destroy_read_struct(&png_ptr,
        (png_infopp)NULL, (png_infopp)NULL);
      fprintf(infile, "Cannot initialize png struct\n");
      return 0;
    }

    png_infop end_info = png_create_info_struct(png_ptr);

    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      return 0;
    }

    png_init_io(png_ptr, infile);
    png_set_sig_bytes(png_ptr, 8);

    // finally read the data
    png_read_info(png_ptr, info_ptr);
    png_uint_32 width, height;
    int bit, color, interlace, compression, filter, channels;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit, &color, &interlace, &compression, &filter);
    channels = png_get_channels(png_ptr, info_ptr);
    png_bytep row_data = (png_bytep) png_malloc(png_ptr, width*channels);
    unsigned char* image_data = new unsigned char[width*height*channels];
    for (unsigned i = 0; i < height; i++) {
      png_read_row(png_ptr, row_data, NULL);
      for (unsigned j = 0; j < width; j++)
        for (int k = 0; k < channels; k++) {
          //printf("  dumping %d\n", (width*channels*i)+(j*channels)+k);
          image_data[((height-1-i)*width*channels)+(j*channels)+k] = row_data[(j*channels)+k];
        }

    }

    //printf("  size %d\n",width*height*channels); 

    // cleanup
    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    Texture* t = CreateTexture(BGIMAGE_TEXTURE, GL_TEXTURE_2D, channels, width, height, GL_RGB, GL_LINEAR, GL_LINEAR, GL_REPLACE, image_data, true);
    png_free(png_ptr, row_data);
    delete image_data;
    fclose(infile);
    return t;
  }
  else if (strcmp(ext,".jpg") == 0 || strcmp(ext,".JPG") == 0 || strcmp(ext,".jpeg") == 0 || strcmp(ext,".JPEG") == 0) {
    // got sample code from libjpeg.doc
    FILE * infile;		/* source file */
    struct jpeg_decompress_struct cinfo; /* jpg file info */
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    if ((infile = fopen(filename, "rb")) == NULL) {
      fprintf(stderr, "can't open %s\n", filename);
      return 0;
    }
    
    //setup
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int row_stride = width * cinfo.output_components;

    unsigned char* image_data = new unsigned char[row_stride*height];

    JSAMPROW row_ptr[1];
    while (cinfo.output_scanline < cinfo.output_height) {
      row_ptr[0] = &image_data[(cinfo.output_height-1-cinfo.output_scanline)*row_stride];
      (void) jpeg_read_scanlines(&cinfo, row_ptr, 1);
    }
    Texture* t = CreateTexture(BGIMAGE_TEXTURE, GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_LINEAR, GL_LINEAR, GL_REPLACE, image_data, true);
    delete image_data;
    fclose(infile);
    return t;
  }
#if 0
  else if (strcmp(GetExtension(filename),".ppm") == 0) {
    return 0;
  }
#endif
  else {
    printf("Extension %s not supported.  Can only read .png, or .jpg",
      GetExtension(filename));
    return 0;

  }
}
