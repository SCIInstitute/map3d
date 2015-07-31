/***
  Filename: readfloat.c
  Author: Rob MacLeod

  A routine to read an float value and return the user's input
  or a default value supplied to the routine.

  Usage:
     floatval = ReadFloat(defaultval);

***/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

float ReadFloat( const float defaultval )
{
    char string[80];
    char *cerror_p;
    cerror_p = fgets(string, 79, stdin);
    if ( cerror_p == NULL) {
	fprintf(stderr," (readfloat.c)\n Error reading string is %d\n",
		cerror_p);
	return 0;
    }
    if ( strlen(string) > 0 )
    {
	return atof(string);
    } else
    {
	return defaultval;
    }
}
    
