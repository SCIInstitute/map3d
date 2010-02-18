#include "graphicsio.h"


extern char nothing[];

/***
 *	setnodes_  set node values
 *
 *	INPUTS:	thisFile    	file info pointer
 *	    	numberOfNodes	number of nodes
 *	    	theNodes    	the node values
 *
 *	OUTPUT: result	    	function result
 *
 ***/

long setnodes_(FileInfoPtr thisFile, long numberOfNodes, NodePtr theNodes)
{
NodeHeaderPtr	    	nodeHeaderPtr;
FilePtr	    	    	nodeHeaderLocation;
long	    	    	bytesWritten;

    if ((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
		/* nodes for this surface do not exist yet */
		/* set up the new node header and initialize it */
	nodeHeaderPtr = (NodeHeaderPtr)calloc(1,sizeof(NodeHeader));
	if (nodeHeaderPtr == 0)
	{
	    SCREAM("No memory available for header", "setnodes",
		   noDynamicMemory,
		   nothing);
	}
	nodeHeaderPtr->headerSize = sizeof(NodeHeader);
	nodeHeaderPtr->numberOfNodes = numberOfNodes;

	
	    	/* write the node header and the nodes to the file */
	nodeHeaderLocation = myLseek(thisFile->fileNumber, 0, SEEK_END);
	nodeHeaderPtr->theNodes = nodeHeaderLocation +
	    	    	    	    sizeof(NodeHeader);
#ifndef VAXVERSION
	convertLong((long*)&(nodeHeaderPtr->headerSize), 3);
#endif
	bytesWritten = write(thisFile->fileNumber, nodeHeaderPtr,
			     sizeof(NodeHeader));

	if (bytesWritten != sizeof(NodeHeader))
	{
	    SCREAM("Error writing the node header", "setnodes",
		   fileWriteError,
		   nothing);
	}
#ifndef VAXVERSION
	convertFloat((float*)theNodes, numberOfNodes * 3, FALSE);
#endif

	bytesWritten = write(thisFile->fileNumber, theNodes,
			     numberOfNodes * sizeof(Node));

	if (bytesWritten != (long) (numberOfNodes * sizeof(Node)))
	{
	    SCREAM("Error writing the nodes", "setnodes", fileWriteError,
		   nothing);
	}

#ifndef VAXVERSION
	convertFloat((float*)theNodes, numberOfNodes * 3, TRUE);
#endif

	    /* now patch up the information in memory and on disk
	       about this surface and its nodes */

	(thisFile->theCurrentSurface)->nodeHeader = nodeHeaderLocation;
	myLseek(thisFile->fileNumber, thisFile->theCurrentSurfaceLocation +
	      NHOFFSET, SEEK_SET);

#ifndef VAXVERSION
	convertLong((long*)&nodeHeaderLocation, 1);
#endif

	bytesWritten = write(thisFile->fileNumber, &nodeHeaderLocation,
			     sizeof(FilePtr));

	if (bytesWritten != sizeof(FilePtr))
	{
	    SCREAM("Error writing node header location", "setnodes",
		   fileWriteError,
		   nothing);
	}
	return(success);
    }
    else
    {
	    /* We must be rewriting existing nodes */
	SCREAM("Rewriting nodes is not supported", "setnodes",
	       invalidOperation,
	       nothing);
    }
}

/****
 *
 *	getnodeinfo_ get information about nodes on the current surface
 *
 *	INPUT:	thisFile    	    	file info pointer
 *
 *	OUTPUTS:numberOfNodes	    	number of nodes on this surface
 *	    	numberOfScalarValues	number of scalar values
 *	    	numberOfVectorValues	number of vector values
 *	    	numberOfTensorValues	number of tensor values
 *	    	result	    	    	function result
 *
 ***/

long getnodeinfo_(FileInfoPtr thisFile, long *numberOfNodes,
		  long *numberOfScalarValues, long *numberOfVectorValues,
		  long *numberOfTensorValues)
{
    NodeHeader		    temp;
    long	    	    	    bytesRead;

    if ((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	*numberOfNodes = 0;
	return(success);
    }

    myLseek(thisFile->fileNumber, (thisFile->theCurrentSurface)->
	  nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &temp, sizeof(NodeHeader));

    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "getnodeinfo", fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
	convertLong((long*)&(temp.headerSize), 9);
#endif

    *numberOfNodes = temp.numberOfNodes;
    *numberOfScalarValues = temp.numberOfNodeScalarValues;
    *numberOfVectorValues = temp.numberOfNodeVectorValues;
    *numberOfTensorValues = temp.numberOfNodeTensorValues;
    return(success);
}


/****
 *
 *	getnodes_  get the node values from the current surface
 *
 *	INPUT:	thisFile    	file info pointer
 *
 *	OUTPUTS:theNodeData 	the node values
 *	    	result	    	function result
 *
 ***/

long getnodes_(FileInfoPtr thisFile, NodePtr theNodeData)
{
    long		    	bytesRead;
    NodeHeader		temp;

    myLseek(thisFile->fileNumber, (thisFile->theCurrentSurface)->
	  nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &temp, sizeof(NodeHeader));

    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "getnodes", fileReadError,
	       nothing);
    }
    
#ifndef VAXVERSION
	convertLong(&(temp.headerSize), 6);
#endif

    myLseek(thisFile->fileNumber, temp.theNodes, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, theNodeData,
	      sizeof(Node) * temp.numberOfNodes);

    if (bytesRead != (long)(sizeof(Node) * temp.numberOfNodes))
    {
	SCREAM("Error reading the nodes", "getnodes", fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
	convertFloat((float*)theNodeData, temp.numberOfNodes * 3, TRUE);
#endif
    return(success);
}

/****
 *
 *	setnodescalars_  set a set of scalars for the current surface
 *
 *	INPUTS:	thisFile    	    file info pointer
 *	    	theType	    	    scalar type (see graphicsio.h 
 *	    	    	    	    for definitions)
 *	    	numberOfScalars	    number of scalars (must match number of
 *	    	    	    	    nodes)
 *	    	theScalarData	    the scalar data
 *
 *	OUTPUT: result	    function result
 *
 ***/

long setnodescalars_(FileInfoPtr thisFile, long theType,
		     long numberOfScalars, ScalarPtr theScalarData)
{
    NodeHeader		theNodeHeader;
    long	    	    	bytesRead, bytesWritten, numberOfScalarValues;
    FilePtr 	    	scalarHeaderLocation, previousHeaderLocation;
    ScalarValueHeader	theScalarHeader, tempHeader;
    long	    	    	loop;

    if((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	SCREAM("Nodes must be written before scalars", "setnodescalars",
	       noNodes,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theNodeHeader, 
		     sizeof(NodeHeader));

    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "setnodescalars", fileReadError,
	       nothing);
    }
#ifndef VAXVERSION
    convertLong((long*)&theNodeHeader, 9);
#endif

    if (theNodeHeader.numberOfNodes != numberOfScalars)
    {
      SCREAM("Number of scalars must equal number of nodes", "setnodescalars",
	     wrongNumber,
	     nothing);
    }
#ifndef VAXVERSION
    convertFloat((float*)theScalarData, numberOfScalars, FALSE);
#endif

    theScalarHeader.headerSize = sizeof(ScalarValueHeader);
    theScalarHeader.nextScalarValueHeader = 0;
    theScalarHeader.scalarValueType = theType;

    if (theNodeHeader.numberOfNodeScalarValues == 0) 
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
	    SCREAM("Error writing scalar value header", "setnodescalars",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theScalarData,
			     numberOfScalars * sizeof(Scalar));

	if (bytesWritten != (long)(numberOfScalars * sizeof(Scalar)))
	{
	    SCREAM("Error writing scalars", "setnodescalars",
		   fileWriteError,
		   nothing);
	}

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->nodeHeader + NONSVOFFSET,
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
	    SCREAM("Error writing number of scalar values",
		   "setnodescalars", fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, &scalarHeaderLocation, 
			     sizeof(FilePtr));

	if (bytesWritten != sizeof(FilePtr))
	{
	    SCREAM("Error writing scalar header location", "setnodescalars",
		   fileWriteError,
		   nothing);
	}

	return(success);
    }
    else
    {	/* This is an additional set of Scalar Values */
	previousHeaderLocation = theNodeHeader.firstNodeScalarValueHeader;
	for (loop = 1; loop < theNodeHeader.numberOfNodeScalarValues; loop ++)
	{
	    myLseek(thisFile->fileNumber, previousHeaderLocation, SEEK_SET);
	    bytesRead = read(thisFile->fileNumber, &tempHeader,
			     sizeof(ScalarValueHeader));

	    if (bytesRead != sizeof(ScalarValueHeader))
	    {
		SCREAM("Error reading scalar value header", " setnodescalars",
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
	    SCREAM("Error writing scalar value header", "setnodescalars",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theScalarData,
			     numberOfScalars * sizeof(long));

	if (bytesWritten != (long)(numberOfScalars * sizeof(long)))
	{
	    SCREAM("error writing scalars", "setnodescalars", fileWriteError,
		   nothing);
	}

	myLseek(thisFile->fileNumber, previousHeaderLocation + NSVHOFFSET, 
	      SEEK_SET);
#ifndef VAXVERSION
	convertLong((long*)&scalarHeaderLocation, 1);
#endif
	bytesWritten = write(thisFile->fileNumber, &scalarHeaderLocation,
			     sizeof(long));

	if (bytesWritten != sizeof(long))
	{
	    SCREAM("Error writing scalar header location", "setnodescalars",
		   fileWriteError,
		   nothing);
	}

	numberOfScalarValues = theNodeHeader.numberOfNodeScalarValues + 1;

#ifndef VAXVERSION
	convertLong((long*)&numberOfScalarValues, 1);
#endif

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->nodeHeader + NONSVOFFSET,
	      SEEK_SET);

	bytesWritten = write(thisFile->fileNumber, &numberOfScalarValues,
			     sizeof(long));

	if (bytesWritten != sizeof(long))
	{
	    SCREAM("Error writing number of scalar values", "setnodescalars",
		   fileWriteError,
		   nothing);
	}
	return(success);
    }
}


/****
 *
 *	getnodescalartypes_  get the type of each set of scalar values
 *	    	    	     for the current surface
 *
 *	INPUT:	thisFile    file info pointer
 *
 *	OUTPUTS:theTypes    an array of types (must be large enough to
 *	    	    	    accomodate the number of scalar values)
 *	    	result	    function result
 *
 ***/

long getnodescalartypes_(FileInfoPtr thisFile, long *theTypes)
{
    NodeHeader		theNodeHeader;
    long	    	    	bytesRead, loop;
    FilePtr 	    	nextScalarHeader;
    ScalarValueHeader	tempScalarValueHeader;

    if((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	SCREAM("There are no nodes on this surface", "getnodescalartypes",
	       noNodes,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theNodeHeader, 
		     sizeof(NodeHeader));

    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "getnodescalartypes",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theNodeHeader.headerSize), 9);
#endif

    if (theNodeHeader.firstNodeScalarValueHeader == 0)
    {
	SCREAM("There are no scalars associated with nodes on this surface",
	       "getnodescalartypes", noScalars,
	       nothing);
    }

    nextScalarHeader = theNodeHeader.firstNodeScalarValueHeader;

    for (loop = 0; loop < theNodeHeader.numberOfNodeScalarValues; loop++)
    {
	myLseek(thisFile->fileNumber, nextScalarHeader, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempScalarValueHeader,
		  sizeof(ScalarValueHeader));

	if (bytesRead != sizeof(ScalarValueHeader))
	{
	    SCREAM("Error reading scalar value header", "getnodescalartypes",
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
 *	getnodescalars_ get a selected set of node scalar values
 *
 *	INPUTS:	thisFile    	file info pointer
 *	    	scalarIndex 	index of desired scalar value set
 *
 *	OUTPUTS: theType    	the type of this set of node scalars
 *	    	 theScalarData	the scalar values
 *	    	 result	    	function result
 *
 ***/

long getnodescalars_(FileInfoPtr thisFile, long scalarIndex,
		     long *theType, ScalarPtr theScalarData)
{
    NodeHeader	    	theNodeHeader;
    long	        bytesRead, loop;
    FilePtr 	    	scalarHeaderLocation;
    ScalarValueHeader	tempScalarHeader;

    if((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	SCREAM("There are no nodes on this surface", "getnodescalars",
	       noNodes,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theNodeHeader, 
		     sizeof(NodeHeader));


    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "getnodescalars",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theNodeHeader.headerSize), 9);
#endif

    if (theNodeHeader.numberOfNodeScalarValues == 0)
    {
	SCREAM("There are no scalars associated with nodes on this surface",
	       "getnodescalars", noScalars,
	       nothing);
    }
    scalarHeaderLocation = theNodeHeader.firstNodeScalarValueHeader;

    for (loop = 0; loop < scalarIndex; loop++)
    {
	myLseek(thisFile->fileNumber, 
			 scalarHeaderLocation, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempScalarHeader,
			 sizeof(ScalarValueHeader));


	if (bytesRead != sizeof(ScalarValueHeader))
	{
	    SCREAM("Error reading scalar value header", "getnodescalars",
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
		     theNodeHeader.numberOfNodes * sizeof(Scalar));


    if (bytesRead != (long) (theNodeHeader.numberOfNodes * sizeof(Scalar)))
    {
	SCREAM("Error reading scalars", "getnodescalars", fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertFloat((float*)theScalarData, theNodeHeader.numberOfNodes, TRUE);
#endif
    return(success);
}





/****
 *
 *	setnodevectors_ set a set of node vectors for the current surface
 *
 *	INPUTS:	thisFile    	file info pointer
 *	    	theType	    	vector value type (see graphicsio.h for
 *	    	    	    	definitions)
 *	    	numberOfVectors	number of vectors (must match number of
 *	    	    	    	nodes)
 *	    	theVectorData	the vector data
 *
 *	OUTPUT:	result	    	function result
 *
 ***/

long setnodevectors_(FileInfoPtr thisFile, long theType,
		     long numberOfVectors, VectorPtr theVectorData)
{
    NodeHeader		theNodeHeader;
    long	    	    	bytesRead, bytesWritten, numberOfVectorValues;
    FilePtr 	    	vectorHeaderLocation, previousHeaderLocation;
    VectorValueHeader	theVectorHeader, tempHeader;
    long	    	    	loop;

    if((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	SCREAM("Nodes must be written before vectors", "setnodevectors",
	       noNodes,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theNodeHeader, 
		     sizeof(NodeHeader));


    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "setnodevectors",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&theNodeHeader, 9);
#endif

    if (theNodeHeader.numberOfNodes != numberOfVectors)
    {
	SCREAM("Number of vectors must match the number of nodes",
	       "setnodevectors", wrongNumber,
	       nothing);
    }
#ifndef VAXVERSION
    convertFloat((float*)theVectorData, 3 * numberOfVectors, FALSE);
#endif

    theVectorHeader.headerSize = sizeof(VectorValueHeader);
    theVectorHeader.nextVectorValueHeader = 0;
    theVectorHeader.vectorValueType = theType;

    if (theNodeHeader.numberOfNodeVectorValues == 0) 
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
	    SCREAM("Error writing vector value header", "setnodevectors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theVectorData,
			     numberOfVectors * sizeof(Vector));

	if (bytesWritten != (long)(numberOfVectors * sizeof(Vector)))
	{
	    SCREAM("Error writing vectors", "setnodevectors",
		   fileWriteError,
		   nothing);
	}

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->nodeHeader + NONVVOFFSET,
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
	    SCREAM("Error writing number of vector values", "setnodevectors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, &vectorHeaderLocation, 
			     sizeof(FilePtr));

	if (bytesWritten != sizeof(FilePtr))
	{
	    SCREAM("Error writing vector header location", "setnodevectors",
		   fileWriteError,
		   nothing);
	}

    }
    else
    {	/* This is an additional set of Vector Values */
	previousHeaderLocation = theNodeHeader.firstNodeVectorValueHeader;
	for (loop = 1; loop < theNodeHeader.numberOfNodeVectorValues; loop ++)
	{
	    myLseek(thisFile->fileNumber, previousHeaderLocation, SEEK_SET);
	    bytesRead = read(thisFile->fileNumber, &tempHeader,
			     sizeof(VectorValueHeader));

	if (bytesRead != sizeof(VectorValueHeader))
	{
	    SCREAM("Error reading vector value header", "setnodevectors",
		   fileReadError,
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
	    SCREAM("Error writing  vector value header", "setnodevectors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theVectorData,
			     numberOfVectors * sizeof(Vector));

	if (bytesWritten != (long)(numberOfVectors * sizeof(Vector)))
	{
	    SCREAM("Error writing vectors", "setnodevectors", fileWriteError,
		   nothing);
	}

	myLseek(thisFile->fileNumber, previousHeaderLocation + NVVHOFFSET, 
	      SEEK_SET);
#ifndef VAXVERSION
	convertLong((long*)&vectorHeaderLocation, 1);
#endif
	bytesWritten = write(thisFile->fileNumber, &vectorHeaderLocation,
			     sizeof(long));

	if (bytesWritten != sizeof(long))
	{
	    SCREAM("Error writing vector header location", "setnodevectors",
		   fileWriteError,
		   nothing);
	}

	numberOfVectorValues = theNodeHeader.numberOfNodeVectorValues + 1;

#ifndef VAXVERSION
	convertLong((long*)&numberOfVectorValues, 1);
#endif

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->nodeHeader + NONVVOFFSET,
	      SEEK_SET);

	bytesWritten = write(thisFile->fileNumber, &numberOfVectorValues,
			     sizeof(long));

	if (bytesWritten != sizeof(long))
	{
	    SCREAM("Error writing number of vector values", "setnodevectors",
		   fileWriteError,
		   nothing);
	}

    }
    return(success);
}

/****
 *
 *	getnodevectortypes_ This routine returns the types of node
 *	    	    	    vector values that are associated with
 *	    	    	    the current node set
 *
 *	    INPUTS: thisFile	file info pointer
 *	    	    theTypes	a pointer an array of longs
 *
 *	    OUTPUTS:theType 	fills in the array of longs with types
 *	    	    result  	function result
 ***/

long getnodevectortypes_(FileInfoPtr thisFile, long *theTypes)
{
    NodeHeader		theNodeHeader;
    long	    	    	bytesRead, loop;
    FilePtr 	    	nextVectorHeader;
    VectorValueHeader	tempVectorValueHeader;

    if((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	SCREAM("There are no nodes on this surface", "getnodevectortypes",
	       noNodes,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theNodeHeader, 
		     sizeof(NodeHeader));

    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "getnodevectortypes",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theNodeHeader.headerSize), 9);
#endif

    if (theNodeHeader.firstNodeVectorValueHeader == 0)
    {
	SCREAM("There are no vectors associated with nodes on this surface", 
	       "getnodevectortypes", noVectors,
	       nothing);
    }

    nextVectorHeader = theNodeHeader.firstNodeVectorValueHeader;

    for (loop = 0; loop < theNodeHeader.numberOfNodeVectorValues; loop++)
    {
	myLseek(thisFile->fileNumber, nextVectorHeader, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempVectorValueHeader,
		  sizeof(VectorValueHeader));

	if (bytesRead != sizeof(VectorValueHeader))
	{
	    SCREAM("Error reading vector value header", "getnodevectortypes",
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
 *	getnodevectors_ get a set of node vectors for the current surface
 *
 *	INPUTS:	thisFile    	file info pointer
 *	    	vectorIndex 	the index of the desired vector set
 *
 *	OUTPUT:	theType	    	the type of this vector set
 *	    	theVectorData	the vector values for this set
 *	    	result	    	function result
 *
 ***/

long getnodevectors_(FileInfoPtr thisFile, long vectorIndex,
		     long *theType, VectorPtr theVectorData)
{
    NodeHeader	    	theNodeHeader;
    long	    	bytesRead, loop;
    FilePtr 	    	vectorHeaderLocation;
    VectorValueHeader	tempVectorHeader;

    if((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	SCREAM("There are no nodes on this surface", "getnodevectors",
	       noNodes,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theNodeHeader, 
		     sizeof(NodeHeader));

    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "getnodevectors",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&(theNodeHeader.headerSize), 9);
#endif

    if (theNodeHeader.numberOfNodeVectorValues == 0)
    {
	SCREAM("There are no vectors associated with nodes on this surface", 
	       "getnodevectors", noVectors,
	       nothing);
    }
    vectorHeaderLocation = theNodeHeader.firstNodeVectorValueHeader;
    
    for (loop = 0; loop < vectorIndex; loop++)
    {
	myLseek(thisFile->fileNumber, 
			 vectorHeaderLocation, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempVectorHeader,
			 sizeof(VectorValueHeader));
	
	if (bytesRead != sizeof(VectorValueHeader))
	{
	    SCREAM("Error reading vector value header", "getnodevectors",
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
		     theNodeHeader.numberOfNodes * sizeof(Vector));
    
    if (bytesRead != (long)(theNodeHeader.numberOfNodes * sizeof(Vector)))
    {
	SCREAM("Error reading vectors", "getnodevectors",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertFloat((float*)theVectorData, 3 * theNodeHeader.numberOfNodes, TRUE);
#endif
    return(success);
}






/****
 *
 *	setnodetensors_
 *
 ***/

long setnodetensors_(FileInfoPtr thisFile, long theType, long theDimension,
		     long numberOfTensors, TensorPtr theTensorData)
{
    NodeHeader		theNodeHeader;
    long	    	    	bytesRead, bytesWritten, numberOfTensorValues;
    FilePtr 	    	tensorHeaderLocation, previousHeaderLocation;
    TensorValueHeader	theTensorHeader, tempHeader;
    long	    	    	loop;

    if((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	SCREAM("Nodes must be written before tensors", "setnodetensors",
	       noNodes,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theNodeHeader, 
		     sizeof(NodeHeader));

    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "setnodetensors",
	       fileReadError,
	       nothing);
    }

#ifndef VAXVERSION
    convertLong((long*)&theNodeHeader, 9);
#endif

    if (theNodeHeader.numberOfNodes != numberOfTensors)
    {
	SCREAM("Number of tensors must match the number of nodes",
	       "setnodetensors", wrongNumber,
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

    if (theNodeHeader.numberOfNodeTensorValues == 0) 
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
	    SCREAM("Error writing tensor value header", "setnodetensors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theTensorData,
			     theDimension * theDimension * numberOfTensors
			      * sizeof(float));

	if (bytesWritten != (long)(theDimension * theDimension * 
				   numberOfTensors * sizeof(float)))
	{
	    SCREAM("Error writing tensors", "setnodetensors", fileWriteError,
		   nothing);
	}

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->nodeHeader + NONTVOFFSET,
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
	    SCREAM("Error writing number of tensor values", "setnodetensors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, &tensorHeaderLocation, 
			     sizeof(FilePtr));

	if (bytesWritten != sizeof(FilePtr))
	{
	    SCREAM("Error writing tensor header location", "setnodetensors",
		   fileWriteError,
		   nothing);
	}

    }
    else
    {	/* This is an additional set of Tensor Values */
	previousHeaderLocation = theNodeHeader.firstNodeTensorValueHeader;
	for (loop = 1; loop < theNodeHeader.numberOfNodeTensorValues; loop ++)
	{
	    myLseek(thisFile->fileNumber, previousHeaderLocation, SEEK_SET);
	    bytesRead = read(thisFile->fileNumber, &tempHeader,
			     sizeof(TensorValueHeader));

	    if (bytesRead != sizeof(TensorValueHeader))
	    {
		SCREAM("Error reading tensor value header", "setnodetensors",
		       fileReadError,
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
	    SCREAM("Error writing tensor value header", "setnodetensors",
		   fileWriteError,
		   nothing);
	}

	bytesWritten = write(thisFile->fileNumber, theTensorData,
			     theDimension * theDimension * numberOfTensors
			      * sizeof(float));

	if (bytesWritten != (long)(theDimension * theDimension * 
				   numberOfTensors * sizeof(float)))
	{
	    SCREAM("Error writing tensors", "setnodetensors", fileWriteError,
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
	    SCREAM("Error writing tensor header location", "setnodetensors",
		   fileWriteError,
		   nothing);
	}

	numberOfTensorValues = theNodeHeader.numberOfNodeTensorValues + 1;

#ifndef VAXVERSION
	convertLong((long*)&numberOfTensorValues, 1);
#endif

	myLseek(thisFile->fileNumber,
	      (thisFile->theCurrentSurface)->nodeHeader + NONTVOFFSET,
	      SEEK_SET);

	bytesWritten = write(thisFile->fileNumber, &numberOfTensorValues,
			     sizeof(long));

	if (bytesWritten != sizeof(long))
	{
	    SCREAM("Error writing number of tensor values", "setnodetensors",
		   fileWriteError,
		   nothing);
	}

    }
    return(success);
}

/****
 *
 *	getnodetensortypes_
 *
 ***/

long getnodetensortypes_(FileInfoPtr thisFile, long *theTypes, 
			 long *theDimensions)
{
    NodeHeader		theNodeHeader;
    long	    	    	bytesRead, loop;
    FilePtr 	    	nextTensorHeader;
    TensorValueHeader	tempTensorValueHeader;

    if((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	SCREAM("There are no nodes on this surface", "getnodetensortypes",
	       noNodes,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theNodeHeader, 
		     sizeof(NodeHeader));

    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "getnodetensortypes",
	       fileReadError,
	       nothing);
    }
    
#ifndef VAXVERSION
    convertLong((long*)&(theNodeHeader.headerSize), 9);
#endif

    if (theNodeHeader.firstNodeTensorValueHeader == 0)
    {
	SCREAM("There are no tensors associated with the nodes", 
	       "getnodetensortypes", noTensors,
	       nothing);
    }

    nextTensorHeader = theNodeHeader.firstNodeTensorValueHeader;

    for (loop = 0; loop < theNodeHeader.numberOfNodeTensorValues; loop++)
    {
	myLseek(thisFile->fileNumber, nextTensorHeader, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempTensorValueHeader,
		  sizeof(TensorValueHeader));
	
	if (bytesRead != sizeof(TensorValueHeader))
	{
	    SCREAM("Error reading tensor value header", "getnodetensortypes",
		   fileReadError,
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
 *	getnodetensors_
 *
 ***/

long getnodetensors_(FileInfoPtr thisFile, long tensorIndex, long *theDimension,
		     long *theType, TensorPtr theTensorData)
{
    NodeHeader	    	theNodeHeader;
    long	       	bytesRead, loop;
    FilePtr 	    	tensorHeaderLocation;
    TensorValueHeader	tempTensorHeader;

    if((thisFile->theCurrentSurface)->nodeHeader == 0)
    {
	SCREAM("There are no nodes on this surface", "getnodetensors",
	       noNodes,
	       nothing);
    }

    myLseek(thisFile->fileNumber,
	  (thisFile->theCurrentSurface)->nodeHeader, SEEK_SET);
    bytesRead = read(thisFile->fileNumber, &theNodeHeader, 
		     sizeof(NodeHeader));

    if (bytesRead != sizeof(NodeHeader))
    {
	SCREAM("Error reading node header", "getnodetensors", fileReadError,
	       nothing);
    }
	    
#ifndef VAXVERSION
    convertLong((long*)&(theNodeHeader.headerSize), 9);
#endif

    if (theNodeHeader.numberOfNodeTensorValues == 0)
    {
	SCREAM("There are no tensors associated with nodes on this surface",
	       "getnodetensors", noTensors,
	       nothing);
    }

    tensorHeaderLocation = theNodeHeader.firstNodeTensorValueHeader;

    for (loop = 0; loop < tensorIndex; loop++)
    {
	myLseek(thisFile->fileNumber, 
			 tensorHeaderLocation, SEEK_SET);
	bytesRead = read(thisFile->fileNumber, &tempTensorHeader,
			 sizeof(TensorValueHeader));

	if (bytesRead != sizeof(TensorValueHeader))
	{
	    SCREAM("Error reading tensor value header", "getnodetensors",
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
		     theNodeHeader.numberOfNodes * 
		     tempTensorHeader.tensorDimension *
		     tempTensorHeader.tensorDimension *
		     sizeof(float));

    if (bytesRead != (long) (theNodeHeader.numberOfNodes *
			     tempTensorHeader.tensorDimension *
			     tempTensorHeader.tensorDimension *
			     sizeof(float)))
    {
	SCREAM("Error reading tensors", "getnodetensors", fileReadError,
	       nothing);
    }
	    
#ifndef VAXVERSION
    convertFloat((float*)theTensorData, 
		 tempTensorHeader.tensorDimension *
		 tempTensorHeader.tensorDimension *
		 theNodeHeader.numberOfNodes, TRUE);
#endif
    return(success);
}


