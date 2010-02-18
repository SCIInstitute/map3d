C***  Last update: Sat Apr 10 13:48:35 1999 by Rob MacLeod
C***   - updated by Rob to fix problems converting C strings back 
C***   to Fortran.
C***********************************************************************
      integer function FCreateFile(fileName, fileType, 
     +    errorLevel, theFile)
      implicit none
      character*(*) fileName
      integer*4 fileType, errorLevel, theFile
      character*80 temp
      integer*2 bytes(40)
      integer*4 createfile,length,i
      equivalence(bytes(1),temp)
      
      
      length = len(fileName)
      do i=length,1,-1
          if (fileName(i:i).ne.' ') go to 3
      end do
 3    temp=fileName
      temp(i+1:i+1) = char(0)
      FCreateFile = createfile(bytes, %val(fileType), %val(errorLevel),
     +    theFile)
      return
      end
      
C***********************************************************************
      integer function FOpenFile(fileName, errorLevel, theFile)
      implicit none
      character*(*) fileName
      integer errorLevel, theFile
      character*80 temp
      integer*2 bytes(40)
      integer openfile,length,i
      equivalence(bytes(1),temp)
      
      
      length = len(fileName)
      do i=length,1,-1
          if (fileName(i:i).ne.' ') go to 3
      end do
 3    temp=fileName
      temp(i+1:i+1) = char(0)
      FOpenFile = openfile(bytes, %val(errorLevel), theFile)
      return
      end

C***********************************************************************
      integer function FOpenAccess(tapeNumber, filePrefix, tapeType, 
     +    errorLevel, theAccess)
      implicit none
      character*(*) filePrefix
      integer tapeNumber, tapeType, errorLevel, theAccess
      integer openaccess, length, i
      character*80 temp
      integer*2 bytes(40)
      equivalence(bytes(1),temp)

      length = len(filePrefix)
      do i=length,1,-1
          if (filePrefix(i:i).ne.' ') go to 3
      end do
 3    temp=filePrefix
      temp(i+1:i+1) = char(0)
      FOpenAccess = openaccess(%val(tapeNumber), bytes,
     +    %val(tapeType), %val(errorLevel), theAccess)
      return
      end

      
C***********************************************************************
      integer function FCloseFile(fileNumber)
      implicit none
      integer fileNumber
      integer closefile

      FCloseFile = closefile(%val(fileNumber))
      return
      end
      
C***********************************************************************
      integer function FGetFileInfo(fileNumber,
     +    fileType, numberOfSurfaces,  
     +    numberOfBoundarySurfaces,
     +    numberOfTimeSeriesBlocks, preferedSettings)
      implicit none
      integer fileNumber, numberOfSurfaces, fileType
      integer numberOfBoundarySurfaces
      integer numberOfTimeSeriesBlocks, getfileinfo
      logical*1 preferedSettings
      
      FGetFileInfo = getfileinfo(%val(fileNumber), fileType,
     +    numberOfSurfaces, numberOfBoundarySurfaces,
     +    numberOfTimeSeriesBlocks, preferedSettings)
      return
      end
      
C***********************************************************************
      integer function FGetMappingFiles(fileNumber, 
     +    theDefaultMap, theMapInUse)
      implicit none
      integer fileNumber, getmappingfiles, i
      character*(*) theDefaultMap
      character*(*) theMapInUse
      character*80 temp, temp1
      logical*1 bytes(128)
      equivalence(bytes(1),temp)
      logical*1 bytes1(128)
      equivalence(bytes1(1),temp1)
      
      FGetMappingFiles =  getmappingfiles(%val(fileNumber), 
     +    bytes, bytes1)
      do i=1,80
          if (bytes(i) .eq. 0) go to 3
      end do
 3    theDefaultMap(1:i-1) = temp(1:i-1)
      theDefaultMap(i:80) = ' '
      do i=1,80
          if (bytes(i) .eq. 0) go to 4
      end do
 4    theMapInUse(1:i-1) = temp(1:i-1)
      theMapInUse(i:80) = ' '
      return
      end

