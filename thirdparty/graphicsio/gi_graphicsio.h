#ifndef __GI_GRAPHICSIO_H__
#define __GI_GRAPHICSIO_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef VAXVERSION
#include <stdlib.h>
#include <stdio.h>
#include <unixio.h>
#include <file.h>
#include <time.h>
#include <string.h>

#else // anything but VAX
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#ifdef OSX
#include <sys/malloc.h>
#else
#include <malloc.h> 
#endif

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#endif

#include "gi_list.h"

typedef unsigned long FilePtr;
typedef float Scalar;
typedef float *ScalarPtr;
typedef float *TensorPtr;
typedef unsigned char Boolean;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



/* Define some general purpose error reporting macros */

#define SCREAM(message, function, returnValue, optionalString) \
if (thisFile->errorLevel) report(message, function, \
returnValue, optionalString); \
 return(returnValue)
#define CHECKRESULT if (result < success) {printf("file error\n"); return(result);}

typedef struct FileHeader
{
    long	headerSize;
    long    	fileType;
    FilePtr	fileInfoBlkPtr;
    long	numberOfSurfaces;
    FilePtr	firstSurfaceHeader;
    long	numberOfBoundSurfaces;
    FilePtr	firstBoundSurfaceHeader;
    long    	numberOfTimeSeries;
    FilePtr 	firstTimeSeriesHeader;
    FilePtr	preferedSettingsInfoBlock; /* header size was 40 bytes to here */
    FilePtr     auditString;
}FileHeader, *FileHeaderPtr;

	/* Offsets from beginning of FileHeader */

#define FTOFFSET    4
#define FIBOFFSET   8
#define NSOFFSET    12
#define FSHOFFSET   16
#define NBSOFFSET   20
#define FBSHOFFSET  24
#define NTSOFFSET   28
#define FTSOFFSET   32
#define PSIBOFFSET  36
#define ASOFFSET    40

enum fileUsage {FILETYPE_DATA = 1, FILETYPE_GEOM};

typedef struct FileInfoBlock
{
    long	blockSize;
    char	expID[80];
    char    	runID[80];
    time_t	creationTime;
    time_t	lastUpdateTime;
    char	text[256];
}FileInfoBlock, *FileInfoBlockPtr;

	/* Offsets from beginning of FileInfoBlock */

#define EXPIDOFFSET 	4
#define RUNIDOFFSET 	84
#define CTIMEOFFSET 	164
#define LUTIMEOFFSET    168
#define TEXTOFFSET  	172

enum ioResult {unspecifiedError = -17, noAuditString, invalidSurface, invalidTimeSeries, noElements, noTensors, 
	       noVectors, noScalars, wrongNumber, noNodes, invalidOperation, 
	       InvalidSurface, fileReadError, cantOpenFile, fileWriteError, 
	       cantCreateFile, noDynamicMemory, success = 0, existingSurface, 
	       firstSurface, newSurface, existingSeries, firstSeries, 
	       newSeries, noPowerCurve};

typedef struct SurfaceHeader
{
    long    headerSize;
    FilePtr nextSurfaceHeader;
    FilePtr nodeHeader;
    FilePtr elementHeader;
    FilePtr surfaceInfoBlk;
}SurfaceHeader, *SurfaceHeaderPtr;

	/* Offsets from beginning of SurfaceHeader */

#define NSHOFFSET 4
#define NHOFFSET 8
#define EHOFFSET 12
#define SIBOFFSET 16

typedef struct SurfaceInfoBlock
{
    long    headerSize;
    char    name[80];
    long    surfaceType;
}SurfaceInfoBlock, *SurfaceInfoBlockPtr;

	/* Offsets from beginning of SurfaceInfoBlock */

#define NMOFFSET 4
#define STOFFSET 84

/*enum SurfaceStatus {invalidSurface= -1, firstSurface,
		    newSurface, validSurface};*/
/* defined above */

typedef struct NodeHeader
{
    long    headerSize;
    long    numberOfNodes;
    FilePtr theNodes;
    long    numberOfNodeScalarValues;
    FilePtr firstNodeScalarValueHeader;
    long    numberOfNodeVectorValues;
    FilePtr firstNodeVectorValueHeader;
    long    numberOfNodeTensorValues;
    FilePtr firstNodeTensorValueHeader;
}NodeHeader, *NodeHeaderPtr;

	/* Offsets from beginning of NodeHeader */

#define NONOFFSET 4
#define TNOFFSET 8
#define NONSVOFFSET 12
#define FNSVHOFFSET 16
#define NONVVOFFSET 20
#define FNVVHOFFSET 24
#define NONTVOFFSET 28
#define FNTVHOFFSET 32

