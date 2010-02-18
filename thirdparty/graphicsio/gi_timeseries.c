#include "graphicsio.h"
void findMappingFile(FileInfoPtr thisFile);

extern char nothing[];

/****
 *
 *	readtimeseriesheader (internal use only)
 *
 ***/

long readtimeseriesheader(FileInfoPtr thisFile, TimeSeriesHeaderPtr theHeader)
{
    long	    bytesRead;
    long    	    headerSize;

    myLseek(thisFile->fileNumber, 
		     thisFile->theCurrentTimeSeriesLocation , SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &headerSize, sizeof(long));
#ifndef VAXVERSION
    convertLong((long*)&headerSize, 1);
#endif
    if (bytesRead != sizeof(long)) return(fileReadError);

    myLseek(thisFile->fileNumber, 
		     thisFile->theCurrentTimeSeriesLocation, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, (char*) theHeader,
		     headerSize);

    if (bytesRead != headerSize) return(fileReadError);

#ifndef VAXVERSION
    convertLong((long*)theHeader, 2);
    convertLong((long*)&(theHeader->dataFormat), 11);
    convertLong((long*)&(theHeader->assocSurfaceNumber), 8);
    if (headerSize > 324) 
    {
	convertLong((long*)&(theHeader->storageFormat), 1);
    }
#endif
    return(success);
}

/****
 *
 *	writetimeseriesheader (internal use only)
 *
 ***/

long writetimeseriesheader(FileInfoPtr thisFile, TimeSeriesHeaderPtr theHeader)
{
    long	    bytesWritten;
    
#ifndef VAXVERSION
    convertLong((long*)theHeader, 2);
    convertLong((long*)&(theHeader->dataFormat), 11);
    convertLong((long*)&(theHeader->assocSurfaceNumber), 9);
#endif
    myLseek(thisFile->fileNumber, 
		   thisFile->theCurrentTimeSeriesLocation, SEEK_SET);
    bytesWritten = write(thisFile->fileNumber, (char*) theHeader,
			 sizeof(TimeSeriesHeader));
    if (bytesWritten != sizeof(TimeSeriesHeader)) return(fileWriteError);
#ifndef VAXVERSION
    convertLong((long*)theHeader, 2);
    convertLong((long*)&(theHeader->dataFormat), 11);
    convertLong((long*)&(theHeader->assocSurfaceNumber), 9);
#endif
    return(success);
}

/****
 *
 *	settimeseriesdatapath
 *
 ***/


long settimeseriesdatapath_(FileInfoPtr thisFile, char *thePath)
{
    strcpy(thisFile->dataPath, thePath);
    return(success);
}

/****
 *
 *	settimeseriesindex
 *
 ***/

long settimeseriesindex_(FileInfoPtr thisFile, long theIndex)
{
    long	    	    	bytesRead, bytesWritten, loop, temp;
    long    	    	    	numberOfTimeSeries, headerSize;
    FilePtr 	    	    	timeSeriesLocation;
    FilePtr			firstTimeSeriesHeader;
    FilePtr 	    	    	nextTimeSeriesHeader;
    TimeSeriesHeaderPtr		aTimeSeriesHeader;
    TimeSeriesHeader	      	theCurrentHeader;
    char    	    	    	finalFileName[132], tempFileName[20];
    char                        errstring[128];
    long                        tapeNumber;

 /* is this just an access (no real data file)?				                                      0 means .acq, < 0 means .raw or .pak */
    if (thisFile->fileNumber <= 0) 
    {
 /* yes it is, do we have a "tape" open already? */
	if (thisFile->externalFileNumber > 0)
	{
	    close(thisFile->externalFileNumber); /* yup, close it */
	    thisFile->externalFileNumber = 0;
	}

	if(thisFile->externalFileType == FILE_TYPE_ACQ)
	{
	    sprintf(tempFileName, "%s-%4.4ld.acq", 
		    thisFile->filePrefix, theIndex);
	    findMappingFile(thisFile); /* check for a mapping file */
	}
	else
	{
	    tapeNumber = thisFile->fileNumber * -1;
	    sprintf(tempFileName, "t%ldf%3.3ld", tapeNumber, theIndex);
	    
	    if (thisFile->externalFileType == FILE_TYPE_RAW)
	    {
		strcat(tempFileName, ".raw");
	    }
	    else if (thisFile->externalFileType == FILE_TYPE_PAK)
	    {
		strcat(tempFileName, ".pak");
	    }
	}
#ifdef VAXVERSION
	fixPath(thisFile, tempFileName, finalFileName);
	thisFile->externalFileNumber = open(finalFileName, 
						O_RDONLY, 0, "ctx=stm");
#endif
#ifndef VAXVERSION
	fixPath(thisFile, tempFileName, finalFileName);
	thisFile->externalFileNumber = open(finalFileName, O_RDONLY);
#endif
	if (thisFile->externalFileNumber == -1)
	{
	    thisFile->externalFileNumber = 0;
	    sprintf(errstring, "Failed to open external file %s",
		    finalFileName);
	    SCREAM(errstring, "settimeseriesindex", invalidTimeSeries,
		   nothing);
	}
	
	return (existingSeries);
    }


	
    myLseek(thisFile->fileNumber, NTSOFFSET, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, (char*) &numberOfTimeSeries, 
		     sizeof(long));
    if (bytesRead != sizeof(long))
    {
	SCREAM("Error reading number of time series", "settimeseriesindex",
	       fileReadError,
	       nothing);
    }

    bytesRead = read(thisFile->fileNumber, (char*) &firstTimeSeriesHeader,
		     sizeof(FilePtr));

    if (bytesRead != sizeof(FilePtr))
    {
	SCREAM("Error reading time series header location",
	       "settimeseriesindex", fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&numberOfTimeSeries, 1);
    convertLong((long*)&firstTimeSeriesHeader, 1);
#endif

    if (theIndex == numberOfTimeSeries + 1)
    {   
	/* this is a new time series */
	
	/* set up a new time series header */
	
	timeSeriesLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
	aTimeSeriesHeader = (TimeSeriesHeaderPtr)
	    calloc(1, sizeof(TimeSeriesHeader));
	aTimeSeriesHeader->headerSize = sizeof(TimeSeriesHeader);
	thisFile->currentTimeSeriesNumberOfFrames = 0;
	thisFile->currentTimeSeriesNumberOfChannels = 0;

#ifndef VAXVERSION
    convertLong((long*)&(aTimeSeriesHeader->headerSize), 1);
#endif

	/* write it to the file */

	bytesWritten = write(thisFile->fileNumber, (char*) aTimeSeriesHeader,
			     sizeof(TimeSeriesHeader));
	
	if (bytesWritten != sizeof(TimeSeriesHeader))
	{
	    SCREAM("Error writing time series header", "settimeseriesindex",
		   fileWriteError,
		   nothing);
	}

	/* patch up the file header and the file info */

	numberOfTimeSeries += 1;
	myLseek(thisFile->fileNumber, NTSOFFSET, SEEK_SET);

#ifndef VAXVERSION
        convertLong((long*)&numberOfTimeSeries, 1);
#endif

	bytesWritten = write(thisFile->fileNumber, (char*) &numberOfTimeSeries,
			     sizeof(long));

	if (bytesWritten != sizeof(long))
	{
	   SCREAM("Error writing number of time series", "settimeseriesindex",
		  fileWriteError,
		  nothing);
	}

#ifndef VAXVERSION
        convertLong((long*)&numberOfTimeSeries, 1);
#endif


	if (theIndex == 1)
	{
	    myLseek(thisFile->fileNumber, FTSOFFSET, SEEK_SET);


#ifndef VAXVERSION
	convertLong((long*)&timeSeriesLocation, 1);
#endif

	    bytesWritten = write(thisFile->fileNumber, 
				 (char*) &timeSeriesLocation, sizeof(FilePtr));

	    if (bytesWritten != sizeof(FilePtr))
	    {
	    SCREAM("Error writing time series location", "settimeseriesindex",
		   fileWriteError,
		   nothing);
	    }
#ifndef VAXVERSION
	convertLong((long*)&timeSeriesLocation, 1);
#endif

	    thisFile->theCurrentTimeSeriesLocation = timeSeriesLocation;
	    thisFile->theCurrentTimeSeriesIndex = theIndex;
	    return (firstSeries);
	}
	else
	{
	    nextTimeSeriesHeader = firstTimeSeriesHeader;

	    for (loop=1; loop < theIndex; loop++)
	    {
		temp = myLseek(thisFile->fileNumber, nextTimeSeriesHeader +
			     NTSHOFFSET, SEEK_SET);

		thisFile->theCurrentTimeSeriesLocation =  temp - NTSHOFFSET;
		bytesRead = read(thisFile->fileNumber,
				 (char*) &nextTimeSeriesHeader,
				 sizeof(FilePtr));

		if (bytesRead != sizeof(FilePtr))
		{
		    SCREAM("Error reading next time series header location",
			   "settimeseriesindex", fileWriteError,
			   nothing);
		}

#ifndef VAXVERSION
		convertLong((long*)&nextTimeSeriesHeader, 1);
#endif
	    }
			
			
		myLseek(thisFile->fileNumber, temp, SEEK_SET);
						     
#ifndef VAXVERSION
	    convertLong((long*)&timeSeriesLocation, 1);
#endif
	    bytesWritten = write(thisFile->fileNumber, 
				 (char*) &timeSeriesLocation, sizeof(FilePtr));
	    if (bytesWritten != sizeof(FilePtr))
	    {
	    SCREAM("Error writing time series location", "settimeseriesindex",
		   fileWriteError,
		   nothing);
	    }
#ifndef VAXVERSION
	    convertLong((long*)&timeSeriesLocation, 1);
#endif
	    thisFile->theCurrentTimeSeriesIndex = theIndex;
	    thisFile->theCurrentTimeSeriesLocation = timeSeriesLocation;
	    return(newSurface);
	}
    }
    else if (theIndex <= numberOfTimeSeries)
    {
	nextTimeSeriesHeader = firstTimeSeriesHeader;
	thisFile->theCurrentTimeSeriesLocation = firstTimeSeriesHeader;

	for (loop=1; loop < theIndex; loop++)
	{
	    temp = myLseek(thisFile->fileNumber, nextTimeSeriesHeader +
			 NTSHOFFSET, SEEK_SET);
	    bytesRead = read(thisFile->fileNumber, 
			     (char*) &nextTimeSeriesHeader, sizeof(FilePtr));
	    if (bytesRead != sizeof(FilePtr))
	    {
		SCREAM("Error reading next time series header location",
		       "settimeseriesindex", fileWriteError,
		       nothing);
	    }

#ifndef VAXVERSION
	    convertLong((long*)&nextTimeSeriesHeader, 1);
#endif
	    thisFile->theCurrentTimeSeriesLocation = nextTimeSeriesHeader;
	}
	thisFile->theCurrentTimeSeriesIndex = theIndex;
	myLseek(thisFile->fileNumber, 
		       thisFile->theCurrentTimeSeriesLocation, SEEK_SET);


	/* find the actual size of the header */

	bytesRead = read(thisFile->fileNumber, &headerSize, sizeof(long));

#ifndef VAXVERSION
	    convertLong((long*)&headerSize, 1);
#endif
	myLseek(thisFile->fileNumber, 
		       thisFile->theCurrentTimeSeriesLocation, SEEK_SET);


	bytesRead = read(thisFile->fileNumber, (char*)&theCurrentHeader, 
			 headerSize);

	if (bytesRead != headerSize)
	{
	    SCREAM("Error reading time series header", "settimeseriesindex",
		   fileReadError,
		   nothing);
	}

	if (thisFile->externalFileNumber > 0)
	{
		close(thisFile->externalFileNumber);
		thisFile->externalFileNumber = 0;
	}
	if (strlen(theCurrentHeader.fileName) > 0)
	{
#ifdef VAXVERSION
	    fixPath(thisFile, theCurrentHeader.fileName, finalFileName);
	    thisFile->externalFileNumber = open(finalFileName, 
						O_RDONLY, 0, "ctx=stm");
#endif
#ifndef VAXVERSION
	    fixPath(thisFile, theCurrentHeader.fileName, finalFileName);
	    thisFile->externalFileNumber = open(finalFileName, 
						O_RDONLY);
#endif
	    if (thisFile->externalFileNumber == -1)
	    {
		printf("Error opening file %s\n", finalFileName);
		thisFile->externalFileNumber = 0;
		sprintf(errstring, "Failed to open external file %s",
			finalFileName);
		SCREAM(errstring, "settimeseriesindex", invalidTimeSeries,
		       nothing);
	    }

	    thisFile->externalFileType = findFileType(thisFile, finalFileName);
	    if (thisFile->externalFileType == FILE_TYPE_UNKNOWN)
	    {
		close(thisFile->externalFileNumber);
		SCREAM("Unknown external file type", "settimeseriesindex",
                       invalidTimeSeries,
                       nothing);
	    }
	 }
			   
	return(existingSeries);
    }
    else
    {
	return(invalidTimeSeries);
    }
}


/****
 *
 *	gettimeseriesindex
 *
 ***/
 
long gettimeseriesindex_(FileInfoPtr thisFile, long *theIndex)
{
    *theIndex = thisFile->theCurrentTimeSeriesIndex;
    return(success);
}

/****
 *
 *	settimeseriesgeomfile
 *
 ***/

long settimeseriesgeomfile_(FileInfoPtr thisFile, char *theFile)
{
    TimeSeriesHeader 	thisHeader;
    long	    	     	loop;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "settimeseriesgeomfile",
	       result,
	       nothing);
    }
    for (loop = 0; loop <= 80; loop++)
    {
	thisHeader.geometryFileName[loop] = theFile[loop];
    }
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "settimeseriesgeomfile",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	gettimeseriesgeomfile
 *
 ***/

