/*-------------------------------------------------------------------------
 *
 * fi.c
 *
 *-----------------------------------------------------------------------*/

#include "fi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Color modes. */
enum { FI_NONE, FI_X11, FI_GLRGB, FI_GLCOLORMAP }; 

/* If this variable is 1 then we print error messages when errors occur
   during calls to the library routines.  If it is 0 then no error messages
   are printed. */
static int gReportLevel;

/* If this variable is TRUE then the library has been properly
   initialized. */ 
static int gInited = FALSE;

/* This variable is used during iterator operations to keep track of where
   we are in the fiducial table. */
static int gIterIndex;

/* This variable remembers what color mode we are using. */
static int gColorMode;

#ifdef FI_USE_GL
/* This variable tells us where in gl's colormap we started replacing color
   values. */
static int gGLColorMapBase;

/* This variable saves the gl color values we replaced. */
static struct RGBColor {
    short red, green, blue;
} *gGLSavedColorMap = NULL;
#endif /* #ifdef FI_USE_GL */

#ifdef FI_USE_X11
/* This variable saves the user's X11 Display pointer. */
static Display *gDisplay;
#endif

/* ***************************************************************** */
/* The # of entries in the fiducial table. BE SURE TO UPDATE THIS    */
/* VALUE IF NEW FIDUCIAL TYPES ARE ADDED. */
#define gFiTableSize (18)
/* ***************************************************************** */

/* Fiducial types definitions. */
const short FI_PON = 0;
const short FI_POFF = 1;
const short FI_QON = 2;
const short FI_RPEAK = 3;
const short FI_SOFF = 4;
const short FI_STOFF = 5;
const short FI_TPEAK = 6;
const short FI_TOFF = 7;
const short FI_ACTPLUS = 8;
const short FI_ACTMINUS = 9;
const short FI_ACT = 10;
const short FI_RECPLUS = 11;
const short FI_RECMINUS = 12;
const short FI_REC = 13;
const short FI_REF = 14;
const short FI_JPT = 15;
const short FI_BL = 16;
const short FI_SUBSERIES = 17;
const short FI_UNKNOWN = -1;
    
/* Fiducial name definitions. */
const char FI_PON_NAME[] = "pon";
const char FI_POFF_NAME[] = "poff";
const char FI_QON_NAME[] = "qon";
const char FI_RPEAK_NAME[] = "rpeak";
const char FI_SOFF_NAME[] = "soff";
const char FI_STOFF_NAME[] = "stoff";
const char FI_TPEAK_NAME[] = "tpeak";
const char FI_TOFF_NAME[] = "toff";
const char FI_ACTPLUS_NAME[] = "actplus";
const char FI_ACTMINUS_NAME[] = "actminus";
const char FI_ACT_NAME[] = "act";
const char FI_RECPLUS_NAME[] = "recplus";
const char FI_RECMINUS_NAME[] = "recminus";
const char FI_REC_NAME[] = "rec";
const char FI_REF_NAME[] = "ref";
const char FI_JPT_NAME[] = "jpt";
const char FI_SUBSERIES_NAME[] = "subseries";
const char FI_BL_NAME[] = "baseline";

/* The all important table of fiducial info. */
static FiducialInfo gFiTable[] = {
    { 0, 0, "P-onset", "slategray", 0, 0, 0 },
    { 0, 0, "P-offset", "slategray", 0, 0, 0 },
    { 0, 0, "Q-onset", "magenta", 0, 0, 0 },
    { 0, 0, "Peak of the R wave", "slategray", 0, 0, 0 },
    { 0, 0, "S-offset == end of QRS", "slategray", 0, 0, 0 },
    { 0, 0, "End of the ST segment", "slategray", 0, 0, 0 },
    { 0, 0, "Peak of the T wave", "slategray", 0, 0, 0 },
    { 0, 0, "End of the T wave", "deepskyblue2", 0, 0, 0 },
    { 0, 0, "Activation time - max + slope in QRS", "slategray", 0, 0, 0 }, 
    { 0, 0, "Activation time - max - slipe in QRS", "slategray", 0, 0, 0 },
    { 0, 0, "The best guess at activation time", "red", 255, 0, 0 },
    { 0, 0, "Recovery - max + slope in T wave", "slategray", 0, 0, 0 },
    { 0, 0, "Recovery - max - slope in T wave", "slategray", 0, 0, 0 },
    { 0, 0, "The best guess at recovery time", "green", 0, 255, 0 },
    { 0, 0, "Reference time, e.g. the time of pacing spike", "yellow", 255,
      255, 0 }, 
    { 0, 0, "J pt computed using lsd2", "slategray", 0, 0, 0 },
	{ 0, 0, "Baseline", "blue", 0, 0, 255 },
	{ 0, 0, "Frame indicating the start of a subseries", "turquoise", 0, 0, 255 },
};

