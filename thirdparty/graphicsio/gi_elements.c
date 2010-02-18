#include "graphicsio.h"


extern char nothing[];

/***
 *
 *	setelements_ set the elements for the current surface
 *
 *	INPUTS:	thisFile    	    file info pointer
 *	    	sizeOfElements	    size of elements (tri = 3 tetra = 4)
 *	    	numberOfElements    number of elements
 *	    	theElements 	    the element data
 *
 *	OUTPUT:	result	    	    function result
 *
 ***/

long setelements_(FileInfoPtr thisFile, long sizeOfElements, 
		  long numberOfElements, long *theElements)
{
    ElementHeaderPtr   		elementHeaderPtr;
    FilePtr 	    	    	elementHeaderLocation;
    long    	    	    	bytesWritten;


    if ((thisFile->theCurrentSurface)->elementHeader == 0)
    {
		/* elements for this surface do not exist yet */
		/* set up the new element header and initialize it */
	elementHeaderPtr = (ElementHeaderPtr)calloc(1,sizeof(ElementHeader));
	if (elementHeaderPtr == 0)
	{
	    SCREAM("No memory available", "setelements", noDynamicMemory,
		   nothing);
	}
	elementHeaderPtr->headerSize = sizeof(ElementHeader);
	elementHeaderPtr->numberOfElements = numberOfElements;
	elementHeaderPtr->sizeOfElements = sizeOfElements;
	    	/* write the element header and the elements to the file */
	elementHeaderLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
	elementHeaderPtr->theElements = elementHeaderLocation +
	    	    	    	    sizeof(ElementHeader);

#ifndef VAXVERSION
	convertLong((long*)&(elementHeaderPtr->headerSize), 4);
#endif

	bytesWritten = write(thisFile->fileNumber, elementHeaderPtr,
			     sizeof(ElementHeader));

	if (bytesWritten != sizeof(ElementHeader))
	{
	    SCREAM("Error writing element header", "setelements", 
		   fileWriteError,
		   nothing);
	}
#ifndef VAXVERSION
	convertLong((long*)theElements, numberOfElements * sizeOfElements);
#endif

	bytesWritten = write(thisFile->fileNumber, theElements,
			     numberOfElements * sizeOfElements * sizeof(long));

	if (bytesWritten != 
	    (long)(numberOfElements * sizeOfElements * sizeof(long)))
	{
	    SCREAM("Error writing elements", "setelements", fileWriteError,
		   nothing);
	}

#ifndef VAXVERSION
	convertLong((long*)theElements, numberOfElements * sizeOfElements);
#endif

	    /* now patch up the information in memory and on disk
	       about this surface and its elements */

	(thisFile->theCurrentSurface)->elementHeader = elementHeaderLocation;
	myLseek(thisFile->fileNumber, 
		       thisFile->theCurrentSurfaceLocation +
		       EHOFFSET, SEEK_SET);

#ifndef VAXVERSION
	convertLong((long*)&elementHeaderLocation, 1);
#endif

	bytesWritten = write(thisFile->fileNumber, &elementHeaderLocation,
			     sizeof(FilePtr));

	if (bytesWritten != sizeof(FilePtr))
	{
	    SCREAM("Error writing element header location", "setelements",
		   fileWriteError,
		   nothing);
	}
	return(success);
    }
    else
    {
	    /* We must be rewriting existing elements */
	SCREAM("Rewriting elements is not supported", "setelements",
	       invalidOperation,
	       nothing);
    }
}

/****
 *
 *	getelementinfo_ get information about the nodes on the current surface
 *
 *	INPUT:	thisFile    	    	file info pointer
 *
 *	OUTPUTS:numberOfElements    	number of elements
 *	    	elementSize 	    	size of elements
 *	    	numberOfScalarValues	number of scalar values
 *	    	numberOfVectorValues	number of vector values
 *	    	numberOfTensorValues	number of tensor values
 *	    	result	    	    	function result
 *
 ***/
	
