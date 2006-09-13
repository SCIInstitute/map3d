 /*** Filename drawsurflmarks.cc
   Author: Rob MacLeod
   Draw coronary arteries if the data is present. 

   Last update: Wed Jun 22 13:25:58 MDT 1994
     - created from drawsurfcor.c
   Last update: Tue Jun 21 11:20:05 MDT 1994
     - updated colour selection for landmarks
   Last update: Mon Jun 20 13:09:53 MDT 1994
     - added code to draw a plane through the geometry.
   Last update: Thu Jun 16 16:07:02 MDT 1994
     - major clean up and new viewing model for the coronaries.
   Last update: Sat Apr  3 10:58:46 MST 1993
     - coronaries are now landmarks and must be drawn differently
       depending on the value of cor_type.
   Last update: Sun Oct  4 18:01:11 MDT 1992
     - have two modes of draw now, and some playing with color maps
       to get the lighted look.
   Last update: Thu Oct  1 20:39:42 MDT 1992
     - created

 ***/

#include "drawlandmarks.h"
#include "glprintf.h"
#include "map3dmath.h"
#include "colormaps.h"
#include "map3d-struct.h"
#include "landmarks.h"
#include "pickinfo.h"
#include "GeomWindow.h"

#include <math.h>
#ifdef OSX
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

extern Map3d_Info map3d_info;

void DrawLandMarks(Land_Mark * onelandmark, Landmark_Draw * onelandmarkdraw, GeomWindow* geom)
{
  long surfnum;
  long segnum;                  //, pickseg, pickpoint;
  surfnum = onelandmark->surfnum;
  if (map3d_info.reportlevel > 2)
    fprintf(stderr, "In DrawSurfLMarks with %ld segments to draw "
            "in surface #%ld\n", onelandmark->numsegs, surfnum + 1);

 /*** Loop through the landmark segments.  ***/

  for (segnum = 0; segnum < onelandmark->numsegs; segnum++) {

 /*** See if we have valid pointers to the coronary points
      and that we really have coronaries here and not some other 
      landmark.  And even if we have other landmarks, if they have more
      then one point, then use this method too - just change the colour
      below. 
  ***/

    if (map3d_info.reportlevel > 2)
      fprintf(stderr, " Drawsurflmarks: drawing segment %ld"
              " of type %ld\n", segnum + 1, onelandmark->segs[segnum].type);

    //  Draw coronary or catheter
    if ((onelandmark->segs[segnum].type ==
         LM_COR || onelandmark->segs[segnum].type == LM_CATH) && onelandmarkdraw->qshowcor) {
      if (map3d_info.reportlevel > 2)
        fprintf(stderr, " a coronary segment with drawtype = %ld\n", onelandmarkdraw->drawtype);
      DrawCorSeg(&onelandmark->segs[segnum], onelandmarkdraw, onelandmark->segs[segnum].type, geom);

 /*** If we have a single point for the landmark, draw a sphere
      there to mark it. ***/

    }
    else if (onelandmark->segs[segnum].type >= LM_OCCLUS &&
             onelandmark->segs[segnum].type <= LM_LEAD && onelandmarkdraw->qshowpoint) {
      DrawLMarkPoint(&onelandmark->segs[segnum], onelandmarkdraw, onelandmark->segs[segnum].type, geom);

 /*** Or else if we have a plane landmark, draw it.  ***/

    }
    else if (onelandmark->segs[segnum].type == LM_PLANE && onelandmarkdraw->qshowplane) {
      DrawLMarkPlane(&onelandmark->segs[segnum], onelandmarkdraw, geom);

 /*** Or else we have a rod so draw that  ***/

    }
    else if ((onelandmark->segs[segnum].type == LM_ROD ||
              onelandmark->segs[segnum].type == LM_PACENEEDLE ||
              onelandmark->segs[segnum].type == LM_RECNEEDLE ||
              onelandmark->segs[segnum].type == LM_FIBER ||
              onelandmark->segs[segnum].type == LM_CANNULA) && onelandmarkdraw->qshowrods) {
      DrawLMarkRod(&onelandmark->segs[segnum], onelandmarkdraw,
                   onelandmark->segs[segnum].type, (long)onelandmark->arrowradius, geom);

    }
  }

  //draw a sphere if in edit mode on the picked landmark point
  if (map3d_info.pickmode == EDIT_LANDMARK_PICK_MODE && onelandmarkdraw->picked_segnum >= 0) {
    LandMarkSeg *onelandmarkseg = &onelandmark->segs[onelandmarkdraw->picked_segnum];
    int ptnum = onelandmarkdraw->picked_ptnum;
    glColor3f(1, 0, 0);
    GLUquadricObj *sphere = gluNewQuadric();
    glTranslatef(onelandmarkseg->pts[ptnum][0], onelandmarkseg->pts[ptnum][1], onelandmarkseg->pts[ptnum][2]);
    gluSphere(sphere, onelandmarkseg->rad[ptnum] * 1.5, 32, 16);
    gluQuadricNormals(sphere, GL_SMOOTH);
  }
}

