/* map3dmath.cxx */

#include <math.h>
#include "BallAux.h"

#ifdef _WIN32
#pragma warning(disable:4514)
#endif

//vector operations
float vectorLength(float a[])
{
  return (float)sqrt(pow(a[0], 2) + pow(a[1], 2) + pow(a[2], 2));
}

void normalizeVector(float a[])
{
  float length = (float)vectorLength(a);
  a[0] /= length;
  a[1] /= length;
  a[2] /= length;
}

void crossProduct(float a[], float b[], float c[])
{
  normalizeVector(a);
  normalizeVector(b);
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
  //normalize(c);
}

float dotProduct(float a[], float b[])
{
  normalizeVector(a);
  normalizeVector(b);
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void AxisAndAngleOfRotation(float orig[], float transformed[], float& angle, float axis[])
{
  crossProduct(orig, transformed, axis);
  //  get angles in positive degrees
  angle = (acos(dotProduct(orig, transformed)) * 180 / 3.1415926f);
  angle = (angle > 180 ? angle - 360 : angle);

  //acos gives you one of only two possible values.  
  //use sin of the length of the  
  //cross product to find the other one and to compensate.
  float checkangle = (asin(vectorLength(transformed)) * 180 / 3.1415926f + 360);
  checkangle = (checkangle > 360 ? checkangle - 360 : checkangle);
  float altangle = 360 - angle;
  float altcheckangle = (540 - checkangle);
  altcheckangle = (altcheckangle > 360 ? altcheckangle - 360 : altcheckangle);

  if ((fabs(altangle - checkangle) < .0001 || fabs(altangle - altcheckangle) < .0001))
    angle = altangle;
  
}


void clearMatrix16(float a[])
{
  for (int i = 0; i < 16; i++) {
    if (!(i % 5))
      a[i] = 1;
    else
      a[i] = 0;
  }
}

//Multiplies two matrices - a is a 4x4 and b & product are 4x1: product = a*b
//  order of elements in these matrices is column-major
void MultMatrix16x4(float *a, float *b, float *product)
{
  product[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
  product[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
  product[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
  product[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

}

//Multiplies two matrices - a is a 3x3 and b & product are 3x1: product = a*b
//  order of elements in these matrices is column-major
void MultMatrix9x3(float *a, float *b, float *product)
{
  product[0] = a[0] * b[0] + a[3] * b[1] + a[6] * b[2];
  product[1] = a[1] * b[0] + a[4] * b[1] + a[7] * b[2];
  product[2] = a[2] * b[0] + a[5] * b[1] + a[8] * b[2];
}

//Multiples two matrices - all are 4x4: product = a*b
//  order of elements in these matrices is column-major
void MultMatrix16x16(float *a, float *b, float *product)
{
  product[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
  product[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
  product[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
  product[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

  product[4] = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
  product[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
  product[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
  product[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];

  product[8] = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
  product[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
  product[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
  product[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];

  product[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
  product[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
  product[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
  product[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];

}

//Calculates the inverse of a 4X4 matrix
//  order of elements in these matrices is column-major
void InvertMatrix16(float *a, float *inverse)
{
  float tmp[16];
  int i, j, k;

  //assign the return value to the identity matrix and tmp to the original matrix
  for (i = 0; i < 16; i++) {
    tmp[i] = a[i];
    if (i % 5 == 0)
      inverse[i] = 1;
    else
      inverse[i] = 0;
  }
  for (i = 0; i < 4; i++) {
    if (tmp[i * 4 + i] != 1) {
      float db = tmp[i * 4 + i];
      for (j = 0; j < 4; j++) {
        tmp[j * 4 + i] /= db;   // switched
        inverse[j * 4 + i] /= db; // switched
      }
    }
    for (j = 0; j < 4; j++) {
      if (j != i)               // && tmp.data[j][i] != 0.0)
      {
        if (tmp[i * 4 + j] != 0.0)  // switched
        {
          float mb = tmp[i * 4 + j];  // switched
          for (k = 0; k < 4; k++) {
            tmp[k * 4 + j] -= mb * tmp[k * 4 + i];  // switched
            inverse[k * 4 + j] -= mb * inverse[k * 4 + i];  // switched
          }
        }
      }
    }
  }
}

void TransposeMatrix16(float *a, float *transpose)
{
  transpose[0] = a[0];
  transpose[1] = a[4];
  transpose[2] = a[8];
  transpose[3] = a[12];
  transpose[4] = a[1];
  transpose[5] = a[5];
  transpose[6] = a[9];
  transpose[7] = a[13];
  transpose[8] = a[2];
  transpose[9] = a[6];
  transpose[10] = a[10];
  transpose[11] = a[14];
  transpose[12] = a[3];
  transpose[13] = a[7];
  transpose[14] = a[11];
  transpose[15] = a[15];
}


void normalizeQuat(Quat * q)
{
  float len = (float)sqrt(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
  q->w = q->w / len;
  q->x = q->x / len;
  q->y = q->y / len;
  q->z = q->z / len;

}
