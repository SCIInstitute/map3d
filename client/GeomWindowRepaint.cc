/* GeomWindowRepaint.cxx */

#include <stddef.h>
#ifdef _WIN32
#  include <windows.h>
#  pragma warning(disable:4505)
#  undef TRACE
#else
#  include <unistd.h>
#endif

#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif
#include <float.h>
#include <limits.h>
#include <deque>
#include <algorithm>
#include <set>
#include <math.h>

using std::set;

#include "Contour_Info.h"
#include "Map3d_Geom.h"
#include "Surf_Data.h"
#include "drawlandmarks.h"
#include "GeomWindow.h"
#include "WindowManager.h"
#include "LegendWindow.h"
#include "PickWindow.h"
#include "BallMath.h"
#include "Transforms.h"
#include "dialogs.h"
#include "glprintf.h"
#include "colormaps.h"
#include "texture.h"
#include "pickinfo.h"
#include "lock.h"
#include "map3dmath.h"
#include "reportstate.h"
#include "GeomWindowMenu.h"
#include "scalesubs.h"

#include <QApplication>
#include <QDesktopWidget>

extern Map3d_Info map3d_info;
#define CHAR_WIDTH .07
#define CHAR_HEIGHT .07

//void GeomWindowRepaint(GeomWindow *priv)
void GeomWindow::paintGL()
{
  int loop = 0;
  int length = meshes.size();
  Mesh_Info *curmesh = 0;
  
  
  /* clear the screen */
  glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], 1);
  glFogfv(GL_FOG_COLOR, bgcolor);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // if it exists, draw a background image
  if (map3d_info.bg_texture)
    DrawBGImage();

  /* the fog's extent is a per window attribute */
  //printf("fog1 = %f\n",fog1);
  //printf("fog2 = %f\n",fog2);
  glFogf(GL_FOG_START, l2norm * fog1);
  glFogf(GL_FOG_END, l2norm * fog2);
  
  if (length > 1 && !map3d_info.lockgeneral) {
    loop = dominantsurf;
    length = loop + 1;
  }
  
#ifdef ROTATING_LIGHT
  // draw rotated light
  HMatrix mNow;
  Ball_Value(&light_pos, mNow);
  glMultMatrixf((float *)mNow);