typedef struct ScalarValueHeader
{
    long    headerSize;
    FilePtr nextScalarValueHeader;
    long    scalarValueType;
    FilePtr theScalars;
}ScalarValueHeader, *ScalarValueHeaderPtr;

	/* Offsets from the beginning of the Scalar Value Header */

#define NSVHOFFSET 4
#define SVTOFFSET 8
#define TSOFFSET 12

enum VectorType {VECTYPE_NORMAL = 1, VECTYPE_FIBDIREC};

typedef struct VectorValueHeader
{
    long    	headerSize;
    FilePtr 	nextVectorValueHeader;
    long	vectorValueType;
    FilePtr 	theVectors;
}VectorValueHeader, *VectorValueHeaderPtr;


	/* Offsets from the beginning of the Vector Value Header */

#define NVVHOFFSET 4
#define VVTOFFSET 8
#define TVOFFSET 12

typedef struct TensorValueHeader
{
    long    headerSize;
    FilePtr nextTensorValueHeader;
    long    tensorValueType;	   
    long    tensorDimension; 
    FilePtr theTensors;
}TensorValueHeader, *TensorValueHeaderPtr;


	/* Offsets from the beginning of the Tensor Value Header */

#define NTVHOFFSET 4
#define TVTOFFSET 8
#define TTOFFSET 12

typedef struct ElementHeader
{
    long    headerSize;
    long    numberOfElements;
    long    sizeOfElements;
    FilePtr theElements;
    long    numberOfElementScalarValues;
    FilePtr firstElementScalarValueHeader;
    long    numberOfElementVectorValues;
    FilePtr firstElementVectorValueHeader;
    long    numberOfElementTensorValues;
    FilePtr firstElementTensorValueHeader;
}ElementHeader, *ElementHeaderPtr;

	/* Offsets from beginning of ElementHeader */

#define NOEOFFSET 4
#define SOEOFFSET 8
#define TEOFFSET 12
#define NOESVOFFSET 16
#define FESVHOFFSET 20
#define NOEVVOFFSET 24
#define FEVVHOFFSET 28
#define NOETVOFFSET 32
#define FETVHOFFSET 36

typedef struct TimeSeriesHeader
{
    long    	    	    headerSize;
    FilePtr 	    	    nextTimeSeriesHeader;
    char    	    	    geometryFileName[80];
    long	    	    dataFormat;
    long    	    	    numberOfChannels;
    long    	    	    numberOfFrames;
    FilePtr 	    	    theData;
    FilePtr 	    	    powerCurve;
    long	    	    qtime;
    long	    	    stime;
    long	    	    ttime;
    long		    rrInterval;
    long	    	    numberOfCorrectedChannels;
    FilePtr 	    	    correctedChannels;
    char    	    	    fileName[80];
    char    	    	    label[80];
    long    	    	    assocSurfaceNumber;
    long	    	    association;
    long	    	    dataUnits;
    FilePtr 	    	    preferedScalingBlock; /* to here is 308 bytes */
    long    	    	    ponset;
    long    	    	    poffset;
    long    	    	    rpeak;
    long    	    	    tpeak; /* to here is 324 bytes */
    long    	    	    storageFormat;
    long                    firstLeadFiducialHeader; /* to here is 332 bytes */
    long		    numberOfFiducialSets; /* to here is 336 bytes */
}TimeSeriesHeader, *TimeSeriesHeaderPtr;

#define FLFHOFFSET 328
#define NOFSOFFSET 332

enum stgFormat {STORAGE_FORMAT_INTEGER, STORAGE_FORMAT_FLOAT};
enum rtnFormat {RETURN_FORMAT_INTEGER, RETURN_FORMAT_FLOAT, RETURN_FORMAT_MICROVOLTS};



/*	Offsets from the beginning of the Time Series Header	*/

#define NTSHOFFSET 4

typedef struct LeadFiducialHeader
{
    long    headerSize;
    FilePtr nextLeadFiducialHeader;
    long    fidDescSize;
    long    numberOfFiducials;
    FilePtr fidDesc;
    FilePtr theFids;
    FilePtr theTypes;
    char    theLabel[132];
}LeadFiducialHeader, *LeadFiducialHeaderPtr;

#define NLFHOFFSET 4
#define FIDDOFFSET 16

typedef struct LeadFiducialData
{
    char theLabel[132];
    short fidDescSize;
    short *fidDesc;
    float *theFids;
    short *theTypes;
}LeadFiducialData, *LeadFiducialDataPtr;

typedef struct FidInfo
{
    long	fidDescSize;
    long	numberOfFiducials;
    char	theLabel[132];
}FidInfo, *FidInfoPtr;



