#include <math.h>
#include "cutil.h"
/*======================================================================*/
double PointDistance( float *node1, float *node2 )
{
    return sqrt( ((double) node1[X] - (double) node2[X]) * 
		 ((double) node1[X] - (double) node2[X]) + 
		 ((double) node1[Y] - (double) node2[Y]) * 
		 ((double) node1[Y] - (double) node2[Y]) + 
		 ((double) node1[Z] - (double) node2[Z]) * 
		 ((double) node1[Z] - (double) node2[Z]) );
}
