/* Transforms.cxx */

#include "Transforms.h"

Transforms::Transforms()
{
  reset();
}

void Transforms::reset()
{
  Ball_Init(&rotate, 1);
  Ball_Place(&rotate, qOne, .8);
  tx = ty = tz = 0;
  itx = ity = itz = 0;
}

void Transforms::setRotationQuaternion(const Quat &q)
{
  rotate.qNow = q;
  Qt_ToMatrix(Qt_Conj(rotate.qNow), rotate.mNow);
  Ball_EndDrag(&rotate);
}

void Transforms::copy(Transforms* tran)
{
  tx = tran->tx;
  ty = tran->ty;
  tz = tran->tz;
  itx = tran->itx;
  ity = tran->ity;
  itz = tran->itz;
  setRotationQuaternion(tran->rotate.qNow);
}
