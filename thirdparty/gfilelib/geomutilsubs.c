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

    xprod[X] = vec1[Y] * vec2[Z] - vec1[Z] * vec2[Y];
    xprod[Y] = vec1[Z] * vec2[X] - vec1[X] * vec2[Z];
    xprod[Z] = vec1[X] * vec2[Y] - vec1[Y] * vec2[X];
    return( xprod );
}

/*====================================================================*/
float DotProd3( float *vec1, float *vec2 )
{

 /*** Compute a dot product of two vectors and return the product. ***/

    float dprod;
/**********************************************************************/

    dprod = vec1[X] * vec2[X] + vec1[Y] * vec2[Y] + vec1[Z] * vec2[Z];
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

    xprod[X] = vec1[Y] * vec2[Z] - vec1[Z] * vec2[Y];
    xprod[Y] = vec1[Z] * vec2[X] - vec1[X] * vec2[Z];
    xprod[Z] = vec1[X] * vec2[Y] - vec1[Y] * vec2[X];

    triarea = sqrt( xprod[X]*xprod[X] + xprod[Y]*xprod[Y] + 
		    xprod[Z]*xprod[Z] );
    if ( triarea < 1.E-10 )
    {
	ReportError("CalcTriNormal", "area value is too small", 0, "");
	printf("Points are:\n 1     2     3\n");
	for(i=0; i<3; i++)
	{
	    printf("  %7.2f %7.2f %7.2f \n", pt1[i], pt2[i], pt3[i] );
	}
	printf(" And cross product is %7.2f %7.2f %7.2f\n",
	       xprod[X], xprod[Y], xprod[Z] );
	return( 0.0 );
    }
    normcosines[X] = (float) (xprod[X] / triarea);
    normcosines[Y] = (float) (xprod[Y] / triarea);
    normcosines[Z] = (float) (xprod[Z] / triarea);
    triarea /= 2.;
    return( (float) triarea);
}