#endif
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  if (clip->front_enabled)
    glEnable(GL_CLIP_PLANE0);
  else
    glDisable(GL_CLIP_PLANE0);

  if (clip->back_enabled)
    glEnable(GL_CLIP_PLANE1);
  else
    glDisable(GL_CLIP_PLANE1);
  
  /* render each mesh in this window : OPAQUE PASS */
  for (; loop < length; loop++) {
    
    /* setup a bunch of variables for quick access */
    curmesh = (meshes[loop]);
    
    if (curmesh->lighting)
      glEnable(GL_LIGHTING);
    if (curmesh->fogging)
      glEnable(GL_FOG);
    
    /* compute the mesh position (rotations, translations, etc.) */
    Transform(curmesh, 0, true);
    
    /* draw the color mapped surface */
    if (curmesh->shadingmodel != SHADE_NONE && curmesh->geom->points[curmesh->geom->geom_index] && !curmesh->shadefids && 
        curmesh->data && curmesh->drawmesh != RENDER_MESH_ELTS && curmesh->drawmesh != RENDER_MESH_ELTS_CONN) {
      glEnable(GL_POLYGON_OFFSET_FILL);
      DrawSurf(curmesh);
      glDisable(GL_POLYGON_OFFSET_FILL);
    }
    
    /* draw fiducial map surface*/
    if (curmesh->data && curmesh->drawfids){
      if(curmesh->drawfidmap < curmesh->fidMaps.size()){
        if (curmesh->shadefids && curmesh->shadingmodel != SHADE_NONE){
          glEnable(GL_POLYGON_OFFSET_FILL);
          DrawFidMapSurf(curmesh,curmesh->fidMaps[curmesh->drawfidmap]);
          glDisable(GL_POLYGON_OFFSET_FILL);
        }
      }
    }
    
    /* draw the mesh */
    if (curmesh->drawmesh && curmesh->geom->points[curmesh->geom->geom_index]) {
      glEnable(GL_POLYGON_OFFSET_FILL);
      if (curmesh->drawmesh == RENDER_MESH_ELTS || curmesh->drawmesh == RENDER_MESH_NONDATA_ELTS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
      else if (curmesh->drawmesh == RENDER_MESH_ELTS_CONN) {
        glPolygonMode(GL_FRONT, GL_FILL);
        glPolygonMode(GL_BACK, GL_LINE);
      }
      else if (curmesh->drawmesh >= RENDER_MESH_ELTS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
      //if it is secondary mesh draw in secondary color
      if (loop == secondarysurf && length > 1)
        DrawMesh(curmesh, 1);
      else
        DrawMesh(curmesh, 0);
      
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDisable(GL_POLYGON_OFFSET_FILL);
    }
    
    glDisable(GL_LIGHTING);
    
    /* draw the contour lines */
    if (curmesh->drawcont && curmesh->data)
      DrawCont(curmesh);
    
    /* draw the fiducial contour lines */
    if (curmesh->data && curmesh->drawfids){
      for(unsigned i = 0; i<curmesh->fidConts.size();i++){
        if (curmesh->drawFidConts[i])
          DrawFidCont(curmesh,curmesh->fidConts[i]);
      }
      if(curmesh->drawfids && curmesh->drawfidmap < curmesh->fidMaps.size() && !curmesh->drawcont){
        DrawFidMapCont(curmesh,curmesh->fidMaps[curmesh->drawfidmap]);
      }
    }
    
    glDisable(GL_FOG);
  }
  
  int start = 0;
  loop = 0;
  length = meshes.size();
  if (length > 1 && !map3d_info.lockgeneral) {
    loop = dominantsurf;
    length = loop + 1;
    start = loop;
  }
  
  /* render each mesh in this window : TRANSPARENT PASS */
  for (; loop < length; loop++) {
    /* setup a bunch of variables for quick access */
    curmesh = (meshes[loop]);
    
    /* compute the mesh position (rotations, translations, etc.) */
    Transform(curmesh, 0, true);
    
    if (curmesh->fogging)
      glEnable(GL_FOG);
    
    /* draw all nodes (and extrema markings) */
    if (curmesh->qshowpnts)
      DrawNodes(curmesh);
				
    //glDisable(GL_FOG);
    
    //draw all axes or one per window, based on all_axes
    if (curmesh->axes && (all_axes || start == loop)) {
      DrawAxes(curmesh);
    }
    
    glDisable(GL_FOG);
    
    /* draw the landmarks */
    if (curmesh->lighting)
      glEnable(GL_LIGHTING);
    if (curmesh->fogging)
      glEnable(GL_FOG);
    
    
    if (curmesh->geom->landmarks && curmesh->landmarkdraw.qshowlmark)
      DrawLandMarks(curmesh->geom->landmarks, &curmesh->landmarkdraw, this);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
  }
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  /* print the window's info text */
  if (showinfotext && !clip->back_enabled && !clip->front_enabled) {
    DrawInfo();
  }
    
  /* draw lock icons */
  if (showlocks && !clip->back_enabled && !clip->front_enabled) {
    if (map3d_info.lockgeneral != LOCK_OFF)
      DrawLockSymbol(0, map3d_info.lockgeneral == LOCK_FULL);
    if (map3d_info.lockrotate != LOCK_OFF)
      DrawLockSymbol(1, map3d_info.lockrotate == LOCK_FULL);
    if (map3d_info.lockframes != LOCK_OFF)
      DrawLockSymbol(2, map3d_info.lockframes == LOCK_FULL);
  }

#if SHOW_OPENGL_ERRORS
  GLenum e = glGetError();
  if (e)
    printf("GeomWindow Repaint OpenGL Error: %s\n", gluErrorString(e));
#endif
}

void DrawMesh(Mesh_Info * curmesh, bool secondary)
{
  //  int curframe = 0;
  int length = 0;
  int index;
  int loop2, loop3;
  float **modelpts = 0;
  float **ptnormals = 0;
  float **fcnormals = 0;
  //  float **contpts1 = 0;
  //  float **contpts2 = 0;
  //  unsigned char* curmap = 0;
  Map3d_Geom *curgeom = 0;
  //  Contour_Info* curcont = 0;
  //  Transforms* curtran = 0;
  
  curgeom = curmesh->geom;
  
  // draw points if we are in points mode or a PTS_ONLY geom and not in no-render mode
  bool drawpts = curmesh->drawmesh == RENDER_MESH_PTS || curmesh->drawmesh == RENDER_MESH_PTS_CONN ||
    (curmesh->drawmesh >= RENDER_MESH_ELTS && !curgeom->elements);
  
  modelpts = curgeom->points[curgeom->geom_index];
  ptnormals = curgeom->ptnormals;
  fcnormals = curgeom->fcnormals;
  
  glLineWidth(curmesh->meshsize);
  glPointSize(curmesh->meshsize);
  
  if (secondary)
    glColor3fv(curmesh->secondarycolor);
  else
    glColor3fv(curmesh->meshcolor);
  
  
  if (curmesh->drawmesh >= RENDER_MESH_ELTS && curgeom->elementsize == 4) {
    //drawpts = false;
    length = curgeom->numelements;
    
    if (curmesh->shadingmodel == SHADE_GOURAUD)
      glShadeModel(GL_SMOOTH);
    else
      glShadeModel(GL_FLAT);
    
    glBegin(GL_TRIANGLES);
    
    for (loop2 = 0; loop2 < length; loop2++) {
      // this inner loop is  to avoid repeating the 
      // following index/glNormal/glVertex code 4 times
      for (loop3 = 0; loop3 < 4; loop3++) {
        int idx1, idx2, idx3;
        if (loop3 == 3) {
          idx1 = 1;
          idx2 = 3;
          idx3 = 2;
        }
        else if (loop3 == 2) {
          idx1 = 0;
          idx2 = loop3;
          idx3 = 1;
        }
        else {
          idx1 = 0;
          idx2 = loop3;
          idx3 = loop2 + 1;
        }
        if (curmesh->shadingmodel == SHADE_GOURAUD) {
          index = curgeom->elements[loop2][idx1];
          glNormal3fv(ptnormals[index]);
          glVertex3fv(modelpts[index]);
          index = curgeom->elements[loop2][idx2];
          glNormal3fv(ptnormals[index]);
          glVertex3fv(modelpts[index]);
          index = curgeom->elements[loop2][idx3];
          glNormal3fv(ptnormals[index]);
          glVertex3fv(modelpts[index]);
        }
        else {
          index = curgeom->elements[loop2][idx1];
          glNormal3fv(fcnormals[index]);
          glVertex3fv(modelpts[index]);
          index = curgeom->elements[loop2][idx2];
          glVertex3fv(modelpts[index]);
          index = curgeom->elements[loop2][idx3];
          glVertex3fv(modelpts[index]);
        }
      }
      
    }
    glEnd();
  }
  else if (curmesh->drawmesh >= RENDER_MESH_ELTS && curgeom->elementsize == 3) {
    length = curgeom->numelements;
    
    if (curmesh->shadingmodel != SHADE_FLAT)
      glShadeModel(GL_SMOOTH);
    else
      glShadeModel(GL_FLAT);
    glBegin(GL_TRIANGLES);
    
    for (loop2 = 0; loop2 < length; loop2++) {
      // if we have no data on at least one node in draw-nondata-mode, then draw
      if (curmesh->drawmesh == RENDER_MESH_NONDATA_ELTS && !(curgeom->channels[curgeom->elements[loop2][0]] <= -1 || 
        curgeom->channels[curgeom->elements[loop2][1]] <= -1 || curgeom->channels[curgeom->elements[loop2][2]] <= -1))
        continue;
      if (curmesh->shadingmodel != SHADE_FLAT) {
        index = curgeom->elements[loop2][0];
        glNormal3fv(ptnormals[index]);
        glVertex3fv(modelpts[index]);
        index = curgeom->elements[loop2][1];
        glNormal3fv(ptnormals[index]);
        glVertex3fv(modelpts[index]);
        index = curgeom->elements[loop2][2];
        glNormal3fv(ptnormals[index]);
        glVertex3fv(modelpts[index]);
      }
      else {
        glNormal3fv(fcnormals[loop2]);
        glVertex3fv(modelpts[curgeom->elements[loop2][0]]);
        glVertex3fv(modelpts[curgeom->elements[loop2][1]]);
        glVertex3fv(modelpts[curgeom->elements[loop2][2]]);
      }
    }
    
    glEnd();
  }
  else if (curmesh->drawmesh >= RENDER_MESH_ELTS  && curgeom->elementsize == 2) {
    drawpts = false;
    length = curgeom->numelements;
    
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    
    for (loop2 = 0; loop2 < length; loop2++) {
      glVertex3fv(modelpts[curgeom->elements[loop2][0]]);
      glVertex3fv(modelpts[curgeom->elements[loop2][1]]);
    }
    
    glEnd();
    
    if (curmesh->lighting)
      glEnable(GL_LIGHTING);
    
  }
  if (drawpts) {
    
    length = curgeom->numpts;
    
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);
    if (curmesh->shadingmodel != SHADE_NONE)
      glColor3f(1., 1., 1.);
    
    for (loop2 = 0; loop2 < length; loop2++)
      glVertex3fv(modelpts[loop2]);
    
    glEnd();
    if (curmesh->lighting)
      glEnable(GL_LIGHTING);
  }
#if SHOW_OPENGL_ERRORS
  GLenum e = glGetError();
  if (e)
    printf("GeomWindow DrawMesh OpenGL Error: %s\n", gluErrorString(e));
#endif
}

