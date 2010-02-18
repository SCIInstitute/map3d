#ifndef __GI_LEADFIUCIALS_H__
#define __GI_LEADFIUCIALS_H__

#ifdef __cplusplus
extern "C" {
#endif


long setleadfiducials_(FileInfoPtr thisFile, char* theLabel,
		      short fidDescSize, short* fidDesc, float* theFids, short* theTypes);
long getleadfiducialcount_(FileInfoPtr thisFile, long* howMany);
long getleadfiducialinfo_(FileInfoPtr thisFile, FidInfoPtr theInfo);
long getleadfiducials_(FileInfoPtr thisFile, short index, short* fidDesc,
		       float* theFids, short* theTypes);

#ifdef __cplusplus
}
#endif

#endif