long getelementinfo_(FileInfoPtr thisFile, long *numberOfElements,
		  long *elementSize, long *numberOfScalarValues, 
		  long *numberOfVectorValues, long *numberOfTensorValues)
{
    ElementHeader     	    temp;
    long	    	    	    bytesRead;

    if ((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	*numberOfElements = 0;
	return(success);
    }
    myLseek(thisFile->fileNumber, (thisFile->theCurrentSurface)->
	  elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &temp, sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading element header", "getelementinfo",
	       fileReadError,
	       nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)&(temp.headerSize), 10);
#endif

    *numberOfElements = temp.numberOfElements;
    *elementSize = temp.sizeOfElements;
    *numberOfScalarValues = temp.numberOfElementScalarValues;
    *numberOfVectorValues = temp.numberOfElementVectorValues;
    *numberOfTensorValues = temp.numberOfElementTensorValues;
    return(success);
}


/****
 *
 *	getelements_ get the elements for the current surface
 *
 *	INPUT:	thisFile    file info pointer
 *
 *	OUTPUTS:theElements the element data
 *	    	result	    function result
 *
 ***/

long getelements_(FileInfoPtr thisFile, long *theElements)
{
    long		    	bytesRead;
    ElementHeader   	temp;

    myLseek(thisFile->fileNumber, (thisFile->theCurrentSurface)->
	  elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &temp, sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading element header", "getelements",
	       fileReadError,
	       nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)&(temp.headerSize), 10);
#endif

    myLseek(thisFile->fileNumber, temp.theElements, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theElements,
		     temp.numberOfElements * temp.sizeOfElements * 
		     sizeof(long));

    if (bytesRead != (long)(temp.numberOfElements * temp.sizeOfElements *
			    sizeof(long)))
    {
	SCREAM("Error reading the elements", "getelements", fileReadError,
	       nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)theElements, temp.numberOfElements * 
	       temp.sizeOfElements);
#endif
    return(success);
}
/****
 *
 *	setelementscalars_
 *
 ***/

