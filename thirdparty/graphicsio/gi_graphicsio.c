/*
 *	graphicsio.c
 *	
 *	A set of c routines for reading and writing files of
 *	graphics data and geometry for CVRTI. These routines account
 *	for the differences in byte ordering between VMS and UNIX
 *	systems. Data are stored in native VMS format in the files
 *	and are converted to UNIX format during reading and writing
 *	when executing on a UNIX systems. Note that all externally called
 *	routines end in  an underscore character (_) so that they may be
 *	called from FORTRAN or C on SGI systems.
 */


#include "graphicsio.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <io.h>
#endif

#ifdef VAXVERSION
#include <libdef.h>
#include <ssdef.h>
#include <rmsdef.h>
#else
#ifndef _WIN32
#include <dirent.h>
#endif
#endif

char nothing[] = " ";

/* report - a general purpose error reporting routine
 *
 * for internal use only
 *
 */

void report(char *message, char *function, long returnValue, 
	    char *optionalString)
{
    char output[256];

#ifdef VAXVERSION
    strcpy(output, "\n======================\n"
	   "ERROR IN: graphicsio.c FUNCTION: ");
#else
    strcpy(output, "\n======================\n"
	   "ERROR IN: graphicsio.c FUNCTION: ");
#endif
    strcat(output, function);
    strcat(output, "\nMESSAGE: ");
    strcat(output, message);
#ifdef VAXVERSION
    strcat(output, " %s\nRETURNING: %d\n"
	   "======================\n");
#else
    strcat(output, " %s\nRETURNING: %d\n"
	   "======================\n");
#endif
    fprintf(stderr, output, optionalString, returnValue);
    return;
}


		  
#ifdef VAXVERSION
typedef struct desc
{
    short length;
    char dtype;
    char class;
    long pointer;
}desc, *desctpr;

/****
 *
 *	findMappingFile(thisFile) look for acq data mapping file
 *	(internal use only)
 *
 ***/

void findMappingFile(FileInfoPtr thisFile)
{
    long result, context = 0, numberOfFiles = 0;
    char filespec[32]="*.mapping;*", theFilename[132];
    desc theFileSpec, theFile;

    if (strlen(thisFile->dataPath) == 0) return;/* do we have a path */
    theFilespec.length = 11;
    theFilespec.dtype = 14;
    theFilespec.class = 1;
    theFilespec.pointer = filespec;

    theFile.length = 132;
    theFile.dtype = 14;
    theFile.class = 1;
    theFile.pointer = theFilename;
    do
    {
	result = LIB$FIND_FILE(&theFilespec, &theFile, &context);
	theFilename[theFile.length-1] = 0;
	if (result == RMS$_NORMAL)
	{
	    /*	    printf("The file is %s\n",theFile.pointer);*/
	    numberOfFiles += 1;
	}
    } while(result == RMS$_NORMAL);
    result = LIB$FIND_FILE_END(&context);
    if(numberOfFiles == 1)
    {
	strcpy(thisFile->theDefaultMap, theFile.pointer);
	strcpy(thisFile->theMapInUse, theFile.pointer);
    }
    return;
}
#else

#ifndef _WIN32
void findMappingFile(FileInfoPtr thisFile)
{
    DIR			*dir;
    struct dirent	*entry;
    int			match = 0;
    char		theMappingFile[80], *result, ext[80];

    if (strlen(thisFile->dataPath) == 0) return;/* do we have a path */
    dir = opendir(thisFile->dataPath);
    while((entry = readdir(dir)) != NULL)
    {
	if((result = strrchr(entry->d_name,'.')))
	{
	    strcpy(ext,result);
	    if(strcmp(ext,".mapping") == 0)
	    {
		strcpy(theMappingFile, entry->d_name);
		match += 1;
	    }
	}
    }
    closedir(dir);
    
    if (match == 1)
    {
	strcpy(thisFile->theDefaultMap, theMappingFile);
	strcpy(thisFile->theMapInUse, theMappingFile);
    }

    return;
}
#else
void findMappingFile(FileInfoPtr thisFile)
{
	// this is going to take a little more work...
}
#endif
    
