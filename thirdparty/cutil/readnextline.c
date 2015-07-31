#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cutil.h"
#define MAXLEN 4096
/*======================================================================*/

char *ReadNextLine( FILE *luin_p )

/***  ReadNextLine

  A simple routine to read a line from the input file and make sure it 
  is not blank of start with a comment character.
  We skip over blank lines.
  Comment is assumed to be #.
  Returns a pointer to the next line -- uses static memory allocated here!!

 ***/
{
    int len;
    char *comment, *instring;
    char stringline[MAXLEN];
/**********************************************************************/
    len = 0;
    while( len < 1 )
    {

 /*** Read the line in. ***/

	if (fgets( stringline, MAXLEN-1, luin_p ) == NULL )
	{
	    fprintf(stderr, " (ReadNextLine in readnextline.c)\n"
		     " End of file hit\n");
	    return NULL;
	}
    
 /*** Get rid of everything after the comment character (#) ***/

	comment = strchr( stringline, '#' );
	if ( comment != NULL )
	    strcpy( comment, " " );
	
 /*** See what we have left in the string. ***/

	len = Trulen( stringline );
    }

 /*** Now set aside enough memory for the line, copy the string, 
      and then pass a pointer to this memory back . ***/

 /*
    instring = (char *) calloc( (size_t) len+1, sizeof(char) );
    strncpy( instring, stringline, (size_t) len );
 */
    instring = (char *) stringline;
    strcat( instring, "");
    return instring;
}


