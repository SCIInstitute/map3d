#include "graphicsio.h"

extern char nothing[];

/****
 *
 *      setleadfiducials
 *
 *      This routine writes a set of lead by lead fiducials to the file
 *
 *
 ***/

long setleadfiducials_(FileInfoPtr thisFile, char* theLabel,
		      short fidDescSize, short* fidDesc, float* theFids, short* theTypes)
{

    TimeSeriesHeader	theTimeSeriesHeader;
    long		bytesRead, bytesWritten, i, total, numberOfFiducialSets;
    LeadFiducialHeader	theFiducialHeader;
    FilePtr		theFidDescLocation, theFidsLocation;
    FilePtr		theFiducialHeaderLocation, location;
    FilePtr		theTypesLocation;
    Boolean		firstLeadFiducialSet;

    if ((thisFile->theCurrentTimeSeriesIndex) == 0)
    {
	SCREAM("Invalid Time Series", "setleadfiducials", invalidTimeSeries, nothing);
    }

    myLseek(thisFile->fileNumber, thisFile->theCurrentTimeSeriesLocation, SEEK_SET);

    bytesRead = read(thisFile->fileNumber, &theTimeSeriesHeader, sizeof(TimeSeriesHeader));
    if (bytesRead != sizeof(TimeSeriesHeader))
    {
	SCREAM("Error reading Time Series Header", "setleadfiducials", fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theTimeSeriesHeader.firstLeadFiducialHeader),2);
#endif

    numberOfFiducialSets = theTimeSeriesHeader.numberOfFiducialSets;

    for (i = 0, theFiducialHeader.numberOfFiducials = 0; i < fidDescSize; i++)
    {
	theFiducialHeader.numberOfFiducials += fidDesc[i];
    }

    theFiducialHeader.fidDescSize = fidDescSize;
    strcpy(theFiducialHeader.theLabel, theLabel);
    theFiducialHeader.headerSize = sizeof(LeadFiducialHeader);
    theFiducialHeader.nextLeadFiducialHeader = 0;

    if (numberOfFiducialSets == 0)
    {
	firstLeadFiducialSet = TRUE;
    }
    else
    {
	firstLeadFiducialSet = FALSE;
    }

    theFiducialHeaderLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
    
#ifndef VAXVERSION
    convertLong((long*)&theFiducialHeader, 7);
#endif
    bytesWritten = write(thisFile->fileNumber, &theFiducialHeader, sizeof(LeadFiducialHeader));
    if (bytesWritten != sizeof(LeadFiducialHeader))
    {
	SCREAM("Error writing Lead Fiducial Header", "setleadfiducials", fileWriteError, nothing);
    }
    
    theFidDescLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
    
#ifndef VAXVERSION
    convertShort((short*)fidDesc, fidDescSize);
#endif
    
    bytesWritten = write(thisFile->fileNumber, fidDesc, fidDescSize * sizeof(short));
    
#ifndef VAXVERSION
    convertShort((short*)fidDesc, fidDescSize); /* clean up the fidDesc array */
#endif
    
    if (bytesWritten != fidDescSize * sizeof(short))
    {
	SCREAM("Error writing Lead Fiducial Desc", "setleadfiducials", fileWriteError, nothing);
    }
    
    for (i = 0, total = 0; i < fidDescSize; i++) /* figure the total number of fiducials */
    {
	total += fidDesc[i];
    }
    
    theFidsLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
    
#ifndef VAXVERSION
    convertFloat((float*)theFids, total, FALSE);
#endif
    
    bytesWritten = write(thisFile->fileNumber, theFids, total * sizeof(float));
    
#ifndef VAXVERSION
    convertFloat((float*)theFids, total, TRUE); /* clean up the fiducial array */
#endif
    
    if (bytesWritten != total * sizeof(float))
    {
	SCREAM("Error writing Lead Fiducials", "setleadfiducials", fileWriteError, nothing);
    }
    theTypesLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
    
#ifndef VAXVERSION
    convertShort((short*)theTypes, total);
#endif
    
    bytesWritten = write(thisFile->fileNumber, theTypes, total * sizeof(short));
    
#ifndef VAXVERSION
    convertShort((short*)theTypes, total); /* clean up the types array */
#endif
    
    if (bytesWritten != total * sizeof(short))
    {
	SCREAM("Error writing Fiducial Types", "setleadfiducials", fileWriteError, nothing);
    }
    
    myLseek(thisFile->fileNumber, theFiducialHeaderLocation +  FIDDOFFSET, SEEK_SET);
    
    theFiducialHeader.fidDesc = theFidDescLocation;
    theFiducialHeader.theFids = theFidsLocation;
    theFiducialHeader.theTypes = theTypesLocation;

#ifndef VAXVERSION
    convertLong((long*)&(theFiducialHeader.fidDesc), 3);
#endif
    
    bytesWritten = write(thisFile->fileNumber, &(theFiducialHeader.fidDesc), 3 * sizeof(FilePtr));
    if (bytesWritten != 3 * sizeof(FilePtr))
    {
	SCREAM("Error updating Lead Fiducial Header", "setleadfiducials", fileWriteError, nothing);
    }
    
    numberOfFiducialSets += 1;
    theTimeSeriesHeader.numberOfFiducialSets = numberOfFiducialSets;
#ifndef VAXVERSION
	convertLong((long*)&theFiducialHeaderLocation, 1);
	convertLong((long*)&(theTimeSeriesHeader.numberOfFiducialSets), 1);
#endif
    if (firstLeadFiducialSet) /* Now go patch up firstLeadFiducialLocation and numberOfFiducialSets */
    {
	myLseek(thisFile->fileNumber, thisFile->theCurrentTimeSeriesLocation + FLFHOFFSET, SEEK_SET);
	bytesWritten = write(thisFile->fileNumber, &theFiducialHeaderLocation, sizeof(FilePtr));
	if (bytesWritten != sizeof(FilePtr))
	{
	    SCREAM("Error updating Lead Fiducial Header location", "setleadfiducials", fileWriteError, nothing);
	}
	write(thisFile->fileNumber, &(theTimeSeriesHeader.numberOfFiducialSets), sizeof(long));
	if (bytesWritten != sizeof(long))
	{
	    SCREAM("Error updating Number of Fiducial Sets", "setleadfiducials", fileWriteError, nothing);
	}
    }
    else /* or just numberOfFiducialSets */
    {
	myLseek(thisFile->fileNumber, thisFile->theCurrentTimeSeriesLocation + NOFSOFFSET, SEEK_SET);
	bytesWritten = write(thisFile->fileNumber, &(theTimeSeriesHeader.numberOfFiducialSets), sizeof(long));
	if (bytesWritten != sizeof(long))
	{
	    SCREAM("Error updating Number of Fiducial Sets", "setleadfiducials", fileWriteError, nothing);
	}

    }

    if (!firstLeadFiducialSet)
    {
	location = theTimeSeriesHeader.firstLeadFiducialHeader; /* Now go patch up the last fiducial
								   header to point to the new one */

	for (i = 0; i < numberOfFiducialSets; i++)
	{
	    myLseek(thisFile->fileNumber, location, SEEK_SET);
	    bytesRead = read(thisFile->fileNumber, &theFiducialHeader, sizeof(LeadFiducialHeader));
	    if (bytesRead != sizeof(LeadFiducialHeader))
	    {
		SCREAM("Error reading next fiducial header", "setleadfiducials",
		       fileReadError, nothing);
	    }

#ifndef VAXVERSION
	    convertLong((long*)&(theFiducialHeader.nextLeadFiducialHeader), 1);
#endif
	    if (theFiducialHeader.nextLeadFiducialHeader != 0)
	    {
		location = theFiducialHeader.nextLeadFiducialHeader;
	    }
	}

	myLseek(thisFile->fileNumber, location + NLFHOFFSET, SEEK_SET);


	bytesWritten = write(thisFile->fileNumber, &theFiducialHeaderLocation, sizeof(FilePtr));
	
	if (bytesWritten != sizeof(FilePtr))
	{
	    SCREAM("Error writing next fiducial header location", "setleadfiducials",
		   fileWriteError, nothing);
	}
    }
    return(success);

}
    