#endif
/****
 *
 *	createfile_   create and open a new file
 *
 *	INPUTS: fileName    name of file to be created
 *	    	fileType    type of file (data or geometry)
 *	    	errorLevel  boolean TRUE= report errors FALSE = don't report
 *	    	    	    errors, just return errors
 *
 *	OUTPUTS:theNewFile  file info pointer (used for all subsequent
 *	    	    	    references to the file)
 *	    	result	    function result
 ***/

long createfile_(char *fileName, long fileType, long errorLevel,
		 FileInfoPtr *theNewFile)
{
    long		fileNumber;
    FileInfo		*thisFile;
    FileInfoBlock	thisFileInfoBlock;
    long		bytesWritten;
    long    	    	fibStart;
    int flags;

    /* Allocate a FileInfo structure for this file */
    
    thisFile = (FileInfo*) calloc(1, sizeof(FileInfo));
    if (thisFile == 0)
    {
	SCREAM("No memory available", "createfile", noDynamicMemory,
	       nothing);
    }

    thisFile->errorLevel = errorLevel;
    
    /* Create and Open the file */
#ifdef _WIN32
    flags = O_CREAT | O_RDWR | O_EXCL | O_BINARY;
#else
    flags = O_CREAT | O_RDWR | O_EXCL;
#endif

#ifdef VAXVERSION
    fileNumber = open(fileName, flags, 0);
#else
    fileNumber = open(fileName, flags, 0666);
#endif
    if (fileNumber == -1)
    {
	SCREAM("File creation failed", "createfile", cantCreateFile,
	       fileName);
    }
    /* Initialize the File Info Structure for this file */
    
    thisFile->fileNumber = fileNumber;
    thisFile->thisFileHeader.headerSize = sizeof(FileHeader);

#ifndef VAXVERSION
    convertLong((long*)&(thisFile->thisFileHeader.headerSize), 1);
#endif

    /* Write the FileHeader to the file */
    
    bytesWritten = write(fileNumber, &(thisFile->thisFileHeader),
			 sizeof(FileHeader));
    if (bytesWritten <= 0)
    {
	close(fileNumber);
	free(thisFile);
	SCREAM("Error writing file header", "createfile", fileWriteError,
	       nothing);
    }
    
    /* Initialize the File Info Block */
    
    thisFileInfoBlock.blockSize = sizeof(FileInfoBlock);
    thisFileInfoBlock.expID[0] = 0;
    thisFileInfoBlock.runID[0] = 0;
    thisFileInfoBlock.creationTime = time(NULL);
    thisFileInfoBlock.lastUpdateTime = 0;
    thisFileInfoBlock.text[0] = 0;

#ifndef VAXVERSION
    convertLong((long*)&(thisFileInfoBlock.blockSize), 1);
    convertLong((long*)&(thisFileInfoBlock.creationTime), 1);
#endif

    /* Write the File Info Block to the file */
    
    bytesWritten = write(fileNumber, &thisFileInfoBlock,
			 sizeof(FileInfoBlock));
    if (bytesWritten <= 0)
    {
	close(fileNumber);
	free(thisFile);
	SCREAM("Error writing File Info Block", "createfile", fileWriteError,
	       nothing);
    }
			 
    /* Set the file pointer for the File Info Block */

    myLseek(fileNumber, FIBOFFSET, SEEK_SET);
    fibStart = (long)sizeof(FileHeader);

#ifndef VAXVERSION
    convertLong(&fibStart, 1);
#endif

    bytesWritten = write(fileNumber, &fibStart, sizeof(long));
    if (bytesWritten <= 0)
    {
	close(fileNumber);
	free(thisFile);
	SCREAM("Error updating File Header", "CreateFile", fileWriteError,
	       nothing);
    }

    /* If were here everything must be groovy */

    *theNewFile = thisFile;
    return(success);
}			 

/****
 *
 *	closefile_  politely close an open file
 *
 *	INPUT:	fileFile    the file info pointer
 *
 *	OUTPUT: result	    function result
 *
 ***/

long closefile_(FileInfoPtr thisFile)
{
    if (thisFile->fileNumber > 0) close(thisFile->fileNumber); /* <0 is just an access */
    free(thisFile);
    return(success);
}


/****
 *
 *	openaccess_ open access to raw or pak files (without an actual 
 *                                                   data file) 
 *
 *	INPUTS:	tapeNumber  number of "tape" to be accessed
 *              tapeType    type of "tape" to be accessed (enum extFile)
 *	    	errorLevel  set error reporting level (see createfile_)
 *
 *	OUTPUTS:theExistingFile	file info pointer for this access (used for
 *	    	    	    	all subsequent references to this access)
 *	    	result	    	function result
 *
 ***/

