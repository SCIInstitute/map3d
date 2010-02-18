/***
  Filename: readint.c
  Author: Rob MacLeod

  A routine to read an integer value and return the user's input
  or a default value supplied to the routine.

  Usage:
     intval = ReadInt(defaultval);

***/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "cutil.h"

int ReadInt( const int defaultval )
{
    char string[80];
    char *cerror_p;
/*********************************************************************/
    cerror_p = fgets(string, 79, stdin);
    if ( cerror_p == NULL) {
	fprintf(stderr," (readint.c)\n Error reading string is %d\n",
		cerror_p);
	return 0;
    }
    if ( strlen(string) > 0 )
    {
	return atoi(string);
    } else
    {
	return defaultval;
    }
}
    