C***********************************************************************
      integer function FSetMappingFile(fileNumber, theMapToUse)
      implicit none
      integer fileNumber
      character*(*) theMapToUse
      character*80 temp
      integer*2 bytes(40)
      integer setmappingfile,length,i
      equivalence(bytes(1),temp)
      
      
      length = len(theMapToUse)
      do i=length,1,-1
          if (theMapToUse(i:i).ne.' ') go to 3
      end do
 3    temp=theMapToUse
      temp(i+1:i+1) = char(0)
      FSetMappingFile =  setmappingfile(%val(fileNumber), bytes)
      return
      end

C***********************************************************************
      integer function FSetExpID(fileNumber, text)
      implicit none
      integer fileNumber
      character*(*) text
      character*80 temp
      integer*2 bytes(40)
      integer setexpid,length,i
      equivalence(bytes(1),temp)
      
      
      length = len(text)
      do i=length,1,-1
          if (text(i:i).ne.' ') go to 3
      end do
 3    temp=text
      temp(i+1:i+1) = char(0)
c      type 100,fileNumber
c 100  format(1x,z12)
      FSetExpID = setexpid(%val(fileNumber),bytes)
      return
      end
      
C***********************************************************************
      integer function FGetExpID(fileNumber, theID)
      implicit none
      integer fileNumber, getexpid, i
      character*(*) theID
      character*80 temp
      integer*2 bytes(128)
      equivalence(bytes(1),temp)
      
      FGetExpID = getexpid(%val(fileNumber),bytes)
C      do i=1,80
C          if (bytes(i) .eq. 0) go to 3
C      end do
C 3    theID(1:i-1) = temp(1:i-1)
C      theID(i:80) = ' '
C***  Fix up the returned string
      CALL FixString (temp, theID)
      return
      end
      
C***********************************************************************
      integer function FSetText(fileNumber, text)
      implicit none
      integer fileNumber
      character*(*) text
      character*256 temp
      integer*2 bytes(128)
      integer settext, length, i
      equivalence(bytes(1),temp)
      
      
      length = len(text)
      do i=length,1,-1
          if (text(i:i).ne.' ') go to 3
      end do
 3    temp=text
      temp(i+1:i+1) = char(0)
      FSetText = settext(%val(fileNumber),bytes)
      return
      end
      
C***********************************************************************
      integer function FGetText(fileNumber, text)
      implicit none
      integer fileNumber, gettext, i
      character*(*) text
      character*256 temp
      integer*2 bytes(128)
      equivalence(bytes(1),temp)
      
      FGetText = gettext(%val(fileNumber),bytes)
C      do i=1,256
C          if (bytes(i) .eq. 0) go to 3
C      end do
C 3    text(1:i-1) = temp(1:i-1)
C      text(i:256) = ' '
C*** Fix up the string
      CALL FixString (temp, text)
      return
      end
      
C***********************************************************************
      integer function FSetSurfaceIndex(fileNumber, theSurface)
      implicit none
      integer fileNumber, theSurface
      integer setsurfaceindex
      
      FSetSurfaceIndex = setsurfaceindex(%val(fileNumber), 
     +    %val(theSurface))
      return
      end
      
C***********************************************************************
      integer function FGetSurfaceIndex(fileNumber, theSurface)
      implicit none
      integer fileNumber, theSurface
      integer getsurfaceindex
      
      FGetSurfaceIndex = getsurfaceindex(%val(fileNumber), theSurface)
      return
      end
 
C***********************************************************************
      integer function FSetSurfaceName(fileNumber, name)
      implicit none
      integer fileNumber
      character*(*) name
      character*80 temp
      integer*2 bytes(40)
      integer setsurfacename,length,i
      equivalence(bytes(1),temp)
      
      
      length = len(name)
      do i=length,1,-1
          if (name(i:i).ne.' ') go to 3
      end do
 3    temp=name
      temp(i+1:i+1) = char(0)
      FSetSurfaceName = setsurfacename(%val(fileNumber),bytes)
      return
      end
      
C***********************************************************************
      integer function FGetSurfaceName(fileNumber, name)
      implicit none
      integer fileNumber, getsurfacename, i
      character*(*) name
      character*80 temp
      integer*2 bytes(128)
      equivalence(bytes(1),temp)
      
      FGetSurfaceName = getsurfacename(%val(fileNumber),bytes)