long openaccess_(long tapeNumber, char *filePrefix, int tapeType, 
		 long errorLevel, FileInfoPtr *theAccess)
{
    FileInfo   *thisFile;


    /* Allocate a FileInfo structure for this file */
    
    thisFile = (FileInfo*) calloc(1,sizeof(FileInfo));
    if (thisFile == 0)
    {
	SCREAM("No memory available", "openaccess", noDynamicMemory,
	       nothing);
    }
    
    thisFile->errorLevel = errorLevel;
    if (tapeType == FILE_TYPE_ACQ)
    {
	strcpy(thisFile->filePrefix, filePrefix);
	thisFile->fileNumber = 0;
	thisFile->externalFileType = tapeType;
    }
    else
    {
	thisFile->fileNumber = tapeNumber * -1;
	thisFile->externalFileType = tapeType;
    }
    *theAccess = thisFile;

    return(success);
}


/****
 *
 *	setmappingfile_	set the lead mapping file for acq data
 *
 *	INPUTS: thisFile    file info pointer
 *		theMapToUse the file name (with path) to use for mapping 
 *                          the data will override the default file if 
 *                          present 
 *
 *	OUTPUT: result	    function result
 *
 ***/
long setmappingfile_(FileInfoPtr thisFile, char* theMapToUse)
{
    strcpy(thisFile->theMapInUse, theMapToUse);
    return(success);
}
    
/****
 *
 *	getmappingfiles_    set the lead mapping file for acq data
 *
 *	INPUTS:	    thisFile    file info pointer
 *
 *	OUTPUTS:    result	    function result
 *		    theDefaultMap   the lead mapping file found with acq files
 *				    (if any) 
 *				    (zero length string returned if map not
 *				    found) 
 *                  theMapInUse	    the lead mapping file set by
 *				    setmappingfile_ (if any)  
 *				    (zero length string returned if map not
 *				    set)		     
 ***/
long getmappingfiles_(FileInfoPtr thisFile, char* theDefaultMap, 
		      char* theMapInUse)
{
    strcpy(theMapInUse, thisFile->theMapInUse);
    strcpy(theDefaultMap, thisFile->theDefaultMap);
    return(success);
}


/****
 *
 *	openfile_ open an existing file for read only access
 *
 *	INPUTS:	fileName    name of file to be opened
 *	    	errorLevel  set error reporting level (see createfile_)
 *
 *	OUTPUTS:theExistingFile	file info pointer for the file (used for
 *	    	    	    	all subsequent references to the file)
 *	    	result	    	function result
 *
 ***/

