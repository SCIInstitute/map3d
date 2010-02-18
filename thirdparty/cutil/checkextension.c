 /*** 
   Filename: checkextension.c
   Author: Rob MacLeod

   A routine to check the extension of the file and add it if none
   is present.
   Based on a Fortran routine of the same name.
   Last update: Mon Aug 22 13:01:54 MDT 1994
     - created

 ***/

#include <stdio.h>
#include <string.h>
long CheckExtension( char *filename, const char *extension )
{

 /*** Input:
   filename    	filename to be checked.  Updated here.
   extension	extension to add to the file is it does not have one.
                NOTE: do not include "." in the extension !!!
      Returns:
                error value -- not currently used for much
 ***/
    char *char_p;
/*********************************************************************/
 /*** First set a point to the last character in the filename, or
      the last "." separating the extension.
  ***/

 /*** If there is no ". in the filename, add it and then add the
      supplied extension. 
 ***/
    char_p = strrchr( filename, '.' );
    if ( char_p == NULL )
    {
	strcat( filename, ".");
	strcat( filename, extension );
	return 0;
    }

 /*** Or else if there is a "." but it is the last character,
      add the extension. 
 ***/
    else if ( char_p+1 == '\0' )
    {
	strcat( filename, extension );
	return 0;
    }
    return 0;
}
