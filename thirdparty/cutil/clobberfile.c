/*** 
  Filename: clobberfile.c
  Author: Rob MacLeod

  A routine to check for the presence of a file and offer the user the
  chance to nuke it if it is present.

   Last update: Mon Jun 27 08:23:14 MDT 1994
     - cleaned up a little and moved from gl/cj to cutil.
 ***/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cutil.h"

int ClobberFile (char *filename)
{

/***
  Input: 
    filename 	filename to be checked -- may be updated here!!!
  Returns:
    0   for no error and no changes to filename
    <0  for error so do not write to this file
    >0  change in the filename made here, but file clobbered
***/
    int qexist, error;
    char string[80];
    FILE *file_p;
/*********************************************************************/

    error = 0;

 /*** See if the file exists and if so offer the user the chance to 
      edit the name or blow the existing file away. ***/

    qexist = FALSE;
    if ( ( file_p = fopen( filename, "r" ) ) != NULL ) 
    {
	qexist = TRUE;
	fclose (file_p);
    }
    while (qexist)
    {
	fprintf(stderr,"\n File %s\n exists or cannot be opened\n"
	       " To enter new filename ...................... n\n"
	       " To exit here and leave things untouched .... e\n"
	       " or clobber the old version ............. return ? ",
	       filename);
	fgets(string, 79, stdin); 

 /*** Finally back to the action - test the input. ***/

	if ( strchr(string,'n') != NULL )
	{
	    ReadFilename (" new filename ", filename);
	    error = 1;
	    if ( ( file_p = fopen( filename, "r" ) ) != NULL ) 
	    {
		qexist = TRUE;
		fclose (file_p);
	    }
	    else
		qexist = FALSE;
	} else if ( strchr(string, 'e') != NULL )
	{
	    error = -99;
	    return (error);
	} else
	{
#ifdef _WIN32
    sprintf (string, "del %s\n", filename);
#else
    sprintf (string, "rm %s\n", filename);
#endif
	    error = system (string);
	    if (error != 0) 
	    {
		fprintf (stderr," (ClobberFile)\n"
			 " Error nuking file is %d\n",
			 error);
		return(error);
	    } else
		fprintf(stderr, " File %s was deleted\n\n", filename);
	    qexist = FALSE;
	}
    }
    return (error);
}