void DrawSurf(Mesh_Info * curmesh)
{
  int curframe = 0;
  int length = 0;
  int index;
  int loop2, loop3;
  float a = 1, b = 0;
  float mean = 0;
  float **modelpts = 0;
  float **ptnormals = 0;
  float **fcnormals = 0;
  Map3d_Geom *curgeom = 0;
  Surf_Data *cursurf = 0;
  Contour_Info *curcont = 0;
  
  curgeom = curmesh->geom;
  cursurf = curmesh->data;
  
  modelpts = curgeom->points[curgeom->geom_index];
  ptnormals = curgeom->ptnormals;
  fcnormals = curgeom->fcnormals;
  
  if (cursurf) {
    curframe = cursurf->framenum;
    curcont = curmesh->cont;
    //if (cursurf->minmaxframes)
    //compute_mapping(curmesh, cursurf, a, b);
  }
  
  if ((int)a == INT_MAX || (int)b == INT_MAX) //change by BJW to avoid crash
    a = b = 0;                  //when there is no data
  
  float potmin, potmax;
  cursurf->get_minmax(potmin, potmax);
  
  if (map3d_info.scale_mapping == SYMMETRIC) {
    if (fabs(potmax) > fabs(potmin))
      potmin = -potmax;
    else
      potmax = -potmin;
  }
  if (map3d_info.scale_mapping == SEPARATE) {
    if (potmax < 0)
      potmax = 0;
    if (potmin > 0)
      potmin = 0;
  }
  
  unsigned char color[3];
  
  bool use_textures = false;
  // gouraud shading
  if (curmesh->shadingmodel == SHADE_GOURAUD) {
    glShadeModel(GL_SMOOTH);
    use_textures = curmesh->gouraudstyle == SHADE_TEXTURED && 
      (curmesh->cmap->type == RAINBOW_CMAP || curmesh->cmap->type == JET_CMAP);

    if (use_textures) {
      glColor3f(1,1,1);
      glEnable(GL_TEXTURE_1D);
      if (curmesh->cmap->type == RAINBOW_CMAP)
        UseTexture(map3d_info.rainbow_texture);
      else
        UseTexture(map3d_info.jet_texture);
    }
  }
  else if (curmesh->shadingmodel == SHADE_FLAT)
    glShadeModel(GL_FLAT);
  
  if (curgeom->elementsize == 3 && curmesh->shadingmodel != SHADE_BANDED) {
    length = curgeom->numelements;
    glBegin(GL_TRIANGLES);
    
    float potval;
    for (loop2 = 0; loop2 < length; loop2++) {
      if (curmesh->shadingmodel == SHADE_GOURAUD) {
	// avoid repeating code 3 times
        for (loop3 = 0; loop3 < 3; loop3++) {
          index = curgeom->elements[loop2][loop3];
          if (cursurf->potvals[curframe][index] == UNUSED_DATA)
            break;
        }
        if (loop3 < 3)
          // we have "UNUSED_DATA" on a node in this triangle, so don't draw here
          continue;
	for (loop3 = 0; loop3 < 3; loop3++) {
	  index = curgeom->elements[loop2][loop3];
	  potval = cursurf->potvals[curframe][index];

	  if (use_textures)
            glTexCoord1f(getContNormalizedValue(potval, potmin, potmax, curmesh->invert));
	  else {
            getContColor(potval, potmin, potmax, curmesh->cmap, color, curmesh->invert);
            glColor3ubv(color);
	  }
	  glNormal3fv(ptnormals[index]);
	  glVertex3fv(modelpts[index]);
	}
      }
      else {
	mean = 0;
	for (loop3 = 0; loop3 < 3; loop3++) {
	  index = curgeom->elements[loop2][loop3];
          potval = cursurf->potvals[curframe][index];
          if (potval == UNUSED_DATA)
            break;
	  mean += potval;
	}
        if (loop3 < 3)
          // we have "UNUSED_DATA" on a node in this triangle, so don't draw here
          continue;

	mean /= 3;
	getContColor(mean, potmin, potmax, curmesh->cmap, color, curmesh->invert);
	glColor3ubv(color);
	glNormal3fv(fcnormals[loop2]);
	
	for (loop3 = 0; loop3 < 3; loop3++) {
	  index = curgeom->elements[loop2][loop3];
	  glVertex3fv(modelpts[index]);
	}
      }
    }
    glEnd();
  }
  else if (curgeom->elementsize == 4 && curmesh->shadingmodel != SHADE_BANDED) {
    length = curgeom->numelements;
    glBegin(GL_TRIANGLES);
    for (loop2 = 0; loop2 < length; loop2++) {
      // this inner loop is a hack to avoid repeating the 
      // following index/glNormal/glVertex code 4 times
      for (loop3 = 0; loop3 < 4; loop3++) {
	int idx1, idx2, idx3;
	if (loop3 == 3) {
	  idx1 = 1;
	  idx2 = 3;
	  idx3 = 2;
	}
	else if (loop3 == 2) {
	  idx1 = 0;
	  idx2 = loop3;
	  idx3 = 1;
	}
	else {
	  idx1 = 0;
	  idx2 = loop3;
	  idx3 = loop2 + 1;
	}
	if (curmesh->shadingmodel == SHADE_GOURAUD) {
	  index = curgeom->elements[loop2][idx1];
	  glNormal3fv(ptnormals[index]);
	  glVertex3fv(modelpts[index]);
	  index = curgeom->elements[loop2][idx2];
	  glNormal3fv(ptnormals[index]);
	  glVertex3fv(modelpts[index]);
	  index = curgeom->elements[loop2][idx3];
	  glNormal3fv(ptnormals[index]);
	  glVertex3fv(modelpts[index]);
	}
	else {
	  glNormal3fv(fcnormals[loop2]);
	  glVertex3fv(modelpts[curgeom->elements[loop2][idx1]]);
	  glVertex3fv(modelpts[curgeom->elements[loop2][idx2]]);
	  glVertex3fv(modelpts[curgeom->elements[loop2][idx3]]);
	}
      }
    }
    
    glEnd();
  }
  
  //band shading
  if (curmesh->shadingmodel == SHADE_BANDED && curcont) {
    if (curmesh->lighting)
      glShadeModel(GL_SMOOTH);

    float** pts;
    float** normals;

    float potval;
    for (loop2 = 0; loop2 < curcont->numbandpolys; loop2++) {
      length = curcont->bandpolys[loop2].numpts;
      pts = curcont->bandpolys[loop2].nodes;
      normals = curcont->bandpolys[loop2].normals;

      int contnum = curcont->bandpolys[loop2].bandcol;
      float* conts = curcont->isolevels;
      if (contnum == -1)
        potval = potmin;
      else if (contnum == curcont->numlevels - 1)
        potval = potmax;
      else {
        potval = conts[contnum] + (conts[contnum+1]-conts[contnum])*(contnum+1)/(curcont->numlevels);
      }
      getContColor(potval, potmin, potmax, curmesh->cmap, color, curmesh->invert);
      glColor3ubv(color);
      
      glBegin(GL_POLYGON);
      

      for (loop3 = 0; loop3 < length; loop3++) {
        glNormal3fv(normals[loop3]);
	glVertex3fv(pts[loop3]);
      }
      glNormal3fv(normals[0]);
      glVertex3fv(pts[0]);
      
      glEnd();
    }
    
    if (curmesh->lighting)
	    glEnable(GL_LIGHTING);
  }
  glDisable(GL_TEXTURE_1D);
#if SHOW_OPENGL_ERRORS
  GLenum e = glGetError();
  if (e)
    printf("GeomWindow DrawSurf OpenGL Error: %s\n", gluErrorString(e));
#endif
}

