/*** 
  File: trulen.c
  Author: Rob MacLeod

   A function to return the true length of a string, that is, the number
   of characters up until the last whitespace.
 ***/
#include <string.h>
#include <stdio.h>
#include <ctype.h>

int Trulen (const char *string)
{
    int len;
    len = (int) strlen(string);
/*    printf("String is checked by strlen to be %d\n", len); */
    len--;
/*    printf("Character #%d is = %d\n", len, string[len]); */
    while ( isspace(string[len]) )
    {
/*
	printf("Character #%d is space\n", len);
	printf("Character #%d is = %d\n", len, string[len]);
*/
	len--;
    }
/*    printf("String is %s and length is %d\n", string, len+1); */
    return (len+1);
}