C      do i=1,80
C          if (bytes(i) .eq. 0) go to 3
C     end do
C 3    name(1:i-1) = temp(1:i-1)
C      name(i:80) = ' '
C***  Fix up the text
      CALL FixString (temp, name)
      return
      end

C***********************************************************************
      integer function FSetSurfaceType(fileNumber, theType)
      implicit none
      integer fileNumber, theType
      integer setsurfacetype

      FSetSurfaceType = setsurfacetype(%val(fileNumber), %val(theType))
      end

C***********************************************************************
      integer function FGetSurfaceType(fileNumber, theType)
      implicit none
      integer fileNumber, theType
      integer getsurfacetype

      FGetSurfaceType = getsurfacetype(%val(fileNumber), theType)
      end

C***********************************************************************
      integer function FSetNodes(fileNumber, 
     +    numberOfNodes, theNodes)
      implicit none
      integer fileNumber
      integer numberOfNodes
      real*4 theNodes(*)        !actually (3,*)
      integer setnodes
      
      FSetNodes = setnodes(%val(fileNumber), 
     +    %val(numberOfNodes), theNodes)
      return
      end
      
C***********************************************************************
      integer function FGetNodeInfo(fileNumber, numberOfNodes,
     +    scalars, vectors, tensors)
      implicit none
      integer fileNumber, numberOfNodes
      integer scalars, vectors, tensors
      integer getnodeinfo
      
      FGetNodeInfo = getnodeinfo(%val(fileNumber), numberOfNodes,
     +    scalars, vectors, tensors)
      return
      end
      
C***********************************************************************
      integer function FGetNodes(fileNumber, theNodes)
      implicit none
      integer fileNumber
      real*4 theNodes(*)        !actually (3,*)
      integer getnodes
      
      FGetNodes = getnodes(%val(fileNumber), theNodes)
      return
      end
      
C***********************************************************************
      integer function FSetElements(fileNumber, sizeOfElements,
     +    numberOfElements, theElements)
      implicit none
      integer fileNumber
      integer sizeOfElements
      integer numberOfElements
      integer theElements(sizeOfElements,*)
      Integer setelements
      
      FSetElements = setelements(%val(fileNumber),
     +    %val(sizeOfElements), %val(numberOfElements),
     +    theElements)
      return
      end
      
C***********************************************************************
      integer function FGetElementInfo(fileNumber, numberOfElements,
     +    elementSize, scalars, vectors, tensors)
      implicit none
      integer fileNumber, numberOfElements
      integer elementSize
      integer scalars, vectors, tensors
      integer getelementinfo

      FGetElementInfo = getelementinfo(%val(fileNumber),
     +    numberOfElements,
     +    elementSize, scalars, vectors, tensors)
      return
      end
          
C***********************************************************************
      integer function FGetElements(fileNumber, theElements)
      implicit none
      integer fileNumber
      integer theElements(*)  !actually (sizeOfElements,*)
      integer getelements
      
      FGetElements = getelements(%val(fileNumber), theElements)
      return
      end
      
C***********************************************************************
      integer function FSetNodeScalars(fileNumber, type,
     +    numberOfScalars, theScalars)
      implicit none
      integer fileNumber
      integer numberOfScalars,type
      real*4 theScalars(*) 
      integer setnodescalars

      FSetNodeScalars = setnodescalars(%val(fileNumber), %val(type),
     +    %val(numberOfScalars), theScalars)
      return
      end
      
C***********************************************************************
      integer function FGetNodeScalars(fileNumber, index,
     +    type, theScalars)
      implicit none
      integer fileNumber, index, type
      real*4 theScalars(*)
      integer getnodescalars
      
      FGetNodeScalars = getnodescalars(%val(fileNumber), %val(index),
     +    type, theScalars)
      return
      end
      
C***********************************************************************
      integer function FGetNodeScalarTypes(fileNumber, theTypes)
      implicit none
      integer fileNumber, theTypes(*)
      integer getnodescalartypes

      FGetNodeScalarTypes = getnodescalartypes(%val(fileNumber),
     +    theTypes)
      return
      end
      
      
