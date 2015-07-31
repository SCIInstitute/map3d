#include <math.h>
#include "cutil.h"
/*======================================================================*/
double NodeDistance( Node node1, Node node2 )
{
    return sqrt( ((double) node1.x - (double) node2.x) * 
		 ((double) node1.x - (double) node2.x) + 
		 ((double) node1.y - (double) node2.y) * 
		 ((double) node1.y - (double) node2.y) + 
		 ((double) node1.z - (double) node2.z) * 
		 ((double) node1.z - (double) node2.z) );
}