typedef struct FileInfo
{
    long		    fileNumber;
    long		    currentSurfaceIndex;
    SurfaceHeaderPtr	    theCurrentSurface;
    FilePtr 	    	    theCurrentSurfaceLocation;
    long		    lastSurfaceIndex;
    FilePtr  	    	    lastSurfaceLocation;
    long	    	    theCurrentTimeSeriesIndex;
    FilePtr 	    	    theCurrentTimeSeriesLocation;
    long		    externalFileNumber;
    FileHeader		    thisFileHeader;
    long    	    	    errorLevel;
    char    	    	    dataPath[80];
    int	    	    	    externalFileType;
    long    	    	    currentTimeSeriesNumberOfFrames;
    long    	    	    currentTimeSeriesNumberOfChannels;
    char		    filePrefix[20];
    char		    theDefaultMap[255];
    char		    theMapInUse[255];
}FileInfo, *FileInfoPtr;

#ifdef FILE_TYPE_UNKNOWN
#undef FILE_TYPE_UNKNOWN
#endif
#ifdef FILE_TYPE_RAW
#undef FILE_TYPE_RAW
#endif
#ifdef FILE_TYPE_PAK
#undef FILE_TYPE_PAK
#endif
#ifdef FILE_TYPE_ACQ
#undef FILE_TYPE_ACQ
#endif


enum extFile {FILE_TYPE_UNKNOWN, FILE_TYPE_RAW, FILE_TYPE_PAK, FILE_TYPE_ACQ};

typedef unsigned char Str11[12];
typedef unsigned char Str29[30];
typedef unsigned char Str79[80];
typedef unsigned char Str13[14];
typedef unsigned char Str255[256];

typedef struct DateTimeRec
{
    short   year;
    short   month;
    short   day;
    short   hour;
    short   minute;
    short   second;
    short   dayOfWeek;
}DateTimeRec;

typedef struct AcqHeader
{
	unsigned short		numMuxedChans;
	unsigned short		numHeaderBytes;
	long			numRefBytes;
	long			numMuxBytes;
	Str79			patientName;
	Str29			patientID;
	Str79			diagnosis;
	Str79			recordingCenter;
	Str11			technicianInitials;
	Str255			additionalNotes;
	Str29			dateString;
	Str11			timeString;
	DateTimeRec		timeAndDate;
	unsigned short		numberOfLeads;
	long			numberOfFrames;
	long			baseline1;
	long			baseline2;
	long			qTime;
	long			sTIme;
	long			tTime;
	short			sex;			/* 0 = male, 1 = female */
	short			sampleRate;
	short			gain;
	Str13			dataSourceBank1;
	Str13			dataSourceBank2;
	Str13			dataSourceBank3;
	Str13			dataSourceBank4;
	Str13			dataSourceBank5;
	Str13			dataSourceBank6;
	Str13			dataSourceBank7;
	Str13			dataSourceBank8;
	char			reserved[274];	/* We want sizeof(FileHeader) = 1024 */
} AcqHeader, *AcqHeaderPtr;

typedef struct PakHeader
{
	short					fileNumber;
	short					tapeNumber;
	short					patientNumber;
	short					runNumber;
	short					numberOfLeads;
	short					numberOfFrames;
	short					qTime;
	short					sTime;
	short					tTime;
	short					rrInterval;
	char					header[40];
}PakHeader, *PakHeaderPtr;

typedef struct RawHeader
{
	short					fileNumber;
	short					tapeNumber;
	short					patientNumber;
	short					runNumber;
	short					numberOfLeads;
	short					numberOfFrames;
	char	    	    	    	    	time[8];
	char					header[40];
}RawHeader, *RawHeaderPtr;

enum DataUnits {UNIT_MVOLTS = 1, UNIT_UVOLTS, UNIT_MSECS, UNIT_VOLTS,
	    	UNIT_MVOLTMSECS};
enum DataAssociation {ASSOC_NODES = 1, ASSOC_WHOLEELEMENT,
	    	    	ASSOC_ELEMENTCENTROID};
enum DataFormat {FORMAT_MUXDATA = 1, FORMAT_SCALARS, FORMAT_CVRTIRAW,
		 FORMAT_CVRTIPAK};

enum ChanAttr { CHAN_ATTR_BAD=(1<<0), CHAN_ATTR_BLANK=(1<<1),
                CHAN_ATTR_INTERP=(1<<2) };

typedef struct Node
{
    float x;
    float y;
    float z;
}Node, *NodePtr;

typedef struct Vector
{
    float x;
    float y;
    float z;
}Vector, *VectorPtr;

typedef struct Tri
{
    long v1;
    long v2;
    long v3;
}Tri, *TriPtr;

typedef struct Tetra
{
    long v1;
    long v2;
    long v3;
    long v4;
}Tetra, *TetraPtr;

typedef long *ElementPtr;