long gettimeseriesgeomfile_(FileInfoPtr thisFile, char *theFile)
{
    TimeSeriesHeader 	thisHeader;
    long	    	    	loop;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "gettimeseriesgeomfile",
	       result,
	       nothing);
    }
    for (loop = 0; loop <= 79; loop++)
    {
	theFile[loop] = thisHeader.geometryFileName[loop];
    }
    return(success);
}

/****
 *
 *	settimeseriesfile
 *
 ***/

long settimeseriesfile_(FileInfoPtr thisFile, char *theFile)
{
    TimeSeriesHeader 	thisHeader;
    long	    	    	loop;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "settimeseriesfile",
	       result,
	       nothing);
    }
    for (loop = 0; loop <= 80; loop++)
    {
	thisHeader.fileName[loop] = theFile[loop];
    }
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "settimeseriesfile",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	gettimeseriesfile
 *
 ***/

long gettimeseriesfile_(FileInfoPtr thisFile, char *theFile)
{
    TimeSeriesHeader	thisHeader;
    long    	    	loop, result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "gettimeseriesfile",
	       result,
	       nothing);
    }
    for (loop = 0; loop <= 79; loop++)
    {
	theFile[loop] = thisHeader.fileName[loop];
    }
    return(success);
}

/****
 *
 *	settimeseriesspecs
 *
 ***/

long settimeseriesspecs_(FileInfoPtr thisFile, long numberOfChannels,
		    long numberOfFrames)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "settimeseriesspecs",
	       result,
	       nothing);
    }
    thisHeader.numberOfChannels = numberOfChannels;
    thisHeader.numberOfFrames = numberOfFrames;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "settimeseriesspecs",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	gettimeseriesspecs
 *
 ***/

long gettimeseriesspecs_(FileInfoPtr thisFile, long *numberOfChannels,
		    long *numberOfFrames)
{
    PakHeader	    	aPakHeader;
    RawHeader	    	aRawHeader;
    AcqHeader		anAcqHeader;
    long    	    	result;
    TimeSeriesHeader	thisHeader;
    Boolean            superHeader;
    
    if (thisFile->externalFileNumber == 0)
    {
	result = readtimeseriesheader(thisFile, &thisHeader);
	if (result != success)
	{
	    SCREAM("Error reading time series header", "gettimeseriesspecs",
		   result,
		   nothing);
	}
    	*numberOfChannels = thisHeader.numberOfChannels;
    	*numberOfFrames = thisHeader.numberOfFrames;
    }
    else
    {
	if (thisFile->externalFileType == FILE_TYPE_PAK)
	{
	    result = readpakheader(thisFile, &aPakHeader);
	    if (result != success)
	    {
		SCREAM("Error reading pak tape header", "gettimeseriesspecs",
		       result,
		       nothing);
	    }
	    *numberOfChannels = aPakHeader.numberOfLeads;
	    *numberOfFrames = aPakHeader.numberOfFrames;
	}
	else if (thisFile->externalFileType == FILE_TYPE_RAW)
	{
	    result = readrawheader(thisFile, &aRawHeader, &superHeader);
	    if (result != success)
	    {
		SCREAM("Error reading raw tape header", "gettimeseriesspecs",
		       result,
		       nothing);
	    }
	    *numberOfChannels = aRawHeader.numberOfLeads;
	    result = findFileSize(thisFile, numberOfFrames);
	}
	else if (thisFile->externalFileType == FILE_TYPE_ACQ)
	{
	    result = readacqheader(thisFile, &anAcqHeader);
	    if (result != success)
	    {
		SCREAM("Error reading acq file header", "gettimeseriesspecs",
		       result,
		       nothing);
	    }
	    *numberOfChannels = anAcqHeader.numberOfLeads;
	    *numberOfFrames = anAcqHeader.numberOfFrames;
	}
    }
    return(success);
}