C***********************************************************************
      integer function FSetNodeVectors(fileNumber, type,
     +    numberOfVectors, theVectors)
      implicit none
      integer fileNumber
      integer numberOfVectors,type
      real*4 theVectors(*)      !actually (3,*)
      integer setnodevectors
      
      FSetNodeVectors = setnodevectors(%val(fileNumber), %val(type),
     +    %val(numberOfVectors), theVectors)
      return
      end
      
C***********************************************************************
      integer function FGetNodeVectors(fileNumber, index,
     +    type, theVectors)
      implicit none
      integer fileNumber, index, type
      real*4 theVectors(*)      !actually (3,*)
      integer getnodevectors
      
      FGetNodeVectors = getnodevectors(%val(fileNumber), 
     +    %val(index),
     +    type, theVectors)
      return
      end
      
C***********************************************************************
      integer function FGetNodeVectorTypes(fileNumber, theTypes)
      implicit none
      integer fileNumber, theTypes(*)
      integer getnodevectortypes

      FGetNodeVectorTypes = getnodevectortypes(%val(fileNumber),
     +    theTypes)
      return
      end
      
C***********************************************************************
      integer function FSetNodeTensors(fileNumber, type,
     +    theDimension, numberOfTensors, theTensors)
      implicit none
      integer fileNumber
      integer numberOfTensors,type, theDimension
      real*4 theTensors(theDimension,theDimension,*)
      integer setnodetensors
      
      
      FSetNodeTensors = setnodetensors(%val(fileNumber), %val(type),
     +    %val(theDimension), %val(numberOfTensors), theTensors)
      return
      end
      
C***********************************************************************
      integer function FGetNodeTensors(fileNumber, index,
     +    theDimension, type, theTensors)
      implicit none
      integer fileNumber, index, type, theDimension
      real*4 theTensors(theDimension,theDimension,*)
      Integer getnodetensors
      
      FGetNodeTensors = getnodetensors(%val(fileNumber), 
     +    %val(index) , theDimension, type, theTensors)
      
      return
      end
      
C***********************************************************************
      integer function FGetNodeTensorTypes(fileNumber, theTypes)
      implicit none
      integer fileNumber, theTypes(*)
      integer getnodetensortypes

      FGetNodeTensorTypes = getnodetensortypes(%val(fileNumber),
     +    theTypes)
      return
      end
      
      
C***********************************************************************
      integer function FSetElementScalars(fileNumber, type,
     +    numberOfScalars, theScalars)
      implicit none
      integer fileNumber
      integer numberOfScalars,type
      real*4 theScalars(*) 
      integer setelementscalars
      
      FSetElementScalars = setelementscalars(%val(fileNumber),
     +    %val(type), %val(numberOfScalars), theScalars)
      return
      end
      
C***********************************************************************
      integer function FGetElementScalars(fileNumber, index,
     +    type, theScalars)
      implicit none
      integer fileNumber, index, type
      real*4 theScalars(*)
      integer getelementscalars
      
      FGetElementScalars = getelementscalars(%val(fileNumber), 
     +    %val(index),
     +    type, theScalars)
      return
      end
      
C***********************************************************************
      integer function FGetElementScalarTypes(fileNumber, theTypes)
      implicit none
      integer fileNumber, theTypes(*)
      integer getelementscalartypes

      FGetElementScalarTypes = getelementscalartypes(%val
     +    (fileNumber), theTypes)
      return
      end
      
      
C***********************************************************************
      integer function FSetElementVectors(fileNumber, type,
     +    numberOfVectors, theVectors)
      implicit none
      integer fileNumber
      integer numberOfVectors,type
      real*4 theVectors(*)      !actually (3,*)
      integer setelementvectors
      
      FSetElementVectors = setelementvectors(%val(fileNumber),
     +    %val(type), %val(numberOfVectors), theVectors)
      return
      end
      
C***********************************************************************
      integer function FGetElementVectors(fileNumber, index,
     +    type, theVectors)
      implicit none
      integer fileNumber, index, type
      real*4 theVectors(*)      !actually (3,*)
      integer getelementvectors
      
      FGetElementVectors = getelementvectors(%val(fileNumber), 
     +    %val(index),
     +    type, theVectors)
      return
      end
      
C***********************************************************************
      integer function FGetElementVectorTypes(fileNumber, theTypes)
      implicit none
      integer fileNumber, theTypes(*)
      integer getelementvectortypes

      FGetElementVectorTypes = getelementvectortypes(%val
     +    (fileNumber), theTypes)
      return
      end

      