/****
 *
 *	getleadfiducialcount      
 *
 ***/

long getleadfiducialcount_(FileInfoPtr thisFile, long* howMany)
{
    TimeSeriesHeader	theTimeSeriesHeader;
    long		bytesRead;

    if ((thisFile->theCurrentTimeSeriesIndex) == 0)
    {
	SCREAM("Invalid Time Series", "getleadfiducialcount", invalidTimeSeries, nothing);
    }

    myLseek(thisFile->fileNumber, thisFile->theCurrentTimeSeriesLocation, SEEK_SET);

    bytesRead = read(thisFile->fileNumber, &theTimeSeriesHeader, sizeof(TimeSeriesHeader));
    if (bytesRead != sizeof(TimeSeriesHeader))
    {
	SCREAM("Error reading Time Series Header", "getleadfiducialcount", fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theTimeSeriesHeader.numberOfFiducialSets),1);
#endif
    
    *howMany = theTimeSeriesHeader.numberOfFiducialSets;
    return(success);
}

/****
 *
 *	getleadfiducialinfo
 *
 ***/

long getleadfiducialinfo_(FileInfoPtr thisFile, FidInfoPtr theInfo)
{

    TimeSeriesHeader	theTimeSeriesHeader;
    long		bytesRead, i, numberOfFiducialSets;
    FilePtr		location;
    LeadFiducialHeader	theFiducialHeader;

    if ((thisFile->theCurrentTimeSeriesIndex) == 0)
    {
	SCREAM("Invalid Time Series", "setleadfiducials", invalidTimeSeries, nothing);
    }

    myLseek(thisFile->fileNumber, thisFile->theCurrentTimeSeriesLocation, SEEK_SET);

    bytesRead = read(thisFile->fileNumber, &theTimeSeriesHeader, sizeof(TimeSeriesHeader));
    if (bytesRead != sizeof(TimeSeriesHeader))
    {
	SCREAM("Error reading Time Series Header", "getleadfiducialinfo", fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theTimeSeriesHeader.firstLeadFiducialHeader),2);
#endif

    numberOfFiducialSets = theTimeSeriesHeader.numberOfFiducialSets;
    location = theTimeSeriesHeader.firstLeadFiducialHeader;

    for (i = 0; i < numberOfFiducialSets; i++)
    {
	myLseek(thisFile->fileNumber, location, SEEK_SET);
        bytesRead = read(thisFile->fileNumber, &theFiducialHeader, sizeof(LeadFiducialHeader));
	if (bytesRead != sizeof(LeadFiducialHeader))
	{
	    SCREAM("Error reading next fiducial header", "getleadfiducialinfo",
		   fileReadError, nothing);
	}

#ifndef VAXVERSION
	convertLong(&(theFiducialHeader.headerSize), 7);
#endif

	location = theFiducialHeader.nextLeadFiducialHeader;
	theInfo[i].fidDescSize = theFiducialHeader.fidDescSize;
	theInfo[i].numberOfFiducials = theFiducialHeader.numberOfFiducials;
	strcpy(theInfo[i].theLabel, theFiducialHeader.theLabel);
    }
    return(success);
}

