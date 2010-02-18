#include "graphicsio.h"

extern char nothing[];


/******************************************************************************
 ******************************************************************************
 
	File Rewrite Routines - The following routines are all involved in 
	rewriting files, usually during editing.
 
*******************************************************************************
******************************************************************************/
 

/****
 *
 *	initrewrite()
 *
 ***/

long initrewrite(rewriteQueuePtr *thisQueue)
{
    rewriteQueuePtr aQueue;

    aQueue = (rewriteQueuePtr)malloc(sizeof(rewriteQueue));
    if (aQueue == NULLPTR)
    {
	report("Can't get memory for rewriteQueue", "initrewrite",
	       noDynamicMemory, nothing);
	return(noDynamicMemory);
    }
    *thisQueue = aQueue;
    aQueue->rewriteQueue = NULLPTR;
    aQueue->addQueue = NULLPTR;

    return(success);
}

/****
 *
 *	addrewriterequest()
 *
 ***/

long addrewriterequest(rewriteQueuePtr thisQueue, rewriteRequestPtr thisRequest)
{
    queuedRewriteRequestPtr	newRequest;
    printf("+++++\n");
    newRequest = (queuedRewriteRequestPtr)malloc(sizeof(queuedRewriteRequest));
    if (newRequest == NULLPTR)
    {
	report("Can't get memory for list element", "addrewriterequest",
	       noDynamicMemory, nothing);
	return(noDynamicMemory);
    }
    ADD(newRequest, thisQueue->rewriteQueue);
    
    newRequest->theRequest = *(thisRequest); /* copy in thisRequest */

    return(success);
}


/****
 *
 *	addnewrequest()
 *
 ***/

long addnewrequest(rewriteQueuePtr thisQueue, rewriteRequestPtr thisRequest)
{
    queuedRewriteRequestPtr	newRequest;

    newRequest = (queuedRewriteRequestPtr)malloc(sizeof(queuedRewriteRequest));
    if (newRequest == NULLPTR)
    {
	report("Can't get memory for list element", "addnewrequest",
	       noDynamicMemory, nothing);
	return(noDynamicMemory);
    }
    ADD(newRequest, thisQueue->addQueue);
    
    newRequest->theRequest = *(thisRequest); /* copy in thisRequest */

    return(success);
}
    
/****
 *
 *	rewritefile
 *
 ***/
	    
