/*** 
   Filename: textfilesubs.c
   Author: Rob MacLeod

      Routines that help manage text files of various formats.
      these should be general purpose routines that we can use all over 
      the place.

   Last update: Mon May 25 23:03:04 1998 by Rob MacLeod
     - created

***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "cutil.h"
/*======================================================================*/

char *ReadComments( FILE *luin, char astring[] )
{
 /*** Read the open file looking for comments and return the first 
      line after the comments are done.
      Comments begin with %,#, or !.
 ***/
    char instring[200], *string_p;
/**********************************************************************/
    do
    {
	string_p = fgets(instring, 199, luin);
    } while ( string_p != NULL && QComment( instring ) );
    if ( string_p != NULL ) 
    {
	strcpy(astring, instring);
	string_p = astring;
    }
    return string_p;
}

/*======================================================================*/
Boolean QComment( char *astring )
{
 /*** Test a string and see if it is a comment or blank. ***/

    if ( strchr( astring, '#' ) != NULL ||
	strchr( astring, '%' ) != NULL ||
	strchr( astring, '!' ) != NULL ||
	astring == " " ||
	strlen(astring) <= 0 ) return TRUE;
    return FALSE;
}