/****
 *
 *	settimeserieslabel
 *
 ***/

long settimeserieslabel_(FileInfoPtr thisFile, char *theLabel)
{
    TimeSeriesHeader 	thisHeader;
    long	    	    	loop;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "settimeserieslabel",
	       result,
	       nothing);
    }
    for (loop = 0; loop < 80; loop++)
    {
	thisHeader.label[loop] = theLabel[loop];
    }
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "settimeserieslabel",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	gettimeserieslabel
 *
 ***/

long gettimeserieslabel_(FileInfoPtr thisFile, char *theLabel)
{
    TimeSeriesHeader 	thisHeader;
    long	    	loop;
    PakHeader		aPakHeader;
    RawHeader	    	aRawHeader;
    AcqHeader		anAcqHeader;
    long    	    	result, i;
    Boolean            superHeader;

    if(thisFile->externalFileNumber == 0)
    {
	result = readtimeseriesheader(thisFile, &thisHeader);
	if (result != success)
	{
	    SCREAM("Error reading time series header",
		   "gettimeserieslabel", result,
		   nothing);
	}
	for (loop = 0; loop <= 79; loop++)
	{
	    theLabel[loop] = thisHeader.label[loop];
	}
    }
    else
    {
	if (thisFile->externalFileType == FILE_TYPE_PAK)
	{
	    result = readpakheader(thisFile, &aPakHeader);
	    if (result != success)
	    {
		SCREAM("Error reading pak tape header", "gettimeserieslabel",
		       result,
		       nothing);
	    }
	    for (loop = 0; loop <= 39; loop++)
	    {
		theLabel[loop] = aPakHeader.header[loop];
	    }
	    theLabel[40] = 0;
	}
	else if (thisFile->externalFileType == FILE_TYPE_RAW)
	{
	    result = readrawheader(thisFile, &aRawHeader, &superHeader);
	    if (result != success)
	    {
		SCREAM("Error reading raw tape header", "gettimeserieslabel",
		       result,
		       nothing);
	    }
	    for (loop = 0; loop <= 39; loop++)
	    {
		theLabel[loop] = aRawHeader.header[loop];
	    }
	    theLabel[40] = 0;
	}
	else if (thisFile->externalFileType == FILE_TYPE_ACQ)
	{
	    result = readacqheader(thisFile, &anAcqHeader);
	    if (result != success)
	    {
		SCREAM("Error reading acq file header", "gettimeseriesspecs",
		       result,
		       nothing);
	    }
	    for (loop = 1, i = 0; loop <= anAcqHeader.patientName[0]; loop++)
	    {
 /* these strings are stored as pascal strings */
		theLabel[i++] = anAcqHeader.patientName[loop];
	    }

	    theLabel[i++] = ' ';

	    for (loop = 1; loop <= anAcqHeader.diagnosis[0]; loop++)
	    {
 /* these strings are stored as pascal strings */
		theLabel[i++] = anAcqHeader.diagnosis[loop];
	    }
	    theLabel[i] = 0;
	}

    }
    return(success);
}

/****
 *
 *	settimeseriesformat
 *
 ***/

long settimeseriesformat_(FileInfoPtr thisFile, long theFormat)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "settimeseriesformat",
	       result,
	       nothing);
    }
    thisHeader.dataFormat = theFormat;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "settimeseriesformat",
	       result,
	       nothing); 
   }
    return(success);
}

/****
 *
 *	gettimeseriesformat
 *
 ***/

long gettimeseriesformat_(FileInfoPtr thisFile, long* theFormat)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "gettimeseriesformat",
	       result,
	       nothing);
    }
    *theFormat = thisHeader.dataFormat;

    return(success);
}

/****
 *
 *	settimeseriesassoc
 *
 ***/

long settimeseriesassoc_(FileInfoPtr thisFile, long theAssociation)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "settimeseriesassoc",
	       result,
	       nothing);
    }
    thisHeader.association = theAssociation;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "settimeseriesassoc",
	       result,
	       nothing); 
   }
    return(success);
}

/****
 *
 *	gettimeseriesassoc
 *
 ***/

long gettimeseriesassoc_(FileInfoPtr thisFile, long* theAssociation)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "gettimeseriesassoc",
	       result,
	       nothing);
    }
    *theAssociation = thisHeader.association;

    return(success);
}

/****
 *
 *	settimeseriesunits
 *
 ***/

long settimeseriesunits_(FileInfoPtr thisFile, long theUnits)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "settimeseriesunits",
	       result,
	       nothing);
    }
    thisHeader.dataUnits = theUnits;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "settimeseriesunits",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	gettimeseriesunits
 *
 ***/

long gettimeseriesunits_(FileInfoPtr thisFile, long* theUnits)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "gettimeseriesunits",
	       result,
	       nothing);
    }
    *theUnits = thisHeader.dataUnits;
    return(success);
}

/****
 *
 *	settimeseriessurface
 *
 ***/

long settimeseriessurface_(FileInfoPtr thisFile, long theSurface)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "gettimeseriessurface",
	       result,
	       nothing);
    }
    thisHeader.assocSurfaceNumber = theSurface;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "gettimeseriessurface",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	gettimeseriessurface
 *
 ***/

long gettimeseriessurface_(FileInfoPtr thisFile, long* theSurface)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "gettimeseriessurface",
	       result,
	       nothing);
    }
    *theSurface = thisHeader.assocSurfaceNumber;
    return(success);
}


/****
 *
 *	settimeseriesassociation
 *
 ***/

long settimeseriesassociation_(FileInfoPtr thisFile, long theAssociation)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "settimeseriesassociation",
	       result,
	       nothing);
    }
    thisHeader.association = theAssociation;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "settimeseriesassociation",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	gettimeseriesassociation
 *
 ***/

long gettimeseriesassociation_(FileInfoPtr thisFile, long* theAssociation)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "gettimeseriesassociation",
	       result,
	       nothing);
    }
    *theAssociation = thisHeader.association;

    return(success);
}

/****
 *
 *	setnumcorrectedleads
 *
 ***/

long setnumcorrectedleads_(FileInfoPtr thisFile, long numberOfCorrectedLeads)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "setnumcorrectedleads",
	       result,
	       nothing);
    }
    thisHeader.numberOfCorrectedChannels = numberOfCorrectedLeads;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "setnumcorrectedleads",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	getnumcorrectedleads
 *
 ***/

long getnumcorrectedleads_(FileInfoPtr thisFile, long* theNumber)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "getnumcorrectedleads",
	       result,
	       nothing);
    }
    *theNumber = thisHeader.numberOfCorrectedChannels;
    return(success);
}

/****
 *
 *	setqsttimes
 *
 ***/

long setqsttimes_(FileInfoPtr thisFile, long qtime, long stime, long ttime)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "setqsttimes",
	       result,
	       nothing);
    }

    thisHeader.qtime = qtime;
    thisHeader.stime = stime;
    thisHeader.ttime = ttime;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "setqsttimes",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	getqsttimes
 *
 ***/

long getqsttimes_(FileInfoPtr thisFile, long *qtime, long *stime, long *ttime)
{
    TimeSeriesHeader 	    thisHeader;
    PakHeader	    	    aPakHeader;
    long    	    	    result;

    if (thisFile->externalFileNumber == 0)
    {
	result = readtimeseriesheader(thisFile, &thisHeader);
	if (result != success)
	{
	    SCREAM("Error reading time series header", "getqsttimes",
		   result,
		   nothing);
	}
	*qtime = thisHeader.qtime;
	*stime = thisHeader.stime;
	*ttime = thisHeader.ttime;
    }
    else
    {
	if (thisFile->externalFileType != FILE_TYPE_PAK)
	{
	    SCREAM("Can only get QST times from a .pak file!", "getqsttimes",
		   invalidOperation,
		   nothing);
	}
	result = readpakheader(thisFile, &aPakHeader);
	if (result != success)
	{
	    SCREAM("Error reading pak tape", "getqsttimes", 
		   result,
		   nothing);
	}
	*qtime = aPakHeader.qTime;
	*stime = aPakHeader.sTime;
	*ttime = aPakHeader.tTime;
    }
    return(success);
}

