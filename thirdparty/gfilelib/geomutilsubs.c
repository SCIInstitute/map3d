 /*** Some utilty functions for nodes, vectors, and triangles. 
   Last update: Sun May 25 21:20:17 1997
     - created
 ***/
#include <math.h>
#include <stdlib.h>
#include "geomutilsubs.h"
#include "geomlib.h"
#include "cutil.h"
/*====================================================================*/
double Dist3D( float node1[3], float node2[3] )
{
    double distance;
    double x = node1[0]-node2[0];
    double y = node1[1]-node2[1];
    double z = node1[2]-node2[2];
    double sum = x*x+y*y+z*z;
    if (sum<0) sum=0;
    distance = sqrt(sum);
    return( distance );
}

/*====================================================================*/
float *CrossProd( float *vec1, float *vec2 )
{

 /*** Compute a cross product of two vectors. ***/

    float* xprod = (float*)malloc(sizeof(float)*3);
    //    float xprod[3];
/**********************************************************************/

    xprod[CUTIL_X] = vec1[CUTIL_Y] * vec2[CUTIL_Z] - vec1[CUTIL_Z] * vec2[CUTIL_Y];
    xprod[CUTIL_Y] = vec1[CUTIL_Z] * vec2[CUTIL_X] - vec1[CUTIL_X] * vec2[CUTIL_Z];
    xprod[CUTIL_Z] = vec1[CUTIL_X] * vec2[CUTIL_Y] - vec1[CUTIL_Y] * vec2[CUTIL_X];
    return( xprod );
}

/*====================================================================*/
float DotProd3( float *vec1, float *vec2 )
{

 /*** Compute a dot product of two vectors and return the product. ***/

    float dprod;
/**********************************************************************/

    dprod = vec1[CUTIL_X] * vec2[CUTIL_X] + vec1[CUTIL_Y] * vec2[CUTIL_Y] + vec1[CUTIL_Z] * vec2[CUTIL_Z];
    return( dprod );
}

/*====================================================================*/
float CalcTriNormal (float *pt1, float *pt2, float *pt3, 
		     float *normcosines )
{

 /*** Compute the normal cosines and the area of a triangle by taking the
   cross product of two of its sides.  The order assumed is counterclockwise 
   viewed from the outside.  So the cross product is between the vectors
   pt3 - pt2 and pt1 - pt2.

   Return the value of the area as a float.  
   ***/

    double vec1[3], vec2[3], xprod[3];
    double triarea;
    long i;
/**********************************************************************/

 /*** Make the vectors and compute the cross product. ***/

    for (i=0; i<3; i++)
    {
	vec1[i] = pt3[i] - pt2[i];
	vec2[i] = pt1[i] - pt2[i];
    }

    xprod[CUTIL_X] = vec1[CUTIL_Y] * vec2[CUTIL_Z] - vec1[CUTIL_Z] * vec2[CUTIL_Y];
    xprod[CUTIL_Y] = vec1[CUTIL_Z] * vec2[CUTIL_X] - vec1[CUTIL_X] * vec2[CUTIL_Z];
    xprod[CUTIL_Z] = vec1[CUTIL_X] * vec2[CUTIL_Y] - vec1[CUTIL_Y] * vec2[CUTIL_X];

    triarea = sqrt( xprod[CUTIL_X]*xprod[CUTIL_X] + xprod[CUTIL_Y]*xprod[CUTIL_Y] + 
		    xprod[CUTIL_Z]*xprod[CUTIL_Z] );
    if ( triarea < 1.E-10 )
    {
	ReportError("CalcTriNormal", "area value is too small", 0, "");
	printf("Points are:\n 1     2     3\n");
	for(i=0; i<3; i++)
	{
	    printf("  %7.2f %7.2f %7.2f \n", pt1[i], pt2[i], pt3[i] );
	}
	printf(" And cross product is %7.2f %7.2f %7.2f\n",
	       xprod[CUTIL_X], xprod[CUTIL_Y], xprod[CUTIL_Z] );
	return( 0.0 );
    }
    normcosines[CUTIL_X] = (float) (xprod[CUTIL_X] / triarea);
    normcosines[CUTIL_Y] = (float) (xprod[CUTIL_Y] / triarea);
    normcosines[CUTIL_Z] = (float) (xprod[CUTIL_Z] / triarea);
    triarea /= 2.;
    return( (float) triarea);
}