long openfile_(char* fileName, long errorLevel, FileInfoPtr* theExistingFile)
{
    long			fileNumber;
    FileInfo			*thisFile;
    long		    	bytesRead;
    long	    	    	loop;
    FilePtr 	    	    	nextSurfaceLocation;
    
    /* Allocate a FileInfo structure for this file */
    
    thisFile = (FileInfo*) calloc(1,sizeof(FileInfo));
    if (thisFile == 0)
    {
	SCREAM("No memory available", "openfile", noDynamicMemory,
	       nothing);
    }
    
    thisFile->errorLevel = errorLevel;
    
    /* Open the file */
    
#ifdef _WIN32
#define OPENFLAGS O_RDONLY|O_BINARY
#else
#define OPENFLAGS O_RDONLY
#endif
    
#ifdef VAXVERSION
    fileNumber = open(fileName, OPENFLAGS, 0);
#endif
#ifndef VAXVERSION
    fileNumber = open(fileName, OPENFLAGS);
#endif
    if (fileNumber == -1)
    {
	perror("And the error is ta da:");
	SCREAM("Failed to open file", "openfile", cantOpenFile,
	       fileName);
    }
    
    thisFile->errorLevel = errorLevel;
    
    /* Read the FileHeader from the file */
    
    bytesRead = read(fileNumber, &(thisFile->thisFileHeader),
		     sizeof(FileHeader));
    if (bytesRead != sizeof(FileHeader))
    {
	close(fileNumber);
	free(thisFile);
	SCREAM("Error reading the header", "openfile", fileReadError,
	       nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)&((thisFile->thisFileHeader).headerSize), 10);
#endif
    
    /* Get the pointer to the first Surface Header */
    myLseek(fileNumber, FSHOFFSET, SEEK_SET);
    bytesRead = read(fileNumber, &nextSurfaceLocation,
		     sizeof(FilePtr));
    
    if (bytesRead != sizeof(FilePtr))
    {
	close(fileNumber);
	free(thisFile);
	SCREAM("Error reading the first surface location", "openfile",
	       fileReadError, nothing);
    }
    
#ifndef VAXVERSION
    convertLong((long*)&nextSurfaceLocation, 1);
#endif
    
    /* "Walk" through the surfaces to the specified surface */
    for (loop =1; loop < thisFile->thisFileHeader.numberOfSurfaces; loop++)
    {
	thisFile->theCurrentSurfaceLocation = myLseek(fileNumber,
						    nextSurfaceLocation +
						    NSHOFFSET, SEEK_SET);
	bytesRead = read(fileNumber, &nextSurfaceLocation,
			 sizeof(FilePtr));
	
	if (bytesRead != sizeof(FilePtr))
	{
	    close(fileNumber);
	    free(thisFile);
	    SCREAM("Error reading the next surface location",
		   "openfile", fileReadError, nothing);
	}
	
#ifndef VAXVERSION
	convertLong((long*)&nextSurfaceLocation, 1);
#endif
    }
    
    /* Set up the File Info Header */
    
    thisFile->fileNumber = fileNumber;
    thisFile->currentSurfaceIndex = 0;
    thisFile->theCurrentSurface = 0;
    thisFile->lastSurfaceLocation = nextSurfaceLocation;
    thisFile->lastSurfaceIndex = thisFile->thisFileHeader.numberOfSurfaces;
    thisFile->theCurrentTimeSeriesIndex = 0;
    thisFile->theCurrentTimeSeriesLocation = 0;
    *theExistingFile = thisFile;
    return(success);
}


/****
 *
 *	openfilerewrite_ open an existing file for read and write acces
 *
 *	INPUTS:	fileName    name of file to be opened
 *	    	errorLevel  set error reporting level (see createfile_)
 *
 *	OUTPUTS:theExistingFile	file info pointer for the file (used for
 *	    	    	    	all subsequent references to the file)
 *	    	result	    	function result
 *
 ***/

long openfilerewrite_(char* fileName, long errorLevel, 
		      FileInfoPtr* theExistingFile)
{
    long			fileNumber;
    FileInfo			*thisFile;
    long		    	bytesRead;
    long	    	    	loop;
    FilePtr 	    	    	nextSurfaceLocation;
    
    /* Allocate a FileInfo structure for this file */
    
    thisFile = (FileInfo*) calloc(1,sizeof(FileInfo));
    if (thisFile == 0)
    {
	SCREAM("No memory available", "openfile", noDynamicMemory,
	       nothing);
    }
    
    thisFile->errorLevel = errorLevel;
    
    /* Open the file */
    
#ifdef VAXVERSION
    fileNumber = open(fileName, O_RDWR, 0);
#endif
#ifndef VAXVERSION
    fileNumber = open(fileName, O_RDWR);
#endif
    if (fileNumber == -1)
    {
	perror("And the error is ta da:");
	SCREAM("Failed to open file", "openfile", cantOpenFile,
	       fileName);
    }
    
    thisFile->errorLevel = errorLevel;
    
    /* Read the FileHeader from the file */
    
    bytesRead = read(fileNumber, &(thisFile->thisFileHeader),
		     sizeof(FileHeader));
    if (bytesRead != sizeof(FileHeader))
    {
	close(fileNumber);
	free(thisFile);
	SCREAM("Error reading the header", "openfile", fileReadError,
	       nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)&((thisFile->thisFileHeader).headerSize), 10);
#endif
    
    /* Get the pointer to the first Surface Header */
    myLseek(fileNumber, FSHOFFSET, SEEK_SET);
    bytesRead = read(fileNumber, &nextSurfaceLocation,
		     sizeof(FilePtr));
    
    if (bytesRead != sizeof(FilePtr))
    {
	close(fileNumber);
	free(thisFile);
	SCREAM("Error reading the first surface location", "openfile",
	       fileReadError, nothing);
    }
    
#ifndef VAXVERSION
    convertLong((long*)&nextSurfaceLocation, 1);
#endif
    
    /* "Walk" through the surfaces to the specified surface */
    for (loop =1; loop < thisFile->thisFileHeader.numberOfSurfaces; loop++)
    {
	thisFile->theCurrentSurfaceLocation = myLseek(fileNumber,
						    nextSurfaceLocation +
						    NSHOFFSET, SEEK_SET);
	bytesRead = read(fileNumber, &nextSurfaceLocation,
			 sizeof(FilePtr));
	
	if (bytesRead != sizeof(FilePtr))
	{
	    close(fileNumber);
	    free(thisFile);
	    SCREAM("Error reading the next surface location",
		   "openfile", fileReadError, nothing);
	}
	
#ifndef VAXVERSION
	convertLong((long*)&nextSurfaceLocation, 1);
#endif
    }
    
    /* Set up the File Info Header */
    
    thisFile->fileNumber = fileNumber;
    thisFile->currentSurfaceIndex = 0;
    thisFile->theCurrentSurface = 0;
    thisFile->lastSurfaceLocation = nextSurfaceLocation;
    thisFile->lastSurfaceIndex = thisFile->thisFileHeader.numberOfSurfaces;
    thisFile->theCurrentTimeSeriesIndex = 0;
    thisFile->theCurrentTimeSeriesLocation = 0;
    *theExistingFile = thisFile;
    return(success);
}

