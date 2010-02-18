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

char* ReadString( const char* defaultval )
{
    char *string;
    char *cerror_p;
/*********************************************************************/
    string = (char *) calloc((size_t) 256, sizeof(char));
    if ( string == NULL ) {
	ReportError("ReadString", "error getting memory", NULL, "");
	return NULL;
    }
    cerror_p = fgets(string, 255, stdin);
    if ( cerror_p == NULL) {
	fprintf(stderr," (readint.c)\n Error reading string is %d\n",
		cerror_p);
	return 0;
    }
    if ( strlen(string) > 0 )
    {
	return string;
    } else
    {
	strcpy(string, defaultval);
	return string;
    }
}
    