C***********************************************************************
      integer function FSetElementTensors(fileNumber, type,
     +    theDimension, numberOfTensors, theTensors)
      implicit none
      integer fileNumber
      integer numberOfTensors,type, theDimension
      real*4 theTensors(theDimension,theDimension,*)
      integer setelementtensors
      
      FSetElementTensors = setelementtensors(%val(fileNumber),
     +    %val(type), %val(theDimension), %val(numberOfTensors), 
     +    theTensors)
      return
      end
      
C***********************************************************************
      integer function FGetElementTensors(fileNumber, index,
     +    theDimension, type, numberOfTensors, theTensors)
      implicit none
      integer fileNumber, index, type, theDimension
      integer numberOfTensors
      real*4 theTensors(theDimension,theDimension,*)
      integer getelementtensors
      
      FGetElementTensors = getelementtensors(%val(fileNumber), 
     +    %val(index),
     +    theDimension, type, theTensors)
      return
      end
      
C***********************************************************************
      integer function FGetElementTensorTypes(fileNumber, theTypes)
      implicit none
      integer fileNumber, theTypes(*)
      integer getelementtensortypes

      FGetElementTensorTypes = getelementtensortypes(%val
     +    (fileNumber), theTypes)
      return
      end
      
C***********************************************************************
      integer function FSetTimeSeriesDataPath(fileNumber, thePath)
      implicit none
      integer fileNumber
      character*(*) thePath
      character*80 temp
      integer*2 bytes(40)
      integer settimeseriesdatapath,length,i
      equivalence(bytes(1),temp)
      
      
      length = len(thePath)
      do i=length,1,-1
          if (thePath(i:i).ne.' ') go to 3
      end do
 3    temp=thePath
      temp(i+1:i+1) = char(0)
      FSetTimeSeriesDataPath = settimeseriesdatapath
     +    (%val(fileNumber),bytes)
      return
      end

C***********************************************************************
      integer function FSetTimeSeriesIndex(fileNumber, theIndex)
      implicit none
      integer fileNumber, theIndex
      integer settimeseriesindex
      
      FSetTimeSeriesIndex = settimeseriesindex(%val(fileNumber), 
     +    %val(theIndex))
      return
      end
      
C***********************************************************************
      integer function FGetTimeSeriesIndex(fileNumber, theIndex)
      implicit none
      integer fileNumber, theIndex
      integer gettimeseriesindex
      
      FGetTimeSeriesIndex = gettimeseriesindex(%val(fileNumber), 
     +    theIndex)
      return
      end
      
C***********************************************************************
      integer function FSetTimeSeriesFile(fileNumber, theFile)
      implicit none
      integer fileNumber
      character*(*) theFile
      character*80 temp
      integer*2 bytes(40)
      integer settimeseriesfile,length,i
      equivalence(bytes(1),temp)
      
      
      length = len(theFile)
      do i=length,1,-1
          if (theFile(i:i).ne.' ') go to 3
      end do
 3    temp=theFile
      temp(i+1:i+1) = char(0)
      FSetTimeSeriesFile = settimeseriesfile(%val(fileNumber),bytes)
      return
      end
      
C***********************************************************************
      integer function FGetTimeSeriesFile(fileNumber, theFile)
      implicit none
      integer fileNumber, gettimeseriesfile, i
      character*(*) theFile
      character*80 temp
      integer*2 bytes(128)
      equivalence(bytes(1),temp)
      
      FGetTimeSeriesFile = gettimeseriesfile(%val(fileNumber),bytes)
C      do i=1,80
C          if (bytes(i) .eq. 0) go to 3
C      end do
C 3    theFile(1:i-1) = temp(1:i-1)
C      theFile(i:80) = ' '
C***  Fix up the text
      CALL FixString (temp, theFile)
      return
      end


C***********************************************************************
      integer function FSetTimeSeriesGeomFile(fileNumber, theFile)
      implicit none
      integer fileNumber
      character*(*) theFile
      character*80 temp
      integer*2 bytes(40)
      integer settimeseriesgeomfile,length,i
      equivalence(bytes(1),temp)
      
      
      length = len(theFile)
      do i=length,1,-1
          if (theFile(i:i).ne.' ') go to 3
      end do
 3    temp=theFile
      temp(i+1:i+1) = char(0)
      FSetTimeSeriesGeomFile = 
     +    settimeseriesgeomfile(%val(fileNumber),bytes)
      return
      end
      
