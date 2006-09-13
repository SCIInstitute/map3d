/* Transforms.h */

#ifndef TRANSFORMS_H
#define TRANSFORMS_H

#include "Ball.h"

class Transforms
{
public:
  Transforms();
  void reset();
  void copy(Transforms* tran);
  void setRotationQuaternion(const Quat &q);
  BallData rotate;
  float tx, ty, tz;
  float itx, ity, itz;
};

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef __cplusplus
}
#endif

#endif
