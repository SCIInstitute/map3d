 /*** Filename: gi_utilities.c
      Author: Phil Ershler, Rob MacLeod, and Ted Dustman
 ***/
#include "graphicsio.h"
#ifndef _WIN32
#ifdef LINUX
#include <endian.h>
#elif defined(OSX)
#include <machine/endian.h>
#elif defined(SOLARIS)
#include <machine/endian.h>
#else
#include <sys/endian.h>
#endif
#endif

extern char nothing[];

off_t myLseek(int fildes, off_t offset, int whence)
{
    if (offset > 100000000)
    {
	fprintf(stderr, "Nonsense file offset = %ld, have to QUIT!\n", 
		(long)offset);
	exit(1);
    }

    return(lseek(fildes, offset, whence));
}

/****
 *
 *	convertFloat
 *
 ***/

void convertFloat(float* theData, long howMany, Boolean toUnix)
{
    union
    {
	char theChars[4];
	float thisFloat;
    }floatUnion;

    long    loop;
    char    temp;
    
    for (loop = 0; loop < howMany; loop++)
    {
	floatUnion.thisFloat = theData[loop];
        if(!toUnix)
	{
#if BYTE_ORDER == LITTLE_ENDIAN
            /* when writing, byte swap first */
	    temp = floatUnion.theChars[0];
	    floatUnion.theChars[0] = floatUnion.theChars[3];
	    floatUnion.theChars[3] = temp;
	    temp = floatUnion.theChars[2];
	    floatUnion.theChars[2] = floatUnion.theChars[1];
	    floatUnion.theChars[1] = temp;
#endif
	    if (floatUnion.theChars[0] != 0)
	    floatUnion.theChars[0] += 1;
	}
	temp = floatUnion.theChars[0];
	floatUnion.theChars[0] = floatUnion.theChars[1];
	floatUnion.theChars[1] = temp;
	temp = floatUnion.theChars[2];
	floatUnion.theChars[2] = floatUnion.theChars[3];
	floatUnion.theChars[3] = temp;
	if(toUnix) 
	{
	    if(floatUnion.theChars[0] != 0)
	    floatUnion.theChars[0] -= 1;
#if BYTE_ORDER == LITTLE_ENDIAN

            /* when reading, byte swap last */
	    temp = floatUnion.theChars[0];
	    floatUnion.theChars[0] = floatUnion.theChars[3];
	    floatUnion.theChars[3] = temp;
	    temp = floatUnion.theChars[2];
	    floatUnion.theChars[2] = floatUnion.theChars[1];
	    floatUnion.theChars[1] = temp;
#endif
	}
	theData[loop] = floatUnion.thisFloat;
    }
    return;
}

/****
 *
 *	convertLong
 *
 ***/
/***
#if !(defined(LINUX) || defined(_WIN32))
***/
void convertLong(long* theData, long howMany)
{
    union
    {
	char theChars[4];
	long thisLong;
    }longUnion;
    
    long    loop;
    char    temp;

#if BYTE_ORDER == BIG_ENDIAN

/***
#ifdef BIG_ENDIAN
***/
    for (loop = 0; loop < howMany; loop++)
    {
	longUnion.thisLong = theData[loop];
	temp = longUnion.theChars[3];
	longUnion.theChars[3] = longUnion.theChars[0];
	longUnion.theChars[0] = temp;
	temp = longUnion.theChars[2];
	longUnion.theChars[2] = longUnion.theChars[1];
	longUnion.theChars[1] = temp;
	theData[loop] = longUnion.thisLong;
    }
#endif
    return;
}
/***
#endif
***/

/****
 *
 *	convertShort
 *
 ***/

/***
#if !defined(LINUX) && !defined(_WIN32)
***/
void convertShort(short* theData, long howMany)
{
    union
    {
	char theChars[2];
	short thisShort;
    }shortUnion;
    
    long    loop;
    char    temp;

#if BYTE_ORDER == BIG_ENDIAN
 /***    
#ifdef BIG_ENDIAN
 ***/
    for (loop = 0; loop < howMany; loop++)
    {
	shortUnion.thisShort = theData[loop];
	temp = shortUnion.theChars[1];
	shortUnion.theChars[1] = shortUnion.theChars[0];
	shortUnion.theChars[0] = temp;
	theData[loop] = shortUnion.thisShort;
    }
#endif
    return;
}
/***
#endif
***/

/****
 *
 *	readpakheader
 *
 ***/
 
long readpakheader(FileInfoPtr thisFile, PakHeaderPtr thisHeader)
{
    long		bytesRead;
    
    myLseek(thisFile->externalFileNumber, 4, SEEK_SET);
    bytesRead = read(thisFile->externalFileNumber, 
		     (char*)thisHeader, sizeof(PakHeader));
    if (bytesRead != sizeof(PakHeader)) return(fileReadError);

#ifndef VAXVERSION
    convertShort((short*)thisHeader, 10);
#endif
    return(success);
}