/* Fiducial info table sorted by name. */
static FiducialInfo *gSortedFiTable[gFiTableSize];

/*-------------------------------------------------------------------------
 *
 * FiTableEntryCmp --
 *
 * Compare the names of the 2 given FiducialInfo structures.  Return 0 if
 * equal 1 if the first name is greater than the second and -1 if the first
 * is less than the first.
 *
 *-----------------------------------------------------------------------*/

static int
FiTableEntryCmp(
    const void *v1,
    const void *v2
    )
{
    const FiducialInfo *fi1 = *((const FiducialInfo **)v1);
    const FiducialInfo *fi2 = *((const FiducialInfo **)v2);
    return strcmp(fi1->name, fi2->name);
}

/*-------------------------------------------------------------------------
 *
 * InitFiTable --
 *
 * Complete the initialization of gFiTable.
 *
 *-----------------------------------------------------------------------*/

static void 
InitFiTable()
{
    int i = 0;
    gFiTable[i].name = FI_PON_NAME; gFiTable[i++].type = FI_PON;
    gFiTable[i].name = FI_POFF_NAME; gFiTable[i++].type = FI_POFF;
    gFiTable[i].name = FI_QON_NAME; gFiTable[i++].type = FI_QON;
    gFiTable[i].name = FI_RPEAK_NAME; gFiTable[i++].type = FI_RPEAK;
    gFiTable[i].name = FI_SOFF_NAME; gFiTable[i++].type = FI_SOFF;
    gFiTable[i].name = FI_STOFF_NAME; gFiTable[i++].type = FI_STOFF;
    gFiTable[i].name = FI_TPEAK_NAME; gFiTable[i++].type = FI_TPEAK;
    gFiTable[i].name = FI_TOFF_NAME; gFiTable[i++].type = FI_TOFF;
    gFiTable[i].name = FI_ACTPLUS_NAME; gFiTable[i++].type = FI_ACTPLUS;
    gFiTable[i].name = FI_ACTMINUS_NAME; gFiTable[i++].type = FI_ACTMINUS;
    gFiTable[i].name = FI_ACT_NAME; gFiTable[i++].type = FI_ACT;
    gFiTable[i].name = FI_RECPLUS_NAME; gFiTable[i++].type = FI_RECPLUS;
    gFiTable[i].name = FI_RECMINUS_NAME; gFiTable[i++].type = FI_RECMINUS;
    gFiTable[i].name = FI_REC_NAME; gFiTable[i++].type = FI_REC;
    gFiTable[i].name = FI_REF_NAME; gFiTable[i++].type = FI_REF;
    gFiTable[i].name = FI_JPT_NAME; gFiTable[i++].type = FI_JPT;
    gFiTable[i].name = FI_BL_NAME; gFiTable[i++].type = FI_BL;
	gFiTable[i].name = FI_SUBSERIES_NAME; gFiTable[i++].type = FI_SUBSERIES;
    assert(i==gFiTableSize);
    for (i=0; i<gFiTableSize; i++) {
        gSortedFiTable[i] = &gFiTable[i];
    }
    qsort(gSortedFiTable, gFiTableSize, sizeof(FiducialInfo *),
          FiTableEntryCmp);
}

#ifdef FI_USE_X11

/*-------------------------------------------------------------------------
 *
 * FreeX11ColorCells --
 *
 * Free our x11 color cells.
 *
 *-----------------------------------------------------------------------*/

static void 
FreeX11ColorCells(
    int beg,                    /* First color cell to free. */
    int end                     /* Last color cell to free. */
    )
{
    int i;
    Colormap colormap = DefaultColormap(gDisplay, DefaultScreen(gDisplay));
    for (i=beg; i<end; i++) {
        XFreeColors(gDisplay, colormap, &(gFiTable[i].xColor.pixel), 1, 0);
    }
}

/*-------------------------------------------------------------------------
 *
 * InitX11ColorCells --
 *
 * Allocate X11 color cells.
 *
 *-----------------------------------------------------------------------*/