C***********************************************************************
      integer function FGetTimeSeriesGeomFile(fileNumber, theFile)
      implicit none
      integer fileNumber, gettimeseriesgeomfile, i
      character*(*) theFile
      character*80 temp
      integer*2 bytes(128)
      equivalence(bytes(1),temp)
      
      FGetTimeSeriesGeomFile = 
     +    gettimeseriesgeomfile(%val(fileNumber),bytes)
C      do i=1,80
C          if (bytes(i) .eq. 0) go to 3
C      end do
C 3    theFile(1:i-1) = temp(1:i-1)
C      theFile(i:80) = ' '
C***  Fix up the text
      CALL FixString (temp, theFile)
      return
      end


C***********************************************************************
      integer function FSetTimeSeriesLabel(fileNumber, theLabel)
      implicit none
      integer fileNumber
      character*(*) theLabel
      character*80 temp
      byte bytes(80)
      integer settimeserieslabel,length,i
      equivalence(bytes(1),temp)
      
      
      length = len(theLabel)
      do i=length,1,-1
          if (theLabel(i:i).ne.' ') go to 3
      end do
 3    temp=theLabel
      temp(i+1:i+1) = char(0)
      FSetTimeSeriesLabel = settimeserieslabel(%val(fileNumber),bytes)
      return
      end
      
C***********************************************************************
      integer function FGetTimeSeriesLabel(fileNumber, theLabel)
      implicit none
      integer fileNumber, gettimeserieslabel, i
      character*(*) theLabel
      character*80 temp
      byte bytes(40)
      equivalence(bytes(1), temp)
      
      FGetTimeSeriesLabel = gettimeserieslabel(%val(fileNumber),
     +    bytes(1))
C      do i=1,80
C          if (bytes(i) .eq. 0) go to 3
C      end do
C 3    theLabel(1:i-1) = temp(1:i-1)
C***  Fix up the text
      CALL FixString (temp, theLabel)
      return
      end

C***********************************************************************
      integer function FSetTimeSeriesSpecs(fileNumber,
     +    numberOfChannels, numberOfFrames)
      implicit none
      integer fileNumber, numberOfChannels, numberOfFrames
      integer settimeseriesspecs
      
      FSetTimeSeriesSpecs = settimeseriesspecs(%val(fileNumber), 
     +    %val(numberOfChannels), %val(numberOfFrames))
      return
      end
      
C***********************************************************************
      integer function FGetTimeSeriesSpecs(fileNumber,
     +    numberOfChannels, numberOfFrames)
      implicit none
      integer fileNumber, numberOfChannels, numberOfFrames
      integer gettimeseriesspecs
      
      FGetTimeSeriesSpecs = gettimeseriesspecs(%val(fileNumber),
     +    numberOfChannels, numberOfFrames)
      return
      end

C***********************************************************************
      integer function FSetTimeSeriesFormat(fileNumber, theFormat)
      implicit none
      integer fileNumber, theFormat
      integer settimeseriesformat
      
      FSetTimeSeriesFormat = settimeseriesformat(%val(fileNumber), 
     +    %val(theFormat))
      return
      end
      
C***********************************************************************
      integer function FGetTimeSeriesFormat(fileNumber)
      implicit none
      integer fileNumber
      integer gettimeseriesformat
      
      FGetTimeSeriesFormat = gettimeseriesformat(%val(fileNumber))
      return
      end
      
C***********************************************************************
      integer function FSetTimeSeriesUnits(fileNumber, theUnits)
      implicit none
      integer fileNumber, theUnits
      integer settimeseriesunits
      
      FSetTimeSeriesUnits = settimeseriesunits(%val(fileNumber), 
     +    %val(theUnits))
      return
      end
      
C***********************************************************************
      integer function FGetTimeSeriesUnits(fileNumber, theUnits)
      implicit none
      integer fileNumber, theUnits
      integer gettimeseriesunits
      
      FGetTimeSeriesUnits = gettimeseriesunits(%val(fileNumber), 
     +    theUnits)
      return
      end
     
