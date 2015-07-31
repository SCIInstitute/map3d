/* geomutilsubs.h */

#ifndef GEOM_UTILSUBS_H
#define GEOM_UTILSUBS_H

#include "geomlib.h"
#ifdef __cplusplus
extern "C" {
#endif

GFILESHARE double Dist3D( float node1[3], float node2[3] );
GFILESHARE float *CrossProd( float *vec1, float *vec2 );
GFILESHARE float DotProd3( float *vec1, float *vec2 );
GFILESHARE float CalcTriNormal (float *pt1, float *pt2, float *pt3, 
		     float *normcosines );

#ifdef __cplusplus
}
#endif

#endif