/****
 *
 *      getleadfiducials
 *
 ***/

long getleadfiducials_(FileInfoPtr thisFile, short index, short* fidDesc,
		       float* theFids, short* theTypes)
{
    LeadFiducialHeader	theFiducialHeader;
    TimeSeriesHeader	theTimeSeriesHeader;
    long		bytesRead, i;
    FilePtr		location;

    if ((thisFile->theCurrentTimeSeriesIndex) == 0)
    {
	SCREAM("Invalid Time Series", "getleadfiducials", invalidTimeSeries, nothing);
    }

    myLseek(thisFile->fileNumber, thisFile->theCurrentTimeSeriesLocation, SEEK_SET);

    bytesRead = read(thisFile->fileNumber, &theTimeSeriesHeader, sizeof(TimeSeriesHeader));
    if (bytesRead != sizeof(TimeSeriesHeader))
    {
	SCREAM("Error reading Time Series Header", "getleadfiducialinfo", fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theTimeSeriesHeader.firstLeadFiducialHeader),2);
#endif

    location = theTimeSeriesHeader.firstLeadFiducialHeader;

    for (i = 1; i <= index; i++)
    {
	myLseek(thisFile->fileNumber, location, SEEK_SET);
        bytesRead = read(thisFile->fileNumber, &theFiducialHeader, sizeof(LeadFiducialHeader));
	if (bytesRead != sizeof(LeadFiducialHeader))
	{
	    SCREAM("Error reading next fiducial header", "getleadfiducials",
		   fileReadError, nothing);
	}

#ifndef VAXVERSION
	convertLong(&(theFiducialHeader.headerSize), 7);
#endif
	location = theFiducialHeader.nextLeadFiducialHeader;
    }

    myLseek(thisFile->fileNumber, theFiducialHeader.fidDesc, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, fidDesc, theFiducialHeader.fidDescSize * sizeof(short));
    
    if (bytesRead != theFiducialHeader.fidDescSize * sizeof(short))
    {
	SCREAM("Error reading fiducial descriptor", "getleadfiducials",
	       fileReadError, nothing);
    }

#ifndef VAXVERSION
    convertShort(fidDesc, theFiducialHeader.fidDescSize);
#endif

    myLseek(thisFile->fileNumber, theFiducialHeader.theFids, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theFids, 
		     theFiducialHeader.numberOfFiducials * sizeof(float));
    
    if (bytesRead != theFiducialHeader.numberOfFiducials * sizeof(float))
    {
	SCREAM("Error reading fiducials", "getleadfiducials",
	       fileReadError, nothing);
    }

#ifndef VAXVERSION
    convertFloat(theFids, theFiducialHeader.numberOfFiducials, TRUE);
#endif


    myLseek(thisFile->fileNumber, theFiducialHeader.theTypes, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theTypes, 
		     theFiducialHeader.numberOfFiducials * sizeof(short));
    
    if (bytesRead != theFiducialHeader.numberOfFiducials * sizeof(short))
    {
	SCREAM("Error reading types", "getleadfiducials",
	       fileReadError, nothing);
    }

#ifndef VAXVERSION
    convertShort(theTypes, theFiducialHeader.numberOfFiducials);
#endif

    return(success);
}
