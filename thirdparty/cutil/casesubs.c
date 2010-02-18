#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include "cutil.h"
/***
   Last update: Sat Apr  9 18:24:42 MDT 1994
     - moved from gl/cj to here for central access.
***/
/*=================================================================*/
char *Lowercase( char *string )
{
    long i;
    for( i=0; i<strlen( string ); i++ )
	if ( isalpha( string[i] ) )
	    string[i] = tolower( string[i] );

    return string;
}
/*------------------------------------------------------------------*/

/*** 
     Uppercase

  Convert a string to upper case. ***/

/*=================================================================*/

char *Uppercase( char *string )

{
    long i;
    for( i=0; i<strlen( string ); i++ )
	if ( isalpha( string[i] ) )
	    string[i] = toupper( string[i] );

    return string;
}
/*------------------------------------------------------------------*/

/*** 
     IsBlank

  Retuns Boolean if line is blank, at least in the sense of not containing
  any characters, which for this purpose is as good as blank. ***/

/*=================================================================*/

 Boolean IsBlank( char *string )

{
    long i;
     Boolean qtestit;
    qtestit = FALSE;
    for( i=0; i<strlen( string ); i++ )
	if ( isalpha( string[i] ) )
	    qtestit = TRUE;

    return !qtestit;
}
/*------------------------------------------------------------------*/