/****
 *
 *	getfileinfo_   get basic information about file contents
 *
 *	INPUTS:	thisFile    file info pointer
 * 
 *	OUTPUTS:fileType    	    	    file type (data or geometry)
 *	    	numberOfSurfaces    	    number of data or geometry surfaces
 *	    	    	    	    	    contained in the file
 *	    	numberOfBounding Surfaces   number of bounding surfaces
 *	    	numberOfTimeSeriesBlocks    number of time series blocks
 *	    	preferedSettingsBlock	    boolean TRUE prefered settings
 *	    	    	    	    	    block contained in the file
 *	    	result	    	    	    function result
 *
 ***/

long getfileinfo_(FileInfoPtr thisFile, long* fileType, long* numberOfSurfaces,
		 long* numberOfBoundSurfaces, long* numberOfTimeSeriesBlocks,
		  Boolean* preferedSettingsBlock)
{
    *fileType = (thisFile->thisFileHeader).fileType;
    *numberOfSurfaces = (thisFile->thisFileHeader).numberOfSurfaces;
    *numberOfBoundSurfaces = (thisFile->thisFileHeader).numberOfBoundSurfaces;
    *numberOfTimeSeriesBlocks = (thisFile->thisFileHeader).numberOfTimeSeries;
    *preferedSettingsBlock = (unsigned char)((thisFile->thisFileHeader).
			      preferedSettingsInfoBlock == 0) ? FALSE : TRUE;
    return(success);
}

/****
 *
 *	setexpid_   set the experiment ID string
 *
 *	INPUTS:	thisFile    file info pointer
 *	    	theExp	    experiment ID string (79 chars max + null = 80)
 *
 *	OUTPUT:	result	    function result
 *
 ***/

