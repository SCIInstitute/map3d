#ifndef SHOEMAKE_ARCBALL_INCLUDED // -*- C++ -*-
#define SHOEMAKE_ARCBALL_INCLUDED

//
// This is the public header file which describes the interface to the
// Arcball manipulator
//

struct Quat
{
  float x, y, z, w;
};
typedef Quat HVect;
typedef float HMatrix[4][4];
extern Quat qOne;

typedef enum AxisSet
{ NoAxes, CameraAxes, BodyAxes, OtherAxes, NSets }
AxisSet;
typedef enum BallType
{ ArcBall, TrackBall }
BallType;
typedef float *ConstraintSet;
typedef struct BallData;

/* Public routines */
extern BallData *Ball_Alloc();
extern void Ball_Free(BallData *);
extern void Ball_Init(BallData * ball, double aspect);
extern void Ball_Place(BallData * ball, HVect center, double radius);
extern void Ball_Mouse(BallData * ball, HVect vNow);
extern void Ball_UseSet(BallData * ball, AxisSet axisSet);
extern void Ball_ShowResult(BallData * ball);
extern void Ball_HideResult(BallData * ball);
extern void Ball_Update(BallData * ball, int al);
extern void Ball_Value(BallData * ball, HMatrix mNow);
extern void Ball_BeginDrag(BallData * ball);
extern void Ball_EndDrag(BallData * ball);
extern void Ball_Draw(BallData * ball, int drawBall, int drawConstraints, int drawArc);
extern int Ball_MouseOutsideSphere(BallData * ball, HVect vMouse);
extern void Ball_SetBallType(BallData * ball, BallType bt);


// SHOEMAKE_ARCBALL_INCLUDED
#endif
