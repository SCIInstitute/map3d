/*** 
  Filename: killfile.c
  Author: Rob MacLeod

  A routine to check for the presence of a file and nuke it if it is present.

   Last update: Thu May 27 15:58:25 2004 by Rob Macleod
     - altered document line to reflect actual function.
   Last update: Mon Jun 27 08:23:14 MDT 1994
     - cleaned up a little and moved from gl/cj to cutil.
 ***/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "cutil.h"

long KillFile (char *filename)
{

/***
  Input: 
    filename 	filename to be checked 
  Returns:
    0   for now error and no changes to filename
    <0  for error so do not write to this file
    >0  change in the filename made here, but file killed
***/
    int error;
    char string[80];
    FILE *file_p=NULL;
/*********************************************************************/

    error = 0;

 /*** See if the file exists and if so blow the existing file away. ***/

    if ( ( file_p = fopen( filename, "r" ) ) != NULL ) 
    {
	fclose (file_p);
        error = unlink(filename);
/*#ifdef _WIN32
  sprintf (string, "del %s\n", filename);
#else
	sprintf (string, "/bin/rm %s > /dev/null\n", filename);
#endif
	error = system (string);
        */
	if (error != 0) 
	{
	    fprintf (stderr," (KillFile)\n"
		     " Error nuking file is %d\n",
		     error);
	    return(error);
	} else {
	    printf(" Finished killing file %s\n", filename );
	}
    }
    return (error);
}
