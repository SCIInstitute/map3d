#ifndef __GI_SURFACES_H__
#define __GI_SURFACES_H__

#ifdef __cplusplus
extern "C" {
#endif

long setsurfaceindex_(FileInfoPtr thisFile, long theSurface);
long getsurfaceindex_(FileInfoPtr thisFile, long *theSurface);
long setsurfacename_(FileInfoPtr thisFile, char* theName);
long getsurfacename_(FileInfoPtr thisFile, char* theName);
long setsurfacetype_(FileInfoPtr thisFile, long theType);
long getsurfacetype_(FileInfoPtr thisFile, long *theType);


#ifdef __cplusplus
}
#endif
#endif



