#include <math.h>
#include "cutil.h"
/*======================================================================*/
double Dist3d( float node1[3], float node2[3] )
{
    return sqrt( ((double) node1[CUTIL_X] - (double) node2[CUTIL_X]) * 
		 ((double) node1[CUTIL_X] - (double) node2[CUTIL_X]) + 
		 ((double) node1[CUTIL_Y] - (double) node2[CUTIL_Y]) * 
		 ((double) node1[CUTIL_Y] - (double) node2[CUTIL_Y]) + 
		 ((double) node1[CUTIL_Z] - (double) node2[CUTIL_Z]) * 
		 ((double) node1[CUTIL_Z] - (double) node2[CUTIL_Z]) );
}

