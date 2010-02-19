/***** Ball.c *****/
/* Ken Shoemake, 1993 */
/* Modified by Victor Ng, Jan. 1996 for OpenGL */

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable:4172 4514 4244 4305)  /* quiet visual c++ */
#endif

#include <math.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "Ball.h"
#include "BallMath.h"

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#ifdef GFX_DEF_FMATH
#define sinf sin
#define cosf cos
#endif

#define false 0
#define true 1

#define LG_NSEGS 4
#define NSEGS (1<<LG_NSEGS)
#define RIMCOLOR()    glColor3f(0.0, 1.0, 1.0);
#define FARCOLOR()    glColor3f(0.8, 0.5, 0.0);
#define NEARCOLOR()   glColor3f(1.0, 0.8, 0.0);
#define DRAGCOLOR()   glColor3f(0.0, 1.0, 1.0);
#define RESCOLOR()    glColor3f(0.8, 0.0, 0.0);

HMatrix mId = { {1, 0, 0, 0}
, {0, 1, 0, 0}
, {0, 0, 1, 0}
, {0, 0, 0, 1}
};
float otherAxis[][4] = { {-0.48, 0.80, 0.36, 1} };

BallData *Ball_Alloc()
{
  return (BallData *) malloc(sizeof(BallData));
}

void Ball_Free(BallData * ball)
{
  free(ball);
}

/* Establish reasonable initial values for controller. */
void Ball_Init(BallData * ball, double aspect)
{
  int i;
  ball->center = qOne;
  ball->radius = 1.0;
  ball->aspect = aspect;
  ball->ballType = ArcBall;
  ball->vDown = ball->vNow = qOne;
  ball->qDown = ball->qNow = qOne;
  for (i = 15; i >= 0; i--)
    ((float *)ball->mNow)[i] = ((float *)ball->mDown)[i] = ((float *)mId)[i];
  ball->showResult = ball->dragging = false;
  ball->axisSet = NoAxes;
  ball->sets[CameraAxes] = mId[X];
  ball->setSizes[CameraAxes] = 3;
  ball->sets[BodyAxes] = ball->mDown[X];
  ball->setSizes[BodyAxes] = 3;
  ball->sets[OtherAxes] = otherAxis[X];
  ball->setSizes[OtherAxes] = 1;
}

/* Set the center and size of the controller. */
void Ball_Place(BallData * ball, HVect center, double radius)
{
  ball->center = center;
  ball->radius = radius;
}

/* Incorporate new mouse position. */
void Ball_Mouse(BallData * ball, HVect vNow)
{
  ball->vNow = vNow;
}

/* Choose a constraint set, or none. */
void Ball_UseSet(BallData * ball, AxisSet axisSet)
{
  /* if (!ball->dragging) */
  ball->axisSet = axisSet;
}

/* Begin drawing arc for all drags combined. */
void Ball_ShowResult(BallData * ball)
{
  ball->showResult = true;
}

/* Stop drawing arc for all drags combined. */
void Ball_HideResult(BallData * ball)
{
  ball->showResult = false;
}

/* Using vDown, vNow, dragging, and axisSet, compute rotation etc. */
/* To use closest constraint axis, set aI to -1 */
void Ball_Update(BallData * ball, int aI)
{
  int setSize = ball->setSizes[ball->axisSet];
  HVect *set = (HVect *) (ball->sets[ball->axisSet]);
  ball->vFrom = MouseOnSphere(ball->vDown, ball->center, ball->radius);
  ball->vTo = MouseOnSphere(ball->vNow, ball->center, ball->radius);
  if (ball->dragging) {
    if (ball->axisSet != NoAxes) {
      aI = (aI == -1) ? ball->axisIndex : aI;
      ball->vFrom = ConstrainToAxis(ball->vFrom, set[aI]);
      ball->vTo = ConstrainToAxis(ball->vTo, set[aI]);
    }
    ball->qDrag = Qt_FromBallPoints(ball->vFrom, ball->vTo, ball->ballType);
    ball->qNow = Qt_Mul(ball->qDrag, ball->qDown);
  }
  else {
    if (ball->axisSet != NoAxes) {
      ball->axisIndex = (aI == -1) ? NearestConstraintAxis(ball->vTo, set, setSize) : aI;
    }
  }
  Qt_ToBallPoints(ball->qDown, &ball->vrFrom, &ball->vrTo);
  Qt_ToMatrix(Qt_Conj(ball->qNow), ball->mNow); /* Gives transpose for GL. */
}

/* Returns true iff vMouse is a point off the arcball sphere */
int Ball_MouseOutsideSphere(BallData * ball, HVect vMouse)
{
  HVect vTest = MouseOnSphere(vMouse, ball->center, ball->radius);

  return vTest.z == 0.0;
}

