#include <math.h>
#include "cutil.h"
/*======================================================================*/
double Dist3d( float node1[3], float node2[3] )
{
    return sqrt( ((double) node1[X] - (double) node2[X]) * 
		 ((double) node1[X] - (double) node2[X]) + 
		 ((double) node1[Y] - (double) node2[Y]) * 
		 ((double) node1[Y] - (double) node2[Y]) + 
		 ((double) node1[Z] - (double) node2[Z]) * 
		 ((double) node1[Z] - (double) node2[Z]) );
}

