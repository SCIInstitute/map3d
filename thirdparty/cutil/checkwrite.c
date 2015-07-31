 /*** 
   Filename: checkwritw.c
   Author: Rob MacLeod

   A routine to check if we can write in the directory supplied as argument.

   ***/
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#define W_OK 2
#define R_OK 4
#define X_OK 0
#endif
#include "cutil.h"
Boolean CheckWrite( char *dirname )
{
 /*** Input:
   dirname	name of the directory to check the write access to
                User gets to update this value if it is locked
  
   Returns:

   Boolean = true if we can write here.
   

 ***/
    long error;
    int amode;
    char instring[100];
/*********************************************************************/

    amode = W_OK + R_OK + X_OK;
    error = access( dirname, amode ) ;
 /*
    printf(" Return from access to %s with amode = %d\n"
	   " is W_OK = %d, R_OK = %d X_OK = %d\n"
	   " error = %d\n",
	   dirname, amode, access( dirname, W_OK ), 
	   access( dirname, R_OK ), access( dirname, X_OK ),
	   error);
 */
    while ( error != 0 )
    {
	printf("*** We cannot write in the directory %s\n"
	       "***  Either enter a new directory name ...\n"
	       "***  or hit return to give up writing in this directory ? ",
	       dirname );
	gets( instring );
	if ( Trulen( instring ) > 1 ) 
	{
	    strcpy( dirname, instring );
	    error = access( dirname, amode );
	} else
	{
	    printf("\n");
	    return( FALSE );
	}
    }
    return( TRUE );
}