/****
 *
 *	setextendedfiducials
 *
 ***/

long setextendedfiducials_(FileInfoPtr thisFile, long ponset, long poffset,
	    	    	    long rpeak, long tpeak)
{
    TimeSeriesHeader 	thisHeader;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "setextendedfiducials",
	       result,
	       nothing);
    }

    thisHeader.ponset = ponset;
    thisHeader.poffset = poffset;
    thisHeader.rpeak = rpeak;
    thisHeader.tpeak = tpeak;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "setextendedfiducials",
	       result,
	       nothing);
    }
    return(success);
}

/****
 *
 *	getextendedfiducials
 *
 ***/

long getextendedfiducials_(FileInfoPtr thisFile, long *ponset, long *poffset,
	    	    	    long *rpeak, long *tpeak)
{
    TimeSeriesHeader 	    thisHeader;
    long    	    	    result;

    if (thisFile->externalFileNumber == 0)
    {
	result = readtimeseriesheader(thisFile, &thisHeader);
	if (result != success)
	{
	    SCREAM("Error reading time series header", "getextendedfiducials",
		   result,
		   nothing);
	}
	if (!(thisHeader.headerSize > 308))
	{
	    SCREAM("Extended fiducials not available", "getextendedfiducials",
		   invalidOperation, nothing);
	}
	*ponset = thisHeader.ponset;
	*poffset = thisHeader.poffset;
	*rpeak = thisHeader.rpeak;
	*tpeak = thisHeader.tpeak;
    }
    else
    {
	if (thisFile->externalFileType == FILE_TYPE_RAW)
	{
	    SCREAM("Can't get extendedfiduciuals from a raw data file!",
		   "getextendedfiducials",
		   invalidOperation,
		   nothing);
	}
	if (thisFile->externalFileType == FILE_TYPE_PAK)
	{
	    SCREAM("Can't get extendedfiduciuals from a pak data file!",
		   "getextendedfiducials",
		   invalidOperation,
		   nothing);
	}
	if (thisFile->externalFileType == FILE_TYPE_ACQ)
	{
	    SCREAM("Can't get extendedfiduciuals from a acq data file!",
		   "getextendedfiducials",
		   invalidOperation,
		   nothing);
	}
    }
    return(success);
}

/****
 *
 *	checkextendedfiducials
 *
 ***/

long checkextendedfiducials_(FileInfoPtr thisFile, Boolean *available)
{
    TimeSeriesHeader 	    thisHeader;
    long    	    	    result;

    if (thisFile->externalFileNumber == 0)
    {
	result = readtimeseriesheader(thisFile, &thisHeader);
	if (result != success)
	{
	    SCREAM("Error reading time series header", "getqsttimes",
		   result,
		   nothing);
	}
	*available =  (unsigned char)((thisHeader.headerSize > 308) ? 
				      TRUE : FALSE);
    }
    else
    {
	if (thisFile->externalFileType == FILE_TYPE_RAW)
	{
	    *available = FALSE;
	}
	if (thisFile->externalFileType == FILE_TYPE_PAK)
	{
	    *available = FALSE;
	}
	if (thisFile->externalFileType == FILE_TYPE_ACQ)
	{
	    *available = FALSE;
	}
    }
    return(success);
}


/****
 *
 *	settimeseriesdata
 *
 ***/

long settimeseriesdata_(FileInfoPtr thisFile, float *theData)
{
    TimeSeriesHeader 	thisHeader;
    FilePtr 	    	dataLocation;
    long	    	bytesWritten, numberOfSamples;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    thisHeader.storageFormat = STORAGE_FORMAT_FLOAT;
    if (result != success)
    {
	SCREAM("Error reading time series header", "settimeseriesdata",
	       result, nothing);
    }
    dataLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
    numberOfSamples = thisHeader.numberOfFrames * thisHeader.numberOfChannels;
 /*** ??? ***/
    printf(" In SetTimeSeriesData, write out %d frames with %d channels\n",
	   thisHeader.numberOfFrames, thisHeader.numberOfChannels);

#ifndef VAXVERSION
    convertFloat(theData, numberOfSamples, FALSE);
#endif

    bytesWritten = write(thisFile->fileNumber, (char*) theData,
			 numberOfSamples * sizeof(float));

    if (bytesWritten != (long)(numberOfSamples * sizeof(float)))
    {
	SCREAM("Error writing the time series data", "settimeseriesdata",
	       fileWriteError,
	       nothing);
    }

    thisHeader.theData = dataLocation;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing the time series header", "settimeseriesdata",
	       result, nothing);
    }
#ifndef VAXVERSION
    convertFloat(theData, numberOfSamples, TRUE);
#endif
    return(success);
}

/****
 *
 *	setsometimeseriesdata
 *
 ***/

long setsometimeseriesdata_(FileInfoPtr thisFile, void *theData, long numberOfFrames,
			    int storageFormat)
{
    TimeSeriesHeader 	thisHeader;
    FilePtr 	    	dataLocation;
    long	    	bytesWritten, numberOfSamples;
    long    	    	result;

 /* is this the first write? */
    if ( thisFile->currentTimeSeriesNumberOfChannels == 0)
    {
	result = readtimeseriesheader(thisFile, &thisHeader);
	if (result != success)
	{
	    SCREAM("Error reading time series header", "settimeseriesdata",
		   result, nothing);
	}
	dataLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);

	thisHeader.theData = dataLocation;
	thisHeader.storageFormat = storageFormat;

	result = writetimeseriesheader(thisFile, &thisHeader);
	if (result != success)
	{
	    SCREAM("Error writing the time series header", "settimeseriesdata",
		   result, nothing);
	}

	thisFile->currentTimeSeriesNumberOfChannels = 
	    thisHeader.numberOfChannels;
	thisFile->currentTimeSeriesNumberOfFrames = thisHeader.numberOfFrames;

    }

    if (storageFormat == STORAGE_FORMAT_FLOAT)
    {
	numberOfSamples = numberOfFrames * 
	    thisFile->currentTimeSeriesNumberOfChannels;
	
#ifndef VAXVERSION
    convertFloat((float*)theData, numberOfSamples, FALSE);
#endif

	bytesWritten = write(thisFile->fileNumber, (char*) theData,
			     numberOfSamples * sizeof(float));
	
	if (bytesWritten != (long)(numberOfSamples * sizeof(float)))
	{
	    SCREAM("Error writing the time series data", "settimeseriesdata",
		   fileWriteError,
		   nothing);
	}
#ifndef VAXVERSION
    convertFloat((float*)theData, numberOfSamples, TRUE);
#endif
    }
    else if (storageFormat == STORAGE_FORMAT_INTEGER)
    {
	numberOfSamples = numberOfFrames * 
	    thisFile->currentTimeSeriesNumberOfChannels;
	
#ifndef VAXVERSION
	convertShort((short*)theData, numberOfSamples);
#endif
	
	bytesWritten = write(thisFile->fileNumber, (char*) theData,
			     numberOfSamples * sizeof(short));
	
	if (bytesWritten != (long)(numberOfSamples * sizeof(short)))
	{
	    SCREAM("Error writing the time series data", "settimeseriesdata",
		   fileWriteError,
		   nothing);
	}
#ifndef VAXVERSION
	convertShort((short*)theData, numberOfSamples);
#endif
    }
    else
    {
	return(invalidOperation);
    }
	
    return(success);
}

/****
 *
 *	getselectedtimeserieschannel
 *
 ***/

