/*
 *
 * C++ HEADER FILE : giofidset.h
 * VERSION : 2.0
 * DESCRIPTION : This data structure handles one set of fiducials values, e.g.
 * one set of activations times or recovery times.
 * It has methods to read fidsets from tsdfc files and eventually to write
 * them too.
 * This is the only place that really understands the structure of the fid
 * sets within the gdbm (.tsdfc) file.  Its original structure has been maintained
 * in the interest of keeping all of the low level reading of a tsdfc file in one
 * class.
 *
 * AUTHOR(S)  : Rob Macleod, modified by Jenny Simpson (simpson@cs.utah.edu) 
 * CREATED : 06/07/2002
 * MODIFIED: 7/15/2002
 * DOCUMENTATION: 
 * http://dante.sci.utah.edu/develop/simpson/scratch/documentation/tsdflib_doc.html
 *
*/

#ifndef GIOFIDSET_H
#define GIOFIDSET_H

#include <string>

using std::string;

// ***************************************************************************
// ***************************************************************************
// *************************** Class GIOFidSet *******************************
// ***************************************************************************
// ***************************************************************************

class GIOFidSet {
public:
  

  // Empty constructor
  GIOFidSet(void) {
    type = 999;                 // fidset's type - should be 1.
    version = 0;              // fidset's version - should be 0.
    size = 0;                   // number of bytes of fidset data,
                                //  not including the type and version
                                //  fields.
    name = "";                 // fidset name.
    auditString = "";            // fidset audit string.
    fidDescArraySize = 0; // size of the fid descrip array.
    // should be the same as the number of channels.
    fidDescArray = NULL;  // the fid descrip array.
    // number of fids for each channel.
    fidArraySize = 0;             // size of the fid arrays.
    // will be the number of *all* fiducial values for
    //  *all* channels, ordered by channel number.
    fidValueArray = NULL; // fid value array.
    fidTypeArray = NULL;  // fid type array.

    // Now the same data organized a little more organically.
    tsdfname = "";             // name of associated tsdf file.
    numfidtypes = 0;              // number of fid types in the set
    //fidtypes = NULL;            // list of all fid types in the set.
    //fidnames = NULL;             // list of fid types by string in the set.
    numfidleads = 0;            // number of leads that have fids.
  }


  // Copy constructor
  GIOFidSet( const GIOFidSet &fs );

  // Overloaded assignment operator
  const GIOFidSet & operator = (const GIOFidSet& fs);

  // Destructor
  ~GIOFidSet() {
    //printf("***********************************In GIOFidSet destructor\n");
   
    if(fidDescArray != NULL){
      delete [] fidDescArray;
    }
    if(fidValueArray != NULL){
      delete [] fidValueArray;
    }
    if(fidTypeArray != NULL){
      delete [] fidTypeArray;
    }
    //if(fidtypes != NULL){
    //  delete [] fidtypes;
    //}
    //if(fidnames != NULL){
    //  delete [] fidnames;
    //}  
    //printf("End of GIOFidSet destructor\n");
  }
   
  
  string Name(void);	       /* Return fidset's name. */ 
  string AuditString(void);     /* Return fidset's audit string. */
  string TsdfName(void);        /* Return fidset's time series file name */
  int DescArraySize(void);	       /* Return size of fidset's */
  const short *DescArray(void);      /* Return fidset's description array. */
  int ArraySize(void);	       /* Return size of fidset's value and 
					type array's. */
  const float *ValueArray(void);     /* Return fidset's value array. */
  const short *TypeArray(void);      /* Return fidset's type array. */ 
  short Type(void);        /* Return the data type. */
  short Version(void);     /* Return the data version. */
  int Size(void);          /* Return the data size. */
  int NumFidLeads();      /* Return the number of leads with fiducial values */
  string * FidNames();  // Return an array of all string names of all fiducials, repeats included 

  // Functions that actually read, write, and parse binary tsdfc file

  int WriteGIOFidSet( string tsdfcFileName, 
		      string timeSeriesDataFileName, const int fsnum );
  int ReadGIOFidSet( string tsdfcFileName,   /* Read a single FS */
	      string timeSeriesDataFileName, int fsnum );

  /*** Return number of fid sets for a paticular file/key combo***/
  int NumSets( string tsdfcFileName, 
		 string timeSeriesDataFileName );
  char * GetRecordInfo( char *cdata_p );
  char *ParseGIOFidSet( char *cdata_p ); /*** Parse a piece of fid data ***/


// Member variables
private:
  short type;                 // fidset's type - should be 1.
  short version;              // fidset's version - should be 0.
  int size;                   // number of bytes of fidset data, 
                                //  not including the type and version
                                //  fields.
  string name;                 // fidset name. 
  string auditString; 		// fidset audit string. 
  int fidDescArraySize;	// size of the fid descrip array. 
                             // should be the same as the number of channels.
  short *fidDescArray;	// the fid descrip array. 
                             // number of fids for each channel.
  int fidArraySize;		// size of the fid arrays. 
                           // will be the number of *all* fiducial values for
                           //  *all* channels, ordered by channel number.
  float *fidValueArray;	// fid value array. 
  short *fidTypeArray;	// fid type array. 
                   // Now the same data organized a little more organically.
  string tsdfname;             // name of associated tsdf file.
  int numfidtypes;		// number of fid types in the set
  //short *fidtypes;            // list of all fid types in the set.
  //string *fidnames;             // list of fid types by string in the set.
  int numfidleads;            // number of leads that have fids.


  /*** See if we have a new fid type. ***/
  bool NewFidType( const short thetype, short *thefidtypes );

};

#endif