long setexpid_(FileInfoPtr thisFile, char* theExp)
{
    FilePtr 	aFilePtr;
    long	    	bytesRead, bytesWritten;
    unsigned	stringLength;


    myLseek(thisFile->fileNumber, FIBOFFSET, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &aFilePtr, sizeof(FilePtr));
    if (bytesRead != sizeof(FilePtr))
    {
	SCREAM("Error reading fileInfoBlockPtr", "setexpid", fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&aFilePtr,1);
#endif

    myLseek(thisFile->fileNumber, aFilePtr + EXPIDOFFSET, SEEK_SET);

    stringLength = strlen(theExp);
    if (stringLength > 79)
    {
	stringLength = 79;
	theExp[79] = 0;
    }
    stringLength = stringLength + 1; /* We want to write the NULL */
    bytesWritten = write(thisFile->fileNumber, theExp, stringLength);
    if (bytesWritten != (long)(stringLength))
    {
	SCREAM("Error writing theExp", "setexpid", fileWriteError,
	       nothing);
    }
    return(success);
}


/****
 *
 *	getexpid_  get the experiment ID from the file
 *
 *	INPUT:	thisFile    file info pointer
 *
 *	OUTPUTS:theExp	    experiment ID string (79 chars max + null = 80)
 *	    	result	    function result
 *
 ***/

long getexpid_(FileInfoPtr thisFile, char* theExp)
{
    FilePtr aFilePtr;
    long	    bytesRead;

    myLseek(thisFile->fileNumber, FIBOFFSET, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &aFilePtr, sizeof(FilePtr));
    if (bytesRead != sizeof(FilePtr))
    {
	SCREAM("Error reading fileInfoBlockPtr", "setexpid", fileReadError,
	       nothing);
    }


#ifndef VAXVERSION
    convertLong((long*)&aFilePtr,1);
#endif

    myLseek(thisFile->fileNumber, aFilePtr + EXPIDOFFSET, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theExp, 80);
    if (bytesRead != 80)
    {
	SCREAM("Error reading theExp", "getexpid", fileReadError,
	       nothing);
    }
    return(success);
}


/****
 *
 *	settext_  set the free form text block
 *
 *	INPUTS:	thisFile    file info pointer
 *	    	theText	    free form text (255 chars max + null = 256)
 *
 *	OUTPUT:	result	    function result
 *
 ***/

long settext_(FileInfoPtr thisFile, char* theText)
{
    FilePtr aFilePtr;
    long	    bytesRead, bytesWritten;
    short   stringLength;

    myLseek(thisFile->fileNumber, FIBOFFSET, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &aFilePtr, sizeof(FilePtr));
    if (bytesRead != sizeof(FilePtr))
    {
	SCREAM("Error reading fileInfoBlockPtr", "settext", fileReadError,
	       nothing);
    }


#ifndef VAXVERSION
    convertLong((long*)&aFilePtr,1);
#endif

    myLseek(thisFile->fileNumber, aFilePtr + TEXTOFFSET, SEEK_SET);
    stringLength = (short)strlen(theText);
    if (stringLength > 255)
    {
	stringLength = 255;
	theText[255] = 0;
    }
    bytesWritten = write(thisFile->fileNumber, theText, stringLength + 1);
    if (bytesWritten != (stringLength + 1))
    {
	SCREAM("Error writing theExp", " settext", fileWriteError,
	       nothing);
    }
    return(success);
}


/****
 *
 *	gettext_  get the free form text block
 *
 *	INPUT:	thisFile    file info pointer
 *
 *	OUTPUTS:theText	    free form text string (255 chars max + null=256)
 *	    	result	    function result
 *
 ***/

 long gettext_(FileInfoPtr thisFile, char* theText)
{
    FilePtr 	aFilePtr;
    long       	bytesRead;

    myLseek(thisFile->fileNumber, FIBOFFSET, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &aFilePtr, sizeof(FilePtr));
    if (bytesRead != sizeof(FilePtr))
    {
	SCREAM("Error reading fileInfoBlockPtr", " gettext", fileReadError,
	       nothing);
    }


#ifndef VAXVERSION
    convertLong((long*)&aFilePtr,1);
#endif

    myLseek(thisFile->fileNumber, aFilePtr + TEXTOFFSET, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theText, 256);
    if (bytesRead != 256)
    {
	SCREAM("Error reading theText",  "gettext", fileReadError,
	       nothing);
    }
    return(success);
}

/****
 *
 *	setauditstring_  set the dymanic size audit string
 *
 *	INPUT:	thisFile    file info pointer
 *
 *	OUTPUTS: result	    function result
 *
 ***/
long setauditstring_(FileInfoPtr thisFile, char* theString)
{
    int    bytesWritten, length;
    FilePtr  location;

    length = strlen(theString);
    length += 1; /* make room for the trailing NULL */
    location = myLseek(thisFile->fileNumber, 0, SEEK_END);
#ifndef VAXVERSION
    convertLong((long*)&length, 1);
#endif

    bytesWritten = write(thisFile->fileNumber, (long*)&length, sizeof(long));
    if (bytesWritten != sizeof(long))
    {
	SCREAM("Error writing audit string length", "setauditstring",
	       fileWriteError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&length, 1);
#endif
    bytesWritten = write(thisFile->fileNumber, theString, length + 1);
    if (bytesWritten != length + 1)
    {
	SCREAM("Error writing audit string", "setauditstring",
	       fileWriteError,
	       nothing);
    }
    /* now patch up the file header to point to the audit string */
    myLseek(thisFile->fileNumber, ASOFFSET, SEEK_SET);
#ifndef VAXVERSION
    convertLong((long*)&location, 1);
#endif
    bytesWritten = write(thisFile->fileNumber,
			 (char*) &location, sizeof(FilePtr));
    if (bytesWritten != sizeof(FilePtr))
    {
	SCREAM("Error writing audit string location", "setauditstring",
	       fileWriteError,
	       nothing);
    }
    return(success);
		       
}

/****
 *
 *	getauditstringlength_  get the length of the audit string
 *
 *	INPUT:	thisFile    file info pointer
 *
 *	OUTPUTS:theLength   length of the audit string
 *		result	    function result
 *
 ***/
long getauditstringlength_(FileInfoPtr thisFile, long* theLength)
{
    FilePtr location;
    long    headerSize, bytesRead;

    myLseek(thisFile->fileNumber, 0, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &headerSize, sizeof(long));
    if (bytesRead != sizeof(long))
    {
	SCREAM("Error reading file header size","getauditstringlength",
	       fileReadError, nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)&headerSize, 1);
#endif
    if (!(headerSize > 40))   /* old file before audit string was born */
    {
	*theLength = 0;
	return(success);
    }
#ifndef VAXVERSION
    convertLong((long*)&headerSize, 1);
#endif

    myLseek(thisFile->fileNumber, ASOFFSET, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &location, sizeof(FilePtr));
    if (bytesRead != sizeof(long))
    {
	SCREAM("Error reading audit string location", "getauditstringlength",
	       fileReadError, nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)&location, 1);
#endif
    if (location == 0)   /* audit string was never written to this file*/
    {
	*theLength = 0;
	return(success);
    }
    else
    {
	myLseek(thisFile->fileNumber, location, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, theLength, sizeof(long));
    if (bytesRead != sizeof(long))
    {
	SCREAM("Error reading audit string length", "getauditstringlength",
	       fileReadError, nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)theLength, 1);
#endif
    return(success);
    }
}

/****
 *
 *	getauditstring_  get the audit string
 *
 *	INPUT:	thisFile    file info pointer
 *		maxLength   maximum length the caller can swallow
 *
 *	OUTPUTS:theString   the audit string
 *		result	    function result
 *
 ***/

long getauditstring_(FileInfoPtr thisFile, long maxLength, char* theString)
{
    FilePtr location;
    long    headerSize, bytesRead, theLength, bytesToRead;
    Boolean terminate;

    myLseek(thisFile->fileNumber, 0, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &headerSize, sizeof(long));
    if (bytesRead != sizeof(long))
    {
	SCREAM("Error reading file header size","getauditstring",
	       fileReadError, nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)&headerSize, 1);
#endif
    if (!(headerSize > 40))   /* old file before audit string was born */
    {
	return(noAuditString);
    }
#ifndef VAXVERSION
    convertLong((long*)&headerSize, 1);
#endif

    myLseek(thisFile->fileNumber, ASOFFSET, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &location, sizeof(FilePtr));
    if (bytesRead != sizeof(long))
    {
	SCREAM("Error reading audit string location", "getauditstring",
	       fileReadError, nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)&location, 1);
#endif
    if (location == 0)   /* audit string was never written to this file*/
    {
	return(noAuditString);
    }
    else
    {
	myLseek(thisFile->fileNumber, location, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &theLength, sizeof(long));
	if (bytesRead != sizeof(long))
	{
	    SCREAM("Error reading audit string length", "getauditstring",
		   fileReadError, nothing);
	}
#ifndef VAXVERSION
	convertLong((long*)&theLength, 1);
#endif

	if (maxLength < theLength) /* caller can't hold it all */
	{
	    bytesToRead = maxLength - 1; /* make room for the trailing NULL */
	    terminate = TRUE;
	}
	else
	{
	    bytesToRead = theLength;
	    terminate = FALSE;
	}
	bytesRead = read(thisFile->fileNumber, theString, bytesToRead);
	if (bytesRead != bytesToRead)
	{
	    SCREAM("Error reading audit string", "getauditstring",
		   fileReadError, nothing);
	}
	if (terminate)
	{
	    theString[maxLength] = '\0';
	}
	return(success);
    }
}
