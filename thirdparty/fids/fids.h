#ifndef __FID_HEADER__
#define __FID_HEADER__
#include "fi.h"
#include "cutil.h"

#if DYNAMIC

#ifdef _WIN32
  #ifdef BUILD_fids
    #define FIDSSHARE __declspec(dllexport)
  #else
    #define FIDSSHARE __declspec(dllimport)
  #endif
#else
  #define FIDSSHARE
#endif

#else
#define FIDSSHARE
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************/
/*                        Fiducials Constants                    */
/*****************************************************************/
#define NUMFIDTYPES 15/*** This is a little ugly as it is linked
                           to Ted's fi library but I neede to know
			   a number like this for dimensioning
			   arrays inside data structures. ***/

typedef struct  /*** Fiducials for one lead ***/
{
    long leadnum; /*** Lead number for these fids (first one = 0)***/
    long numfids; /*** Total number of fid values for this lead***/
    short *fidtypes; /*** Type of fid for each one in the list ***/
    float *fidvals; /*** Actual fiducial values ***/
} Lead_Fids;

typedef struct  /*** One full set of fids (for one time series) ***/
{
    long tsnum;	/*** Time series number for this set of fids (first=0) ***/
    long pakfilenum; /*** Pak file number to which these fids belong ***/
    long numfidtypes; /*** Number of different types in this ts. ***/
    short *fidtypes; /*** List of fid types in this time series***/
    char **fidnames; /*** Names of all fid types in the structure. ***/
    char fidlabel[132]; /*** Label for this fid set (132 chars) ***/
    long numfidleads; /*** Number of leads worth of fids in this series. ***/
    Lead_Fids *leadfids; /*** Array of fids, "numfidleads" of them ***/
} Series_Fids;

/********************* Prototypes **********************/

FIDSSHARE long AddLeadfidtype( Lead_Fids *oneleadfids, const short fidtype );
FIDSSHARE void DisplayLeadFids ( Lead_Fids *oneleadfids );
FIDSSHARE long FindFidfileSeries( FILE *luin, long *timeseriesnum, 
				  long *pakfilenum );
FIDSSHARE long FindNumSeriesFidvals( Series_Fids *oneseriesfids );
FIDSSHARE long FindNumLeadFidvals( Lead_Fids *oneleadfids, short fidtype );
FIDSSHARE float *GetLeadFidvals( Lead_Fids *oneleadfids, short fidtype, 
		       long *numleadfidvals );
FIDSSHARE float GetARIVal( Lead_Fids *oneleadfids );
FIDSSHARE float GetQTIVal( Lead_Fids *oneleadfids );
FIDSSHARE long GetNumFidfileSeries( char *fidfilename );
FIDSSHARE Series_Fids *LoadDfileFids( char *datafilename, char *datafilepath,
			   long reportlevel);
FIDSSHARE Boolean QNewFidname( short *fidtypelist, long numfidtypes, 
		     const char *fidname );
FIDSSHARE Boolean QNewFidtype( short *fidtypelist, long numfidtypes, 
			       short fidtype );
FIDSSHARE long ReadFidfileSeries( long report_level, char *fidfilename,
			long seriesnum, long pakfilenum,
		  Series_Fids *onefidlist );
FIDSSHARE long ReadOneFidfileSeries( long report_level, FILE *luin, 
			   Series_Fids *onefidlist );
FIDSSHARE long ScanFidTypes( Series_Fids *oneseriesfids );
FIDSSHARE long SkipFidfileSeries( FILE *luin, long *seriesnum, long *paknum );
FIDSSHARE long WhichFidnum( short *fidtypelist, long numfidtypes, 
			    const short fidtype );

#ifdef __cplusplus
}
#endif

#endif