void DrawCont(Mesh_Info * curmesh)
{
  int length = 0;
  int loop2;
  float **contpts1 = 0;
  float **contpts2 = 0;
  Surf_Data *cursurf = 0;
  Contour_Info *curcont = 0;
  
  cursurf = curmesh->data;
  
  if (cursurf) {
    curcont = curmesh->cont;    
  }
  
  float potmin, potmax;
  cursurf->get_minmax(potmin, potmax);
  
  if (map3d_info.scale_mapping == SYMMETRIC) {
    if (fabs(potmax) > fabs(potmin))
      potmin = -potmax;
    else
      potmax = -potmin;
  }
  
  if (map3d_info.scale_mapping == SEPARATE) {
    if (potmax < 0)
      potmax = 0;
    if (potmin > 0)
      potmin = 0;
  }
  
  unsigned char color[3];
  
  if (curcont) {
    contpts1 = curcont->contpt1;
    contpts2 = curcont->contpt2;
	  length = curcont->numisosegs;  

  }
  
  glLineWidth(curmesh->contsize);
  glPointSize(curmesh->contsize);
  
  
  float potval;
  for (loop2 = 0; loop2 < length; loop2++) {    
    potval = curcont->isolevels[curcont->contcol[loop2]];
    getContColor(potval, potmin, potmax, curmesh->cmap, color, curmesh->invert);
    
    if(map3d_info.contour_antialiasing){
      glEnable (GL_LINE_SMOOTH);
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    }
    
    if (curmesh->shadingmodel == SHADE_NONE)
      glColor3ubv(color);
    else
      glColor3ub(0,0,0);
    if (curcont->isolevels[curcont->contcol[loop2]] < 0 && curmesh->negcontdashed)
      glEnable(GL_LINE_STIPPLE);
        
    glBegin(GL_LINES);
    glVertex3fv(contpts1[loop2]);
    glVertex3fv(contpts2[loop2]);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    if(map3d_info.contour_antialiasing){
      glDisable (GL_LINE_SMOOTH);
      glDisable (GL_BLEND);
    }

  }
#if SHOW_OPENGL_ERRORS
  GLenum e = glGetError();
  if (e)
    printf("GeomWindow DrawCont OpenGL Error: %s\n", gluErrorString(e));
#endif
}

void DrawFidMapCont(Mesh_Info * curmesh, Contour_Info *cont)
{
  int length = 0;
  int loop2;
  float **contpts1 = 0;
  float **contpts2 = 0;
  Surf_Data *cursurf = 0;
  Contour_Info *curcont = 0;
  
  cursurf = curmesh->data;
  
  if (cursurf) {
    curcont = cont;    //used to be curframe instead of 0
                       //if (cursurf->minmaxframes)
                       //compute_mapping(curmesh, cursurf, a, b);
  }
  
  float fidmin, fidmax;
  cursurf->get_fid_minmax(fidmin, fidmax, cont->datatype);
  
  //   if (map3d_info.scale_mapping == SYMMETRIC) {
  //     if (fabs(potmax) > fabs(potmin))
  //       potmin = -potmax;
  //     else
  //       potmax = -potmin;
  //   }
  
  //   if (map3d_info.scale_mapping == SEPARATE) {
  //     if (potmax < 0)
  //       potmax = 0;
  //     if (potmin > 0)
  //       potmin = 0;
  //   }
  
  unsigned char color[3];
  
  if (curcont) {
    contpts1 = curcont->contpt1;
    contpts2 = curcont->contpt2;
  }
  
  glLineWidth(curmesh->contsize);
  glPointSize(curmesh->contsize);
  
  length = curcont->numisosegs;
  //printf("length = %d\n", length);
  //printf("levels = %d\n", curcont->numlevels);
  
  
  //  if (curmesh->shadingmodel != SHADE_NONE) {
  float fidval;
  for (loop2 = 0; loop2 < length; loop2++) {
    //printf("curcont->contcol[%d] = %d\n",loop2,curcont->contcol[loop2]);
    //printf("curcont->numtrisegs[curcont->trinum[%d] = %d\n",curcont->trinum[loop2],curcont->numtrisegs[curcont->trinum[loop2]]);
    //printf("curcont->trinum[%d] = %d\n",loop2,curcont->trinum[loop2]);
    
    fidval = curcont->isolevels[curcont->contcol[loop2]];
    getContColor(fidval, fidmin, fidmax, curmesh->cmap, color, curmesh->invert);
    
    if(map3d_info.contour_antialiasing){
      glEnable (GL_LINE_SMOOTH);
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    }
    
    if (curmesh->shadingmodel == SHADE_NONE)
      glColor3ubv(color);
    else
      glColor3ub(0,0,0);
    if (curcont->isolevels[curcont->contcol[loop2]] < 0 && curmesh->negcontdashed)
      glEnable(GL_LINE_STIPPLE);
    
    //     printf("contpts1[%d][0] = %f\n",loop2,contpts1[loop2][0]);
    //     printf("contpts1[%d][1] = %f\n",loop2,contpts1[loop2][1]);
    //     printf("contpts1[%d][2] = %f\n",loop2,contpts1[loop2][2]);
    
    //     printf("contpts2[%d][0] = %f\n",loop2,contpts2[loop2][0]);
    //     printf("contpts2[%d][1] = %f\n",loop2,contpts2[loop2][1]);
    //     printf("contpts2[%d][2] = %f\n",loop2,contpts2[loop2][2]);
    
    glBegin(GL_LINES);
    glVertex3fv(contpts1[loop2]);
    glVertex3fv(contpts2[loop2]);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    if(map3d_info.contour_antialiasing){
      glDisable (GL_LINE_SMOOTH);
      glDisable (GL_BLEND);
    }
  }
  //  }
  /*  else {
    glColor4ub(0, 0, 0, 128);
  
  for (loop2 = 0; loop2 < length; loop2++) {
    if (curcont->isolevels[curcont->contcol[loop2]] < 0 && curmesh->negcontdashed)
      glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    
    glVertex3fv(contpts1[loop2]);
    glVertex3fv(contpts2[loop2]);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
  }
  }
*/
#if SHOW_OPENGL_ERRORS
  GLenum e = glGetError();
  if (e)
    printf("GeomWindow DrawCont OpenGL Error: %s\n", gluErrorString(e));
#endif
}