C***********************************************************************
      integer function FSetTimeSeriesSurface(fileNumber, theSurface)
      implicit none
      integer fileNumber, theSurface
      integer settimeseriessurface
      
      FSetTimeSeriesSurface = settimeseriessurface(%val(fileNumber), 
     +    %val(theSurface))
      return
      end
      
C***********************************************************************
      integer function FGetTimeSeriesSurface(fileNumber, theSurface)
      implicit none
      integer fileNumber, theSurface
      integer gettimeseriessurface
      
      FGetTimeSeriesSurface = gettimeseriessurface(%val(fileNumber), 
     +    theSurface)
      return
      end
      
C***********************************************************************
      integer function FSetTimeSeriesAssoc(fileNumber, theAssociation)
      implicit none
      integer fileNumber, theAssociation
      integer settimeseriesassoc
      
      FSetTimeSeriesAssoc = settimeseriesassoc(%val(fileNumber), 
     +    %val(theAssociation))
      return
      end
      
C***********************************************************************
      integer function FGetTimeSeriesAssoc(fileNumber, theAssociation)
      implicit none
      integer fileNumber, theAssociation
      integer gettimeseriesassoc
      
      FGetTimeSeriesAssoc = gettimeseriesassoc(%val(fileNumber),
     +    theAssociation)
      return
      end
      
C***********************************************************************
      integer function FSetTimeSeriesData(fileNumber, theData)
      implicit none
      integer fileNumber
      real  theData(*)
      integer settimeseriesdata
      
      FSetTimeSeriesData = settimeseriesdata(%val(fileNumber), 
     +   theData)
      return
      end
      


C***********************************************************************
      integer function FGetTimeSeriesData(fileNumber, theData)
      implicit none
      integer fileNumber
      real theData(*)
      integer gettimeseriesdata
      
      FGetTimeSeriesData = gettimeseriesdata(%val(fileNumber),
     +    theData)
      return
      end
      


C***********************************************************************
      integer function FGetSelectedTimeSeriesData(fileNumber,
     +    startingFrame, numberOfFrames, theData, returnedFormat)
      implicit none
      integer fileNumber, theData(*), startingFrame, numberOfFrames
      integer getselectedtimeseriesdata, returnedFormat

      FGetSelectedTimeSeriesData = getselectedtimeseriesdata(
     $    %val(fileNumber), %val(startingFrame), numberOfFrames,
     $    theData, %val(returnedFormat))
      return
      end
      
C***********************************************************************
      integer function FGetSelectedTimeSeriesChannel(fileNumber,
     +    selectedChannel, startingFrame, numberOfFrames,
     +    theChannel, returnedFormat)
      implicit none
      integer fileNumber, theChannel(*), startingFrame, numberOfFrames
      integer getselectedtimeserieschannel, returnedFormat
      integer selectedChannel

      FGetSelectedTimeSeriesChannel = getselectedtimeserieschannel(
     +    %val(fileNumber), %val(selectedChannel), %val(startingFrame), 
     +    numberOfFrames, theChannel, %val(returnedFormat))
      return
      end
      


C***********************************************************************
      integer function FSetNumCorrectedLeads(fileNumber,
     +    numberOfCorrectedLeads)
      implicit none
      integer fileNumber, numberOfCorrectedLeads
      integer setnumcorrectedleads
      
      FSetNumCorrectedLeads = setnumcorrectedleads(%val(fileNumber), 
     +    %val(numberOfCorrectedLeads))
      return
      end
      
C***********************************************************************
      integer function FGetNumCorrectedLeads(fileNumber,
     +    numberOfCorrectedLeads)
      implicit none
      integer fileNumber
      integer getnumcorrectedleads
      integer numberOfCorrectedLeads

      FGetNumCorrectedLeads = getnumcorrectedleads(%val(fileNumber),
     +    numberOfCorrectedLeads)
      return
      end
      
C***********************************************************************
      integer function FSetCorrectedLeads(fileNumber, theLeads)
      implicit none
      integer fileNumber, theLeads(*)
      integer setcorrectedleads
      
      FSetCorrectedLeads = setcorrectedleads(%val(fileNumber), 
     +    theLeads)
      return
      end
      
