/* -*- c -*- */

#ifndef FIDUCIALINFO_H
#define FIDUCIALINFO_H

/*-------------------------------------------------------------------------
 *
 * fi.h
 *
 *-----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FI_USE_X11
#include <X11/Xlib.h>
#endif
#ifdef FI_USE_GL
#include <gl/gl.h>
#endif

/* Fiducial types. */
extern const short FI_PON;
extern const short FI_POFF;
extern const short FI_QON;
extern const short FI_RPEAK;
extern const short FI_SOFF;
extern const short FI_STOFF;
extern const short FI_TPEAK;
extern const short FI_TOFF;
extern const short FI_ACTPLUS;
extern const short FI_ACTMINUS;
extern const short FI_ACT;
extern const short FI_RECPLUS;
extern const short FI_RECMINUS;
extern const short FI_REC;
extern const short FI_REF;
extern const short FI_JPT;
extern const short FI_BL;
extern const short FI_SUBSERIES;
extern const short FI_UNKNOWN;
    
/* Fiducial names. */
extern const char FI_PON_NAME[];
extern const char FI_POFF_NAME[];
extern const char FI_QON_NAME[];
extern const char FI_RPEAK_NAME[];
extern const char FI_SOFF_NAME[];
extern const char FI_STOFF_NAME[];
extern const char FI_TPEAK_NAME[];
extern const char FI_TOFF_NAME[];
extern const char FI_ACTPLUS_NAME[];
extern const char FI_ACTMINUS_NAME[];
extern const char FI_ACT_NAME[];
extern const char FI_RECPLUS_NAME[];
extern const char FI_RECMINUS_NAME[];
extern const char FI_REC_NAME[];
extern const char FI_REF_NAME[];
extern const char FI_JPT_NAME[];
extern const char FI_SUBSERIES_NAME[];
extern const char FI_BL_NAME[];    

/*-------------------------------------------------------------------------
 *
 * C/C++ interface.
 *
 *-----------------------------------------------------------------------*/

/* Variables of type `FiducialInfo' are used in routines FI_GetInfoByName
   and FI_GetInfoByType. */     
typedef struct FiducialInfo {
    const char *name;           /* Fiducial name. */
    short type;                 /* Fiducial type. */
    const char *label;          /* Short descrip of the fiducial. */
    const char *xColorName;     /* The X11 color name for the fiducial. For
                                    use with X11. */
    short red, green, blue;     /* The RGB color values for the
                                   fiducial. For use with X11 or gl's rgb
                                   mode and maybe other unforeseen color
                                   models. */ 
#ifdef FI_USE_X11    
    XColor xColor;              /* X11 color value for use with X11. */
#endif
#ifdef FI_USE_GL
    Colorindex glColorIndex;    /* The gl color index for the
                                   fiducial. For use with gl's colormap
                                   mode. */
#endif
} FiducialInfo;

int FI_Init(int reportLevel);
#ifdef FI_USE_X11
int FI_InitX11Colors(Display* display);
#endif
#ifdef FI_USE_GL
int FI_InitGLColorMapMode(void);
int FI_InitGLRGBMode(void);
#endif
int FI_Done(void);
int FI_GetNumTypes(void);
int FI_GetInfoByName(FiducialInfo *fi);
int FI_GetInfoByType(FiducialInfo *fi);
short FI_NameToType(const char *name);
int FI_IsValidType(short type);
int FI_IsValidName(const char *name);
int FI_First(FiducialInfo *fi);
int FI_Next(FiducialInfo *fi);
int FI_FirstR(FiducialInfo *fi, int *iterIndex);
int FI_NextR(FiducialInfo *fi, int *iterIndex);

#ifdef __cplusplus
}
#endif

#endif