void DrawCorSeg(LandMarkSeg * onelandmarkseg, Landmark_Draw * draw, long type, GeomWindow* geom)
{
  long i;
  int slices = 3;
  GLUquadricObj *cylinder = gluNewQuadric();
  GLUquadricObj *sphere = gluNewQuadric();
  gluQuadricTexture(cylinder, GL_TRUE);

  if (map3d_info.pickmode == EDIT_LANDMARK_PICK_MODE || !draw->drawtype) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1, 1, 1);
  }
  else if (type == LM_COR) {
    glColor3fv(draw->coronarycolor);
    slices = 8;
  }
  else if (type == LM_CATH) {
    //glEnable(GL_LIGHTING);
    //glEnable(GL_FOG);
    glColor3fv(draw->cathcolor);
    slices = 8;
  }
  for (i = 0; i < onelandmarkseg->numpts - 1; i++) {
    float newvector[] = { onelandmarkseg->pts[i + 1][0] - onelandmarkseg->pts[i][0],
      onelandmarkseg->pts[i + 1][1] - onelandmarkseg->pts[i][1],
      onelandmarkseg->pts[i + 1][2] - onelandmarkseg->pts[i][2]
    };
    float length = vectorLength(newvector);
    float origvector[] = { 0, 0, 1 };
    float rotvector[3];

    float angle;
    AxisAndAngleOfRotation(origvector, newvector, angle, rotvector);

    glPushMatrix();
    //normalize(rotvector);
    glTranslatef(onelandmarkseg->pts[i][0], onelandmarkseg->pts[i][1], onelandmarkseg->pts[i][2]);
    //draw sphere at that point to fill gaps
    if (i > 0 && draw->drawtype) {
      gluSphere(sphere, onelandmarkseg->rad[i] * .95, 32, 16);
      gluQuadricNormals(sphere, GL_SMOOTH);
    }

    glRotatef(angle, rotvector[0], rotvector[1], rotvector[2]);

    gluCylinder(cylinder, onelandmarkseg->rad[i], onelandmarkseg->rad[i + 1], length, slices, 6);
    gluQuadricNormals(cylinder, GL_SMOOTH);
    glPopMatrix();
    if (map3d_info.pickmode == EDIT_LANDMARK_PICK_MODE || !draw->drawtype) {
      renderString3f(onelandmarkseg->pts[i][0] +
                     onelandmarkseg->rad[i] + 1,
                     onelandmarkseg->pts[i][1] +
                     onelandmarkseg->rad[i] + 1,
                     onelandmarkseg->pts[i][2] +
                     onelandmarkseg->rad[i] + 1, (int)geom->small_font, 
                     "%ld,%ld", onelandmarkseg->segnum, i);
    }
    else if (draw->qshowlabels && strlen(onelandmarkseg->labels[i]) > 0) {
      renderString3f(onelandmarkseg->pts[i][0] +
                     onelandmarkseg->rad[i] + 1,
                     onelandmarkseg->pts[i][1] +
                     onelandmarkseg->rad[i] + 1,
                     onelandmarkseg->pts[i][2] +
                     onelandmarkseg->rad[i] + 1, (int)geom->small_font, 
                     "%s", onelandmarkseg->labels[i]);
    }
  }

  //draw value and sphere at last point
  //glPushMatrix();
  if (map3d_info.pickmode == EDIT_LANDMARK_PICK_MODE || !draw->drawtype) {
    renderString3f(onelandmarkseg->pts[i][0] + onelandmarkseg->rad[i] + 1,
                   onelandmarkseg->pts[i][1] + onelandmarkseg->rad[i] + 1,
                   onelandmarkseg->pts[i][2] + onelandmarkseg->rad[i] + 1,
                   (int)geom->small_font,
                   "%ld,%ld", onelandmarkseg->segnum, i);
  }
  else if (draw->qshowlabels && strlen(onelandmarkseg->labels[i]) > 0) {
    renderString3f(onelandmarkseg->pts[i][0] +
                    onelandmarkseg->rad[i] + 1,
                    onelandmarkseg->pts[i][1] +
                    onelandmarkseg->rad[i] + 1,
                    onelandmarkseg->pts[i][2] +
                    onelandmarkseg->rad[i] + 1, (int)geom->small_font, 
                    "%s", onelandmarkseg->labels[i]);
  }
  //glTranslatef(onelandmarkseg->pts[i][0], onelandmarkseg->pts[i][1], 
  // onelandmarkseg->pts[i][2]);
  //gluSphere(sphere, onelandmarkseg->rad[i], 32,16);
  //gluQuadricNormals(sphere, GL_SMOOTH);
  //glPopMatrix();

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  return;
}