C***********************************************************************
      integer function FGetCorrectedLeads(fileNumber, theLeads)
      implicit none
      integer fileNumber, theLeads(*)
      integer getcorrectedleads
      
      FGetCorrectedLeads = getcorrectedleads(%val(fileNumber),
     +    theLeads)
      return
      end
      
C***********************************************************************
      integer function FSetPowerCurve(fileNumber, powerCurveData)
      implicit none
      integer fileNumber
      real powerCurveData(*)
      integer setpowercurve
      
      FSetPowerCurve = setpowercurve(%val(fileNumber), 
     +    powerCurveData)
      return
      end
      
C***********************************************************************
      integer function FGetPowerCurve(fileNumber, powerCurveData)
      implicit none
      integer fileNumber
      real powerCurveData(*)
      integer getpowercurve
      
      FGetPowerCurve = getpowercurve(%val(fileNumber), 
     +    powerCurveData)
      return
      end
      
C***********************************************************************
      integer function FSetQSTTimes(fileNumber, qTime, sTime, tTime)
      implicit none
      integer fileNumber, qTime, sTime, tTime
      integer setqsttimes
      
      FSetQSTTimes = setqsttimes(%val(fileNumber), 
     +    %val(qTime), %val(sTime), %val(tTime))
      return
      end
      
C***********************************************************************
      integer function FGetQSTTimes(fileNumber, qTime, sTime, tTime)
      implicit none
      integer fileNumber, qTime, sTime, tTime
      integer getqsttimes
      
      FGetQSTTimes = getqsttimes(%val(fileNumber), qTime, sTime, tTime)
      return
      end
      
C***********************************************************************
      integer function FSetExtendedFiducials(fileNumber, ponset,
     +    poffset, rpeak, tpeak)
      implicit none
      integer fileNumber, ponset, poffset, rpeak, tpeak
      integer setextendedfiducials
      
      FSetExtendedFiducials = setextendedfiducials(%val(fileNumber), 
     +    %val(ponset), %val(poffset), %val(rpeak), %val(tpeak))
      return
      end
      
C***********************************************************************
      integer function FGetExtendedFiducials(fileNumber, ponset,
     +    poffset, rpeak, tpeak)
      implicit none
      integer fileNumber, ponset, poffset, rpeak, tpeak
      integer getextendedfiducials
      
      FGetExtendedFiducials = getextendedfiducials(%val(fileNumber),
     +    ponset, poffset, rpeak, tpeak)
      return
      end
      
C***********************************************************************
      integer function FCheckExtendedFiducials(
     +    fileNumber, available)
      implicit none
      integer fileNumber
      logical*1 available
      integer checkextendedfiducials

      FCheckExtendedFiducials = checkextendedfiducials
     +    (%val(fileNumber), available)
      return
      end

C***********************************************************************

      SUBROUTINE FixString ( instring, outstring)

C***  A utility routine to fix strings that come back from C
C***  calls in graphicsio.
C***  
C***  Input: 
C***   instring     string to be cleaned up
C***  Output:
C***   outstring    cleaned up string
C***  Note: instring can be the same as outstring, but instring is 
C***        never changed directly.
C***  
C***  Cleaned up here means that all unprintiable characters and 
C***  string temrinator characters get removed and a string ready
C***  for use in Fortran is returned.
C***  
C***********************************************************************
      IMPLICIT NONE 
      INTEGER i, start, end
      CHARACTER instring*(*), outstring*(*), astring*200
C***********************************************************************
C***  Run through the string from the back to get last good character

      astring = instring
      i = len(astring)
      DO WHILE (astring(i:i) .LT. '!' .OR. astring(i:i) .GT. '~')
          i = i - 1
      END DO 
      end = i

C***  Now run from the front to the back to get rid of junk there.

      i = 1
      DO WHILE (i .LE. end .AND. astring(i:i) .NE. char(0))
          IF ( astring(i:i) .LT. '!' .OR. astring(i:i) .GT. '~') 
     +        astring(i:i) = ' '
          i = i + 1
      END DO 
      end = i - 1
      start = 1

C***  Now take the string between start and end and send it back.

      outstring = astring(start:end)//' '
      RETURN
      END