void DrawFidMapSurf(Mesh_Info * curmesh,Contour_Info *cont)
{
  int curframe = 0;
  int length = 0;
  int index;
  int loop2, loop3;
  float a = 1, b = 0;
  float mean = 0;
  float **modelpts = 0;
  float **ptnormals = 0;
  float **fcnormals = 0;
  Map3d_Geom *curgeom = 0;
  Surf_Data *cursurf = 0;
  Contour_Info *curcont = 0;
  
  curgeom = curmesh->geom;
  cursurf = curmesh->data;
  
  modelpts = curgeom->points[curgeom->geom_index];
  ptnormals = curgeom->ptnormals;
  fcnormals = curgeom->fcnormals;
  
  if (cursurf) {
    curframe = cursurf->framenum;
    curcont = cont;
    //if (cursurf->minmaxframes)
    //compute_mapping(curmesh, cursurf, a, b);
  }
  
  if ((int)a == INT_MAX || (int)b == INT_MAX) //change by BJW to avoid crash
    a = b = 0;                  //when there is no data
  
  float fidmin, fidmax;
  cursurf->get_fid_minmax(fidmin, fidmax, cont->datatype);  
//  if (map3d_info.scale_mapping == SYMMETRIC) {
//    if (fabs(potmax) > fabs(potmin))
//      potmin = -potmax;
//    else
//      potmax = -potmin;
//  }
//  if (map3d_info.scale_mapping == SEPARATE) {
//    if (potmax < 0)
//      potmax = 0;
//    if (potmin > 0)
//      potmin = 0;
//  }
  
  unsigned char color[3];
  
  bool use_textures = false;
  // gouraud shading
  if (curmesh->shadingmodel == SHADE_GOURAUD) {
    use_textures = curmesh->gouraudstyle == SHADE_TEXTURED && 
    (curmesh->cmap->type == RAINBOW_CMAP || curmesh->cmap->type == JET_CMAP);
    
    if (use_textures) {
      glShadeModel(GL_SMOOTH);
      glEnable(GL_TEXTURE_1D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glColor3f(1,1,1);
      glBindTexture(GL_TEXTURE_1D, curmesh->cmap->type);
    }
    else
      glShadeModel(GL_SMOOTH);
  }
  else if (curmesh->shadingmodel == SHADE_FLAT)
    glShadeModel(GL_FLAT);
  
  if (curgeom->elementsize == 3 && curmesh->shadingmodel != SHADE_BANDED) {
    length = curgeom->numelements;
    glBegin(GL_TRIANGLES);
    
    float fidval;
    for (loop2 = 0; loop2 < length; loop2++) {
      if (curmesh->shadingmodel == SHADE_GOURAUD) {
        // avoid repeating code 3 times
        for (loop3 = 0; loop3 < 3; loop3++) {
          index = curgeom->elements[loop2][loop3];
          fidval = 0;
          //for(int fidsets = 0; fidsets < cursurf->numfs; fidsets++){
            if(index < cursurf->fids.numfidleads){
              for(int numfids = 0; numfids < cursurf->fids.leadfids[index].numfids; numfids++){
                if((cursurf->fids.leadfids[index].fidtypes[numfids] == cont->datatype)){
                  fidval = cursurf->fids.leadfids[index].fidvals[numfids];
                }
              }
            }
          //}
          
          if (use_textures)
            glTexCoord1f(getContNormalizedValue(fidval, fidmin, fidmax, curmesh->invert));
          else {
            getContColor(fidval, fidmin, fidmax, curmesh->cmap, color, curmesh->invert);
            glColor3ubv(color);
          }
          glNormal3fv(ptnormals[index]);
          glVertex3fv(modelpts[index]);
        }
      }
      else {
        mean = 0;
        for (loop3 = 0; loop3 < 3; loop3++) {
          index = curgeom->elements[loop2][loop3];
          //for(int fidsets = 0; fidsets < cursurf->numfs; fidsets++){
            if(index < cursurf->fids.numfidleads){
              for(int numfids = 0; numfids < cursurf->fids.leadfids[index].numfids; numfids++){
                if((cursurf->fids.leadfids[index].fidtypes[numfids] == cont->datatype)){
                  mean += cursurf->fids.leadfids[index].fidvals[numfids];
                }
              }
            }
          //}
        }
        mean /= 3;
        getContColor(mean, fidmin, fidmax, curmesh->cmap, color, curmesh->invert);
        glColor3ubv(color);
        glNormal3fv(fcnormals[loop2]);
        
        for (loop3 = 0; loop3 < 3; loop3++) {
          index = curgeom->elements[loop2][loop3];
          glVertex3fv(modelpts[index]);
        }
      }
    }
    glEnd();
  }
  else if (curgeom->elementsize == 4 && curmesh->shadingmodel != SHADE_BANDED) {
    length = curgeom->numelements;
    glBegin(GL_TRIANGLES);
    for (loop2 = 0; loop2 < length; loop2++) {
      // this inner loop is a hack to avoid repeating the 
      // following index/glNormal/glVertex code 4 times
      for (loop3 = 0; loop3 < 4; loop3++) {
        int idx1, idx2, idx3;
        if (loop3 == 3) {
          idx1 = 1;
          idx2 = 3;
          idx3 = 2;
        }
        else if (loop3 == 2) {
          idx1 = 0;
          idx2 = loop3;
          idx3 = 1;
        }
        else {
          idx1 = 0;
          idx2 = loop3;
          idx3 = loop2 + 1;
        }
        if (curmesh->shadingmodel == SHADE_GOURAUD) {
          index = curgeom->elements[loop2][idx1];
          glNormal3fv(ptnormals[index]);
          glVertex3fv(modelpts[index]);
          index = curgeom->elements[loop2][idx2];
          glNormal3fv(ptnormals[index]);
          glVertex3fv(modelpts[index]);
          index = curgeom->elements[loop2][idx3];
          glNormal3fv(ptnormals[index]);
          glVertex3fv(modelpts[index]);
        }
        else {
          glNormal3fv(fcnormals[loop2]);
          glVertex3fv(modelpts[curgeom->elements[loop2][idx1]]);
          glVertex3fv(modelpts[curgeom->elements[loop2][idx2]]);
          glVertex3fv(modelpts[curgeom->elements[loop2][idx3]]);
        }
      }
    }
    
    glEnd();
  }
  
  //band shading
  if (curmesh->shadingmodel == SHADE_BANDED) {
    //if (curmesh->lighting)
    //glShadeModel(GL_SMOOTH);
    
    float** pts;
    
    //glDisable(GL_LIGHTING);
    float fidval;
    for (loop2 = 0; loop2 < curcont->numbandpolys; loop2++) {
      length = curcont->bandpolys[loop2].numpts;
      pts = curcont->bandpolys[loop2].nodes;
      //normals = curcont->bandpolys[loop2].normals;
      
      //potval = curcont->bandpolys[loop2].bandpotval;
      int contnum = curcont->bandpolys[loop2].bandcol;
      float* conts = curcont->isolevels;
      if (contnum == -1)
        fidval = fidmin;
      else if (contnum == curcont->numlevels - 1)
        fidval = fidmax;
      else {
        fidval = conts[contnum] + (conts[contnum+1]-conts[contnum])*(contnum+1)/(curcont->numlevels);
      }
      getContColor(fidval, fidmin, fidmax, curmesh->cmap, color, curmesh->invert);
      glColor3ubv(color);
      
      glBegin(GL_POLYGON);
      
      
      for (loop3 = 0; loop3 < length; loop3++) {
        //glNormal3fv(normals[loop3]);
        glVertex3fv(pts[loop3]);
      }
      //glNormal3fv(normals[0]);
      glVertex3fv(pts[0]);
      
      glEnd();
    }
    
    if (curmesh->lighting)
      glEnable(GL_LIGHTING);
  }
  glDisable(GL_TEXTURE_1D);
#if SHOW_OPENGL_ERRORS
  GLenum e = glGetError();
  if (e)
    printf("GeomWindow DrawSurf OpenGL Error: %s\n", gluErrorString(e));
#endif
}




void DrawFidCont(Mesh_Info * curmesh, Contour_Info *cont)
{
  //printf("drawing fids!!!\n");
  //printf("fidtype %d\n",cont->datatype);
  int length = 0;
  int loop2;
  float **contpts1 = 0;
  float **contpts2 = 0;
  Surf_Data *cursurf = 0;
  Contour_Info *curcont = 0;
  
  cursurf = curmesh->data;
  
  if (cursurf) {
    curcont = cont;    //used to be curframe instead of 0
                       //if (cursurf->minmaxframes)
                       //compute_mapping(curmesh, cursurf, a, b);
  }
  
  
  if (curcont) {
    contpts1 = curcont->contpt1;
    contpts2 = curcont->contpt2;
  }
  
  glLineWidth(curcont->fidContSize);
  glPointSize(curcont->fidContSize);
  
  length = curcont->numisosegs;
  if(curcont->fidContSize > 0){
    for (loop2 = 0; loop2 < length; loop2++) {
      if(map3d_info.contour_antialiasing){
        glEnable (GL_LINE_SMOOTH);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
      }
      glColor3ub(curcont->fidcolor.red(),curcont->fidcolor.green(),curcont->fidcolor.blue());
//          if(curcont->datatype == 10)
//            glColor3ub(255,0,0);
//          if(curcont->datatype == 13)
//            glColor3ub(255,0,255);
//          if(curcont->datatype == 11)
//            glColor3ub(0,0,255);
      if (curcont->isolevels[curcont->contcol[loop2]] < 0 && curmesh->negcontdashed)
        glEnable(GL_LINE_STIPPLE);
      
      glBegin(GL_LINES);
      glVertex3fv(contpts1[loop2]);
      glVertex3fv(contpts2[loop2]);
      glEnd();
      glDisable(GL_LINE_STIPPLE);
      if(map3d_info.contour_antialiasing){
        glDisable (GL_LINE_SMOOTH);
        glDisable (GL_BLEND);
      }
    }
  }
  
#if SHOW_OPENGL_ERRORS
  GLenum e = glGetError();
  if (e)
    printf("GeomWindow DrawCont OpenGL Error: %s\n", gluErrorString(e));
#endif
}



void GeomWindow::Transform(Mesh_Info * curmesh, float factor, bool compensateForRetinaDisplay)
{
  HMatrix mNow, cNow;           // arcball rotation matrices
  
  GLdouble front_plane[] = { 0, 0, 1, clip->front };
  GLdouble back_plane[] = { 0, 0, -1, clip->back };
  

  int pixelFactor = 1;

  if (compensateForRetinaDisplay)
  {
    pixelFactor=QApplication::desktop()->devicePixelRatio();
    // this compensates for the "Retina" display ratio.  See http://doc.qt.io/qt-5/highdpi.html
    //  (for some reason the picking doesn't need this)
  }
  glViewport(0, 0, width()*pixelFactor, height()*pixelFactor);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(vfov, width() / (float)height(), l2norm, 3 * l2norm);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  /* get the arcall rotation */
  Ball_Value(&curmesh->tran->rotate, mNow);
  Ball_Value(&clip->bd, cNow);
  
  /* current mousing translation */
  glTranslatef(curmesh->tran->tx, curmesh->tran->ty, curmesh->tran->tz);
  
  /* finally, move the mesh to in front of the eye */
  glTranslatef(0, 0, l2norm * (factor - 2));
  
  /* draw clipping planes (if enabled) */
  glPushMatrix();
  glMultMatrixf((float *)cNow);
  glTranslatef(xcenter, ycenter, zcenter);
  
  glClipPlane(GL_CLIP_PLANE0, front_plane);
  glClipPlane(GL_CLIP_PLANE1, back_plane);
  
  glPopMatrix();
  
  /* include the arcball rotation */
  glMultMatrixf((float *)mNow);
		
  /* move center of mesh to origin */
  glTranslatef(-xcenter, -ycenter, -zcenter);
}

// draw all types of node marks, in this precedence:
//   triangulating nodes, leads, picks, extrema, all
void GeomWindow::DrawNodes(Mesh_Info * curmesh)
{
  //  int curframe = 0;
  int length = 0, loop = 0;
  float min = 0, max = 0, value = 0;
  float mNowI[16];
  float **modelpts = 0;
  Map3d_Geom *curgeom = 0;
  Surf_Data *cursurf = 0;
  HMatrix mNow;
  
  ColorMap *curmap = 0;
  curgeom = curmesh->geom;
  cursurf = curmesh->data;
  
  modelpts = curgeom->points[curgeom->geom_index];
  
  if (cursurf) {
    curmap = curmesh->cmap;
  }
  
  length = curgeom->numpts;
  
  /* set the transform for billboarding */
  Ball_Value(&curmesh->tran->rotate, mNow);
  TransposeMatrix16((float *)mNow, mNowI);
  Transform(curmesh, 0.01f, true);
  glPushMatrix();
  
  //if (!cursurf)
  //return;
  if (cursurf)
    cursurf->get_minmax(min, max);
  unsigned char color[3];
  
  map<int, char*> lead_labels;
  set<int> pick_nodes;

  for (int i = 0; i <= curmesh->pickstacktop; i++)
    pick_nodes.insert(curmesh->pickstack[i]->node);

  for (int i  = 0; i < curgeom->numleadlinks; i++) {
    lead_labels[curgeom->leadlinks[i]] = curgeom->leadlinklabels[i];
  }
  
  if (curmesh->mark_all_sphere || curmesh->mark_extrema_sphere || curmesh->mark_ts_sphere || curmesh->mark_lead_sphere || 
      ((map3d_info.pickmode == TRIANGULATE_PICK_MODE || map3d_info.pickmode == EDIT_NODE_PICK_MODE) && curmesh->num_selected_pts > 0)) {
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, .4f);
    float sphere_size = 1.0f;
    for (loop = 0; loop < length; loop++) {
      if (cursurf ) {
        value = cursurf->potvals[cursurf->framenum][loop];
        getContColor(value, min, max, curmap, color, curmesh->invert);
      }
      
      // in the process of triangulating these nodes (if we're in the right pick mode)
      if ((curmesh->num_selected_pts > 0 && curmesh->selected_pts[0] == loop) ||
          (curmesh->num_selected_pts > 1 && curmesh->selected_pts[1] == loop)) {
        
        float max_size;
        max_size = MAX(curmesh->mark_all_size, curmesh->mark_extrema_size);
        max_size = MAX(max_size, curmesh->mark_ts_size);
        max_size = MAX(max_size, curmesh->mark_lead_size);
        
        glColor3f(fabs(curmesh->meshcolor[0] - .4), fabs(curmesh->meshcolor[1] - .4), fabs(curmesh->meshcolor[2] - .4));
        glPointSize(height() / 200 * curmesh->mark_triangulate_size);
        sphere_size = curmesh->mark_triangulate_size;
        
      }
      
      // leadlink node
      else if (curmesh->mark_lead_sphere && lead_labels[loop] != 0) {
        glColor3f(curmesh->mark_lead_color[0], curmesh->mark_lead_color[1], curmesh->mark_lead_color[2]);
        glPointSize(height() / 200 * curmesh->mark_lead_size);
        sphere_size = curmesh->mark_lead_size;
      }
      
      // pick node
      else if (curmesh->mark_ts_sphere && pick_nodes.size() > 0 && pick_nodes.find(loop) != pick_nodes.end()) {
        if (loop == curmesh->curpicknode)
          glColor3f(1.0, 0.1, 1.f);
        else
          glColor3f(curmesh->mark_ts_color[0], curmesh->mark_ts_color[1], curmesh->mark_ts_color[2]);
        glPointSize(height() / 200 * curmesh->mark_ts_size);
        sphere_size = curmesh->mark_ts_size;
      }
      
      // extrema node
      else if (cursurf && (value == max || value == min) && curmesh->mark_extrema_sphere) {
        // switch for proper color-mapping
        if (value == max)
          value = min;
        else if (value == min)
          value = max;
        // if shading is on, assign the extrema to the opposite color or else you won't see it
        if (curmesh->shadingmodel != SHADE_NONE) 
          getContColor(value, min, max, curmap, color, curmesh->invert);
        glColor3ubv(color);
        sphere_size = curmesh->mark_extrema_size;
        glPointSize(height() / 200 * curmesh->mark_extrema_size);
      }
      
      // 'all' node
      else if (curmesh->mark_all_sphere) {
        if (curmesh->mark_all_sphere_value && cursurf && value != UNUSED_DATA) {
          glColor3ubv(color);
        }
        else {
          glColor3f(curmesh->mark_all_color[0], curmesh->mark_all_color[1], curmesh->mark_all_color[2]);
        }
        glPointSize(height() / 200 * curmesh->mark_all_size);
        sphere_size = curmesh->mark_all_size;
      }
      else {
        continue;
      }
      
      glTranslatef(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
      glMultMatrixf((float *)mNowI);
      glTranslatef(-modelpts[loop][0], -modelpts[loop][1], -modelpts[loop][2]);
      
      // try to convert the sphere size from geometry units to pixels
      // 400 is a good number to use to normalize the l2norm
      sphere_size = sphere_size*l2norm/400;
      if (curmesh->draw_marks_as_spheres)
        DrawDot(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2], sphere_size);
      else {
        glBegin(GL_POINTS);
        glVertex3f(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
        glEnd();
      }
      glPopMatrix();
      glPushMatrix();
    }
    
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
  }
  
  if (map3d_info.pickmode == TRIANGULATE_PICK_MODE && curmesh->num_selected_pts == 2) {  
    // draw a line between the picked nodes
    glColor3f(fabs(curmesh->meshcolor[0] - .4), fabs(curmesh->meshcolor[1] - .4), fabs(curmesh->meshcolor[2] - .4));
    glBegin(GL_LINES);
    int pt1 = curmesh->selected_pts[0];
    int pt2 = curmesh->selected_pts[1];
    glVertex3f(modelpts[pt1][0], modelpts[pt1][1], modelpts[pt1][2]);
    glVertex3f(modelpts[pt2][0], modelpts[pt2][1], modelpts[pt2][2]);
    glEnd();
  }
  
  float pos[3];
  
  if (curmesh->mark_all_number || (curmesh->mark_extrema_number && curmesh->data) ||
      curmesh->mark_ts_number || curmesh->mark_lead_number) {
    //glDepthMask(GL_FALSE);
    
    for (loop = 0; loop < length; loop++) {
      if (cursurf) {
        value = cursurf->potvals[cursurf->framenum][loop];
        getContColor(value, min, max, curmap, color, curmesh->invert);
      }
      glTranslatef(modelpts[loop][0], modelpts[loop][1], modelpts[loop][2]);
      glMultMatrixf((float *)mNowI);
      glTranslatef(-modelpts[loop][0], -modelpts[loop][1], -modelpts[loop][2]);
      pos[0] = modelpts[loop][0];
      pos[1] = modelpts[loop][1];
      pos[2] = modelpts[loop][2];
      
      int number = 0;
      
      // leadlink node
      if (curmesh->mark_lead_number && lead_labels[loop] != 0) {
        glColor3f(curmesh->mark_lead_color[0], curmesh->mark_lead_color[1], curmesh->mark_lead_color[2]);
        number = curmesh->mark_lead_number;
      }
      
      // pick node
      else if (curmesh->mark_ts_number && pick_nodes.find(loop) != pick_nodes.end()) {
        if (loop == curmesh->curpicknode)
          glColor3f(1.0, 0.1, 1.f);
        else
          glColor3f(curmesh->mark_ts_color[0], curmesh->mark_ts_color[1], curmesh->mark_ts_color[2]);
        number = curmesh->mark_ts_number;
      }
      else if (cursurf && (value == max || value == min) && curmesh->mark_extrema_number && cursurf->potvals) {
        if (value == max)
          value = min;
        else if (value == min)
          value = max;
        // if shading is on, assign the extrema to the opposite color or else you won't see it
        if (curmesh->shadingmodel != SHADE_NONE) 
          getContColor(value, min, max, curmap, color, curmesh->invert);
        glColor3ubv(color);
        number = curmesh->mark_extrema_number;
      }
      else if (curmesh->mark_all_number) {
        // this is a function of the fov (zoom), the ratio of
        // mesh's l2norm to the window's l2norm and the window
        // height to determine whether the numbers will be too
        // close together or not
        
        glColor3f(curmesh->mark_all_color[0], curmesh->mark_all_color[1], curmesh->mark_all_color[2]);
        number = curmesh->mark_all_number;
      }

      float scale = fontScale();
      switch (number) {
      case 1:
        renderString3f(pos[0], pos[1], pos[2], (int)small_font, QString::number(loop + 1), scale);
        break;
      case 2:
        if (curgeom->channels[loop]+1 > 0)
          renderString3f(pos[0], pos[1], pos[2], (int)small_font, QString::number(curgeom->channels[loop] + 1), scale);
        break;
      case 3:
        if (cursurf && cursurf->potvals && cursurf->potvals[cursurf->framenum][loop] != UNUSED_DATA)
          renderString3f(pos[0], pos[1], pos[2], (int)small_font, 
            QString::number(cursurf->potvals[cursurf->framenum][loop], 'g', 2), scale);
        break;
      case 4:
        // case 4 is dependent on which type of mark it is
        //   if it is a leadlink, and its value is 4, then print the lead label.
        //   if it is a fid, and its value is 4, then print the fid label
        if (curmesh->mark_lead_number == 4 && lead_labels[loop]) {
          renderString3f(pos[0], pos[1], pos[2], (int)small_font, lead_labels[loop], scale);
          break;
        }
        else if (curmesh->mark_all_number == 4 && cursurf->fids.numfidleads > 0){
          float fid = 0;
          if(loop < cursurf->fids.numfidleads){
            for(int numfids = 0; numfids < cursurf->fids.leadfids[loop].numfids; numfids++){
              if((cursurf->fids.leadfids[loop].fidtypes[numfids] == curmesh->drawfidmapcont)){
                fid = cursurf->fids.leadfids[loop].fidvals[numfids];
              }
            }
          }
          
          renderString3f(pos[0], pos[1], pos[2], (int)small_font, QString::number(fid, 'g', 2), scale);
          break;
        }
      }
      glPopMatrix();
      glPushMatrix();
    }
    glDepthMask(GL_TRUE);
  }
  
  glPopMatrix();
  
#if SHOW_OPENGL_ERRORS
  GLenum e = glGetError();
  if (e)
    printf("GeomWindow DrawNodes OpenGL Error: %s\n", gluErrorString(e));
#endif
}


void GeomWindow::DrawInfo()
{
  int nummesh = meshes.size();
  int surfnum = 0;
  float position[3] = { -1.f, static_cast<float>(height() - 15.0), 0.f };
  Mesh_Info *dommesh = 0;
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width(), 0, height());
  glColor3f(fgcolor[0], fgcolor[1], fgcolor[2]);
  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glPopMatrix();
  if (nummesh == 1 || (dominantsurf != -1 && nummesh > dominantsurf)) {
    dommesh = nummesh == 1 ? meshes[0] : meshes[dominantsurf];
    char surfstr[24];
    if (dommesh->geom->subsurf <= 0)
      sprintf(surfstr, "Surface #%d", dommesh->geom->surfnum);
    else
      sprintf(surfstr, "Surface #%d-%d", dommesh->geom->surfnum, dommesh->geom->subsurf);
    surfnum = dommesh->geom->surfnum;
    position[0] = (float)width()/2.0 -((float)getFontWidth((int)large_font, surfstr)/2.0);
    position[1] = height() - getFontHeight((int)large_font);
    
    renderString3f(position[0], position[1], position[2], (int)large_font, surfstr);
    
    position[1] -= getFontHeight((int)med_font)*.8 ;
    if (dommesh->data){
      char * slash = 0;// FIX shorten_filename(dommesh->data->potfilename);
      //position[0] = (float)width()/2.0 - ((float)getFontWidth((int)med_font, slash)/2.0);
      // FIX renderString3f(position[0], position[1], position[2], (int)med_font, "%s@%d", slash, dommesh->data->seriesnum+1);
    }
    else{
      char * slash = 0; // FIX shorten_filename(dommesh->geom->basefilename);
      //position[0] = (float)width()/2.0 - ((float)getFontWidth((int)med_font, slash)/2.0);
      // FIX renderString3f(position[0], position[1], position[2], (int)med_font, slash);
    }
  }
  else if (nummesh > 1) {
    position[0] = (float)width()/2.0 -((float)getFontWidth((int)large_font, "All Surfaces")/2.0);
    renderString3f(position[0], position[1], position[2], (int)large_font, "All Surfaces");
  }
}