long getselectedtimeserieschannel_(FileInfoPtr thisFile, 
				   long selectedChannel, long startingFrame,
				   long *numberOfFrames, void *theLead, 
				   int returnedFormat)
{
    short   	*shortPtr, *shortBuffer;
    float   	*floatPtr, *floatBuffer;
    long    	result, tempNumberOfChannels, tempNumberOfFrames, 
	        thisNumberOfFrames;
    long    	i, j;

    result = gettimeseriesspecs_(thisFile, &tempNumberOfChannels, 
				 &tempNumberOfFrames);
    thisNumberOfFrames = *numberOfFrames;

    if (returnedFormat == RETURN_FORMAT_INTEGER)
    {
	shortBuffer = (short*)malloc(thisNumberOfFrames * 
				     tempNumberOfChannels * sizeof(short));
	
	if (shortBuffer == 0)
	{
	    SCREAM("Can't get memory for temp buffer", 
		   "getselectedtimeserieschannel",
		   noDynamicMemory, nothing);
	}

	result = getselectedtimeseriesdata_(thisFile, 
					    startingFrame, &thisNumberOfFrames,
					    (void*) shortBuffer, 
					    RETURN_FORMAT_INTEGER);

	if (result != success)
	{
	    SCREAM("Bad return from getselectedtimeseriesdata", 
		   "getselectedtimeserieschannel",
		   unspecifiedError, nothing);
	}
	    
	for (i = selectedChannel - 1, j = 0, 
		 shortPtr = (short*)theLead; j < thisNumberOfFrames;)
	{
	    shortPtr[j++] = shortBuffer[i];
	    i += tempNumberOfChannels;
	}
	free(shortPtr);
    }
    else if (returnedFormat == RETURN_FORMAT_FLOAT)
    {
	floatBuffer = (float*)malloc(thisNumberOfFrames * 
				     tempNumberOfChannels * sizeof(float));
	
	if (floatBuffer == 0)
	{
	    SCREAM("Can't get memory for temp buffer", 
		   "getselectedtimeserieschannel",
		   noDynamicMemory, nothing);
	}

	result = getselectedtimeseriesdata_(thisFile, startingFrame, 
					    &thisNumberOfFrames,
					    (void*) floatBuffer, 
					    RETURN_FORMAT_FLOAT);

	if (result != success)
	{
	    SCREAM("Bad return from getselectedtimeseriesdata", 
		   "getselectedtimeserieschannel",
		   unspecifiedError, nothing);
	}
	    
	for (i = selectedChannel - 1, j = 0, 
		 floatPtr = (float*)theLead; j < thisNumberOfFrames;)
	{
	    floatPtr[j++] = floatBuffer[i];
	    i += tempNumberOfChannels;
	}
	free(floatBuffer);

    }
    *numberOfFrames = thisNumberOfFrames;
    return(success);
}

/****
 *
 *	getselectedtimeseriesdata
 *
 ***/