/*======================================================================*/
void DrawLMarkPoint(LandMarkSeg * onelandmarkseg, Landmark_Draw * draw, long type, GeomWindow* geom)
{
  GLUquadricObj *sphere = gluNewQuadric();

  if (type == LM_OCCLUS && draw->qshowocclus)
    glColor3fv(draw->occluscolor);
  else if (type == LM_STIM && draw->qshowstim)
    glColor3fv(draw->stimcolor);
  else if (type == LM_STITCH && draw->qshowstitch)
    glColor3fv(draw->occluscolor);
  else if (type == LM_LEAD && draw->qshowlead)
    glColor3fv(draw->leadcolor);
  else
    return;

  //glEnable(GL_LIGHTING);
  //glEnable(GL_FOG);
  glPushMatrix();

  glTranslatef(onelandmarkseg->pts[0][0], onelandmarkseg->pts[0][1], onelandmarkseg->pts[0][2]);

  gluSphere(sphere, onelandmarkseg->rad[0], 32, 16);
  gluQuadricNormals(sphere, GL_SMOOTH);
  glPopMatrix();

  if (draw->qshowlabels && strlen(onelandmarkseg->labels[0]) > 0) {
    renderString3f(onelandmarkseg->pts[0][0] +
                    onelandmarkseg->rad[0] + 1,
                    onelandmarkseg->pts[0][1] +
                    onelandmarkseg->rad[0] + 1,
                    onelandmarkseg->pts[0][2] +
                    onelandmarkseg->rad[0] + 1, (int)geom->small_font, 
                    "%s", onelandmarkseg->labels[0]);
  }
  glDisable(GL_LIGHTING);
  //glDisable(GL_FOG);
  return;
}

