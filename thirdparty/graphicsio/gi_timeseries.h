#ifndef __GI_TIMESERIES_H__
#define __GI_TIMESERIES_H__

#ifdef __cplusplus
extern "C" {
#endif

long settimeseriesdatapath_(FileInfoPtr thisFile, char *thePath);
long settimeseriesindex_(FileInfoPtr thisFile, long theIndex);
long gettimeseriesindex_(FileInfoPtr thisFile, long *theIndex);
long settimeseriesfile_(FileInfoPtr thisFile, char *theFile);
long gettimeseriesfile_(FileInfoPtr thisFile, char *theFile);
long settimeseriesgeomfile_(FileInfoPtr thisFile, char *theFile);
long gettimeseriesgeomfile_(FileInfoPtr thisFile, char *theFile);

long settimeseriesspecs_(FileInfoPtr thisFile, long numberOfChannels, 
			 long numberOfFrames);
long gettimeseriesspecs_(FileInfoPtr thisFile, long *numberOfChannels,
			 long *numberOfFrames);
long settimeserieslabel_(FileInfoPtr thisFile, char *theLabel);
long gettimeserieslabel_(FileInfoPtr thisFile, char *theLabel);
long settimeseriesformat_(FileInfoPtr thisFile, long theFormat);
long gettimeseriesformat_(FileInfoPtr thisFile, long *theFormat);
long settimeseriesunits_(FileInfoPtr thisFile, long theUnits);
long gettimeseriesunits_(FileInfoPtr thisFile, long *theUnits);
long settimeseriessurface_(FileInfoPtr thisFile, long theSurface);
long gettimeseriessurface_(FileInfoPtr thisFile, long *theSurface);
long settimeseriesassoc_(FileInfoPtr thisFile, long theAssociation);
long gettimeseriesassoc_(FileInfoPtr thisFile, long *theAssociation);
long settimeseriesdata_(FileInfoPtr thisFile, float *theData);
long setsometimeseriesdata_(FileInfoPtr thisFile, void *theData, 
			    long numberOfFrames, int storageFormat);
long gettimeseriesdata_(FileInfoPtr thisFile, float *theData);
long getselectedtimeseriesdata_(FileInfoPtr thisFile, long startingFrame,
				long *numberOfFrames, void *theData, int returnedFormat);
long getselectedtimeserieschannel_(FileInfoPtr thisFile, long selectedChannel, long startingFrame,
				long *numberOfFrames, void *theLead, int returnedFormat);
long setnumcorrectedleads_(FileInfoPtr thisFile, long numberOfCorrectedLeads);
long getnumcorrectedleads_(FileInfoPtr thisFile, long *numberOfCorrectedLeads);
long setcorrectedleads_(FileInfoPtr thisFile, long *theLeads);
long getcorrectedleads_(FileInfoPtr thisFile, long *theLeads);
long setpowercurve_(FileInfoPtr thisFile, float *powerCurveData);
long getpowercurve_(FileInfoPtr thisFile, float *powerCurveData);

long setqsttimes_(FileInfoPtr thisFile, long qtime, long stime, long ttimes);
long getqsttimes_(FileInfoPtr thisFile, long *qtime, long *stime,
		  long *ttimes);
long checkextendedfiducials_(FileInfoPtr thisFile, Boolean *available);
long setextendedfiducials_(FileInfoPtr thisFile, long ponset, long poffset,
	    	    	    long rpeak, long tpeak);
long getextendedfiducials_(FileInfoPtr thisFile, long *ponset, long *poffset,
	    	    	    long *rpeak, long *tpeak);

long readtimeseriesheader(FileInfoPtr thisFile, TimeSeriesHeaderPtr theHeader);
long writetimeseriesheader(FileInfoPtr thisFile, 
			   TimeSeriesHeaderPtr theHeader);

#ifdef __cplusplus
}
#endif

#endif
