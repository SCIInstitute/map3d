 /*** 
   Filename: findnoderange.c
   Author: Rob MacLeod
 ***/
/****************** Includes *****************************/
#include "geomlib.h"
#include "cutil.h"
/****************** Externals *****************************/
/****************** Prototypes ****************************/
/***************** The Code *******************************/
/*======================================================================*/

void FindNodeRange( float **nodes, long numpts, 
		float maxpoint[3], float minpoint[3] )
{
 /*** Find the range of values in the nodes array. ***/

    long i;
    long pnum;
/**********************************************************************/

    for ( i=0; i<3; i++ )
    {
	maxpoint[i] = -1.E10;
	minpoint[i] = 1.E10;
    }
    for ( pnum=0; pnum< numpts; pnum++ )
    {

	for ( i=0; i<3; i++ )
	{
	    maxpoint[i] = MaxFloat( maxpoint[i], nodes[pnum][i] );
	    minpoint[i] = MinFloat( minpoint[i], nodes[pnum][i] );
	}
    }
}

