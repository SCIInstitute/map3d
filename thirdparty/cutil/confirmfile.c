 /*** 
   Filename: confirmfile.c

   Author: Rob MacLeod

   Last update: Sun Apr 20 10:02:27 1997
     - created
   
 ***/

/****************** Includes *****************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cutil.h"
/***************** The Code *******************************/

/*======================================================================*/
long ConfirmFile( char *filename, char *prompt )
{
 /*** See if the file exists and if not, prompt the user for some input.   ***/

    while (! CheckExist( filename )) {
	printf(" The files %s does not exist\n"
	       " enter q to quit\n", filename);
	ReadFilename( prompt, filename );
	if ( filename[0] == 'q' )
	    return(ERR_MISC);
    }
    return( 0 );
}
