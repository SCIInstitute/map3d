#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cutil.h"
/*======================================================================*/

char *ReadLine(char *instring, FILE *luin_p )

/***  ReadLine

  A simple routine to read a line from the input file and make sure it 
  is not blank or starts with a comment character.
  Comment is assumed to be #.

 ***/
{
    int len;
    char *comment;
/**********************************************************************/
    len = 0;
    while( len < 1 )
    {

 /*** Read the line in. ***/

	if (fgets( instring, 255, luin_p ) == NULL )
	{
	    fprintf(stderr, " (ReadLine in readline.c\n"
		     " End of file hit\n");
	    return NULL;
	}
    
 /*** Get rid of everything after the comment character (#) ***/

	comment = strchr( instring, '#' );
	if ( comment != NULL )
	    strcpy( comment, " " );
	
 /*** Get rid of the line feed ***/

	comment = strchr( instring, '\n' );
	if ( comment != NULL )
	    strcpy( comment, " " );
	
 /*** See what we have left in the string. ***/

	len = Trulen( instring );
    }
    return instring;
}


