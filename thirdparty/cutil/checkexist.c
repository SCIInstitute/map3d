 /*** 
   Filename: checkexist.c
   Author: Rob MacLeod

   A routine to check the existance of a file.

   ***/
#include <stdio.h>
#include <string.h>
#include "cutil.h"
Boolean CheckExist( char *filename )
{
 /*** Input:
   filename	name of the file to check the existance of
  
   Returns:

   Boolean = true if file exists.

 ***/
    FILE *file_p;
/*********************************************************************/

 /*** See if the file is there by seeing if we can open it. ***/

    if ( ( file_p = fopen( filename, "r" ) ) != NULL ) 
    {
	fclose (file_p);
	return( TRUE );
    } else
	return( FALSE );
}