static int 
InitX11ColorCells(
    Display *display
    )
{
    int i;
    Status status;
    Colormap colormap;
    XColor exactColor;
    gDisplay = display;
    colormap = DefaultColormap(display, DefaultScreen(display));
    for (i=0; i<gFiTableSize; i++) {
        status = XAllocNamedColor(display, colormap,
                                  gFiTable[i].xColorName,
                                  &(gFiTable[i].xColor), &exactColor);
        if (status == 0) {
            if (gReportLevel == 1) {
                fprintf(stderr, "? Failed to allocate x11 color %s\n",
                        gFiTable[i].xColorName);
            }
            break;
        }
    }
    if (status == 0) {
        FreeX11ColorCells(0, i);
        return FALSE;
    }
    return TRUE;
}

#endif /* #ifdef FI_USE_X11 */

#ifdef FI_USE_GL

/*-------------------------------------------------------------------------
 *
 * InitGLColorMap --
 *
 * Initialize the gl color map.  This routine modifies the gl color map.
 *
 *-----------------------------------------------------------------------*/

static int 
InitGLColorMap()
{
    int i, j;
    gGLColorMapBase = 256 - gFiTableSize;
    gGLSavedColorMap =
        (struct RGBColor *) malloc(gFiTableSize*sizeof(struct RGBColor));
    if (gGLSavedColorMap == NULL) {
        if (gReportLevel == 1) {
            fprintf(stderr, "? Failed to allocate gGLSavedColorMap\n");
        }
        return FALSE;
    }
    for (i=gGLColorMapBase, j=0; j<gFiTableSize; i++, j++) {
        getmcolor(i, &(gGLSavedColorMap[j].red), &(gGLSavedColorMap[j].green),
                  &(gGLSavedColorMap[j].blue));
        mapcolor(i, gFiTable[j].red, gFiTable[j].green, gFiTable[j].blue);
        gFiTable[j].glColorIndex = i;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 *
 * RestoreGLColorMap --
 *
 * Restore gl colormap to original state.
 *
 *-----------------------------------------------------------------------*/

static void 
RestoreGLColorMap(void)
{
    int i, j;
    for (i=gGLColorMapBase,j=0; j<gFiTableSize; i++, j++) {
        mapcolor(i, gGLSavedColorMap[j].red, gGLSavedColorMap[j].green,
                 gGLSavedColorMap[j].blue); 
    }
    free(gGLSavedColorMap);
    gGLSavedColorMap = NULL;
}

#endif /* #ifdef FI_USE_GL */

/*-------------------------------------------------------------------------
 *
 * CheckInited --
 *
 * Return TRUE if the lib has been properly initialized, FALSE otherwise.
 *
 *-----------------------------------------------------------------------*/

static int 
CheckInited()
{
    if ((gInited == FALSE) && (gReportLevel == 1)) {
        fprintf(stderr, "? Did you forget to call FI_Init?\n");
    }
    return gInited;
}

#if defined(FI_USE_X11) || defined(FI_USE_GL)

/*-------------------------------------------------------------------------
 *
 * ColorModeCheck --
 *
 * Make sure gColorMode is FI_NONE.
 *
 *-----------------------------------------------------------------------*/

static int 
ColorModeCheck()
{
    int result = gColorMode == FI_NONE;
    if ((result == FALSE) && (gReportLevel == 1)) {
        fprintf(stderr, "? You can be in only one color mode at a time");
    }
    return result;
}

#endif

/*-------------------------------------------------------------------------
 *
 * FI_Init --
 *
 * Initialize the FI library.
 *
 *-----------------------------------------------------------------------*/

int 
FI_Init(
    int rl                     /* Report level: 0 or 1. */
    )
{
    int result;
    gColorMode = FI_NONE;
    if (rl < 0 || rl > 1) {
        result = FALSE;
        fprintf(stderr, "? Report level must be 0 or 1\n");
        gInited = FALSE;
    } else {
        result = TRUE;
        gReportLevel = rl;
        InitFiTable();
        gInited = TRUE;
    }
    return result;
}

#ifdef FI_USE_X11

/*-------------------------------------------------------------------------
 *
 * FI_InitX11Colors --
 *
 * Initialize the FI system to use the x11 color model.
 *
 *-----------------------------------------------------------------------*/

int 
FI_InitX11Colors(
    Display *display            /* X11 display pointer. */
    )
{
    int result;
    if (ColorModeCheck() == FALSE) {
        result = FALSE;
    } else {
        if (display == NULL) {
            if (gReportLevel == 1) {
                fprintf(stderr, "? NULL display pointer\n");
            }
            result = FALSE;
        } else {
            result = InitX11ColorCells(display);
            gColorMode = result == TRUE ? FI_X11 : FI_NONE;
        }
    }
    return result;
}

#endif /* #define FI_X11 */

#ifdef FI_USE_GL

/*-------------------------------------------------------------------------
 *
 * FI_InitGLColorMapMode --
 *
 * Initialize the FI system to use gl colormap mode.
 *
 *-----------------------------------------------------------------------*/

int 
FI_InitGLColorMapMode(void)
{
    int result;
    if (ColorModeCheck() == FALSE) {
        result = FALSE;
    } else {
        result = InitGLColorMap();
        gColorMode = result == TRUE ? FI_GLCOLORMAP : FI_NONE;
    }
    return result;
}

/*-------------------------------------------------------------------------
 *
 * FI_InitGLRGBMode --
 *
 * Initialize the FI system to use gl rgb mode. 
 *
 *-----------------------------------------------------------------------*/

int 
FI_InitGLRGBMode(void)
{
    if (ColorModeCheck() == FALSE) {
        return FALSE;
    }
    gColorMode = FI_GLRGB;
    return TRUE;
}

#endif /* #ifdef FI_USE_GL */

/*-------------------------------------------------------------------------
 *
 * FI_Done --
 *
 * Finalize the FI library.  Call this when you are done.
 *
 *-----------------------------------------------------------------------*/

int 
FI_Done(void)
{
#ifdef FI_USE_X11
    if (gColorMode == FI_X11) {
        FreeX11ColorCells(0, gFiTableSize);
    }
#endif

#ifdef FI_USE_GL
    if (gColorMode == FI_GLCOLORMAP) {
        RestoreGLColorMap();
    }
#endif
    
    gColorMode = FI_NONE;
    return TRUE;
}

/*-------------------------------------------------------------------------
 *
 * FI_GetNumTypes --
 *
 * Return the # of fiducial types in the database.  It will return 0 if
 * FI_Init has not been called first.
 * 
 *-----------------------------------------------------------------------*/

int 
FI_GetNumTypes(void)
{
    
    return CheckInited() ? gFiTableSize : 0;
}

/*-------------------------------------------------------------------------
 *
 * FI_GetInfoByName --
 *
 * Use the `name' field of the FiducialInfo structure to retrieve all the
 * other poop. Return FALSE if `name' is not in the fiducial database,
 * TRUE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
FI_GetInfoByName(
    FiducialInfo *fiKey         /* On input fiKey->name should point to the
                                   name of the fiducial type to locate.  On
                                   output the rest of the fields will be
                                   filled in appropriately. */
    )
{
    int result;
    if (CheckInited() == TRUE) {
        const FiducialInfo **fi =
            (const FiducialInfo **)bsearch(&fiKey, gSortedFiTable,
                                           gFiTableSize,
                                           sizeof(FiducialInfo *),
                                           FiTableEntryCmp);
        if (fi) {
            *fiKey = **fi;
            result = TRUE;
        } else {
            if (gReportLevel == 1) {
                fprintf(stderr, "? Don't know the fiducial name: %s\n",
                        fiKey->name);
            }
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

/*-------------------------------------------------------------------------
 *
 * FI_GetInfoByType --
 *
 * Use the `type' field of the FiducialInfo structure to retrieve all the
 *  other poop. Return FALSE if `type' is not in the fiducial database, TRUE
 *  otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
FI_GetInfoByType(
    FiducialInfo *fi            /* On input fi->type should contain the
                                   the fiducial type to locate.  On
                                   output the rest of the fields will be
                                   filled in appropriately. */
    )
{
    if (CheckInited() == FALSE) {
        return FALSE;
    }
    if ((fi->type >= gFiTableSize) || (fi->type < 0)) {
        if (gReportLevel == 1) {
            fprintf(stderr, "? Don't know the fiducial type: %d\n", fi->type);
        }
        return FALSE;
    }
    *fi = gFiTable[fi->type];
    return TRUE;
}

/*-------------------------------------------------------------------------
 *
 * FI_NameToType --
 *
 * Returns a fiducial type number given the fiducial's name.  Behavior is
 * undefined if the fiducial name is invalid.
 *
 *-----------------------------------------------------------------------*/

short
FI_NameToType(
    const char *name
    )
{
    FiducialInfo fi;
    int result;
    short type;

    fi.name = name;
    result = FI_GetInfoByName(&fi);
    if (result == FALSE) {
        fprintf(stderr, "? FI_NameToType failed.  No such name: %s\n",
                name);
        type = FI_UNKNOWN;
    } else {
        type = fi.type;
    }
    return type;
}

/*-------------------------------------------------------------------------
 *
 * FI_IsValidType --
 *
 * Returns TRUE if the given type is a valid type, FALSE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
FI_IsValidType(
    short type
    )
{
    return type < FI_GetNumTypes();
}

/*-------------------------------------------------------------------------
 *
 * FI_IsValidName --
 *
 * Returns TRUE if the given name is a valid name, FALSE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
FI_IsValidName(
    const char *name
    )
{
    FiducialInfo fi;
    fi.name = name;
    return FI_GetInfoByName(&fi);
}

/*-------------------------------------------------------------------------
 *
 * FI_First --
 *
 * Return the first entry in the database.  Return TRUE if successful,
 * FALSE otherwise. Call FI_Next to get the remainder of the database
 * entries.  Database entries are returned in alphabetical order
 *
 *-----------------------------------------------------------------------*/

int 
FI_First(
    FiducialInfo *fi            /* When done, *fi will contain all the info
                                   for the first fiducial in the database. */
    )
{
    return FI_FirstR(fi, &gIterIndex);
}

/*-------------------------------------------------------------------------
 *
 * FI_Next --
 *
 * Return the `next' entry in the database.  Returns FALSE if no more
 *  entries exist or an error occurs.
 *
 *-----------------------------------------------------------------------*/

int 
FI_Next(
    FiducialInfo *fi            /* When done, *fi will contain all the info
                                   for the next fiducial in the database. */
    )
{
    return FI_NextR(fi, &gIterIndex);
}


/*-------------------------------------------------------------------------
 *
 * FI_FirstR --
 *
 * Reentrant version of FI_First.  Note second parameter in call.
 *
 *-----------------------------------------------------------------------*/

int 
FI_FirstR(
    FiducialInfo *fi,
    int *iterIndex              /* Pointer to an int that is used to
                                   keep track of where we are in the
                                   current iteration. */
    )
{
    if (CheckInited() == FALSE) {
        return FALSE;
    }
    *fi = *(gSortedFiTable[0]);
    *iterIndex = 1;
    return TRUE;
}

/*-------------------------------------------------------------------------
 *
 * int FI_NextR --
 *
 * Reentrant version of FI_Next.
 *
 *-----------------------------------------------------------------------*/

int FI_NextR(
    FiducialInfo *fi,
    int *iterIndex
    )
{
    if (CheckInited() == FALSE) {
        return FALSE;
    }
    if (*iterIndex < 1) {
        if (gReportLevel == 1) {
            fprintf(stderr, "? Did you forget to call FI_First/FI_FirstR?\n");
        }
        return FALSE;
    }
    if (*iterIndex == gFiTableSize) {
        *iterIndex = 0;
        return FALSE;
    }
    *fi = *(gSortedFiTable[(*iterIndex)++]);
    return TRUE;
}

#ifdef FI_SUPPORT_FORTRAN

/* ---------- FORTRAN interface ---------- */

#ifdef __cplusplus
extern "C" {
#endif

int fi_finit_(int *reportLevel);
#ifdef FI_USE_GL
int fi_finitglcolormapmode_(void);
int fi_finitglrgbmode_(void);
#endif
int fi_fdone_(void);
int fi_fgetnumtypes_(void);
int fi_fnametotype_(const char *name, short *type, int nameLen);
int fi_ftypetoname_(short *type, char *name, int nameLen);
int fi_fisvalidname_(const char *name, int nameLen);
int fi_fgetlabel_(short *type, char *label, int labelLen);
int fi_fgetxcolorname_(short *type, char *xColorName, int xColorNameLen);
int fi_fgetrgbvalues_(short *type, short *red, short *green, short *blue);
#ifdef FI_USE_GL
int fi_fgetglcolorindex_(short *type, Colorindex *index);
#endif
int fi_ffirst_(short *type);
int fi_fnext_(short *type);

#ifdef __cplusplus
}
#endif


/*-------------------------------------------------------------------------
 *
 * fi_finit_ --
 *
 * Initialize the FI system.
 *
 *-----------------------------------------------------------------------*/

int 
fi_finit_(
    int *rl
    )
{
    return FI_Init(*rl);
}

#ifdef FI_USE_GL

/*-------------------------------------------------------------------------
 *
 * fi_finitglcolormapmode_ --
 *
 * Initialize GL colormap mode.
 *
 *-----------------------------------------------------------------------*/

int 
fi_finitglcolormapmode_(void)
{
    return FI_InitGLColorMapMode();
}

/*-------------------------------------------------------------------------
 *
 * fi_finitglrgbmode_ --
 *
 * Initialize GL RGB mode.
 *
 *-----------------------------------------------------------------------*/

int 
fi_finitglrgbmode_(void)
{
    return FI_InitGLRGBMode();
}

#endif /* #ifdef FI_USE_GL */

/*-------------------------------------------------------------------------
 *
 * fi_fdone_ --
 *
 * Finalize the FI system.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fdone_(void)
{
    return FI_Done();
}

/*-------------------------------------------------------------------------
 *
 * fi_fgetnumtypes_ --
 *
 * Return the # of fiducials in the database.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fgetnumtypes_(void)
{
    return FI_GetNumTypes();
}

/*-------------------------------------------------------------------------
 *
 * fi_fnametotype_ --
 *
 * Given the fiducial's name, find its type.  Returns FALSE if the name is
 * not in the fiducial database, TRUE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fnametotype_(
    const char *name,           /* Name of fiducial. */
    short *type,                /* We return its type here. */
    int nameLen                 /* Length of the FORTRAN string `name'.
                                   This parameter is supplied implicitly by
                                   the FORTRAN compiler. */
    )
{
    int i, result;
    FiducialInfo fi;
    char *tname = (char *)malloc((nameLen+1)*sizeof(char));
    strncpy(tname, name, nameLen);
    for (i = nameLen - 1; (i >= 0) && (tname[i] == ' '); i--)
        ;
    tname[++i] = '\0';
    fi.name = tname;
    result = FI_GetInfoByName(&fi);
    free(tname);
    *type = fi.type;
    return result;
}

/*-------------------------------------------------------------------------
 *
 * fi_ftypetoname_ --
 *
 * Given the fiducial's type, find its name.  Returns FALSE if the type is
 *  not in the fiducial database, TRUE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
fi_ftypetoname_(
    short *type,                /* Fiducial type to locate. */
    char *name,                 /* Fiducial's name. */
    int nameLen                 /* Length of the FORTRAN string `name'.
                                   This parameter is supplied implicitly by
                                   the FORTRAN compiler. */
    )
{
    int i, result, len;
    FiducialInfo fi;
    fi.type = *type;
    result = FI_GetInfoByType(&fi);
    if (result == TRUE) {
        len = strlen(fi.name);
        if (len <= nameLen) {
            for (i=0; i<len; i++) {
                name[i] = fi.name[i];
            }
            for (; i<nameLen; i++) {
                name[i] = ' ';
            }
        } else {
            if (gReportLevel == 1) {
                fprintf(stderr, "? Name char array is too short\n");
            }
            result = FALSE;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------
 *
 * fi_fisvalidname_ --
 *
 * Returns TRUE if the given name is a valid name, FALSE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fisvalidname_(
    const char *name,           /* Determine if this name is valid. */
    int nameLen                 /* Length of the FORTRAN string `name'.
                                   This parameter is supplied implicitly by
                                   the FORTRAN compiler. */
    )
{
    short type;
    return fi_fnametotype_(name, &type, nameLen);
}

/*-------------------------------------------------------------------------
 *
 * fi_fgetlabel_ --
 *
 * Find a fiducial's label given its type.  Returns FALSE if the type is
 *  not in the fiducial database, TRUE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fgetlabel_(
    short *type,                /* Fiducial type to locate. */
    char *label,                /* Fiducial's label. */
    int labelLen                /* Length of the FORTRAN string `label'.
                                   This parameter is supplied implicitly by
                                   the FORTRAN compiler. */
    )
{
    int i, result, len;
    FiducialInfo fi;
    fi.type = *type;
    result = FI_GetInfoByType(&fi);
    if (result == TRUE) {
        len = strlen(fi.label);
        if (len <= labelLen) {
            for (i=0; i<len; i++) {
                label[i] = fi.label[i];
            }
            for (; i<labelLen; i++) {
                label[i] = ' ';
            }
        } else {
            if (gReportLevel == 1) {
                fprintf(stderr, "? Label char array is too short\n");
            }
            result = FALSE;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------
 *
 * fi_fgetxcolorname_ --
 *
 * Given the fiducial's type, find its x color name.  Returns FALSE if the
 *  type is not in the fiducial database, TRUE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fgetxcolorname_(
    short *type,                /* A fiducial type. */
    char *xColorName,           /* Corresponding x11 color name. */
    int xColorNameLen           /* Length of the FORTRAN string
                                   `xColorName'. This parameter is supplied
                                   implicitly by the FORTRAN compiler. */
    )
{
    int i, result, len;
    FiducialInfo fi;
    fi.type = *type;
    result = FI_GetInfoByType(&fi);
    if (result == TRUE) {
        len = strlen(fi.xColorName);
        if (len <= xColorNameLen) {
            for (i=0; i<len; i++) {
                xColorName[i] = fi.xColorName[i];
            }
            for (; i<xColorNameLen; i++) {
                xColorName[i] = ' ';
            }
        } else {
            if (gReportLevel == 1) {
                fprintf(stderr, "? X color name char array is too short\n");
            }
            result = FALSE;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------
 *
 * fi_fgetrgbvalues_ --
 *
 * Given the fiducial's type, find its RGB color values.  Returns FALSE if
 *  the type is not in the fiducial database, TRUE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fgetrgbvalues_(
    short *type,                /* A fiducial type. */
    short *red,                 /* RGB values here. */
    short *green,
    short *blue
    )
{
    int result;
    FiducialInfo fi;
    fi.type = *type;
    result = FI_GetInfoByType(&fi);
    if (result == TRUE) {
        *red = fi.red;
        *green = fi.green;
        *blue = fi.blue;
    }
    return result;
}

#ifdef FI_USE_GL

/*-------------------------------------------------------------------------
 *
 * fi_fgetglindex_ --
 *
 * Given the fiducial's type, find its gl color index.  Returns FALSE if
 *  the type is not in the fiducial database, TRUE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fgetglindex_(
    short *type,                /* A fiducial type. */
    Colorindex *index           /* GL color index returned here. */
    )
{
    int result;
    FiducialInfo fi;
    fi.type = *type;
    result = FI_GetInfoByType(&fi);
    if (result == TRUE) {
        *index = fi.glColorIndex;
    }
    return result;
}

#endif /* #ifdef FI_USE_GL */

/*-------------------------------------------------------------------------
 *
 * fi_ffirst_ --
 *
 * Find the first entry in the database and get its type.  Returns TRUE if
 *  successful, FALSE otherwise.
 *
 *-----------------------------------------------------------------------*/

int 
fi_ffirst_(
    short *type
    )
{
    int result;
    FiducialInfo fi;
    result = FI_First(&fi);
    *type = fi.type;
    return result;
}

/*-------------------------------------------------------------------------
 *
 * fi_fnext_ --
 *
 * Get the next entry in the database and determine its type.  Return TRUE
 *  if successful, FALSE if all entries have been exhausted.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fnext_(
    short *type
    )
{
    int result;
    FiducialInfo fi;
    result = FI_Next(&fi);
    *type = fi.type;
    return result;
}

/*-------------------------------------------------------------------------
 *
 * fi_ffirst_r_ --
 *
 * Rentrant version of fi_ffirst.
 *
 *-----------------------------------------------------------------------*/

int 
fi_ffirst_r_(
    short *type,
    int *iterIndex
    )
{
    int result;
    FiducialInfo fi;
    result = FI_FirstR(&fi, iterIndex);
    *type = fi.type;
    return result;
}

/*-------------------------------------------------------------------------
 *
 * fi_fnext_r_ --
 *
 * Rentrant version of fi_ffirst.
 *
 *-----------------------------------------------------------------------*/

int 
fi_fnext_r_(
    short *type,
    int *iterIndex
    )
{
    int result;
    FiducialInfo fi;
    result = FI_NextR(&fi, iterIndex);
    *type = fi.type;
    return result;
}

#endif /* #ifdef FI_SUPPORT_FORTRAN */
