#include <math.h>
#include "cutil.h"
/*======================================================================*/
double PointDistance( float *node1, float *node2 )
{
    return sqrt( ((double) node1[CUTIL_X] - (double) node2[CUTIL_X]) * 
		 ((double) node1[CUTIL_X] - (double) node2[CUTIL_X]) + 
		 ((double) node1[CUTIL_Y] - (double) node2[CUTIL_Y]) * 
		 ((double) node1[CUTIL_Y] - (double) node2[CUTIL_Y]) + 
		 ((double) node1[CUTIL_Z] - (double) node2[CUTIL_Z]) * 
		 ((double) node1[CUTIL_Z] - (double) node2[CUTIL_Z]) );
}
