#ifndef __GI_UTILITIES_H__
#define __GI_UTILITIES_H__

#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

void fixPath(FileInfoPtr thisFile, char* inputFileName, char* outputFileName);
long	readNextRecordSize(int fileNumber);
long readpakheader(FileInfoPtr thisFile, PakHeaderPtr aPakHeader);
long readrawheader(FileInfoPtr thisFile, RawHeaderPtr aRawHeader, 
		   Boolean *superHeader);
long readacqheader(FileInfoPtr thisFile, AcqHeaderPtr thisHeader);
int findFileType(FileInfoPtr thisFile, char* fileName);
long findFileSize(FileInfoPtr thisFile, long *numberOfFrames);
void convertLong(long* theData, long howMany);
void convertFloat(float* theData, long howMany, Boolean toUnix);
void convertShort(short* theData, long howMany);
off_t	myLseek(int fildes, off_t offset, int whence);
FilePtr	movePastHeader(FileInfoPtr theFile, long *numberOfLeads, long *numberOfFrames);
int* getmapinfo(char* filename, int* numberofchannels);


#ifdef __cplusplus
}
#endif

#endif