/*======================================================================*/
void DrawLMarkPlane(LandMarkSeg * onelandmarkseg, Landmark_Draw * draw, GeomWindow* geom)
{
 /*** Draw a plane through the first three points in the landmark
       list, and use the radius to determine the thickness.
       
       The assumed value of the landmark description for the plane is:
       
       X,Y,Z,           Radius
       ======           ======
       Point1        Radius of plane
       Point2        Thickness of plane
       Point3        unused
       
       The plane is drawn as a disk, of given radius, and given thickness.
       
  ***/

  int i;
  long pntnum;
  float plane_radius;
  float vec1[3];       /*** Vector from point 1 to point 2 ***/
  float vec2[3];       /*** Vector from point 1 to point 3 ***/
  float normalvec[3];     /*** Normal to the plane ***/
  float rotvector[3];     /*** axis of rotation ***/
  float centroid[3];     /*** Centroid of the three plane points. ***/
 /*********************************************************************/
  pntnum = 0;
  plane_radius = onelandmarkseg->rad[pntnum];
  if (map3d_info.reportlevel > 3) {
    for (i = 0; i <= onelandmarkseg->numpts; i++)
      fprintf(stderr, " %d %f %f %f %f\n",
              i,
              onelandmarkseg->pts[i][X], onelandmarkseg->pts[i][Y], onelandmarkseg->pts[i][Z], onelandmarkseg->rad[i]);
  }
  centroid[0] = (onelandmarkseg->pts[pntnum][0] +
                 onelandmarkseg->pts[pntnum + 1][0] + onelandmarkseg->pts[pntnum + 2][0]) / 3.0;
  centroid[1] = (onelandmarkseg->pts[pntnum][1] +
                 onelandmarkseg->pts[pntnum + 1][1] + onelandmarkseg->pts[pntnum + 2][1]) / 3.0;
  centroid[2] = (onelandmarkseg->pts[pntnum][2] +
                 onelandmarkseg->pts[pntnum + 1][2] + onelandmarkseg->pts[pntnum + 2][2]) / 3.0;
  if (map3d_info.reportlevel > 3) {
    printf(" Centroid is %f, %f, %f\n", centroid[0], centroid[1], centroid[2]);
  }
  vec1[0] = onelandmarkseg->pts[pntnum + 1][0] - onelandmarkseg->pts[pntnum][0];
  vec1[1] = onelandmarkseg->pts[pntnum + 1][1] - onelandmarkseg->pts[pntnum][1];
  vec1[2] = onelandmarkseg->pts[pntnum + 1][2] - onelandmarkseg->pts[pntnum][2];
  vec2[0] = onelandmarkseg->pts[pntnum + 2][0] - onelandmarkseg->pts[pntnum][0];
  vec2[1] = onelandmarkseg->pts[pntnum + 2][1] - onelandmarkseg->pts[pntnum][1];
  vec2[2] = onelandmarkseg->pts[pntnum + 2][2] - onelandmarkseg->pts[pntnum][2];

    /*** Find the normal vector to the plane. ***/
  crossProduct(vec1, vec2, normalvec);

  // Now take the normal and cross it with <0,0,1> to find the axis of rotation
  vec1[0] = normalvec[0];
  vec1[1] = normalvec[1];
  vec1[2] = normalvec[2];
  vec2[0] = 0;
  vec2[1] = 0;
  vec2[2] = 1;
  crossProduct(vec2, vec1, rotvector);

  //  get angles in positive degrees
  float angle = (acos(dotProduct(vec1, vec2)) * 180 / 3.1415926);
  angle = (angle > 180 ? angle - 360 : angle);

  // acos gives you one of only two possible values.  
  // use sin of the length of the 
  // cross product to find the other one and to compensate.
  float checkangle = (asin(vectorLength(rotvector)) * 180 / 3.1415926 + 360);
  checkangle = (checkangle > 360 ? checkangle - 360 : checkangle);
  float altangle = 360 - angle;
  float altcheckangle = (540 - checkangle);
  altcheckangle = (altcheckangle > 360 ? altcheckangle - 360 : altcheckangle);

  if ((fabs(altangle - checkangle) < .0001 || fabs(altangle - altcheckangle) < .0001))
    angle = altangle;

  //glEnable(GL_LIGHTING);
  if (draw->qtransplane) {
    //glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  glColor4fv(draw->planecolor);
  glPushMatrix();
  //normalize(rotvector);
  glTranslatef(centroid[0], centroid[1], centroid[2]);
  glRotatef(angle, rotvector[0], rotvector[1], rotvector[2]);

  GLUquadricObj *disk = gluNewQuadric();

  //glMultMatrixf((float*)trans);
  gluDisk(disk, 0, plane_radius, 20, 20);
  gluQuadricNormals(disk, GL_SMOOTH);

  glPopMatrix();

  glDisable(GL_BLEND);
  glDisable(GL_LIGHTING);
  //glDisable(GL_FOG);

  // draw labels for the three original points
  for (int i = 0; i < 3; i++)
    if (draw->qshowlabels && strlen(onelandmarkseg->labels[i]) > 0) {
      renderString3f(onelandmarkseg->pts[i][0],
                      onelandmarkseg->pts[i][1],
                      onelandmarkseg->pts[i][2], (int)geom->small_font, 
                      "%s", onelandmarkseg->labels[i]);
    }
}

/*======================================================================*/

void DrawLMarkRod(LandMarkSeg * onelandmarkseg, Landmark_Draw * draw, long type, long radius, GeomWindow* geom)
{
 /*** Draw a rods through the first two points in the landmark
      list, and use the radius to determine the thickness.
      
      The assumed value of the landmark description for the rod is:

      X,Y,Z,           Radius
      ======           ======
      Point1        Radius of rod
      Point2        unused

      The plane is drawn as a line, of given radius.
 ***/

  GLUquadricObj *cylinder = gluNewQuadric();

  if (type == LM_ROD && draw->qshowrod)
    glColor3fv(draw->rodcolor);
  else if (type == LM_RECNEEDLE && draw->qshowrecneedle)
    glColor3fv(draw->recneedlecolor);
  else if (type == LM_PACENEEDLE && draw->qshowpaceneedle)
    glColor3fv(draw->paceneedlecolor);
  else if (type == LM_FIBER && draw->qshowfiber)
    glColor3fv(draw->fibercolor);
  else if (type == LM_CANNULA && draw->qshowcannula)
    glColor3fv(draw->cannulacolor);
  else
    return;


  float newvector[] = { onelandmarkseg->pts[1][0] - onelandmarkseg->pts[0][0],
    onelandmarkseg->pts[1][1] - onelandmarkseg->pts[0][1],
    onelandmarkseg->pts[1][2] - onelandmarkseg->pts[0][2]
  };
  float length = vectorLength(newvector);
  float origvector[] = { 0, 0, 1 };
  float rotvector[3];

  crossProduct(origvector, newvector, rotvector);
  //  get angles in positive degrees
  float angle = (acos(dotProduct(origvector, newvector)) * 180 / 3.1415926);
  angle = (angle > 180 ? angle - 360 : angle);

  //acos gives you one of only two possible values.  
  //use sin of the length of the 
  //cross product to find the other one and to compensate.
  float checkangle = (asin(vectorLength(rotvector)) * 180 / 3.1415926 + 360);
  checkangle = (checkangle > 360 ? checkangle - 360 : checkangle);
  float altangle = 360 - angle;
  float altcheckangle = (540 - checkangle);
  altcheckangle = (altcheckangle > 360 ? altcheckangle - 360 : altcheckangle);

  if ((fabs(altangle - checkangle) < .0001 || fabs(altangle - altcheckangle) < .0001))
    angle = altangle;

  glEnable(GL_LIGHTING);
  //glEnable(GL_FOG);


  glPushMatrix();
  //normalize(rotvector);
  glTranslatef(onelandmarkseg->pts[0][0], onelandmarkseg->pts[0][1], onelandmarkseg->pts[0][2]);
  glRotatef(angle, rotvector[0], rotvector[1], rotvector[2]);

  gluCylinder(cylinder, onelandmarkseg->rad[0], onelandmarkseg->rad[0], length, 20, 6);
  gluQuadricNormals(cylinder, GL_SMOOTH);
  glPopMatrix();

  if (draw->qshowlabels && strlen(onelandmarkseg->labels[0]) > 0) {
    renderString3f(onelandmarkseg->pts[0][0] +
                    onelandmarkseg->rad[0] + 1,
                    onelandmarkseg->pts[0][1] +
                    onelandmarkseg->rad[0] + 1,
                    onelandmarkseg->pts[0][2] +
                    onelandmarkseg->rad[0] + 1, (int)geom->small_font, 
                    "%s", onelandmarkseg->labels[0]);
  }

  if (draw->qshowlabels && strlen(onelandmarkseg->labels[1]) > 0) {
    renderString3f(onelandmarkseg->pts[1][0] +
                    onelandmarkseg->rad[1] + 1,
                    onelandmarkseg->pts[1][1] +
                    onelandmarkseg->rad[1] + 1,
                    onelandmarkseg->pts[1][2] +
                    onelandmarkseg->rad[1] + 1, (int)geom->small_font, 
                    "%s", onelandmarkseg->labels[1]);
  }
  if (type == LM_FIBER) {
    radius = 0;
    //hack to rid warning - will draw an arrowhead later
  }

  glDisable(GL_LIGHTING);
  //glDisable(GL_FOG);
}

/*======================================================================*/
void DrawArrowhead(float tip[3], float arrowpts[4][3])
{
 /*** Draw an arrowhead around tip using arrowpts  ***/

  glLineWidth(1);
  glBegin(GL_POLYGON);
  glColor3f(1, 0, 0);
  glVertex3f(arrowpts[0][0], arrowpts[0][1], arrowpts[0][2]);
  glVertex3f(arrowpts[1][0], arrowpts[1][1], arrowpts[1][2]);
  glVertex3f(arrowpts[3][0], arrowpts[3][1], arrowpts[3][2]);
  glVertex3f(arrowpts[2][0], arrowpts[2][1], arrowpts[2][2]);
  glEnd();

  glBegin(GL_POLYGON);
  glColor3f(0, 0, 1);
  glVertex3f(tip[0], tip[1], tip[2]);
  glVertex3f(arrowpts[0][0], arrowpts[0][1], arrowpts[0][2]);
  glVertex3f(arrowpts[1][0], arrowpts[1][1], arrowpts[1][2]);
  glEnd();
  glBegin(GL_POLYGON);
  glColor3f(0, 0, 1);
  glVertex3f(tip[0], tip[1], tip[2]);
  glVertex3f(arrowpts[1][0], arrowpts[1][1], arrowpts[1][2]);
  glVertex3f(arrowpts[3][0], arrowpts[3][1], arrowpts[3][2]);
  glEnd();
  glBegin(GL_POLYGON);
  glVertex3f(tip[0], tip[1], tip[2]);
  glVertex3f(arrowpts[3][0], arrowpts[3][1], arrowpts[3][2]);
  glVertex3f(arrowpts[2][0], arrowpts[2][1], arrowpts[2][2]);
  glEnd();
  glBegin(GL_POLYGON);
  glVertex3f(tip[0], tip[1], tip[2]);
  glVertex3f(arrowpts[2][0], arrowpts[2][1], arrowpts[2][2]);
  glVertex3f(arrowpts[0][0], arrowpts[0][1], arrowpts[0][2]);
  glEnd();

  glBegin(GL_LINES);
  glColor3f(.7, .7, .7);
  glVertex3f(tip[0], tip[1], tip[2]);
  glVertex3f(arrowpts[0][0], arrowpts[0][1], arrowpts[0][2]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3f(tip[0], tip[1], tip[2]);
  glVertex3f(arrowpts[3][0], arrowpts[3][1], arrowpts[3][2]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3f(tip[0], tip[1], tip[2]);
  glVertex3f(arrowpts[1][0], arrowpts[1][1], arrowpts[1][2]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3f(tip[0], tip[1], tip[2]);
  glVertex3f(arrowpts[2][0], arrowpts[2][1], arrowpts[2][2]);
  glEnd();
}