long rewritefile(rewriteQueuePtr thisQueue, char *oldFileName, char *newFileName)
{
    FileInfoPtr	    	inputFile, outputFile;
    long		result, errorLevel = 1;
    long	        fileType, numberOfSurfaces, numberOfBoundingSurfaces;
    long		numberOfTimeSeriesBlocks;
    Boolean	        preferedSettingsBlock, numberOfNodesChanged;
    Boolean	    	numberOfElementsChanged;
    Boolean		externalFile, deallocate;
    char		theID[80], theText[256], surfaceName[80], *charPtr;
    char    	    	theFileName[80];
    char		theGeomFileName[80], theTimeSeriesLabel[80];
    long		surfaceType, *longPtr;
    long		numberOfInputNodes, numberOfNodeScalarValues;
    long		numberOfNodeVectorValues, numberOfNodeTensorValues;
    long	        numberOfOutputNodes, theType, thisValue, theDimension;
    long		thisSurface, numberOfInputElements;
    long    	    	numberOfOutputElements;
    long		elementSize, thisElementSet;
    long    	    	numberOfElementScalarValues;
    long		numberOfElementVectorValues;
    long    	    	numberOfElementTensorValues;
    long		thisTimeSeries, theFormat, theUnits, theSurface;
    long    	    	theAssociation;
    long		numberOfChannels, numberOfFrames;
    long    	    	numberOfCorrectedLeads, numberOfFiducialSets, 
	thisFiducialSet;
    long		qtime, stime, ttime;
    long    	    	ponset, poffset, rpeak, tpeak;
    FidInfoPtr          theFidInfo;
    short               *fidDesc, *theTypes;
    float		*floatPtr, *theFids;
    void		*voidPtr;
    rewriteRequestPtr	aRequest;
    char    	    	extraString[80];
    queuedRewriteRequestPtr queuedRequest;

    result = openfile_(oldFileName, errorLevel, &inputFile);
    CHECKRESULT
    result = getfileinfo_(inputFile, &fileType, &numberOfSurfaces,
			  &numberOfBoundingSurfaces, &numberOfTimeSeriesBlocks,
			  &preferedSettingsBlock);
    CHECKRESULT
    result = createfile_(newFileName, fileType, errorLevel, &outputFile);
    CHECKRESULT
    
    
    /**** expID ****/
    
    deallocate = FALSE;
    aRequest = searchList(thisQueue->rewriteQueue, EXPID,0,0); 
    if (aRequest != NULLPTR)
    {
	if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	{	   
	    charPtr = (*(PTRfPTRchar)aRequest->callBackRoutine)(aRequest);
	    if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	}
	else
	{
	    charPtr = (char*)aRequest->dataPointer;
	}
    }
    else
    {
	result = getexpid_(inputFile, theID);
	CHECKRESULT
	charPtr = theID;
    }
    result = setexpid_(outputFile, charPtr);
    CHECKRESULT
    if (deallocate) free(charPtr);
    
    /**** free form text ****/

    deallocate = FALSE;
    aRequest = searchList(thisQueue->rewriteQueue, TEXT,0,0); /*Is there a 
				       request to replace the text?*/
    if (aRequest != NULLPTR)
    {
	if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	{
	    charPtr = (*(PTRfPTRchar)aRequest->callBackRoutine)(aRequest);
	    if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	}
	else
	{
	    charPtr = (char*)aRequest->dataPointer;
	}
    }
    else
    {
	result = gettext_(inputFile, theText);
	CHECKRESULT
	charPtr = theText;
    }
    result = settext_(outputFile, charPtr);
    CHECKRESULT
    if (deallocate) free(charPtr);

    /**** loop through all the surfaces in the file ****/
    
    for (thisSurface = 1; thisSurface <= numberOfSurfaces; thisSurface++)
    {
	printf("Now doing surface %ld\n",thisSurface);
	result = setsurfaceindex_(inputFile, thisSurface);
	CHECKRESULT
	result = setsurfaceindex_(outputFile, thisSurface);
	CHECKRESULT
	
	/***** surface name *****/
	
	deallocate = FALSE;
	aRequest = searchList(thisQueue->rewriteQueue, 
			      SURFACENAME,thisSurface,0); 
	if (aRequest != NULLPTR)
	{
	    if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	    {
		charPtr = (*(PTRfPTRchar)aRequest->callBackRoutine)(aRequest);
		if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	    }
	    else
	    {
		charPtr = (char*)aRequest->dataPointer;
	    }
	}
	else
	{
	    result = getsurfacename_(inputFile, surfaceName);
	    CHECKRESULT
	    charPtr = surfaceName;
	}
	result = setsurfacename_(outputFile, charPtr);
	CHECKRESULT
	if (deallocate) free(charPtr);

	/***** surface type *****/
	
	deallocate = FALSE;
	aRequest = searchList(thisQueue->rewriteQueue, 
			      SURFACETYPE,thisSurface,0);
	if (aRequest != NULLPTR)
	{ 
	    if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	    { 
		longPtr = (*(PTRfPTRlong)aRequest->callBackRoutine)(aRequest);
		if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	    } 
	    else
	    {
		longPtr = (long*)aRequest->dataPointer;
	    }
	} 
	else
	{
	    result = getsurfacetype_(inputFile, &surfaceType);
	    CHECKRESULT
	    longPtr = &surfaceType;
	}
	result = setsurfacetype_(outputFile, *longPtr);
	CHECKRESULT
	if (deallocate) free(longPtr);

	/***** nodes *****/
	
	deallocate = FALSE;
	result = getnodeinfo_(inputFile, &numberOfInputNodes, 
			      &numberOfNodeScalarValues,
			      &numberOfNodeVectorValues, 
			      &numberOfNodeTensorValues);
	
	numberOfNodesChanged = FALSE;
	numberOfOutputNodes = numberOfInputNodes;	    		  
	aRequest = searchList(thisQueue->rewriteQueue, NODES,thisSurface,0);
	
	if (aRequest != NULLPTR)
	{
	    if (aRequest->quantity != numberOfInputNodes)
	    {
		numberOfNodesChanged = TRUE;
		numberOfOutputNodes = aRequest->quantity;
	    }
	    
	    if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	    {
		floatPtr = (*(PTRfPTRfloat)aRequest->callBackRoutine)
	    			(aRequest);
		if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	    }
	    else
	    {
		floatPtr = (float*)aRequest->dataPointer;
	    }
	    result = setnodes_(outputFile, numberOfOutputNodes, 
			       (NodePtr)floatPtr);
	}
	else
	{
	    floatPtr = (float*) malloc(numberOfOutputNodes * sizeof(Node));
	    if (floatPtr == NULLPTR)
	    {
		report("Can't get memory for temp node list", "rewritefile",
		       noDynamicMemory, nothing);
		return(noDynamicMemory);
	    }
	    result = getnodes_(inputFile, (NodePtr)floatPtr);
	    CHECKRESULT
	    result = setnodes_(outputFile, numberOfOutputNodes, 
			       (NodePtr)floatPtr);
	    CHECKRESULT
	    deallocate = TRUE;
	}
	if (deallocate) free(floatPtr);


	/***** node scalar values *****/
	
	for (thisValue = 1; thisValue <= numberOfNodeScalarValues; thisValue++)
	{ 
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, 
				  NODESCALARS,thisSurface,thisValue);
	    
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{
		    floatPtr = (*(PTRfPTRfloat)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR)
			deallocate = TRUE;
		}
		else
		{
		    floatPtr = (float*)aRequest->dataPointer;
		}
		result = setnodescalars_(outputFile, aRequest->valueType,
					 aRequest->quantity, floatPtr);
	    }
	    else if (!numberOfNodesChanged)
	    {
		floatPtr = (float*)malloc(numberOfOutputNodes * sizeof(float));
		if (floatPtr == NULLPTR)
		{
		    report("Can't get memory for temp node scalar list",
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = getnodescalars_(inputFile, thisValue, 
					 &theType, floatPtr);
		CHECKRESULT
		result = setnodescalars_(outputFile, theType, 
					 numberOfOutputNodes, floatPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    if (deallocate) free(floatPtr);
	}
	
	/***** node vector values *****/
	
	for (thisValue = 1; thisValue <= numberOfNodeVectorValues; thisValue++)
	{ 
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, 
				  NODEVECTORS,thisSurface,thisValue);
	    
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{
		    floatPtr = (*(PTRfPTRfloat)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    floatPtr = (float*)aRequest->dataPointer;
		}
		result = setnodescalars_(outputFile, aRequest->valueType,
					 aRequest->quantity, floatPtr);
	    }
	    else if (!numberOfNodesChanged)
	    {
		floatPtr = (float*)malloc(numberOfOutputNodes * 
					  sizeof(Vector));
		if (floatPtr == NULLPTR)
		{
		    report("Can't get memory for temp node vector list",
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = getnodevectors_(inputFile, thisValue, &theType, 
					 (VectorPtr)floatPtr);
		CHECKRESULT
		result = setnodevectors_(outputFile, theType, 
					 numberOfOutputNodes, 
					 (VectorPtr)floatPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    if (deallocate) free(floatPtr);
	}
	
	/***** node tensor values *****/
	
	for (thisValue = 1; thisValue <= numberOfNodeTensorValues; thisValue++)
	{ 
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, 
				  NODETENSORS,thisSurface,thisValue);
	    
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{
		    floatPtr = (*(PTRfPTRfloat)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    floatPtr = (float*)aRequest->dataPointer;
		}
		result = setnodetensors_(outputFile, aRequest->valueType, 
					 aRequest->theDimension,
					 aRequest->quantity, floatPtr);
	    }
	    else if (!numberOfNodesChanged)
	    {
		floatPtr = (float*)malloc(numberOfOutputNodes * 
				  aRequest->theDimension *
				  aRequest->theDimension * sizeof(float));
		if (floatPtr == NULLPTR)
		{
		    report("Can't get memory for temp node tensor list", 
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = getnodetensors_(inputFile, thisValue, &theDimension, 
					 &theType, floatPtr);
		CHECKRESULT
		result = setnodetensors_(outputFile, theType, theDimension,
					 numberOfOutputNodes, 
					 (TensorPtr)floatPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    if (deallocate) free(floatPtr);
	}
	
	for (thisElementSet = 1; thisElementSet <= 1; thisElementSet++)
	{
	    deallocate = FALSE;
	    result = getelementinfo_(inputFile, &numberOfInputElements, 
				     &elementSize, 
				     &numberOfElementScalarValues, 
				     &numberOfElementVectorValues,
				     &numberOfElementTensorValues);
	    aRequest = searchList(thisQueue->rewriteQueue, ELEMENTS, 
				  thisSurface, thisElementSet);
	    
	    numberOfElementsChanged = FALSE;
	    numberOfOutputElements = numberOfInputElements;    	  
	    
	    
	    if (aRequest != NULLPTR)
	    {
		
		if (aRequest->quantity != numberOfInputElements)
		{
		    numberOfElementsChanged = TRUE;
		    numberOfOutputElements = aRequest->quantity;
		}
		
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{
		    longPtr = (*(PTRfPTRlong)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    longPtr = (long*)aRequest->dataPointer;
		}
		result = setelements_(outputFile, aRequest->theDimension,
				      aRequest->quantity, longPtr);
		CHECKRESULT
	    }
	    else
	    {
		longPtr = (long*)malloc(numberOfOutputElements * elementSize * 
				 sizeof(long));
		if (longPtr == NULLPTR)
		{
		    report("Can't get memory for temp element list", 
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = getelements_(inputFile, longPtr);
		CHECKRESULT
		result = setelements_(outputFile, elementSize, 
				      numberOfOutputElements,
				      longPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    if (deallocate) free(longPtr);
	}
	
	/***** element scalar values *****/
	
	for (thisValue = 1; thisValue <= numberOfElementScalarValues; 
	     thisValue++)
	{ 
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, 
				  ELEMENTSCALARS,thisSurface,thisValue);
	    
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{
		    floatPtr = (*(PTRfPTRfloat)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    floatPtr = (float*)aRequest->dataPointer;
		}
		result = setelementscalars_(outputFile, aRequest->valueType,
					    aRequest->quantity, floatPtr);
	    }
	    else if (!numberOfElementsChanged)
	    {
		floatPtr = (float*)malloc(numberOfOutputElements * 
					  sizeof(float));
		if (floatPtr == NULLPTR)
		{
		    report("Can't get memory for temp element scalar list", 
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = getelementscalars_(inputFile, thisValue, 
					    &theType, floatPtr);
		CHECKRESULT
		result = setelementscalars_(outputFile, theType, 
					    numberOfOutputElements, floatPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    if (deallocate) free(floatPtr);
	}
	
	/***** element vector values *****/
	
	for (thisValue = 1; thisValue <= numberOfElementVectorValues; 
	     thisValue++)
	{ 
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, 
				  ELEMENTVECTORS,thisSurface,thisValue);
	    
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{
		    floatPtr = (*(PTRfPTRfloat)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    floatPtr = (float*)aRequest->dataPointer;
		}
		result = setelementscalars_(outputFile, aRequest->valueType,
					    aRequest->quantity, floatPtr);
	    }
	    else if (!numberOfElementsChanged)
	    {
		floatPtr = (float*)malloc(numberOfOutputElements * 
					  sizeof(Vector));
		if (floatPtr == NULLPTR)
		{
		    report("Can't get memory for temp element vector list",
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = getelementvectors_(inputFile, thisValue, 
					    &theType, (VectorPtr)floatPtr);
		CHECKRESULT
		result = setelementvectors_(outputFile, theType, 
					    numberOfOutputElements, 
					    (VectorPtr)floatPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    if (deallocate) free(floatPtr);
	}
	
	/***** element tensor values *****/
	
	for (thisValue = 1; thisValue <= numberOfElementTensorValues; 
	     thisValue++)
	{ 
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, ELEMENTTENSORS,
				  thisSurface,thisValue);
	    
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{
		    floatPtr = (*(PTRfPTRfloat)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    floatPtr = (float*)aRequest->dataPointer;
		}
		result = setelementtensors_(outputFile, aRequest->valueType, 
					    aRequest->theDimension,
					    aRequest->quantity, floatPtr);
	    }
	    else if (!numberOfElementsChanged)
	    {
		floatPtr = (float*)malloc(numberOfOutputElements * 
				  aRequest->theDimension *
				  aRequest->theDimension * sizeof(float));
		if (floatPtr == NULLPTR)
		{
		    report("Can't get memory for temp element tensor list",
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = getelementtensors_(inputFile, thisValue, 
					    &theDimension, 
					    &theType, floatPtr);
		CHECKRESULT
		result = setelementtensors_(outputFile, theType, theDimension,
					    numberOfOutputElements, 
					    (TensorPtr)floatPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    if (deallocate) free(floatPtr);
	}
    }
    
    for (thisTimeSeries = 1; thisTimeSeries <= numberOfTimeSeriesBlocks; 
	 thisTimeSeries++)
    {
	settimeseriesindex_(inputFile, thisTimeSeries);
	gettimeseriesfile_(inputFile, theFileName);
	getleadfiducialcount_(inputFile, &numberOfFiducialSets);
	if (strlen(theFileName) == 0)
	{
	    externalFile = FALSE;
	}
	else
	{
	    externalFile = TRUE;
	    gettimeseriesspecs_(inputFile, &numberOfChannels, &numberOfFrames);
	}
	settimeseriesindex_(outputFile, thisTimeSeries);
	getnumcorrectedleads_(inputFile, &numberOfCorrectedLeads);
	/*	printf("external file is %d\n",externalFile);
	printf("numberOfChannels %ld numberOfFrames %ld\n",numberOfChannels, 
	       numberOfFrames);*/
	
	if (externalFile)
	{
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, TIMESERIESFILE, 
				  thisTimeSeries, 0); 
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{	   
		    charPtr = (*(PTRfPTRchar)aRequest->callBackRoutine)
			(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    charPtr = (char*)aRequest->dataPointer;
		}
	    }
	    else
	    {
		result = gettimeseriesfile_(inputFile, theFileName);
		CHECKRESULT
	        charPtr = theFileName;
	    }
	    result = settimeseriesfile_(outputFile, charPtr);
	    CHECKRESULT
	    if (deallocate) free(charPtr);
	}
	
	if (!externalFile)
	{
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, TIMESERIESSPECS, 
				  thisTimeSeries, 0); 
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{	   
		    voidPtr = (*(PTRfPTRvoid)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    voidPtr = (long*)aRequest->dataPointer;
		}
		numberOfFrames = ((timeseriesspecsPtr)voidPtr)->numberOfFrames;
		numberOfChannels = ((timeseriesspecsPtr)voidPtr)->
		    	    	    numberOfChannels;
	    }
	    else
	    {
		result = gettimeseriesspecs_(inputFile, &numberOfChannels, 
					     &numberOfFrames);
		CHECKRESULT
	    }
	    result = settimeseriesspecs_(outputFile, numberOfChannels, 
					 numberOfFrames);
	    CHECKRESULT
	    if (deallocate) free(voidPtr);
	}
	
	aRequest = searchList(thisQueue->rewriteQueue, TIMESERIESGEOMFILE, 
			      thisTimeSeries, 0); 
	deallocate = FALSE;
	if (aRequest != NULLPTR)
	{
	    if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	    {	   
		charPtr = (*(PTRfPTRchar)aRequest->callBackRoutine)(aRequest);
		if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	    }
	    else
	    {
		charPtr = (char*)aRequest->dataPointer;
	    }
	}
	else
	{
	    result = gettimeseriesgeomfile_(inputFile, theGeomFileName);
	    CHECKRESULT
	    charPtr = theGeomFileName;
	}
	result = settimeseriesgeomfile_(outputFile, charPtr);
	CHECKRESULT
	if (deallocate) free(charPtr);

	if (!externalFile)
	{
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, TIMESERIESLABEL, 
				  thisTimeSeries, 0); 
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{	   
		    charPtr = (*(PTRfPTRchar)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    charPtr = (char*)aRequest->dataPointer;
		}
	    }
	    else
	    {
		result = gettimeserieslabel_(inputFile, theTimeSeriesLabel);
		CHECKRESULT
		charPtr = theTimeSeriesLabel;
	    }
	    result = settimeserieslabel_(outputFile, charPtr);
	    CHECKRESULT
	    if (deallocate) free(charPtr);
	}
	
	aRequest = searchList(thisQueue->rewriteQueue, TIMESERIESFORMAT, 
			      thisTimeSeries, 0);
	deallocate = FALSE;
	if (aRequest != NULLPTR)
	{
	    if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	    {	   
		longPtr = (*(PTRfPTRlong)aRequest->callBackRoutine)(aRequest);
		if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	    }
	    else
	    {
		longPtr = (long*)aRequest->dataPointer;
	    }
	    theFormat = *longPtr;
	}
	else
	{
	    result = gettimeseriesformat_(inputFile, &theFormat);
	    CHECKRESULT
	}
	result = settimeseriesformat_(outputFile, theFormat);
	CHECKRESULT
	if (deallocate) free(longPtr);

	aRequest = searchList(thisQueue->rewriteQueue, TIMESERIESUNITS,
			      thisTimeSeries, 0); 
	deallocate = FALSE;
	if (aRequest != NULLPTR)
	{
	    if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	    {	   
		longPtr = (*(PTRfPTRlong)aRequest->callBackRoutine)(aRequest);
		if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	    }
	    else
	    {
		longPtr = (long*)aRequest->dataPointer;
	    }
	    theUnits = *longPtr;
	}
	else
	{
	    result = gettimeseriesunits_(inputFile, &theUnits);
	    CHECKRESULT
	}
	result = settimeseriesunits_(outputFile, theUnits);
	CHECKRESULT
	if (deallocate) free(longPtr);

	aRequest = searchList(thisQueue->rewriteQueue, TIMESERIESSURFACE, 
			      thisTimeSeries, 0); 
	deallocate = FALSE;
	if (aRequest != NULLPTR)
	{
	    if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	    {	   
		longPtr = (*(PTRfPTRlong)aRequest->callBackRoutine)(aRequest);
		if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	    }
	    else
	    {
		longPtr = (long*)aRequest->dataPointer;
	    }
	    theSurface = *longPtr;
	}
	else
	{
	    result = gettimeseriessurface_(inputFile, &theSurface);
	    CHECKRESULT
	}
	result = settimeseriessurface_(outputFile, theSurface);
	CHECKRESULT
	if (deallocate) free(longPtr);

	aRequest = searchList(thisQueue->rewriteQueue, TIMESERIESASSOC, 
			      thisTimeSeries, 0); 
	deallocate = FALSE;
	if (aRequest != NULLPTR)
	{
	    if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	    {	   
		longPtr = (*(PTRfPTRlong)aRequest->callBackRoutine)(aRequest);
		if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	    }
	    else
	    {
		longPtr = (long*)aRequest->dataPointer;
	    }
	    theAssociation = *longPtr;
	}
	else
	{
	    result = gettimeseriesassoc_(inputFile, &theAssociation);
	    CHECKRESULT
	}
	result = settimeseriesassoc_(outputFile, theAssociation);
	CHECKRESULT
	if (deallocate) free(longPtr);

	deallocate = FALSE;
	if (!externalFile)
	{
	    aRequest = searchList(thisQueue->rewriteQueue, TIMESERIESDATA, 
				  thisTimeSeries, 0);
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{	   
		    floatPtr = (*(PTRfPTRfloat)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    floatPtr = (float*)aRequest->dataPointer;
		}
	    }
	    else
	    {
		floatPtr = (float*)malloc(numberOfChannels * 
					  numberOfFrames * sizeof(float));
		if (floatPtr == NULLPTR)
		{
		    report("Can't get memory for temp time series data",
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = gettimeseriesdata_(inputFile, floatPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    result = settimeseriesdata_(outputFile, floatPtr);
	    CHECKRESULT
	    if (deallocate) free(floatPtr);
	}
	
	aRequest = searchList(thisQueue->rewriteQueue, NUMCORRECTEDLEADS, 
			      thisTimeSeries, 0); 
	deallocate = FALSE;
	if (aRequest != NULLPTR)
	{
	    if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
	    {	   
		longPtr = (*(PTRfPTRlong)aRequest->callBackRoutine)(aRequest);
		if (aRequest->callBackRoutine == NULLPTR) deallocate = TRUE;
	    }
	    else
	    {
		longPtr = (long*)aRequest->dataPointer;
	    }
	    numberOfCorrectedLeads = *longPtr;
	}
	else
	{
	    result = getnumcorrectedleads_(inputFile, &numberOfCorrectedLeads);
	    CHECKRESULT
	}
	result = setnumcorrectedleads_(outputFile, numberOfCorrectedLeads);
	CHECKRESULT
	if (deallocate) free(longPtr);

	if (numberOfCorrectedLeads != 0)
	{
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, CORRECTEDLEADS, 
				  thisTimeSeries, 0);
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) 
		    /* Is the data in memory? */
		{	   
		    longPtr = (*(PTRfPTRlong)aRequest->callBackRoutine)
			(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    longPtr = (long*)aRequest->dataPointer;
		}
	    }
	    else
	    {
		longPtr = (long*)malloc(numberOfCorrectedLeads * sizeof(long));
		if (longPtr == NULLPTR)
		{
		    sprintf(extraString, "numberOfCorrectedLeads is %ld",
			    numberOfCorrectedLeads);
		    report("Can't get memory for temp corrected leads", 
			   "rewritefile", noDynamicMemory, extraString);
		    return(noDynamicMemory);
		}
		result = getcorrectedleads_(inputFile, longPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    result = setcorrectedleads_(outputFile, longPtr);
	    CHECKRESULT
	    if (deallocate) free(longPtr);
	}

	if (!externalFile)
	{
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, POWERCURVE, 
				  thisTimeSeries, 0);
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{	   
		    floatPtr = (*(PTRfPTRfloat)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    floatPtr = (float*)aRequest->dataPointer;
		}
	    }
	    else
	    {
		floatPtr = (float*)malloc(numberOfFrames * sizeof(float));
		if (floatPtr == NULLPTR)
		{
		    report("Can't get memory for temp power curve data",
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = getpowercurve_(inputFile, floatPtr);
		CHECKRESULT
		deallocate = TRUE;
	    }
	    result = setpowercurve_(outputFile, floatPtr);
	    CHECKRESULT
	    if(deallocate) free(floatPtr);
	}
	
	if (!externalFile)
	{
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, QSTTIMES, 
				  thisTimeSeries, 0); 
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{	   
		    voidPtr = (*(PTRfPTRvoid)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    voidPtr = (long*)aRequest->dataPointer;
		}
		qtime = ((fiducialsPtr)voidPtr)->qtime;
		stime = ((fiducialsPtr)voidPtr)->stime;
		ttime = ((fiducialsPtr)voidPtr)->ttime;
	    }
	    else
	    {
		result = getqsttimes_(inputFile, &qtime, &stime, &ttime);
		CHECKRESULT
	    }
	    result = setqsttimes_(outputFile, qtime, stime, ttime);
	    CHECKRESULT
	    if (deallocate)
	    {
		free((fiducialsPtr)voidPtr);
	    }
	}

	if (!externalFile)
	{
	    deallocate = FALSE;
	    aRequest = searchList(thisQueue->rewriteQueue, EXTENDEDFIDUCIALS, 
				  thisTimeSeries, 0); 
	    if (aRequest != NULLPTR)
	    {
		if (aRequest->callBackRoutine != NULL) /* Is the data 
							  in memory? */
		{	   
		    voidPtr = (*(PTRfPTRvoid)aRequest->callBackRoutine)
		    	    	(aRequest);
		    if (aRequest->callBackRoutine == NULLPTR) 
			deallocate = TRUE;
		}
		else
		{
		    voidPtr = (long*)aRequest->dataPointer;
		}
		ponset = ((extendedFiducialsPtr)voidPtr)->ponset;
		poffset = ((extendedFiducialsPtr)voidPtr)->poffset;
		rpeak = ((extendedFiducialsPtr)voidPtr)->rpeak;
		tpeak = ((extendedFiducialsPtr)voidPtr)->tpeak;
	    }
	    else
	    {
		result = getextendedfiducials_(inputFile, &ponset, &poffset,
					       &rpeak, &tpeak);
		CHECKRESULT
	    }
	    result = setextendedfiducials_(outputFile, ponset, poffset,
					   rpeak, tpeak);
	    CHECKRESULT
	    if (deallocate) free(voidPtr);
	}
    
	result = getleadfiducialcount_(inputFile, &numberOfFiducialSets);
	if (numberOfFiducialSets > 0)
	{
	    theFidInfo = (FidInfoPtr) malloc(numberOfFiducialSets * 
					     sizeof(FidInfo));
	    if (theFidInfo == NULLPTR)
	    {
		report("Can't get memory for temp fidInfo data",
		       "rewritefile", noDynamicMemory, nothing);
		return(noDynamicMemory);
	    }
	    result = getleadfiducialinfo_(inputFile, theFidInfo);
	}
	
	for (thisFiducialSet = 0; thisFiducialSet < numberOfFiducialSets; 
	     thisFiducialSet++)
	{
	    aRequest = searchList(thisQueue->rewriteQueue, LEADFIDUCIALS, 
				  thisTimeSeries, thisFiducialSet+1);
	    if (aRequest != NULLPTR) /* We've got a request for a rewrite */
	    {
                deallocate = FALSE;
                printf("*****aRequest=%ld %ld %ld\n", aRequest->index,
                       aRequest->valueType, aRequest->quantity);
		if (aRequest->quantity == -1)
		{
                    printf("Omitting %d\n", aRequest->valueType);
		    continue; /* This is a request to delete a fiducial set */
		}
		if (aRequest->callBackRoutine != NULL) 
		    /* Is the data in memory? */
		{
		    voidPtr = (*(PTRfPTRvoid)aRequest->callBackRoutine)
			(aRequest);
                    deallocate = aRequest->callBackRoutine == NULLPTR;
		}
		else
		{
		    voidPtr = (LeadFiducialDataPtr)aRequest->dataPointer;
		}
		result = 
		    setleadfiducials_(outputFile, 
				   ((LeadFiducialDataPtr)voidPtr)->theLabel,
				   ((LeadFiducialDataPtr)voidPtr)->fidDescSize,
				   ((LeadFiducialDataPtr)voidPtr)->fidDesc,
				   ((LeadFiducialDataPtr)voidPtr)->theFids,
				   ((LeadFiducialDataPtr)voidPtr)->theTypes);
		CHECKRESULT;
		if (deallocate)
		{
		    free(((LeadFiducialDataPtr)voidPtr)->fidDesc);
		    free(((LeadFiducialDataPtr)voidPtr)->theFids);
		    free(((LeadFiducialDataPtr)voidPtr)->theTypes);
		    free(voidPtr);
		}
	    }
	    else /* Copy the existing data */
	    {
		fidDesc = (short*)malloc(theFidInfo[thisFiducialSet].
					 fidDescSize * sizeof(short));
		if (fidDesc == NULLPTR)
		{
		    report("Can't get memory for temp fid descriptor data",
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		theFids = (float*)malloc(theFidInfo[thisFiducialSet].
					 numberOfFiducials * sizeof(float));
		if (theFids == NULLPTR)
		{
		    report("Can't get memory for temp fid data",
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		theTypes = 
		    (short*)malloc(theFidInfo[thisFiducialSet].
				   numberOfFiducials * sizeof(short));
		if (theTypes == NULLPTR)
		{
		    report("Can't get memory for temp fiducial type data",
			   "rewritefile", noDynamicMemory, nothing);
		    return(noDynamicMemory);
		}
		result = getleadfiducials_(inputFile, 
					   (short)(thisFiducialSet + 1), fidDesc,
					   theFids, theTypes);
		result = 
		    setleadfiducials_(outputFile, 
				      theFidInfo[thisFiducialSet].theLabel,
				      (short)(theFidInfo[thisFiducialSet].
					      fidDescSize),
				      fidDesc, theFids, theTypes);
		CHECKRESULT
		free(fidDesc);
		free(theFids);
		free(theTypes);
	    }
	}

        /* check to see if we're going to add any fiducial sets */
        while( (queuedRequest = 
		searchAddList2(thisQueue->addQueue, LEADFIDUCIALS, 
			       thisTimeSeries)) != NULLPTR)
        {
            aRequest = &(queuedRequest->theRequest);
            deallocate = FALSE;
            if (aRequest->callBackRoutine != NULL) /* Is the data in memory? */
            {
                voidPtr = (*(PTRfPTRvoid)aRequest->callBackRoutine)(aRequest);
                deallocate =
		    (unsigned char)(aRequest->callBackRoutine == NULLPTR);
            }
            else
            {
                voidPtr = (LeadFiducialDataPtr)aRequest->dataPointer;
            }
            result = 
		setleadfiducials_(outputFile, 
				  ((LeadFiducialDataPtr)voidPtr)->theLabel,
				  ((LeadFiducialDataPtr)voidPtr)->fidDescSize,
				  ((LeadFiducialDataPtr)voidPtr)->fidDesc,
				  ((LeadFiducialDataPtr)voidPtr)->theFids,
				  ((LeadFiducialDataPtr)voidPtr)->theTypes);
            CHECKRESULT;
            if (deallocate)
            {
                free(((LeadFiducialDataPtr)voidPtr)->fidDesc);
                free(((LeadFiducialDataPtr)voidPtr)->theFids);
                free(((LeadFiducialDataPtr)voidPtr)->theTypes);
                free(voidPtr);
            }
            DEL(queuedRequest,thisQueue->addQueue);
            free(queuedRequest);
        }
    }
    FREE_LIST(thisQueue->rewriteQueue); 
    FREE_LIST(thisQueue->addQueue); 
    free(thisQueue);
    return(success);
}    
    
/****
 *
 *	searchList
 *
 ***/

rewriteRequestPtr searchList(queuedRewriteRequestPtr theRewriteQueue, 
			     long theDataType, long whichSurface, 
			     long whichOne)
{
    
    queuedRewriteRequestPtr traceVariable;
    
    /*    printf("Looking for data type %ld\n", theDataType); 
    if (whichSurface != 0) printf ("    for surface or timeseries %ld\n", 
    whichSurface);
    if (whichOne != 0) printf("	for index %ld\n", whichOne);*/
    TRACE(traceVariable, theRewriteQueue)
    {
	if ((traceVariable->theRequest).dataType == theDataType)
	{
	    if ((theDataType == EXPID) || (theDataType == TEXT))
	    {
		/*printf("Rewrite this data hunk\n");*/
		return(&(traceVariable->theRequest));
	    }
	    else if ((traceVariable->theRequest).index == whichSurface)
	    {
		if (((traceVariable->theRequest).dataType >= SURFACENAME) &&
		    ((traceVariable->theRequest).dataType <= 
		     EXTENDEDFIDUCIALS))
		{
		    /*printf("Rewrite this data hunk\n");*/
		    return(&(traceVariable->theRequest));
		}
		else if (((traceVariable->theRequest).dataType >= ELEMENTS) &&
			 ((traceVariable->theRequest).dataType <= 
			  ELEMENTTENSORS) &&
			 ((traceVariable->theRequest).index == whichOne))
		{
		    /*printf("Rewrite this data hunk\n");*/
		    return(&(traceVariable->theRequest));
		}
		else if (((traceVariable->theRequest).dataType == 
			  LEADFIDUCIALS) &&
			 ((traceVariable->theRequest).index == whichSurface) &&
			 ((traceVariable->theRequest).valueType == whichOne))
		{
		    /*printf("Rewrite this data hunk\n");*/
		    return(&(traceVariable->theRequest));
		}
	    }
	}
    }
    /*    printf("No request to rewrite this data hunk\n");*/
    return(NULLPTR); /* if we're here, there must be no requests 
			of this type on the list */
}

rewriteRequestPtr searchAddList(queuedRewriteRequestPtr addQueue, 
				long theDataType, long whichSurface)
{
    
    queuedRewriteRequestPtr traceVariable;
    traceVariable = searchAddList2(addQueue, theDataType, whichSurface);
    return (traceVariable == NULLPTR) ? NULLPTR : &(traceVariable->theRequest);
}

queuedRewriteRequestPtr searchAddList2(queuedRewriteRequestPtr 
				       theAddQueue, long theDataType, 
				       long whichSurface)
{
    
    queuedRewriteRequestPtr traceVariable;
    
    /*    printf("Looking for data type to add %ld\n", theDataType);
    if (whichSurface != 0) printf ("    for surface or timeseries %ld\n", 
    whichSurface);*/
    TRACE(traceVariable, theAddQueue)
    {
	if ((traceVariable->theRequest).dataType == theDataType)
	{
	    if ((theDataType == EXPID) || (theDataType == TEXT))
	    {
		printf("Adds not supported for this data hunk\n");
		return(NULLPTR);
	    }
	    else if ((traceVariable->theRequest).index == whichSurface)
	    {
		if (((traceVariable->theRequest).dataType >= SURFACENAME) &&
		    ((traceVariable->theRequest).dataType <= 
		     EXTENDEDFIDUCIALS))
		{
		    printf("Adds not supported for this data hunk\n");
		    return(NULLPTR);
		}
		else if (((traceVariable->theRequest).dataType >= ELEMENTS) &&
			 ((traceVariable->theRequest).dataType <= 
			  ELEMENTTENSORS))
		{
		    printf("Adds not supported for this data hunk\n");
		    return(NULLPTR);
		}
		else if (((traceVariable->theRequest).dataType == 
			  LEADFIDUCIALS) &&
			 ((traceVariable->theRequest).index == whichSurface))
		{
		    /*printf("Add this data hunk\n");*/
		    return(traceVariable);
		}
	    }
	}
    }
    /*    printf("No request to rewrite this data hunk\n");*/
    return(NULLPTR); /* if we're here, there must be no requests 
			of this type on the list */
}