void GeomWindow::DrawLockSymbol(int which, bool full)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  float aspect = (float)height() / width();
  float offset = .1 * aspect * which;
  float width;
  float texmax;
  if (full) {
    width = .1;
    texmax = 1;
  }
  else {
    width = .05;
    texmax = .5;
  }
  
  UseTexture(map3d_info.lock_texture);  
  switch (which) {
    case 0:
      glColor3f(.8, .8, 0);
      break;
    case 1:
      glColor3f(.8, 0, 0);
      break;
    case 2:
      glColor3f(0, .8, 0);
      break;
  }
  
  glEnable(GL_TEXTURE_2D);
  
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(-.95 + offset, -.85);
  glTexCoord2f(0, 1);
  glVertex2f(-.95 + offset, -.95);
  glTexCoord2f(texmax, 1);
  glVertex2f(-.95 + width * aspect + offset, -.95);
  glTexCoord2f(texmax, 0);
  glVertex2f(-.95 + width * aspect + offset, -.85);
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

void DrawDot(float x, float y, float z, float size) 
{
  glEnable(GL_TEXTURE_2D);
  UseTexture(map3d_info.dot_texture);
#if 0
  glBindTexture(GL_TEXTURE_2D, DOT_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
#endif

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex3f(x-size, y-size, z);
  glTexCoord2f(0, 1);
  glVertex3f(x-size, y+size, z);
  glTexCoord2f(1, 1);
  glVertex3f(x+size, y+size, z);
  glTexCoord2f(1, 0);
  glVertex3f(x+size, y-size, z);
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  
}

void GeomWindow::DrawAxes(Mesh_Info * mesh)
{
  //float normal[3]={0.f,0.f,-1.f};
  //float up[3]={0.f,1.f,0.f};
  float pos[3];
  float delta_x, delta_y, delta_z;
  
  bool xneg = false, yneg = false, zneg = false;
  
  if (mesh->geom->xmax >= xcenter)
    delta_x = mesh->geom->xmax + .1 * (mesh->geom->xmax - mesh->geom->xmin);
  else {
    delta_x = mesh->geom->xmin - .15 * (mesh->geom->xmax - mesh->geom->xmin);
    xneg = true;
  }
  if (mesh->geom->ymax >= ycenter)
    delta_y = mesh->geom->ymax + .15 * (mesh->geom->ymax - mesh->geom->ymin);
  else {
    delta_y = mesh->geom->ymin - .15 * (mesh->geom->ymax - mesh->geom->ymin);
    yneg = true;
  }
  if (mesh->geom->zmax >= zcenter)
    delta_z = mesh->geom->zmax + .15 * (mesh->geom->zmax - mesh->geom->zmin);
  else {
    delta_z = mesh->geom->zmin - .15 * (mesh->geom->zmax - mesh->geom->zmin);
    zneg = true;
  }
  
  //glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
  
  glLineWidth(3);
  glBegin(GL_LINES);
  
  //float d = l2norm*mesh->mark_all_size * 25;
  
  //draw axes
  if(rgb_axes){
    glColor3f(255,0,0);
  }
  else{
    glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
  }
  glVertex3f(xcenter, ycenter, zcenter);
  glVertex3f(delta_x, ycenter, zcenter);
  if(rgb_axes){
    glColor3f(0,255,0);
  }
  else{
    glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
  }
  glVertex3f(xcenter, ycenter, zcenter);
  glVertex3f(xcenter, delta_y, zcenter);
  if(rgb_axes){
    glColor3f(0,0,255);
  }
  else{
    glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
  }
  glVertex3f(xcenter, ycenter, zcenter);
  glVertex3f(xcenter, ycenter, delta_z);
  
  glEnd();
  
  //write axes labels - negative if dimension max < window's dimension center
  if (showinfotext) {
    pos[0] = xcenter + delta_x;
    pos[1] = ycenter;
    pos[2] = zcenter;
    
    if(rgb_axes){
      glColor3f(255,0,0);
    }
    else{
      glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
    }
    if (xneg)
      renderString3f(pos[0], pos[1], pos[2], (int)small_font, "-X");
    else
      renderString3f(pos[0], pos[1], pos[2], (int)small_font, "X");
    pos[0] = xcenter;
    pos[1] = delta_y;
    
    if(rgb_axes){
      glColor3f(0,255,0);
    }
    else{
      glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
    }
    if (yneg)
      renderString3f(pos[0], pos[1], pos[2], (int)small_font, "-Y");
    else
      renderString3f(pos[0], pos[1], pos[2], (int)small_font, "Y");
    pos[1] = ycenter;
    pos[2] = delta_z;
    
    if(rgb_axes){
      glColor3f(0,0,255);
    }
    else{
      glColor3f(mesh->axescolor[0], mesh->axescolor[1], mesh->axescolor[2]);
    }
    if (zneg)
      renderString3f(pos[0], pos[1], pos[2], (int)small_font, "-Z");
    else
      renderString3f(pos[0], pos[1], pos[2], (int)small_font, "Z");
  }
  
}

void GeomWindow::DrawBGImage()
{
  if (map3d_info.gi->bgcoords[0] != 0 || map3d_info.gi->bgcoords[3] != 1) {
    // user used a manual orientation - line it up as such
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(vfov, width() / (float)height(), l2norm, 3 * l2norm);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2 * l2norm);
    glTranslatef(-xcenter, -ycenter, -zcenter);
 }
  else {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }
  float* pmin = &map3d_info.gi->bgcoords[0];
  float* pmax = &map3d_info.gi->bgcoords[3];

  UseTexture(map3d_info.bg_texture);  
  
  glEnable(GL_TEXTURE_2D);
  
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);  glVertex3f(pmin[0], pmin[1], pmin[2]);
  glTexCoord2f(0, 1);  glVertex3f(pmin[0], pmax[1], pmax[2]);
  glTexCoord2f(1, 1);  glVertex3f(pmax[0], pmax[1], pmax[2]);
  glTexCoord2f(1, 0);  glVertex3f(pmax[0], pmin[1], pmin[2]);
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  glClear(GL_DEPTH_BUFFER_BIT);

}