/****
 *
 *	readacqheader
 *
 ***/

long readacqheader(FileInfoPtr thisFile, AcqHeaderPtr thisHeader)
{
    long bytesRead;

    myLseek(thisFile->externalFileNumber, 0, SEEK_SET);
    bytesRead = read(thisFile->externalFileNumber, thisHeader, sizeof(AcqHeader));
    if (bytesRead != sizeof(AcqHeader)) return(fileReadError);
#ifdef VAXVERSION
    convertShort((short*)&(thisHeader->numberOfLeads), 2);
#endif
    return(success);
}

/****
 *
 *	readrawheader
 *
 ***/
 
long readrawheader(FileInfoPtr thisFile, RawHeaderPtr thisHeader, 
		   Boolean *superHeader)
{
    int	     bytesRead;
    short    recordSize;           


    myLseek(thisFile->externalFileNumber, 0, SEEK_SET);
    bytesRead = read(thisFile->externalFileNumber, 
		     &recordSize, 2);
#ifndef VAXVERSION
    convertShort((short*)&recordSize, 1);
#endif
    if (recordSize == 1086)
    {
	*superHeader = TRUE;
    }
    else
    {
	*superHeader = FALSE;
    }

    myLseek(thisFile->externalFileNumber, 4, SEEK_SET);
    bytesRead = read(thisFile->externalFileNumber, 
		     (char*)thisHeader, sizeof(RawHeader));

    if (bytesRead != sizeof(RawHeader)) return(fileReadError);

#ifndef VAXVERSION
    convertShort((short*)thisHeader, 6);
#endif
    return(success);
}

void fixPath(FileInfoPtr thisFile, char* inputFileName, char* outputFileName)
{

    char    *pathEnd;
    int	    c;

#ifndef VAXVERSION
    c = 47; /* 47 is a "/" */
#endif

#ifdef VAXVERSION
    c = 93; /* 93 is a "]" */
#endif

    if (strlen(thisFile->dataPath) == 0) /* do we have a new path? */
    {
	strcpy(outputFileName, inputFileName); /* nope, just use what we got */
    }
    else
    {
	strcpy(outputFileName, thisFile->dataPath); /* yup, get the new path */
	
	pathEnd = strrchr(inputFileName, c); /* find last occurrence of c */
	if (pathEnd == NULL) /* There is no path name in the data file */
	{
	    strcat(outputFileName, inputFileName); 
	}
	else
	{
	    pathEnd++;  	    	     /* bump past the last path char */
	    strcat(outputFileName, pathEnd);    /* get just the file name */
	}
    }
    /*    printf("FINAL FILE NAME is %s\n",outputFileName);*/
}

int findFileType(FileInfoPtr thisFile, char* finalFileName)
{
    size_t  	nameLength;
    char    	ext[5];
    int	    	i;
    
    nameLength = strlen(finalFileName);
    if (nameLength < 4)
    {
	thisFile->externalFileType = FILE_TYPE_UNKNOWN;
	return(FILE_TYPE_UNKNOWN);
    }
    nameLength -= 1;	    	    	/* backup one to avoid the null */
    while (finalFileName[nameLength] == '\t' || 
	   finalFileName[nameLength--] == ' ')
    {
    }; /* backup over white space */
    nameLength -= 2;	/* back over what's left of the extentsion */
    
    ext[0] = finalFileName[nameLength++]; /* get the '.' */
    
    for (i = 1; i <= 3; i++) /* get the extentsion and convert to lower case */
    {
	ext[i] = (char)tolower(finalFileName[nameLength++]);
    }

    ext[4] = '\0'; /* put an null on the end of the string */

    if ((strcmp(ext, ".raw")) == 0)
    {
	thisFile->externalFileType = FILE_TYPE_RAW;
    }
    else if ((strcmp(ext, ".pak")) == 0)
    {
	thisFile->externalFileType = FILE_TYPE_PAK;
    }
    else if ((strcmp(ext, ".acq")) == 0)
    {
	thisFile->externalFileType = FILE_TYPE_ACQ;
    }
    else
    {
	thisFile->externalFileType = FILE_TYPE_UNKNOWN;
    }
    return(thisFile->externalFileType);
}

