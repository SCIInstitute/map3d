/***** Ball.h *****/
#ifndef _H_Ball
#define _H_Ball
#include "BallAux.h"

typedef enum AxisSet
{ NoAxes, CameraAxes, BodyAxes, OtherAxes, NSets }
AxisSet;
typedef enum BallType
{ ArcBall, TrackBall }
BallType;
typedef float *ConstraintSet;
struct BallData
{
  HVect center;
  double radius, aspect;
  Quat qNow, qDown, qDrag;
  HVect vNow, vDown, vFrom, vTo, vrFrom, vrTo;
  HMatrix mNow, mDown;
  int showResult, dragging;
  ConstraintSet sets[NSets];
  int setSizes[NSets];
  AxisSet axisSet;
  int axisIndex;
  BallType ballType;
};

/* Public routines */
#ifdef __cplusplus
extern "C"
{
#endif

  BallData *Ball_Alloc();
  void Ball_Free(BallData *);
  void Ball_Init(BallData * ball, double aspect);
  void Ball_Place(BallData * ball, HVect center, double radius
                  /*, Quat orient */ );
  void Ball_Mouse(BallData * ball, HVect vNow);
  void Ball_UseSet(BallData * ball, AxisSet axisSet);
  void Ball_ShowResult(BallData * ball);
  void Ball_HideResult(BallData * ball);
  void Ball_Update(BallData * ball, int al);
  void Ball_Value(BallData * ball, HMatrix mNow);
  void Ball_BeginDrag(BallData * ball);
  void Ball_EndDrag(BallData * ball);
  void Ball_Draw(BallData * ball, int drawBall, int drawConstraints, int drawArc);
  int Ball_MouseOutsideSphere(BallData * ball, HVect vMouse);
  void Ball_SetBallType(BallData * ball, BallType bt);
  /* Private routines */
  void DrawAnyArc(HVect vFrom, HVect vTo);
  void DrawHalfArc(HVect n);
  void Ball_DrawConstraints(BallData * ball);
  void Ball_DrawDragArc(BallData * ball);
  void Ball_DrawResultArc(BallData * ball);

#ifdef __cplusplus
}
#endif

#endif
