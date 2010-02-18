#include "graphicsio.h"

extern char nothing[];

/****
 *
 *	setsurfaceindex_  specify surface for subsequent accesses
 *
 *	INPUTS:	thisFile    	    file info pointer
 *	    	theSurfaceIndex	    the desired surface
 * 
 *	OUTPUT:	result	    	    function result
 *
 ***/

long setsurfaceindex_(FileInfoPtr thisFile, long theSurfaceIndex)
{
    FilePtr	    	newSurfaceLocation, previousSurfaceLocation;
    FilePtr 	 	nextSurfaceLocation;
    long		bytesWritten, bytesRead;
    long	    	loop, tempNumberOfSurfaces;
    SurfaceInfoBlockPtr infoBlockPtr;

	    /* Dispose of the old surface temp info (if any) */
    if (thisFile->theCurrentSurface != NULL) free(thisFile->theCurrentSurface);

    if (theSurfaceIndex == (thisFile->lastSurfaceIndex) + 1)
    {	    	
		/* Set up a new Surface Header */
	thisFile->theCurrentSurface = (SurfaceHeaderPtr)
	    	    	    	    	calloc(1,sizeof(SurfaceHeader)); 
	if (thisFile->theCurrentSurface == 0)
	{
	    SCREAM("No memory available", "setsurfaceindex", noDynamicMemory,
		   "writing surface header");
	}
	(thisFile->theCurrentSurface)->headerSize = sizeof(SurfaceHeader);

		/* Write it to the end of the file */
	newSurfaceLocation = myLseek(thisFile->fileNumber, 0, SEEK_END); 
	(thisFile->theCurrentSurface)->surfaceInfoBlk = newSurfaceLocation +
	sizeof(SurfaceHeader);
#ifndef VAXVERSION
	convertLong((long*)&((thisFile->theCurrentSurface)->headerSize), 1);
	convertLong((long*)&((thisFile->theCurrentSurface)->surfaceInfoBlk),
		    1);
#endif

	bytesWritten = write(thisFile->fileNumber, 
			     thisFile->theCurrentSurface,
			     sizeof(SurfaceHeader));

	if (bytesWritten != sizeof(SurfaceHeader))
	{
	    SCREAM("Error writing surface header", "setsurfaceindex",
		   fileWriteError, nothing);
	}
#ifndef VAXVERSION
	convertLong((long*)&((thisFile->theCurrentSurface)->headerSize), 1);
	convertLong((long*)&((thisFile->theCurrentSurface)->surfaceInfoBlk),
		    1);
#endif
	infoBlockPtr = (SurfaceInfoBlockPtr)calloc(1, sizeof(SurfaceInfoBlock));
	if (infoBlockPtr == 0)
	{
	    SCREAM("No memory available", "setsurfaceindex", noDynamicMemory,
		   "writing surface info block");
	}
	infoBlockPtr->headerSize = sizeof(SurfaceInfoBlock);

#ifndef VAXVERSION
	convertLong((long*)&(infoBlockPtr->headerSize), 1);
#endif

	bytesWritten = write(thisFile->fileNumber, 
			     infoBlockPtr,
			     sizeof(SurfaceInfoBlock));

	if (bytesWritten != sizeof(SurfaceInfoBlock))
	{
	    SCREAM("Error writing surface info block", "setsurfaceindex",
		   fileWriteError, nothing);
	}
	free(infoBlockPtr);

		 /* Patch up the file links and information */

	thisFile->theCurrentSurfaceLocation = newSurfaceLocation;
	if (theSurfaceIndex == 1) 
	{
	/* This will be the first surface in the file */

	    	/* Fix up the information already in memory */
	    thisFile->thisFileHeader.numberOfSurfaces = 1; 
	    thisFile->thisFileHeader.firstSurfaceHeader =
	    	    	    		    newSurfaceLocation;
	    thisFile->lastSurfaceLocation = newSurfaceLocation;
	    thisFile->currentSurfaceIndex = 1;

	    	/* Fix up the information in the file */
	    myLseek(thisFile->fileNumber, NSOFFSET, SEEK_SET); 
	    tempNumberOfSurfaces = thisFile->thisFileHeader.
				   numberOfSurfaces;

#ifndef VAXVERSION
	    convertLong(&tempNumberOfSurfaces, 1);
#endif

	    	/* Set the number of Surfaces */
	    bytesWritten = write(thisFile->fileNumber, 
				 &tempNumberOfSurfaces, sizeof(long));
	    if (bytesWritten != sizeof(long))
	    {
		SCREAM("Error writing number of surfaces", "setsurfaceindex",
		       fileWriteError,
		       nothing);
	    }
#ifndef VAXVERSION
	    convertLong((long*)&newSurfaceLocation, 1);
#endif

	    	/* Point to this first Surface */
	    bytesWritten = write(thisFile->fileNumber, &newSurfaceLocation,
				 sizeof(FilePtr));

	    if (bytesWritten != sizeof(FilePtr))
	    {
	       SCREAM("Error writing new surface location", "setsurfaceindex",
		      fileWriteError,
		      nothing);
	    }
	    thisFile->lastSurfaceIndex = 1;

	    	/* Return the fact that we initialized surface structures */
	    return(firstSurface);
	}
	else /* We are adding an additional surface to the file */
	{	

	    	/* Fix up the information already in memory */
	    thisFile->thisFileHeader.numberOfSurfaces += 1;
	    previousSurfaceLocation = thisFile->lastSurfaceLocation;
	    thisFile->lastSurfaceLocation = newSurfaceLocation;
	    thisFile->currentSurfaceIndex = theSurfaceIndex;
	    thisFile->theCurrentSurfaceLocation = newSurfaceLocation;

	    	/* Fix up the information in the file */
	    myLseek(thisFile->fileNumber, NSOFFSET, SEEK_SET); 
	    tempNumberOfSurfaces = thisFile->thisFileHeader.
				   numberOfSurfaces;

#ifndef VAXVERSION
	    convertLong(&tempNumberOfSurfaces, 1);
#endif


	    	/* Set the number of Surfaces */
	    bytesWritten = write(thisFile->fileNumber,
				 &tempNumberOfSurfaces, sizeof(long));
	    if (bytesWritten != sizeof(long))
	    {
		SCREAM("Error writing number of surfaces", "setsurfaceindex",
		       fileWriteError,
		       nothing);
	    }
	    myLseek(thisFile->fileNumber, previousSurfaceLocation + NSHOFFSET,
		  SEEK_SET);

#ifndef VAXVERSION
	    convertLong((long*)&newSurfaceLocation, 1);
#endif

	    	/* Point the previous surface to this additional surface */
	    bytesWritten = write(thisFile->fileNumber, &newSurfaceLocation, 
				 sizeof(FilePtr));
	    if (bytesWritten != sizeof(FilePtr))
	    {
	       SCREAM("Error writing new surface location", "setsurfaceindex",
		      fileWriteError,
		      nothing);
	    }
	    thisFile->lastSurfaceIndex += 1;

	    return(newSurface);
	}
    }
    else if ((theSurfaceIndex > 0) && 
	     (theSurfaceIndex <= thisFile->lastSurfaceIndex))
    { 
	    /* We are selecting an existing Surface */
	thisFile->theCurrentSurface = (SurfaceHeaderPtr) malloc 
	    	    	    	    	(sizeof(SurfaceHeader));

	    /* Get the pointer to the first Surface Header */
	myLseek(thisFile->fileNumber, FSHOFFSET, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &nextSurfaceLocation,
			 sizeof(FilePtr));

	if (bytesRead != sizeof(FilePtr))
	{
	    SCREAM("Error reading next surface location", "setsurfaceindex",
		   fileReadError,
		   nothing);
	}
#ifndef VAXVERSION
	convertLong((long*)&nextSurfaceLocation, 1);
#endif

	    /* "Walk" through the surfaces to the specified surface */
	for (loop =1; loop < theSurfaceIndex; loop++)
	{
	    thisFile->theCurrentSurfaceLocation = myLseek(thisFile->fileNumber,
							nextSurfaceLocation +
							NSHOFFSET, SEEK_SET);
	    bytesRead = read(thisFile->fileNumber, &nextSurfaceLocation,
		      sizeof(FilePtr));

	    if (bytesRead != sizeof(FilePtr))
	    {
	      SCREAM("Error reading next surface location", "setsurfaceindex",
		     fileReadError,
		     nothing);
	    }
#ifndef VAXVERSION
	convertLong((long*)&nextSurfaceLocation, 1);
#endif
	}
	
	myLseek(thisFile->fileNumber, nextSurfaceLocation, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, thisFile->theCurrentSurface, 
	sizeof(SurfaceHeader));

	if (bytesRead != sizeof(SurfaceHeader))
	{
	    SCREAM("Error reading next surface header", "setsurfaceindex",
		   fileReadError,
		   nothing);
	}

#ifndef VAXVERSION
	convertLong((long*)thisFile->theCurrentSurface, 5);
#endif

	thisFile->currentSurfaceIndex = theSurfaceIndex;
	thisFile->theCurrentSurfaceLocation = nextSurfaceLocation;
	return(existingSurface);
    }
    else
    {
	thisFile->currentSurfaceIndex = 0;
	thisFile->theCurrentSurface = NULL;
	thisFile->theCurrentSurfaceLocation = 0;
	return(invalidSurface);
    }
}

