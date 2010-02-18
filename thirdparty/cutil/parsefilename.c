 /*** 
   Parsefilenane: break up a filename into directory, basefilename and 
   extension.

   Input:
     filename	path to parse
   Returns:
     dirname	    directory name
     basefilename   core of the filename 
     exteniosn	    file extension without "."
 ***/
#include <string.h>
#include <stdlib.h>
#include "cutil.h"
/*======================================================================*/
void ParseFilename (char *filename, char *dirname, 
		     char *basefilename, char *extension )

{
    char *char_p, *infilename;
/********************************************************************/

    if ( Trulen( filename ) < 1 ) 
    {
	dirname = NULL;
	basefilename = NULL;
	extension = NULL;
	return;
    }
    infilename = (char *) calloc( (size_t) Trulen(filename), sizeof(char));
    strcpy( infilename, filename );

 /*** See if we have an extension ***/

    if( (char_p = strrchr( infilename, '.' )) != NULL )
    {
	strcpy( extension, char_p );
	*char_p = '\0';
    } else
	extension = NULL;

    if( (char_p = strrchr( infilename, '/' )) != NULL )
    {
	strcpy( basefilename, (char_p+1) );
	*char_p = '\0';
    } else {
	dirname = NULL;
	strcpy( basefilename, infilename );
	return;
    }
    strcpy( dirname, infilename );
    return;
}
