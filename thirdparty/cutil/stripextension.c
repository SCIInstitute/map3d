/*** StripExtension - a routine to strip away the extension from the
   end of a filename. 

   Input
    filename	the filename to work on.
   Returns
    filename    with the extension removed.

***/
#include <stdio.h>
#include <string.h>

void StripExtension (char *filename )

{
    char *extension;

/********************************************************************/

  /*** Find the last occurance of "." in the filename and assume that 
   everything to the right of it is to be handled as an extension.
   Then set the spot in filename where the "." is to \0. Simple, right?
  ***/

    if( (extension = strrchr( filename, '.' )) != NULL )
       *extension = '\0';

    return;
}