/****
 *
 *	getsurfaceindex_ get the current surface index
 *
 *	INPUT:	thisFile    file info pointer
 *
 *	OUTPUTS:theSurface  the current surface index
 *	    	result	    function result
 *
 ***/

long getsurfaceindex_(FileInfoPtr thisFile, long* theSurface)
{
    *theSurface = thisFile->currentSurfaceIndex;
    return(success);
}

/****
 *
 *	setsurfacename_ set a name for the current surface
 *
 *	INPUTS:	thisFile    file info pointer
 *	    	theName	    name string (79 chars max + null = 80)
 *
 *	OUTPUT: result	    function result
 *
 ***/

long setsurfacename_(FileInfoPtr thisFile, char *theName)
{
    long    stringLength, bytesWritten;
    
    myLseek(thisFile->fileNumber,
		   (thisFile->theCurrentSurface)->surfaceInfoBlk + NMOFFSET,
		   SEEK_SET);
    stringLength = strlen(theName);
    if (stringLength > 80)
    {
	stringLength = 79;
	theName[80] = 0;
    }
    bytesWritten = write(thisFile->fileNumber, theName, stringLength + 1);
    if (bytesWritten != (stringLength + 1))
    {
	SCREAM("Error writing theName", "setsurfacename", fileWriteError,
	       nothing);
    }
    return(success);
}

			    
/****
 *
 *	getsurfacename_   get name of current surface
 *
 *	INPUT:	thisFile    file info pointer
 *
 *	OUTPUTS:theName	    surface name string (79 chars max + null = 80)
 *	    	result	    function result
 *
 ***/

