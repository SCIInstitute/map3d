/*** 
  Filename: getfilelength.c
  Author: Rob MacLeod
   Last update: Sat Nov  7 19:07:58 MST 1992
     - created

***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cutil.h"

long GetFileLength ( FILE *filePtr )
{
 /*** 
   A routine to find out how many lines an ASCII file has.

   Input:
    *filePtr      pointer to the file that is assumed to be open.

   Returns
                  number of lines in the file
                  negative value in case of error.
 ***/

    long i, nlastblanks;
    char instring[100];
/************************************************************************/
    if ( fgets( instring, 99, filePtr ) == NULL )
    {
	fprintf(stderr,"**** GetFileLength Error: end of file hit "
		" at start of file"
		" ***\n");
	exit(-1);
    }
    nlastblanks = 0;
    i = 1;
    while ( fgets( instring, 100, filePtr ) != NULL )
    {
	if ( instring[0] == '\n' ) 
	{
	    nlastblanks++;
	} else
	{
	    nlastblanks = 0;
	}
	i++;
    }
    return i-nlastblanks;
}