long setelementscalars_(FileInfoPtr thisFile, long theType,
		     long numberOfScalars, ScalarPtr theScalarData)
{
    ElementHeader	theElementHeader;
    long	    	    	bytesRead, bytesWritten, numberOfScalarValues;
    FilePtr 	    	scalarHeaderLocation, previousHeaderLocation;
    ScalarValueHeader	theScalarHeader, tempHeader;
    long	    	    	loop;

    if((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	SCREAM("Elements do not exist", "setelementscalars", noElements,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theElementHeader, 
		     sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading element header", "setelementscalars",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&theElementHeader, 6);
#endif

    if (theElementHeader.numberOfElements != numberOfScalars)
    {
	SCREAM("Wrong number of scalars", "setelementscalars",
	       wrongNumber,
	       nothing);
    }

#ifndef VAXVERSION
    convertFloat((float*)theScalarData, numberOfScalars,FALSE);
#endif

    theScalarHeader.headerSize = sizeof(ScalarValueHeader);
    theScalarHeader.nextScalarValueHeader = 0;
    theScalarHeader.scalarValueType = theType;

    if (theElementHeader.numberOfElementScalarValues == 0) 
    {	/*  This is first of the Scalar Values */
	scalarHeaderLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
	theScalarHeader.theScalars = scalarHeaderLocation + 
	    	    	    	    	sizeof(ScalarValueHeader);
#ifndef VAXVERSION
	convertLong((long*)&theScalarHeader, 4);
#endif
	bytesWritten = write(thisFile->fileNumber, &theScalarHeader,
			     sizeof(ScalarValueHeader));

	if (bytesWritten != sizeof(ScalarValueHeader))
	{
	    SCREAM("Error writing scalar value header", "setelementscalars",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theScalarData,
			     numberOfScalars * sizeof(long));

	if (bytesWritten != (long)(numberOfScalars * sizeof(long)))
	{
	    SCREAM("Error writing the scalars", "setelementscalars",
		   fileWriteError,
		   nothing);
	}


	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->elementHeader + NOESVOFFSET,
	      SEEK_SET);
	numberOfScalarValues = 1;

#ifndef VAXVERSION
	convertLong((long*)&scalarHeaderLocation, 1);
	convertLong((long*)&numberOfScalarValues, 1);
#endif
    
	bytesWritten = write(thisFile->fileNumber, &numberOfScalarValues,
			     sizeof(long));

	if (bytesWritten != sizeof(long))
	{
	  SCREAM("Error writing number of scalar values", "setelementscalars",
		 fileWriteError,
		 nothing);
	}

	bytesWritten = write(thisFile->fileNumber, &scalarHeaderLocation, 
			     sizeof(FilePtr));

	if (bytesWritten != sizeof(FilePtr))
	{
	   SCREAM("Error writing scalar header location", "setelementscalars",
		  fileWriteError,
		  nothing);
	}

    }
    else
    {	/* This is an additional set of Scalar Values */
	previousHeaderLocation = theElementHeader.
	    	    	    	 firstElementScalarValueHeader;
	for (loop = 1; loop < theElementHeader.numberOfElementScalarValues;
	     loop ++)
	{
	    myLseek(thisFile->fileNumber, previousHeaderLocation, SEEK_SET);
	    bytesRead = read(thisFile->fileNumber, &tempHeader,
			     sizeof(ScalarValueHeader));
	    
	    if (bytesRead != sizeof(ScalarValueHeader))
	    {
	      SCREAM("Error reading scalar value header", "setelementscalars",
		     fileReadError,
		     nothing);
	    }
#ifndef VAXVERSION
	 convertLong((long*)&tempHeader, 4);
#endif
	    if (tempHeader.nextScalarValueHeader != 0)
	    previousHeaderLocation = tempHeader.nextScalarValueHeader;
	}
	scalarHeaderLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
	theScalarHeader.theScalars = scalarHeaderLocation + 
	    	    	    	    	sizeof(ScalarValueHeader);
#ifndef VAXVERSION
	convertLong((long*)&theScalarHeader, 4);
#endif
	bytesWritten = write(thisFile->fileNumber, &theScalarHeader,
			     sizeof(ScalarValueHeader));

	if (bytesWritten != sizeof(ScalarValueHeader))
	{
	    SCREAM("Error writing scalar value header", "setelementscalars",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theScalarData,
			     numberOfScalars * sizeof(long));

	if (bytesWritten != (long)(numberOfScalars * sizeof(long)))
	{
	    SCREAM("Error writing scalars", "setelementscalars",
		   fileWriteError,
		   nothing);
	}


	myLseek(thisFile->fileNumber, previousHeaderLocation + NSVHOFFSET, 
	      SEEK_SET);
#ifndef VAXVERSION
	convertLong((long*)&scalarHeaderLocation, 1);
#endif
	bytesWritten = write(thisFile->fileNumber, &scalarHeaderLocation,
			     sizeof(FilePtr));

	if (bytesWritten != sizeof(FilePtr))
	{
	   SCREAM("Error writing scalar header location", "setelementscalars",
		  fileWriteError,
		  nothing);
       }

	numberOfScalarValues = theElementHeader.numberOfElementScalarValues 
	    	    	    	+ 1;

#ifndef VAXVERSION
	convertLong((long*)&numberOfScalarValues, 1);
#endif

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->elementHeader + NOESVOFFSET,
	      SEEK_SET);

	bytesWritten = write(thisFile->fileNumber, &numberOfScalarValues,
			     sizeof(long));
	
	if (bytesWritten != sizeof(long))
	{
          SCREAM("Error writing number of scalar values", "setelementscalars",
		 fileWriteError,
		 nothing);
	}
    }
#ifndef VAXVERSION
    convertFloat((float*)theScalarData, numberOfScalars, TRUE);
#endif
    return(success);
}

/****
 *
 *	getelementscalartypes_
 *
 ***/

long getelementscalartypes_(FileInfoPtr thisFile, long *theTypes)
{
    ElementHeader	theElementHeader;
    long	    	    	bytesRead, loop;
    FilePtr 	    	nextScalarHeader;
    ScalarValueHeader	tempScalarValueHeader;

    if((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	SCREAM("There are no elements on this surface",
	       "getelementscalartypes", noElements,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theElementHeader, 
		     sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading element header", "getelementscalartypes",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theElementHeader.headerSize), 9);
#endif

    if (theElementHeader.firstElementScalarValueHeader == 0)
    {
	SCREAM("There are no scalars associated with elements on this surface",
	       "getelementscalartypes", noScalars,
	       nothing);
    }

    nextScalarHeader = theElementHeader.firstElementScalarValueHeader;

    for (loop = 0; loop < theElementHeader.numberOfElementScalarValues; loop++)
    {
	myLseek(thisFile->fileNumber, nextScalarHeader, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempScalarValueHeader,
		  sizeof(ScalarValueHeader));
	
	if (bytesRead != sizeof(ScalarValueHeader))
	{
	  SCREAM("Error reading scalar value header", "getelementscalartypes",
		 fileReadError,
		 nothing);
	}

#ifndef VAXVERSION
    convertLong((long*)&(tempScalarValueHeader.headerSize), 4);
#endif
	theTypes[loop] = tempScalarValueHeader.scalarValueType;
	nextScalarHeader = tempScalarValueHeader.nextScalarValueHeader;
    }

    return(success);
}

/****
 *
 *	getelementscalars_
 *
 ***/

long getelementscalars_(FileInfoPtr thisFile, long scalarIndex,
		     long *theType, ScalarPtr theScalarData)
{
    ElementHeader	theElementHeader;
    long	       	bytesRead, loop;
    FilePtr 	    	scalarHeaderLocation;
    ScalarValueHeader	tempScalarHeader;

    if((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	SCREAM("There are no elements on this surface", "getelementscalars",
	       noElements,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theElementHeader, 
		     sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading element header", "getelementscalars",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theElementHeader.headerSize), 9);
#endif

    if (theElementHeader.numberOfElementScalarValues == 0)
    {
	SCREAM("There are no scalars associated with elements on this surface",
	       "getelementscalars", noScalars,
	       nothing);
    }
    
    scalarHeaderLocation = theElementHeader.firstElementScalarValueHeader;

    for (loop = 0; loop < scalarIndex; loop++)
    {
	myLseek(thisFile->fileNumber, 
			 scalarHeaderLocation, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempScalarHeader,
			 sizeof(ScalarValueHeader));

	if (bytesRead != sizeof(ScalarValueHeader))
	{
	    SCREAM("Error reading scalar value header", "getelementscalars",
		   fileReadError,
		   nothing);
	}

#ifndef VAXVERSION
    convertLong((long*)&(tempScalarHeader.headerSize), 4);
#endif
	scalarHeaderLocation = tempScalarHeader.nextScalarValueHeader;
    }
    *theType = tempScalarHeader.scalarValueType;

    myLseek(thisFile->fileNumber, tempScalarHeader.theScalars,
		     SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theScalarData,
		     theElementHeader.numberOfElements * sizeof(long));

    if (bytesRead != (long)(theElementHeader.numberOfElements * sizeof(long)))
    {
	SCREAM("Error reading the scalars", "getelementscalars",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertFloat((float*)theScalarData, theElementHeader.numberOfElements,
		 TRUE);
#endif
    return(success);
}





/****
 *
 *	setelementvectors_
 *
 ***/

long setelementvectors_(FileInfoPtr thisFile, long theType,
		     long numberOfVectors, VectorPtr theVectorData)
{
    ElementHeader	theElementHeader;
    long	    	    	bytesRead, bytesWritten, numberOfVectorValues;
    FilePtr 	    	vectorHeaderLocation, previousHeaderLocation;
    VectorValueHeader	theVectorHeader, tempHeader;
    long	    	    	loop;

    if((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	SCREAM("Elements do not exist", "setelementvectors",
	       noElements,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theElementHeader, 
		     sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading the element header", "setelementvectors",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&theElementHeader, 10);
#endif

    if (theElementHeader.numberOfElements != numberOfVectors)
    {
	SCREAM("Wrong number of vectors", "setelementvectors",
	       wrongNumber,
	       nothing);
    }

#ifndef VAXVERSION
    convertFloat((float*)theVectorData, 3 * numberOfVectors, FALSE);
#endif

    theVectorHeader.headerSize = sizeof(VectorValueHeader);
    theVectorHeader.nextVectorValueHeader = 0;
    theVectorHeader.vectorValueType = theType;

    if (theElementHeader.numberOfElementVectorValues == 0) 
    {	/*  This is first of the Vector Values */
	vectorHeaderLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
	theVectorHeader.theVectors = vectorHeaderLocation + 
	    	    	    	    	sizeof(VectorValueHeader);
#ifndef VAXVERSION
	convertLong((long*)&theVectorHeader, 4);
#endif
	bytesWritten = write(thisFile->fileNumber, &theVectorHeader,
			     sizeof(VectorValueHeader));

	if (bytesWritten != sizeof(VectorValueHeader))
	{
	    SCREAM("Error writing vector value header", "setelementvectors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theVectorData,
			     numberOfVectors * sizeof(Vector));

	if (bytesWritten != (long)(numberOfVectors * sizeof(Vector)))
	{
	    SCREAM("Error writing vectors", "setelementvectors",
		   fileWriteError,
		   nothing);
	}

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->elementHeader + NOEVVOFFSET,
	      SEEK_SET);
	numberOfVectorValues = 1;

#ifndef VAXVERSION
	convertLong((long*)&vectorHeaderLocation, 1);
	convertLong((long*)&numberOfVectorValues, 1);
#endif
    
	bytesWritten = write(thisFile->fileNumber, &numberOfVectorValues,
			     sizeof(long));
	
	if (bytesWritten != sizeof(long))
	{
          SCREAM("Error writing number of vector values", "setelementvectors",
		 fileWriteError,
		 nothing);
	}

	bytesWritten = write(thisFile->fileNumber, &vectorHeaderLocation, 
			     sizeof(FilePtr));
	
	if (bytesWritten != sizeof(FilePtr))
	{
	   SCREAM("Error writing vector header location", "setelementvectors",
		  fileWriteError,
		  nothing);
	}
    }
    else
    {	/* This is an additional set of Vector Values */
	previousHeaderLocation = theElementHeader.
	    	    	    	 firstElementVectorValueHeader;
	for (loop = 1; loop < theElementHeader.numberOfElementVectorValues;
	     loop ++)
	{
	    myLseek(thisFile->fileNumber, previousHeaderLocation, SEEK_SET);
	    bytesRead = read(thisFile->fileNumber, &tempHeader,
			     sizeof(VectorValueHeader));

	    if (bytesRead != sizeof(VectorValueHeader))
	    {
		SCREAM("Error reading vector value header", 
		       "setelementvectors", fileReadError,
		       nothing);
	    }
	    
#ifndef VAXVERSION
	 convertLong((long*)&tempHeader, 4);
#endif
	    if (tempHeader.nextVectorValueHeader != 0)
	    previousHeaderLocation = tempHeader.nextVectorValueHeader;
	}
	vectorHeaderLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
	theVectorHeader.theVectors = vectorHeaderLocation + 
	    	    	    	    	sizeof(VectorValueHeader);
#ifndef VAXVERSION
	convertLong((long*)&theVectorHeader, 4);
#endif
	bytesWritten = write(thisFile->fileNumber, &theVectorHeader,
			     sizeof(VectorValueHeader));

	if (bytesWritten != sizeof(VectorValueHeader))
	{
	    SCREAM("Error writing vector value header", "setelementvectors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theVectorData,
			     numberOfVectors * sizeof(Vector));

	if (bytesWritten != (long)(numberOfVectors * sizeof(Vector)))
	{
	    SCREAM("Error writing vectors", "setelementvectors",
		   fileWriteError,
		   nothing);
	}

	myLseek(thisFile->fileNumber, previousHeaderLocation + NVVHOFFSET, 
	      SEEK_SET);
#ifndef VAXVERSION
	convertLong((long*)&vectorHeaderLocation, 1);
#endif
	bytesWritten = write(thisFile->fileNumber, &vectorHeaderLocation,
			     sizeof(FilePtr));
	
	if (bytesWritten != sizeof(FilePtr))
	{
	    SCREAM("Error writing vector value header", "setelementvectors",
		   fileWriteError,
		   nothing);
	}

	numberOfVectorValues = theElementHeader.numberOfElementVectorValues
	    	    	       + 1;

#ifndef VAXVERSION
	convertLong((long*)&numberOfVectorValues, 1);
#endif

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->elementHeader + NOEVVOFFSET,
	      SEEK_SET);

	bytesWritten = write(thisFile->fileNumber, &numberOfVectorValues,
			     sizeof(long));
	
	if (bytesWritten != sizeof(long))
	{
	    SCREAM("Error writing number of vector values",
		   "setelementvectors", fileWriteError,
		   nothing);
	}
    }
    return(success);
}

/****
 *
 *	getelementvectortypes_
 *
 ***/

long getelementvectortypes_(FileInfoPtr thisFile, long *theTypes)
{
    ElementHeader	theElementHeader;
    long	    	    	bytesRead, loop;
    FilePtr 	    	nextVectorHeader;
    VectorValueHeader	tempVectorValueHeader;

    if((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	SCREAM("There are no elements on this surface",
	       "setelementvectortypes", noElements,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theElementHeader, 
		     sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading element header", "getelementvectortypes",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theElementHeader.headerSize), 10);
#endif

    if (theElementHeader.firstElementVectorValueHeader == 0)
    {
	SCREAM("There are no vectors associated with elements on this surface",
	       "getelementvectortypes", noVectors,
	       nothing);
    }

    nextVectorHeader = theElementHeader.firstElementVectorValueHeader;

    for (loop = 0; loop < theElementHeader.numberOfElementVectorValues; loop++)
    {
	myLseek(thisFile->fileNumber, nextVectorHeader, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempVectorValueHeader,
		  sizeof(VectorValueHeader));

	if (bytesRead != sizeof(VectorValueHeader))
	{
          SCREAM("Error reading vector value header", "getelementvectortypes",
		 fileReadError,
		 nothing);
	}

#ifndef VAXVERSION
    convertLong((long*)&(tempVectorValueHeader.headerSize), 4);
#endif
	theTypes[loop] = tempVectorValueHeader.vectorValueType;
	nextVectorHeader = tempVectorValueHeader.nextVectorValueHeader;
    }

    return(success);
}

/****
 *
 *	getelementvectors_
 *
 ***/

long getelementvectors_(FileInfoPtr thisFile, long vectorIndex,
		     long *theType, VectorPtr theVectorData)
{
    ElementHeader	theElementHeader;
    long	       	bytesRead, loop;
    FilePtr 	    	vectorHeaderLocation;
    VectorValueHeader	tempVectorHeader;

    if((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	SCREAM("There are no elements on this surface", "getelementvectors",
	       noElements,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theElementHeader, 
		     sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading element header", "getelementvectortypes",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theElementHeader.headerSize), 10);
#endif

    if (theElementHeader.numberOfElementVectorValues == 0)
    {
	SCREAM("There are no vectors associated with elements on this surface",
	       "getelementvectortypes", noVectors,
	       nothing);
    }

    vectorHeaderLocation = theElementHeader.firstElementVectorValueHeader;

    for (loop = 0; loop < vectorIndex; loop++)
    {
	myLseek(thisFile->fileNumber, 
			 vectorHeaderLocation, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempVectorHeader,
			 sizeof(VectorValueHeader));

	if (bytesRead != sizeof(VectorValueHeader))
	{
	    SCREAM("Error reading vector value header", "getelementvectors",
		   fileReadError,
		   nothing);
	}
	
#ifndef VAXVERSION
    convertLong((long*)&(tempVectorHeader.headerSize), 4);
#endif
	vectorHeaderLocation = tempVectorHeader.nextVectorValueHeader;
    }
    *theType = tempVectorHeader.vectorValueType;

    myLseek(thisFile->fileNumber, tempVectorHeader.theVectors,
		     SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theVectorData,
		     theElementHeader.numberOfElements * sizeof(Vector));

    if (bytesRead != (long)(theElementHeader.numberOfElements * 
			    sizeof(Vector)))
    {
	SCREAM("Error reading vectors", "getelementvectors",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertFloat((float*)theVectorData, 3 * theElementHeader.numberOfElements,
		 TRUE);
#endif
    return(success);
}



/****
 *
 *	setelementtensors_
 *
 ***/

long setelementtensors_(FileInfoPtr thisFile, long theType, long theDimension,
		     long numberOfTensors, TensorPtr theTensorData)
{
    ElementHeader	theElementHeader;
    long	    	    	bytesRead, bytesWritten, numberOfTensorValues;
    FilePtr 	    	tensorHeaderLocation, previousHeaderLocation;
    TensorValueHeader	theTensorHeader, tempHeader;
    long	    	    	loop;

    if((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	SCREAM("Elements must be written before tensors",
	       "setelementtensors", noElements,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theElementHeader, 
		     sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading element header", "setelementheader",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&theElementHeader, 10);
#endif

    if (theElementHeader.numberOfElements != numberOfTensors)
    {
	SCREAM("Wrong number of tensors", "setelementtensors",
	       wrongNumber,
	       nothing);
    }

#ifndef VAXVERSION
    convertFloat((float*)theTensorData, 
		 theDimension * theDimension  * numberOfTensors, FALSE);
#endif

    theTensorHeader.headerSize = sizeof(TensorValueHeader);
    theTensorHeader.nextTensorValueHeader = 0;
    theTensorHeader.tensorValueType = theType;
    theTensorHeader.tensorDimension = theDimension;

    if (theElementHeader.numberOfElementTensorValues == 0) 
    {	/*  This is first of the Tensor Values */
	tensorHeaderLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
	theTensorHeader.theTensors = tensorHeaderLocation + 
	    	    	    	    	sizeof(TensorValueHeader);
#ifndef VAXVERSION
	convertLong((long*)&theTensorHeader, 5);
#endif
	bytesWritten = write(thisFile->fileNumber, &theTensorHeader,
			     sizeof(TensorValueHeader));

	if (bytesWritten != sizeof(TensorValueHeader))
	{
	    SCREAM("Error writing tensor value header", "setelementtensors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theTensorData,
			     theDimension * theDimension * numberOfTensors
			      * sizeof(float));

	if (bytesWritten != (long)(theDimension * theDimension * 
				   numberOfTensors * sizeof(float)))
	{
	    SCREAM ("Error writing the tensors", "setelementtensors",
		    fileWriteError,
		    nothing);
	}

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->elementHeader + NOETVOFFSET,
	      SEEK_SET);
	numberOfTensorValues = 1;

#ifndef VAXVERSION
	convertLong((long*)&tensorHeaderLocation, 1);
	convertLong((long*)&numberOfTensorValues, 1);
#endif
    
	bytesWritten = write(thisFile->fileNumber, &numberOfTensorValues,
			     sizeof(long));
	if (bytesWritten != sizeof(long))
	{
	    SCREAM("Error writing number of tensor values",
		   "setelementtensors", fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, &tensorHeaderLocation, 
			     sizeof(FilePtr));
	if (bytesWritten != sizeof(FilePtr))
	{
	    SCREAM("Error writing tensor header location", "setelementtensors",
		   fileWriteError,
		   nothing);
	}
    }
    else
    {	/* This is an additional set of Tensor Values */
	previousHeaderLocation = theElementHeader.
	    	    	    	 firstElementTensorValueHeader;
	for (loop = 1; loop < theElementHeader.numberOfElementTensorValues;
	     loop ++)
	{
	    myLseek(thisFile->fileNumber, previousHeaderLocation, SEEK_SET);
	    bytesRead = read(thisFile->fileNumber, &tempHeader,
			     sizeof(TensorValueHeader));
	    if (bytesRead != sizeof(TensorValueHeader))
	    {
		SCREAM("Error reading tensor value header",
		       "setelementtensors", fileReadError,
		       nothing);
	    }

#ifndef VAXVERSION
	 convertLong((long*)&tempHeader, 4);
#endif
	    if (tempHeader.nextTensorValueHeader != 0)
	    previousHeaderLocation = tempHeader.nextTensorValueHeader;
	}
	tensorHeaderLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
	theTensorHeader.theTensors = tensorHeaderLocation + 
	    	    	    	    	sizeof(TensorValueHeader);
#ifndef VAXVERSION
	convertLong((long*)&theTensorHeader, 5);
#endif
	bytesWritten = write(thisFile->fileNumber, &theTensorHeader,
			     sizeof(TensorValueHeader));

	if (bytesWritten != sizeof(TensorValueHeader))
	{
	    SCREAM("Error writing tensor value header", "setelementtensors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theTensorData,
			     theDimension * theDimension * numberOfTensors
			      * sizeof(float));

	if (bytesWritten != (long)(theDimension * theDimension * 
				   numberOfTensors * sizeof(float)))
	{
	    SCREAM ("Error writing the tensors", "setelementtensors",
		    fileWriteError,
		    nothing);
	}

	myLseek(thisFile->fileNumber, previousHeaderLocation + NTVHOFFSET, 
	      SEEK_SET);
#ifndef VAXVERSION
	convertLong((long*)&tensorHeaderLocation, 1);
#endif
	bytesWritten = write(thisFile->fileNumber, &tensorHeaderLocation,
			     sizeof(FilePtr));
	if (bytesWritten != sizeof(FilePtr))
	{
          SCREAM("Error writing tensor header location", "setelementtensors",
		 fileWriteError,
		 nothing);
	}
	numberOfTensorValues = theElementHeader.numberOfElementTensorValues
	    	    	       + 1;

#ifndef VAXVERSION
	convertLong((long*)&numberOfTensorValues, 1);
#endif

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->elementHeader + NOETVOFFSET,
	      SEEK_SET);

	bytesWritten = write(thisFile->fileNumber, &numberOfTensorValues,
			     sizeof(long));
	if (bytesWritten != sizeof(long))
	{
	   SCREAM("Error writing number of tensor values",
		  "setelementtensors", fileWriteError,
		  nothing);
	}
    }
    return(success);
}

/****
 *
 *	getelementtensortypes_
 *
 ***/

long getelementtensortypes_(FileInfoPtr thisFile, long *theTypes, 
			 long *theDimensions)
{
    ElementHeader	theElementHeader;
    long	    	    	bytesRead, loop;
    FilePtr 	    	nextTensorHeader;
    TensorValueHeader	tempTensorValueHeader;

    if((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	SCREAM("There are no elements on this surface",
	       "getelementtensortypes", noElements,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theElementHeader, 
		     sizeof(ElementHeader));

    if (bytesRead != sizeof(ElementHeader))
    {
	SCREAM("Error reading element header", "getelementtensortypes",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theElementHeader.headerSize), 10);
#endif

    if (theElementHeader.firstElementTensorValueHeader == 0)
    {
	SCREAM("There are no tensors associated with elements on this surface",
	       "getelementtensortypes", noTensors,
	       nothing);
    }

    nextTensorHeader = theElementHeader.firstElementTensorValueHeader;

    for (loop = 0; loop < theElementHeader.numberOfElementTensorValues; loop++)
    {
	myLseek(thisFile->fileNumber, nextTensorHeader, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempTensorValueHeader,
		  sizeof(TensorValueHeader));

	if (bytesRead != sizeof(TensorValueHeader))
	{
	    SCREAM("Error reading tensor value header",
		   "getelementtensortypes", fileReadError,
		   nothing);
	}

#ifndef VAXVERSION
    convertLong((long*)&(tempTensorValueHeader.headerSize), 5);
#endif
	theTypes[loop] = tempTensorValueHeader.tensorValueType;
	theDimensions[loop] = tempTensorValueHeader.tensorDimension;
	nextTensorHeader = tempTensorValueHeader.nextTensorValueHeader;
    }

    return(success);
}

/****
 *
 *	getelementtensors_
 *
 ***/

long getelementtensors_(FileInfoPtr thisFile, long tensorIndex,
			long *theDimension, long *theType, 
			TensorPtr theTensorData)
{
    ElementHeader	theElementHeader;
    long	       	bytesRead, loop;
    FilePtr 	    	tensorHeaderLocation;
    TensorValueHeader	tempTensorHeader;

    if((thisFile->theCurrentSurface)->elementHeader == 0)
    {
	SCREAM("There are no elements on this surface", "getelementtensors",
	       noElements,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->elementHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theElementHeader, 
		     sizeof(ElementHeader));

#ifndef VAXVERSION
    convertLong((long*)&(theElementHeader.headerSize), 10);
#endif

    if (theElementHeader.numberOfElementTensorValues == 0)
    {
	SCREAM("There are no tensors associated with elements on this surface",
	       "getelementtensors", noTensors,
	       nothing);
    }

    tensorHeaderLocation = theElementHeader.firstElementTensorValueHeader;

    for (loop = 0; loop < tensorIndex; loop++)
    {
	myLseek(thisFile->fileNumber, 
			 tensorHeaderLocation, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempTensorHeader,
			 sizeof(TensorValueHeader));

	if (bytesRead != sizeof(TensorValueHeader))
	{
	    SCREAM("Error reading tensor value header", "getelementensors",
		   fileReadError,
		   nothing);
	}

#ifndef VAXVERSION
    convertLong((long*)&(tempTensorHeader.headerSize), 5);
#endif
	tensorHeaderLocation = tempTensorHeader.nextTensorValueHeader;
    }
    *theType = tempTensorHeader.tensorValueType;
    *theDimension = tempTensorHeader.tensorDimension;

    myLseek(thisFile->fileNumber, tempTensorHeader.theTensors,
		     SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theTensorData,
		     theElementHeader.numberOfElements * 
		     tempTensorHeader.tensorDimension *
		     tempTensorHeader.tensorDimension *
		     sizeof(float));

    if (bytesRead != (long)(theElementHeader.numberOfElements *
			    tempTensorHeader.tensorDimension *
			    tempTensorHeader.tensorDimension *
			    sizeof(float)))
    {
	SCREAM("Error reading tensors", "getelementtensors",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertFloat((float*)theTensorData, 
		 tempTensorHeader.tensorDimension *
		 tempTensorHeader.tensorDimension *
		 theElementHeader.numberOfElements, TRUE);
#endif
    return(success);
}