long getsurfacename_(FileInfoPtr thisFile, char *theName)
{
    long    bytesRead;
    
    myLseek(thisFile->fileNumber,
		   (thisFile->theCurrentSurface)->surfaceInfoBlk + NMOFFSET,
		   SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theName, 80);
    if (bytesRead != 80)
    {
	SCREAM("Error reading theName", "getsurfacename", fileReadError,
	       nothing);
    }
    return(success);
}

/****
 *
 *	setsurfacetype_  set surface type
 *
 *	INPUTS:	thisFile    file info pointer
 *	    	theType surface type (see graphicsio.h for definitions)
 *
 *	OUTPUT:	result	    function result
 *
 ***/

long setsurfacetype_(FileInfoPtr thisFile, long theType)
{
    long bytesWritten;

    myLseek(thisFile->fileNumber,
		   (thisFile->theCurrentSurface)->surfaceInfoBlk + STOFFSET,
		   SEEK_SET);
    bytesWritten = write(thisFile->fileNumber, &theType, sizeof(long));
    if( bytesWritten != sizeof(long))
    {
	SCREAM("Error writing theType", "setsurfacetype", fileWriteError,
	       nothing);
    }
    return(success);
}

/****
 *
 *	getsurfacetype_  set the surface type
 *
 *	INPUT:	thisFile    file info pointer
 *
 *	OUTPUTS:theType	    the surface type (see graphicsio.h for definitions)
 *	    	result	    function result
 *
 ***/

long getsurfacetype_(FileInfoPtr thisFile, long *theType)
{
    long bytesRead;

    myLseek(thisFile->fileNumber,
		   (thisFile->theCurrentSurface)->surfaceInfoBlk + STOFFSET,
		   SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theType, sizeof(long));
    if( bytesRead != sizeof(long))
    {
	SCREAM("Error reading theType", "setsurfacetype", fileReadError,
	       nothing);
    }
    return(success);
}