long findFileSize(FileInfoPtr thisFile, long *numberOfFrames)
{
    PakHeader	    aPakHeader;
    RawHeader	    aRawHeader;
    short   	    numberOfLeads;
    Boolean        superHeader;
    off_t   	    endOfFile;

    /*	find the number of leads    */

    if (thisFile->externalFileType == FILE_TYPE_RAW)
    {
	readrawheader(thisFile, &aRawHeader, &superHeader);
	numberOfLeads = aRawHeader.numberOfLeads;
    }
    else if (thisFile->externalFileType == FILE_TYPE_PAK)
    {
	readpakheader(thisFile, &aPakHeader);
	numberOfLeads = aPakHeader.numberOfLeads;
    }
    else
    {
	return(invalidOperation);
    }

    /*	really find the number of data bytes	*/

/*    bytesRead = 1;
*    result = myLseek(thisFile->externalFileNumber, 0, SEEK_SET);
*    while(bytesRead > 0)
*    {
*	bytesRead = read(thisFile->externalFileNumber, stuff, 4);
*	convertShort(stuff, 2);
*	if ((bytesRead > 0) && pastHeader)
*	{
*		totalBytes += (stuff[0] - 2);
*	}
*	if (((stuff[1] & 2) == 2) && !pastHeader)
*	{
*	    pastHeader = 1;
*	}
*	myLseek(thisFile->externalFileNumber, stuff[0] - 2, SEEK_CUR);
*    }
*    *numberOfFrames = (totalBytes / 2) / numberOfLeads;
*/

    /* take a DUMB guess at the number of data bytes (should always be > the actual number) */

    endOfFile = lseek(thisFile->externalFileNumber, 0, SEEK_END);

    *numberOfFrames = (endOfFile / 2) / numberOfLeads;

    return(success);
}
    



long	readNextRecordSize(int fileNumber)
{
    short   	stuff[2];
    long    	bytesRead, numberOfBytes;
    
    bytesRead = read(fileNumber, (char*)stuff, 4);
    if (bytesRead != 4)
    {
	return(0);
    }
#ifndef VAXVERSION
    convertShort(stuff, 2);
#endif
    numberOfBytes = (stuff[0] - 2);
    return(numberOfBytes);
}


FilePtr	movePastHeader(FileInfoPtr thisFile, long *numberOfLeads, long *numberOfFrames)
{
    PakHeader	aPakHeader;
    RawHeader	aRawHeader;
    AcqHeader	anAcqHeader;
    FilePtr    	headerOffset;
    Boolean	superHeader;
    long    	numFrames, result;

    if (thisFile->externalFileType == FILE_TYPE_PAK)
    {
	result = readpakheader(thisFile, &aPakHeader);
	if (result != success)
	{
	    SCREAM("Error reading pak tape header", "movePastHeader",
		   result, nothing);
	}
	*numberOfFrames = aPakHeader.numberOfFrames; 
	*numberOfLeads = aPakHeader.numberOfLeads;
	headerOffset = 8076;
    }
    else if (thisFile->externalFileType == FILE_TYPE_RAW)
    {
	result = readrawheader(thisFile, &aRawHeader, &superHeader);
	if (result != success)
	{
	    SCREAM("Error reading raw tape header", "movePastHeader",
		   result, nothing);
	}
	result = findFileSize(thisFile, &numFrames);
	*numberOfFrames = numFrames;
	*numberOfLeads = aRawHeader.numberOfLeads;
	
	
	if (superHeader)
	{
	    headerOffset = 1088;
	}
	else
	{
	    headerOffset = 64;
	}
	
    }
    else if (thisFile->externalFileType == FILE_TYPE_ACQ)
    {
	result = readacqheader(thisFile, &anAcqHeader);
	if (result != success)
	{
	    SCREAM("Error reading raw tape header", "movePastHeader",
		   result, nothing);
	}
	*numberOfFrames = anAcqHeader.numberOfFrames;
	*numberOfLeads =  anAcqHeader.numberOfLeads;
	headerOffset = 1024;
    }
    return(headerOffset);
    
}    

/****
 *
 *	int* getmapinfo(char* filename, int* numberofchannels) 
 *	(for internal use only)
 *	get the info from a mapping file (for acq data)
 *
 *	filename: name of mapping file
 *      numberofchannels: number of channels in mapping file
 *
 *	returns: a pointer to a malloc'ed buffer holding the mapping info
 ***/

int* getmapinfo(char* filename, int* numberofchannels)
{

    char buffer[128], *delims = " \n", *token;
    FILE *fp;
    int k, *dataptr;

    fp = fopen(filename, "r");
    if (fp == 0)
    {
	printf("cannot open map file %s\n", filename);
	exit(0);
    }
    fgets(buffer, 128, fp);
    sscanf(buffer,"%d",numberofchannels);

    dataptr = (int*)malloc(*numberofchannels*sizeof(int));

    k = 0;
    while(fgets(buffer, 128, fp))
    {
	token = strtok(buffer, delims);
	while(token != NULL)
	{
	    sscanf(token,"%ld",&(dataptr[k++]));
	    token = strtok(NULL, delims);
	}
    }
    fclose(fp);
    return(dataptr);
}