long getselectedtimeseriesdata_(FileInfoPtr thisFile, long startingFrame,
				long *numberOfFrames, void *theData, 
				int returnedFormat)
{
    TimeSeriesHeader 	thisHeader;
    long	    	bytesRead, numberOfSamples, numberOfBytes;
    FilePtr 	    	headerOffset;
    RawHeader	    	aRawHeader;
    long		loop, index, bytesToBeRead, i, numberOfFramesToRead;
    long    	    	result, datumSize, startingOffset, bytesToSkip;
    long    	    	tempNumberOfChannels, tempNumberOfFrames, 
			remainingBytes;
    long		numberOfChannels;
    long                bytesToSkipInThisRecord;
    long		framesPerRead, framesLeftOver, frame, lead, k, j;
    Boolean            superHeader, recastData, oldStyleData, done;
    void    	    	*tempBuffer;
    int			*mapBuffer;
    short		*readBuffer;		
    short               *shortPtr, *shortBuffer;
    float   	    	*floatPtr, *floatBuffer;
    char		finalMapName[255];
    long		integralReads, iindex;
    int			numberOfMappedChannels;


 /* are the data actually in this file? */
    if (thisFile->externalFileNumber == 0)  
    {
	result = readtimeseriesheader(thisFile, &thisHeader); /* yup */
     
	if (result != success)
	{
	    SCREAM("Error reading time series header", 
		   "getselectedtimeseriesdata",
		   result,
		   nothing);
	}
	
	numberOfFramesToRead = *numberOfFrames;

	if ((startingFrame + numberOfFramesToRead - 1) > 
	    thisHeader.numberOfFrames)
	{
	    numberOfFramesToRead = thisHeader.numberOfFrames - startingFrame;
	}
	    
	numberOfSamples = numberOfFramesToRead * 
	    	    	    thisHeader.numberOfChannels;

	if (!(thisHeader.headerSize > 324))
	{
	    oldStyleData = TRUE;    /* data are stored as floats */
	}
	else
	{
         /* time series header will tell us how the data are stored */
	    oldStyleData = FALSE;   
	}


	if (!oldStyleData)
	{
	    if (thisHeader.storageFormat != returnedFormat) /* yes */
	    {
		recastData = TRUE;
		if (returnedFormat == RETURN_FORMAT_INTEGER)
		{
		    datumSize = sizeof(float);
		    floatBuffer = (float*)malloc(numberOfSamples * datumSize);
		    tempBuffer = floatBuffer;
		    if (floatBuffer == 0)
		    {
			SCREAM("Can't get memory for float buffer", 
			       "getselectedtimeseriesdata",
			       noDynamicMemory, nothing);
		    }		
		}
		else
		{
		    datumSize = sizeof(short);
		    shortBuffer = (short*)malloc(numberOfSamples * datumSize);
		    tempBuffer = shortBuffer;
		    if (shortBuffer == 0)
		    {
			SCREAM("Can't get memory for short buffer", 
			       "getselectedtimeseriesdata",
			       noDynamicMemory, nothing);
		    }		
		}
	    }
	    else
	    {
		recastData = FALSE;
		if (returnedFormat == RETURN_FORMAT_INTEGER)
		{
		    datumSize = sizeof(short);
		}
		else if (returnedFormat == RETURN_FORMAT_FLOAT)
		{
		    datumSize = sizeof(float);
		}
		else
		{
		    datumSize = 0;
		}
	    }
	}
	else
	{
           /* this is old format data and we want ints */
	    if (returnedFormat == RETURN_FORMAT_INTEGER) 
	    {
		recastData = TRUE;
		datumSize = sizeof(float);
		floatBuffer = (float*)malloc(numberOfSamples * datumSize);
		tempBuffer = floatBuffer;
		if (floatBuffer == 0)
		{
		    SCREAM("Can't get memory for float buffer", 
			   "getselectedtimeseriesdata",
			   noDynamicMemory, nothing);
		}		
	    }
	}

	startingOffset = datumSize * thisHeader.numberOfChannels * 
	    (startingFrame - 1);

	myLseek(thisFile->fileNumber, thisHeader.theData + startingOffset, 
		SEEK_SET);

	bytesToBeRead = numberOfFramesToRead * thisHeader.numberOfChannels * 
	    datumSize;

	if (recastData)
	{
	    bytesRead = read(thisFile->fileNumber, 
			     (char*) tempBuffer, bytesToBeRead);
	}
	else
	{
	    bytesRead = read(thisFile->fileNumber, 
			     (char*) theData, bytesToBeRead);
	}


	if (bytesRead != bytesToBeRead)
	{
	    SCREAM("Error reading the time series data", 
		   "getselectedtimeseriesdata",
		   fileReadError, nothing);
	}


#ifndef VAXVERSION
	if (oldStyleData || (thisHeader.storageFormat == STORAGE_FORMAT_FLOAT))
	{
	    if (recastData)
	    {
		convertFloat(floatBuffer, numberOfSamples, TRUE);
	    }
	    else
	    {
		convertFloat((float*)theData, numberOfSamples, TRUE);
	    }
	}
	else
	{
	    if (recastData)
	    {
		convertShort(shortBuffer, numberOfSamples);
	    }
	    else
	    {
		convertShort((short*)theData, numberOfSamples);
	    }
	}
#endif


	if (recastData)
	{
	    if (returnedFormat == RETURN_FORMAT_FLOAT)
	    {
		for (i = 0; i < numberOfSamples; i++)
		{
		    ((float*)theData)[i] = (float)shortBuffer[i];
		}
	    }
	    else
	    {
		for (i=0; i < numberOfSamples; i++)
		{
		    ((short*)theData)[i] = (short)floatBuffer[i];
		}
	    }
	    free(tempBuffer);
	}
	
	*numberOfFrames = numberOfFramesToRead;
	return(success);
    }
    else /* else read data from an external file */
    {
	headerOffset = movePastHeader(thisFile, &tempNumberOfChannels, 
				      &tempNumberOfFrames);
	numberOfSamples = (*numberOfFrames) * tempNumberOfChannels;
	myLseek(thisFile->externalFileNumber, headerOffset, SEEK_SET);
	bytesToSkip = (startingFrame - 1) * tempNumberOfChannels * 2;
	numberOfChannels = tempNumberOfChannels;
	numberOfFramesToRead = *numberOfFrames;

	tempBuffer = (short*)malloc(numberOfSamples * sizeof(short));
	if (tempBuffer == 0)
	{
	    SCREAM("Can't get memory for temp buffer", 
		   "getselectedtimeseriesdata",
		   noDynamicMemory, nothing);
	}

	if (thisFile->externalFileType == FILE_TYPE_ACQ)
	{
	    myLseek(thisFile->externalFileNumber, bytesToSkip, SEEK_SET);

	    if (strlen(thisFile->theMapInUse) == 0) /* fake a map */
	    {
		mapBuffer = (int*)malloc(numberOfChannels * sizeof(int));
		if (mapBuffer == 0)
		{
		    SCREAM("Can't get memory for map buffer", 
			   "getselectedtimeseriesdata",
			   noDynamicMemory, nothing);
		}
		for(i = 0; i < numberOfChannels; i++)
		{
		    mapBuffer[i] = i + 1;
		}
	    }
	    else /* get the map */
	    {
		if (strlen(thisFile->theDefaultMap) == 0)
		{
		    strcpy(finalMapName, thisFile->theMapInUse);
		}
		else
		{
		    fixPath(thisFile, thisFile->theMapInUse, finalMapName);
		}
		mapBuffer =  getmapinfo(finalMapName, &numberOfMappedChannels);
		for (i = 0; i< numberOfMappedChannels; i++)
		{
		    if (mapBuffer[i] == 0) mapBuffer[i] = 1;
		}
	    }


	    readBuffer = (short*)malloc(25600 * sizeof(short));
	    if (readBuffer == 0)
	    {
		SCREAM("Can't get memory for read buffer", 
		       "getselectedtimeseriesdata",
		       noDynamicMemory, nothing);
	    }

	    framesPerRead = 25600 / numberOfChannels;
	    integralReads = numberOfFramesToRead / framesPerRead;

	    framesLeftOver = numberOfFramesToRead - 
		integralReads * framesPerRead;

	    for(i = 0, k = 0; i < integralReads; i++)
	    {
		bytesRead = 
		    read(thisFile->externalFileNumber, 
			 (char*)readBuffer,
			 framesPerRead * numberOfChannels  * sizeof(short));
		if (bytesRead != (long)( framesPerRead * numberOfChannels * 
					 sizeof(short)))
		{
		    SCREAM("Error reading acq file", 
			   "getselectedtimeseriesdata", fileReadError,
			   "reading data");
		}
		j = 0;
		for (frame = 0; frame < framesPerRead; frame++)
		{
		    for(lead = 0; lead < numberOfChannels; lead++)
		    {
			iindex = frame*numberOfChannels + mapBuffer[lead] - 1;
			((short*)tempBuffer)[k+j] = readBuffer[iindex];
			j += 1;
		    }
		}
		k += 25600;
	    }		


	    if (framesLeftOver > 0)
	    {
		bytesRead = 
		    read(thisFile->externalFileNumber, (char*)readBuffer,
			 framesLeftOver * numberOfChannels  * sizeof(short));
		if (bytesRead != (long)(framesLeftOver * numberOfChannels * 
					sizeof(short)))
		{
		    SCREAM("Error reading acq file", 
			   "getselectedtimeseriesdata", fileReadError,
			   "reading data");
		}

		j = 0;
		for (frame = 0; frame < framesLeftOver; frame++)
		{
		    for(lead = 0; lead < numberOfChannels; lead++)
		    {
			iindex = frame*numberOfChannels + mapBuffer[lead] - 1;
			((short*)tempBuffer)[k+j] = readBuffer[iindex];
			j += 1;
		    }
		}
	    }

	    free(mapBuffer);
	    free(readBuffer);

 /*	    bytesRead = read(thisFile->externalFileNumber, (char*)tempBuffer,
			     numberOfSamples * sizeof(short));
	    if (bytesRead != (long)(numberOfSamples * sizeof(short)))
	    {
		SCREAM("Error reading acq file", "getselectedtimeseriesdata",
		       fileReadError,
		       "reading data");
	    } 
 */

#ifdef VAXVERSION
	    convertShort(tempBuffer, numberOfSamples);
#endif
	}
	else /* We're reading a raw or a pak file */
	{
	    index = 0;

	    numberOfBytes = readNextRecordSize(thisFile->externalFileNumber);
	    
	    if (numberOfBytes == 0)
	    {
		SCREAM("Error reading initial RMS byte count", 
		       "getselectedtimeseriesdata",
		       fileReadError, nothing);
	    }
	    
	    done = FALSE;
	    
	    while(!done)
	    {
		if ((index + numberOfBytes) == bytesToSkip)
		{
		    done = TRUE;
		    remainingBytes = 0;
		}
		
		if ((index + numberOfBytes) > bytesToSkip)
		{
		    bytesToSkipInThisRecord = bytesToSkip - index;
		    remainingBytes = numberOfBytes - bytesToSkipInThisRecord;
		    numberOfBytes = (bytesToSkip - index);
		    done = TRUE;
		}
		
		lseek(thisFile->externalFileNumber, numberOfBytes, SEEK_CUR);
		if (done)
		{
		    numberOfBytes = remainingBytes;
		    break;
		}
		
		index += numberOfBytes / 2;
		
		numberOfBytes = 
		    readNextRecordSize(thisFile->externalFileNumber);
		if (numberOfBytes == 0)
		{
		    printf("file is shorter than expected\n");
		    printf("number of channels * number of frames is %ld\n",
			   numberOfSamples);
		    printf("index %ld\n", index);
		    numberOfSamples = index;
		    done = TRUE;
		}
	    }
	    
	    index = 0;
	    
	    if (numberOfBytes == 0)
	    {
		numberOfBytes = 
		    readNextRecordSize(thisFile->externalFileNumber);
		
		if (numberOfBytes == 0)
		{
		    SCREAM("Error reading initial RMS byte count after"
			   " skipping",
			   "getselectedtimeseriesdata",fileReadError, nothing);
		}
	    }

	    done = FALSE;
	    
	    while(!done)
	    {
		if ((index + numberOfBytes/2) == numberOfSamples)
		{
		    done = TRUE;
		}
		
		if ((index + numberOfBytes/2) > numberOfSamples)
		{
		    numberOfBytes = (numberOfSamples - index) * 2;
		    done = TRUE;
		}
		
		bytesRead = read(thisFile->externalFileNumber,
				 (char*) &((short*)tempBuffer)[index], 
				 numberOfBytes);
		if (bytesRead != numberOfBytes)
		{
		    SCREAM("Error reading raw or pak tape", 
			   "getselectedtimeseriesdata",
			   fileReadError,
			   "reading data");
		}
		index += numberOfBytes / 2;
		
		numberOfBytes = 
		    readNextRecordSize(thisFile->externalFileNumber);
		if (numberOfBytes == 0)
		{
		    printf("file is shorter than expected\n");
		    printf("number of channels * number of frames is %ld\n",
			   numberOfSamples);
		    printf("index %ld\n", index);
		    numberOfSamples = index;
		    done = TRUE;
		}
	    }

	/* now swap the bytes if need be */
#ifndef VAXVERSION
	    convertShort((short*)tempBuffer, numberOfSamples);
#endif
	}
	/* and lastly massage the data appropriately */
	if (thisFile->externalFileType == FILE_TYPE_PAK)
	{
	    if (returnedFormat == RETURN_FORMAT_FLOAT)
	    {
		for (loop = 0; loop < numberOfSamples; loop++)
		{
		    ((float*)theData)[loop] = 
			(float)(((short*)tempBuffer)[loop] * 5.0);
		}
	    }
	    else if (returnedFormat == RETURN_FORMAT_INTEGER)
	    {
		for (loop = 0; loop < numberOfSamples; loop++)
		{
		    ((short*)theData)[loop] = ((short*)tempBuffer)[loop];
		}
	    }
	    else
	    {
		SCREAM("Nonsense return data format specified", 
		       "getselectedtimeseriesdata",
		       invalidOperation, nothing);
	    }		
	}
	if (thisFile->externalFileType == FILE_TYPE_RAW)
	{
	    readrawheader(thisFile, &aRawHeader, &superHeader);

	    if (returnedFormat == RETURN_FORMAT_FLOAT)
	    {
		floatPtr = (float*)theData;

		if (aRawHeader.numberOfLeads == 32)
		{
		    for (loop = 0; loop < numberOfSamples; loop++)
		    {
			floatPtr[loop] = (float)((short*)tempBuffer)[loop];
		    }
		}
		else
		{
		    for (loop = 0; loop < numberOfSamples; loop++)
		    {
			floatPtr[loop] = 
			    (float)((((short*)tempBuffer)[loop] & 0xFFF) - 
				    2048);
		    }
		}
	    }
	    else if (returnedFormat == RETURN_FORMAT_INTEGER)
	    {
		shortPtr = (short*)theData;

		if (aRawHeader.numberOfLeads == 32)
		{
		    for (loop = 0; loop < numberOfSamples; loop++)
		    {
			shortPtr[loop] = ((short*)tempBuffer)[loop];
		    }
		}
		else
		{
		    for (loop = 0; loop < numberOfSamples; loop++)
		    {
			shortPtr[loop] = 
			    (short)((((short*)tempBuffer)[loop] & 0xFFF) - 
				    2048);
		    }
		}
	    }
	    else
	    {
		SCREAM("Nonsense return data format specified", 
		       "getselectedtimeseriesdata",
		       invalidOperation, nothing);
	    }		
	}
	if (thisFile->externalFileType == FILE_TYPE_ACQ)
	{
	    if (returnedFormat == RETURN_FORMAT_FLOAT)
	    {
		floatPtr = (float*)theData;

		for (loop = 0; loop < numberOfSamples; loop++)
		{
		    floatPtr[loop] = 
			(float)((((short*)tempBuffer)[loop] & 0xFFF) - 2048);
		}
	    }
	    else if (returnedFormat == RETURN_FORMAT_INTEGER)
	    {
		shortPtr = (short*)theData;

		for (loop = 0; loop < numberOfSamples; loop++)
		{
		    shortPtr[loop] = 
			((((short*)tempBuffer)[loop] & 0xFFF) - 2048);
		}
	    }
	    else
	    {
		SCREAM("Nonsense return data format specified", 
		       "getselectedtimeseriesdata",
		       invalidOperation, nothing);
	    }		
	}
	free(tempBuffer);
	*numberOfFrames = numberOfSamples / tempNumberOfChannels;
	return(success);
    }
}



