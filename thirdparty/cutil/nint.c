/*** Nint a function to return the nearest integer to a supplied
 double real value. Uses the "ceiling" amd "floor" functions to 
 find which integer value is closest to the real supplied.

 Input	    
  val	    double value which for which nearest integer is found.

 Returns
  Nint	    nearest integer value to "val".

 Last update: Sun Jul 28 15:12:00 MDT 1991 

***/

#include <math.h>

long Nint( double val )
{
    long nintval;
    
    nintval = (long) ( val - floor( val ) < ceil( val ) - val ?
		      floor( val ) : ceil( val ) );
    return ( nintval );
}