typedef void* (*PTRfPTRvoid)(void*);
typedef char* (*PTRfPTRchar)(void*);
typedef long* (*PTRfPTRlong)(void*);
typedef float* (*PTRfPTRfloat)(void*);
typedef NodePtr (*PTRfPTRnode)(void*);
typedef ElementPtr (*PTRfPTRelement)(void*);
typedef TriPtr (*PTRfPTRtri)(void*);
typedef TetraPtr (*PTRfPTRtetra)(void*);
typedef ScalarPtr (*PTRfPTRscalar)(void*);
typedef VectorPtr (*PTRfPTRvector)(void*);
typedef TensorPtr (*PTRfPTRtensor)(void*);
typedef LeadFiducialDataPtr (*PTRfPTRFiducialData)(void*);

typedef struct rewriteRequest
{
    long    	    dataType;	    /* Data type see enum below */
    long    	    surfaceNumber;  /* Which surface */
    long	    index;  	    /* Index of associated values/timeseries */
    long    	    valueType;	    /* Value type for associated values or 
                                            lead fiducial set index */
    long    	    quantity;	    /* How many (nodes, tris, etc.) */
    long    	    theDimension;   /* Dimension for tensors */
    void    	    *dataPointer;   /* Pointer to data, zero for call back */
    PTRfPTRvoid	    callBackRoutine;/* Address of call back routine */
}rewriteRequest, *rewriteRequestPtr;

/* CAUTION, searchList depends on the proper 
   order of the dataType enum below! */

enum dataType {EXPID, TEXT, SURFACENAME, SURFACETYPE, NODES, TIMESERIESFILE,
	       TIMESERIESGEOMFILE, TIMESERIESSPECS, TIMESERIESLABEL,
	       TIMESERIESFORMAT, TIMESERIESUNITS, TIMESERIESSURFACE,
	       TIMESERIESASSOC, TIMESERIESDATA, NUMCORRECTEDLEADS,
	       CORRECTEDLEADS, POWERCURVE, QSTTIMES, EXTENDEDFIDUCIALS,
	       ELEMENTS, NODESCALARS, NODEVECTORS, NODETENSORS, ELEMENTSCALARS,
	       ELEMENTVECTORS, ELEMENTTENSORS, LEADFIDUCIALS};


typedef struct timeseriesspecs
{
	long	numberOfChannels;
	long	numberOfFrames;
}timeseriesspecs, *timeseriesspecsPtr;

typedef struct fiducials
{
	long	qtime;
	long	stime;
	long	ttime;
}fiducials, *fiducialsPtr;

typedef struct extendedFiducials
{
	long	ponset;
	long	poffset;
	long	rpeak;
	long	tpeak;
}extendedFiducials, *extendedFiducialsPtr;

typedef struct queuedRewriteRequest
{
    TLISTHDR(struct queuedRewriteRequest);
    rewriteRequest  theRequest;
}queuedRewriteRequest, *queuedRewriteRequestPtr;

typedef struct rewriteQueues
{
	queuedRewriteRequestPtr rewriteQueue;
	queuedRewriteRequestPtr addQueue;
}rewriteQueue, *rewriteQueuePtr;


	/* Function Prototypes */


/* Basic File Routines */

long createfile_(char *fileName, long fileType, 
		 long errorLevel, FileInfoPtr *theFile);
long closefile_(FileInfoPtr thisFile);
long openfile_(char* fileName, long errorLevel, FileInfoPtr *theFile);
long openfilerewrite_(char* fileName, long errorLevel, FileInfoPtr *theFile);
long openaccess_(long tapeNumber, char *filePrefix, int tapeType, 
		 long errorLevel, FileInfoPtr *theAccess);
long setmappingfile_(FileInfoPtr thisFile, char* theMapFile);
long getmappingfiles_(FileInfoPtr thisFile, char* theDefaultMap, 
		      char* theMapInUse);
long getfileinfo_(FileInfoPtr thisFile, long* fileType, long* numberOfSurfaces,
		 long* numberOfBoundSurfaces, long* numberOfTimeSeriesBlocks,
		 Boolean* preferedSettingsBlock);
long setexpid_(FileInfoPtr thisFile, char* theExp);
long getexpid_(FileInfoPtr thisFile, char* theExp);
long settext_(FileInfoPtr thisFile, char* theText);
long gettext_(FileInfoPtr thisFile, char* theText);
long setauditstring_(FileInfoPtr thisFile, char* theString);
long getauditstringlength_(FileInfoPtr thisFile, long* theLength);
long getauditstring_(FileInfoPtr thisFile, long maxLength, char* theString);
void report(char *message, char *function, long returnValue, 
	    char *optionalString);

#ifdef __cplusplus
}
#endif

#endif