/****
 *
 *	gettimeseriesdata
 *
 ***/

long gettimeseriesdata_(FileInfoPtr thisFile, float *theData)
{
    TimeSeriesHeader 	thisHeader;
    long	    	bytesRead, numberOfLeads, numberOfFrames;
    long                numberOfSamples, numberOfBytes;
    FilePtr 	    	headerOffset;
    RawHeader	    	aRawHeader;
    long		loop, index, i, k, framesPerRead, framesLeftOver;
    long                frame, lead;
    short   	    	*tempBuffer, *readBuffer;
    int			*mapBuffer;		
    long    	    	result, integralReads;
    char		finalMapName[255];
    int			numberOfMappedChannels;
    Boolean            superHeader;
/**********************************************************************/
    if (thisFile->externalFileNumber == 0)
    {
	result = readtimeseriesheader(thisFile, &thisHeader);
	numberOfSamples = thisHeader.numberOfFrames * 
	    thisHeader.numberOfChannels;
	if (result != success)
	{
	    SCREAM("Error reading time series header", "gettimeseriesdata",
		   result,
		   nothing);
	}
	myLseek(thisFile->fileNumber, thisHeader.theData, SEEK_SET);
        if ((thisHeader.headerSize <= 324) || 
	    ((thisHeader.headerSize > 324) && 
	     (thisHeader. storageFormat == STORAGE_FORMAT_FLOAT)))
	{
	    bytesRead = read(thisFile->fileNumber, (char*) theData,
			     numberOfSamples * sizeof(float));
	    if (bytesRead != (long)(numberOfSamples * sizeof(float)))
	    {
		SCREAM("Error reading the time series data", 
		       "gettimeseriesdata",
		       fileReadError, nothing);
	    }

#ifndef VAXVERSION
	    convertFloat(theData, numberOfSamples, TRUE);
#endif
	}
	else /* the data are stored internally as shorts */
	{
	    tempBuffer = (short*)malloc(numberOfSamples * sizeof(short));
	    if (tempBuffer == 0)
	    {
		SCREAM("Can't get memory for temp buffer", "gettimeseriesdata",
		       noDynamicMemory, nothing);
	    }
	    bytesRead = read(thisFile->fileNumber, (char*) tempBuffer,
			     numberOfSamples * sizeof(short));
	    if (bytesRead != (long)(numberOfSamples * sizeof(short)))
	    {
		SCREAM("Error reading the time series data", 
		       "gettimeseriesdata",
		       fileReadError, nothing);
	    }
#ifndef VAXVERSION
	    convertShort(tempBuffer, numberOfSamples);
#endif
	    for (loop = 0; loop < numberOfSamples; loop++)
	    {
		theData[loop] = (float)tempBuffer[loop];
	    }
	    free(tempBuffer);
	}	    
    }
    else /* We're reading an external file */
    {
	headerOffset = movePastHeader(thisFile, &numberOfLeads, 
				      &numberOfFrames);
	numberOfSamples = numberOfLeads * numberOfFrames;

	myLseek(thisFile->externalFileNumber, headerOffset, SEEK_SET);
	
	tempBuffer = (short*)malloc(numberOfSamples * sizeof(short));
	memset(tempBuffer, 0, numberOfSamples * sizeof(short));
	if (tempBuffer == 0)
	{
	    SCREAM("Can't get memory for temp buffer", "gettimeseriesdata",
		   noDynamicMemory, nothing);
	}

	if (thisFile->externalFileType == FILE_TYPE_ACQ)
	{
	    if (strlen(thisFile->theMapInUse) == 0) /* fake a map */
	    {
		mapBuffer = (int*)malloc(numberOfLeads * sizeof(short));
		if (mapBuffer == 0)
		{
		    SCREAM("Can't get memory for map buffer", 
			   "gettimeseriesdata",
			   noDynamicMemory, nothing);
		}
		for(i = 1; i <= numberOfLeads; i++)
		{
		    mapBuffer[i] = i;
		}
	    }
	    else /* get the map */
	    {
		fixPath(thisFile, thisFile->theMapInUse, finalMapName);
		mapBuffer =  getmapinfo(finalMapName, &numberOfMappedChannels);
	    }


	    readBuffer = (short*)malloc(25600 * sizeof(short));
	    if (readBuffer == 0)
	    {
		SCREAM("Can't get memory for read buffer", "gettimeseriesdata",
		       noDynamicMemory, nothing);
	    }

	    framesPerRead = 25600 / numberOfLeads;
	    integralReads = numberOfFrames / framesPerRead;

	    framesLeftOver = numberOfFrames - integralReads * framesPerRead;

	    for(i = 0, k = 0; i < integralReads; i++)
	    {
		bytesRead = read(thisFile->externalFileNumber, 
				 (char*)readBuffer,
			 framesPerRead * numberOfLeads  * sizeof(short));
		if (bytesRead != (long)(framesPerRead * numberOfLeads * 
					sizeof(short)))
		{
		    SCREAM("Error reading acq file", "gettimeseriesdata",
			   fileReadError,
			   "reading data");
		}
		for (frame = 0; frame < framesPerRead; frame++)
		{
		    for(lead = 0; lead < numberOfLeads; lead++)
		    {
			if (mapBuffer[lead] != 0)
			{
			    tempBuffer[k] = readBuffer[mapBuffer[lead]];
			}
			else
			{
			    tempBuffer[k] = 0;
			}
			k += 1;
		    }
		}
	    }		


	    if (framesLeftOver > 0)
	    {
		bytesRead = read(thisFile->externalFileNumber, 
				 (char*)readBuffer,
			 framesLeftOver * numberOfLeads  * sizeof(short));
		if (bytesRead != 
		    (long)(framesLeftOver * numberOfLeads * sizeof(short)))
		{
		    SCREAM("Error reading acq file", "gettimeseriesdata",
			   fileReadError,
			   "reading data");
		}
		for (frame = 0; frame < framesLeftOver; frame++)
		{
		    for(lead = 0; lead < numberOfLeads; lead++)
		    {
			if (mapBuffer[lead] != 0)
			{
			    tempBuffer[k] = readBuffer[mapBuffer[lead]];
			}
			else
			{
			    tempBuffer[k] = 0;
			}
			k += 1;
		    }
		}
	    }

	    free(mapBuffer);
	    free(readBuffer);

#ifdef VAXVERSION
	    convertShort(tempBuffer, numberOfSamples);
#endif
	    for (loop = 0; loop < numberOfSamples; loop++)
	    {
		theData[loop] = (float)((tempBuffer[loop] & 0xFFF) - 2048);
	    }
	}
	else /* We're reading a raw or a pak file */
	{
	    index = 0;
	    
	    numberOfBytes = readNextRecordSize(thisFile->externalFileNumber);
	    
	    if (numberOfBytes == 0)
	    {
		SCREAM("Error reading initial RMS byte count", 
		       "gettimeseriesdata", fileReadError, nothing);
	    }
	    
	    while(TRUE)
	    {
		if (index + numberOfBytes/2 >= numberOfSamples) {
		    break;
		}
		bytesRead = read(thisFile->externalFileNumber,
				 (char*) &tempBuffer[index], numberOfBytes);
		if (bytesRead != numberOfBytes)
		{
		    SCREAM("Error reading raw or pak tape", 
			   "gettimeseriesdata", fileReadError, "reading data");
		}
		index += numberOfBytes / 2;
		
		numberOfBytes = 
		    readNextRecordSize(thisFile->externalFileNumber);
		if (numberOfBytes == 0)
		{
		    printf("file is shorter than expected\n");
		    printf("number of channels * number of frames is %ld\n",
			   numberOfSamples);
		    printf("index %ld\n", index);
		    numberOfSamples = index;
		    break;
		}
	    }
	    
	    /* now swap the bytes if need be */
#ifndef VAXVERSION
	    convertShort(tempBuffer, numberOfSamples);
#endif
	    /* and lastly massage the data appropriately */
	    if (thisFile->externalFileType == FILE_TYPE_PAK)
	    {
		for (loop = 0; loop < numberOfSamples; loop++)
		{
		    theData[loop] = (float)((float)tempBuffer[loop] * 5.0);
		}
	    }
	
	    if (thisFile->externalFileType == FILE_TYPE_RAW)
	    {

 /*** Rob's attempt to fix this error (aRawHeader used before
      it has been defined)...

 Old code:
		if (aRawHeader.numberOfLeads == 32)
		{
		    for (loop = 0; loop < numberOfSamples; loop++)
		    {
			theData[loop] = (float)tempBuffer[loop];
		    }
		}
		else
		{
		    for (loop = 0; loop < numberOfSamples; loop++)
		    {
			theData[loop] = (float)((tempBuffer[loop] & 0xFFF) 
			- 2048);
                    }	
		}
             

  End of old code: ***/
 /*** Start of new code: ***/

		readrawheader(thisFile, &aRawHeader, &superHeader);
		
		if (aRawHeader.numberOfLeads == 32)
		{
		    for (loop = 0; loop < numberOfSamples; loop++)
		    {
			theData[loop] = (float)tempBuffer[loop];
		    }
		}
		else
		{
		    for (loop = 0; loop < numberOfSamples; loop++)
		    {
			theData[loop] = (float)((tempBuffer[loop] & 0xFFF) 
						- 2048);
		    }
		}
 /*** End of new code: ***/
	    }
	    free(tempBuffer);
	}
    }
    return(success);
}

