#include <stdio.h>
#include <stdlib.h>
#include "cutil.h"
#include <string.h>

void ReportError(const char *function, const char *message, long returnvalue,
            const char *optionalstring)
{
 /*** Error reporting mechanism for any errors in the program code. ***/
    char output[256];

    strcpy(output, "\n*** "
           "Error in Function: ");
    strcat(output, function);
    strcat(output, "\n*** Message: %s\n");
    strcat(output, "*** Returning: %d\n");
    fprintf(stderr, output, message, returnvalue);
    if ( strlen ( optionalstring ) > 1 ) 
	fprintf(stderr,"*** %s\n", optionalstring);
    return;
}