/* Set the BallType: 'ArcBall' rotates through _twice_ the angle
* between the two points on the sphere, and exhibits no
* hysteresis. 'TrackBall' rotates through exactly the angle between
* the two points on the sphere, like a physical trackball, and
* consequently exhibits hysteresis. */
void Ball_SetBallType(BallData * ball, BallType bt)
{
  ball->ballType = bt;
}

/* Return rotation matrix defined by controller use. */
void Ball_Value(BallData * ball, HMatrix mNow)
{
  int i;
  for (i = 15; i >= 0; i--)
    ((float *)mNow)[i] = ((float *)ball->mNow)[i];
}


/* Begin drag sequence. */
void Ball_BeginDrag(BallData * ball)
{
  ball->dragging = true;
  ball->vDown = ball->vNow;
}

/* Stop drag sequence. */
void Ball_EndDrag(BallData * ball)
{
  int i;
  ball->dragging = false;
  ball->qDown = ball->qNow;
  for (i = 15; i >= 0; i--)
    ((float *)ball->mDown)[i] = ((float *)ball->mNow)[i];
}

/* Draw the controller with all its arcs. */
void Ball_Draw(BallData * ball, int drawBall, int drawConstraints, int drawArc)
{
  float r = ball->radius;

  glPushAttrib(GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_DEPTH_TEST);

  // Must use near and far clip planes at -1.0 and 1.0 or
  // risk clipping out the circle and arcs drawn.
  // glOrtho(-1.0,1.0,-1.0,1.0,-1.0,1.0);

  if (ball->aspect > 1)
    glOrtho(-ball->aspect, ball->aspect, -1.0, 1.0, -1.0, 1.0);
  else
    glOrtho(-1.0, 1.0, -1.0 / ball->aspect, 1.0 / ball->aspect, -1.0, 1.0);


  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glScalef(r, r, r);

  RIMCOLOR();

  if (drawBall) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 36; i++)
      glVertex3f((float)cos(i * 2.0 * M_PI / 36.0), (float)sin(i * 2.0 * M_PI / 36.0), 0.0);
    glEnd();
  }

  if (drawArc)
    Ball_DrawResultArc(ball);
  if (drawConstraints)
    Ball_DrawConstraints(ball);
  if (drawArc)
    Ball_DrawDragArc(ball);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glPopAttrib();
}

/* Draw an arc defined by its ends. */
void DrawAnyArc(HVect vFrom, HVect vTo)
{
  int i;
  HVect pts[NSEGS + 1];
  double dot;
  pts[0] = vFrom;
  pts[1] = pts[NSEGS] = vTo;
  for (i = 0; i < LG_NSEGS; i++)
    pts[1] = V3_Bisect(pts[0], pts[1]);
  dot = 2.0 * V3_Dot(pts[0], pts[1]);
  for (i = 2; i < NSEGS; i++) {
    pts[i] = V3_Sub(V3_Scale(pts[i - 1], dot), pts[i - 2]);
  }
  glBegin(GL_LINE_STRIP);
  for (i = 0; i <= NSEGS; i++)
    glVertex3fv((float *)&pts[i]);
  glEnd();
}

/* Draw the arc of a semi-circle defined by its axis. */
void DrawHalfArc(HVect n)
{
  HVect p, m;
  p.z = 0;
  if (n.z != 1.0) {
    p.x = n.y;
    p.y = -n.x;
    p = V3_Unit(p);
  }
  else {
    p.x = 0;
    p.y = 1;
  }
  m = V3_Cross(p, n);
  DrawAnyArc(p, m);
  DrawAnyArc(m, V3_Negate(p));
}

/* Draw all constraint arcs. */
void Ball_DrawConstraints(BallData * ball)
{
  ConstraintSet set;
  HVect axis;
  int i, axisI, setSize = ball->setSizes[ball->axisSet];
  if (ball->axisSet == NoAxes)
    return;
  set = ball->sets[ball->axisSet];
  for (axisI = 0; axisI < setSize; axisI++) {
    if (ball->axisIndex != axisI) {
      if (ball->dragging)
        continue;
      FARCOLOR();
    }
    else
      NEARCOLOR();
    axis = *(HVect *) & set[4 * axisI];
    if (axis.z == 1.0) {
      glBegin(GL_LINE_LOOP);
      for (i = 0; i < 36; i++)
        glVertex3f((float)cos(i * 2.0 * M_PI / 36.0), (float)sin(i * M_PI / 36.0), 0.0);
      glEnd();
    }
    else {
      DrawHalfArc(axis);
    }
  }
}

/* Draw "rubber band" arc during dragging. */
void Ball_DrawDragArc(BallData * ball)
{
  DRAGCOLOR();
  if (ball->dragging)
    DrawAnyArc(ball->vFrom, ball->vTo);
}

/* Draw arc for result of all drags. */
void Ball_DrawResultArc(BallData * ball)
{
  RESCOLOR();
  if (ball->showResult)
    DrawAnyArc(ball->vrFrom, ball->vrTo);
}