/****
 *
 *	setcorrectedleads
 *
 ***/

long setcorrectedleads_(FileInfoPtr thisFile, long *theLeads)
{
    TimeSeriesHeader 	thisHeader;
    FilePtr 	    	leadLocation;
    long	    	    	bytesWritten;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "setcorrectedleads",
	       fileReadError,
	       nothing);
    }
    leadLocation = myLseek(thisFile->fileNumber, 0, SEEK_END); 

#ifndef VAXVERSION
    convertLong(theLeads, thisHeader.numberOfCorrectedChannels);
#endif

    bytesWritten = write(thisFile->fileNumber, (char*) theLeads,
			 thisHeader.numberOfCorrectedChannels *
			 sizeof(long));
    if (bytesWritten != (long)(thisHeader.numberOfCorrectedChannels * 
			       sizeof(long)))
    {
	SCREAM("Error writing the corrected leads", "setcorrectedleads",
	       fileWriteError,
	       nothing);
    }

    thisHeader.correctedChannels = leadLocation;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "setcorrectedleads",
	       fileWriteError,
	       nothing);
    }
    return(success);
}

/****
 *
 *	getcorrectedleads
 *
 ***/

long getcorrectedleads_(FileInfoPtr thisFile, long *theLeads)
{
    TimeSeriesHeader 	thisHeader;
    long	    	    	bytesRead;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading the time series header", "getcorrectedleads",
	       fileReadError,
	       nothing);
    }
    myLseek(thisFile->fileNumber, thisHeader.correctedChannels,
		   SEEK_SET);
    bytesRead = read(thisFile->fileNumber, (char*) theLeads,
		     thisHeader.numberOfCorrectedChannels * sizeof(long));


    if (bytesRead != 
	(long)(thisHeader.numberOfCorrectedChannels * sizeof(long)))
    {
	SCREAM("Error reading the corrected leads", "getcorrectedleads",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong(theLeads, thisHeader.numberOfCorrectedChannels);
#endif

    return(success);
}

/****
 *
 *	setpowercurve
 *
 ***/

long setpowercurve_(FileInfoPtr thisFile, float *powerCurveData)
{
    TimeSeriesHeader 	thisHeader;
    FilePtr 	    	powerCurveLocation;
    long	    	    	bytesWritten;
    long    	    	result;

    result = readtimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error reading time series header", "setpowercurve",
	       fileReadError,
	       nothing);
    }
    powerCurveLocation = myLseek(thisFile->fileNumber, 0, SEEK_END); 

#ifndef VAXVERSION
    convertFloat((float*)powerCurveData, thisHeader.numberOfFrames, FALSE);
#endif

    bytesWritten = write(thisFile->fileNumber, (char*) powerCurveData,
			 thisHeader.numberOfFrames * sizeof(float));
    if (bytesWritten != (long)(thisHeader.numberOfFrames * sizeof(float)))
    {
	SCREAM("Error writing the power curve data", "setpowercurve",
	       fileWriteError,
	       nothing);
    }
    thisHeader.powerCurve = powerCurveLocation;
    result = writetimeseriesheader(thisFile, &thisHeader);
    if (result != success)
    {
	SCREAM("Error writing time series header", "setpowercurve",
	       fileWriteError,
	       nothing);
    }
    return(success);
}

/****
 *
 *	getpowercurve
 *
 ***/

long getpowercurve_(FileInfoPtr thisFile, float *powerCurveData)
{
    TimeSeriesHeader 	thisHeader;
    long	    	bytesRead, numberOfChannels, numberOfFrames;
    long    	    	i, result;
    short   	    	*tempBuffer;

    if (thisFile->externalFileNumber == 0)
    {
	result = readtimeseriesheader(thisFile, &thisHeader);
	if (result != success)
	{
	    SCREAM("Error reading time series header", "getpowercurve",
		   fileReadError,
		   nothing);
	}
	if (thisHeader.powerCurve == 0) return (noPowerCurve);
	myLseek(thisFile->fileNumber, 
		       thisHeader.powerCurve, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, (char*) powerCurveData,
			 thisHeader.numberOfFrames * sizeof(float));
	if (bytesRead != (long)(thisHeader.numberOfFrames * sizeof(float)))
	{
	    SCREAM("Error reading the power curve data", "getpowercurve",
		   fileReadError,
		   nothing);
	}
#ifndef VAXVERSION
	convertFloat((float*)powerCurveData, thisHeader.numberOfFrames, TRUE);
#endif
    }
    else
    {
	result = gettimeseriesspecs_(thisFile, &numberOfChannels, 
				     &numberOfFrames);
	if (result != success)
	{
	    SCREAM("Error getting time series specs", "getpowercurve",
		   fileReadError,
		   nothing);
	}
	tempBuffer = (short*) malloc(numberOfFrames * sizeof (short));
	if (tempBuffer == 0)
	{
	    SCREAM("Can't get memory for temp buffer","getpowercurve",
		   noDynamicMemory, nothing);
	}
	myLseek(thisFile->externalFileNumber, 64, SEEK_SET);
	bytesRead = read(thisFile->externalFileNumber, 
			 (char*)tempBuffer, numberOfFrames * 
			 sizeof(short));
	if (bytesRead != (long)(numberOfFrames * sizeof(short)))
	{
	    SCREAM("Error reading power curve", "getpowercurve",
		   fileReadError,
		   nothing);
	}
#ifndef VAXVERSION
	convertShort((short*)tempBuffer, numberOfFrames);
#endif
	for (i = 0; i < numberOfFrames; i++)
	{
	    powerCurveData[i] = (float) tempBuffer[i];
	}
	free(tempBuffer);
    }
    return(success);
}
