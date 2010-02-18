
/*
 *
 * C++ METHODS (CC) FILE : fidset.cc
 * VERSION : 2.0
 * DESCRIPTION : This data structure handles one set of fiducials values, e.g., one 
 * set of activations times or recovery times.
 * It has methods to read fidsets from tsdfc files and eventually to write
 * them too.
 * This is the only place that really understands the structure of the fid
 * sets within the gdbm (.tsdfc) file.  Its original structure has been maintained 
 * in the interest of keeping all of the low level reading of a tsdfc file in one 
 * class.

 * 
 * AUTHOR(S)  : Rob Macleod and Jenny Simpson (simpson@cs.utah.edu) 
 * CREATED : 06/07/02
 * MODIFIED: 07/15/02
 * DOCUMENTATION: 
 * http://dante.sci.utah.edu/develop/simpson/scratch/documentation/tsdflib_doc.html
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gdbm.h"
#include "fi.h"
#include "cutil.h"
#include "fidset.h"
#include "giofidset.h"
#include "myfile.h"

using std::string;

// ***************************************************************************
// ***************************************************************************
// *************************** Class GIOFidSet *******************************
// ***************************************************************************
// ***************************************************************************
   

/*====================================================================*/

// * Post: Copy Contructor
//
// * Arguments: 
// const GIOFidSet &fs - GIOFidSet to copy
//
GIOFidSet::GIOFidSet( const GIOFidSet &fs )
{
  //printf("In GIOFidSet copy constructor\n");
  type = fs.type;                 // fidset's type - should be 1.
  version = fs.version;              // fidset's version - should be 0.
  size = fs.size;                   // number of bytes of fidset data,
                                //  not including the type and version
                                //  fields.
  name = fs.name;    // fidset name
  auditString = fs.auditString;            // fidset audit string.
  fidDescArraySize = fs.fidDescArraySize; // size of the fid descrip array.
  // should be the same as the number of channels.
  fidDescArray = new short[fidDescArraySize];  // the fid descrip array.
  // number of fids for each channel.
  for(int i = 0; i < fidDescArraySize; i++){
    fidDescArray[i] = fs.fidDescArray[i];
  }
  fidArraySize = fs.fidArraySize;             // size of the fid arrays.
  // will be the number of *all* fiducial values for
  //  *all* channels, ordered by channel number.
  fidValueArray = new float[fidArraySize]; // fid value array.
  fidTypeArray = new short[fidArraySize];  // fid type array.
  for(int i = 0; i < fidArraySize; i++){
    fidValueArray[i] = fs.fidValueArray[i];
    fidTypeArray[i] = fs.fidTypeArray[i];
  }

  // Now the same data organized a little more organically.
  tsdfname = fs.tsdfname;             // name of associated tsdf file.
  numfidtypes = fs.numfidtypes;              // number of fid types in the set
  //fidtypes = new short[numfidtypes];            // list of all fid types in the set.
  //for(int i = 0; i < numfidtypes; i++){
  //  fidtypes[i] = fs.fidtypes[i];
  //}
  //fidnames = new string [numfidtypes];
  //for(int i = 0; i < numfidtypes; i++){
  //  fidnames[i] = fs.fidnames[i];
  // }
  
  numfidleads = fs.numfidleads;            // number of leads that have fids.

  //printf("End of GIOFidSet copy constructor\n");    
}

/*====================================================================*/

// * Post: Overloaded assignment operator
//
// * Arguments: 
// const GIOFidSet& fs - GIOFidSet to copy
//
const GIOFidSet& GIOFidSet::operator = (const GIOFidSet& fs)
{ 
  //printf("In GIOFidSet assignment operator\n");
  if(this != &fs){
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
    // }
    //if(fidnames != NULL){
    //  delete [] fidnames;
    //}    

    /*** Constructor for copy and initialization ***/
    type = fs.type;                 // fidset's type - should be 1.
    version = fs.version;              // fidset's version - should be 0.
    size = fs.size;                   // number of bytes of fidset data,
                                //  not including the type and version
                                //  fields.
    name = fs.name;    // fidset name
    auditString = fs.auditString;            // fidset audit string.
    fidDescArraySize = fs.fidDescArraySize; // size of the fid descrip array.
    // should be the same as the number of channels.
    fidDescArray = new short[fidDescArraySize];  // the fid descrip array.
    // number of fids for each channel.
    for(int i = 0; i < fidDescArraySize; i++){
      fidDescArray[i] = fs.fidDescArray[i];
    }
    fidArraySize = fs.fidArraySize;             // size of the fid arrays.
    // will be the number of *all* fiducial values for
    //  *all* channels, ordered by channel number.
    fidValueArray = new float[fidArraySize]; // fid value array.
    fidTypeArray = new short[fidArraySize];  // fid type array.
    for(int i = 0; i < fidArraySize; i++){
      fidValueArray[i] = fs.fidValueArray[i];
      fidTypeArray[i] = fs.fidTypeArray[i];
    }

    // Now the same data organized a little more organically.
    tsdfname = fs.tsdfname;             // name of associated tsdf file.
    numfidtypes = fs.numfidtypes;              // number of fid types in the set
    //fidtypes = new short[numfidtypes];            // list of all fid types in the set.
    //for(int i = 0; i < numfidtypes; i++){
    //  fidtypes[i] = fs.fidtypes[i];
    // }
    //fidnames = new string [numfidtypes];
    //for(int i = 0; i < numfidtypes; i++){
    //  fidnames[i] = fs.fidnames[i];
    // }

    numfidleads = fs.numfidleads;            // number of leads that have fids.


  }
  //printf("End of GIOFidSet assignment operator\n");
  return *this;
}
/*====================================================================*/

// * Post: Return fidset's type (not the same as fidtype).
//
// * Arguments: none 
//
short GIOFidSet::Type(void)
{
  return type;
}

/*====================================================================*/

// * Post: Return fidset's version number. 
//
// * Arguments: none 
//
short GIOFidSet::Version(void)
{
  return version;
}

/*====================================================================*/

// * Post: Return fidset's size of the data block (in bytes) for this fidset. 
//
// * Arguments: none 
//
int GIOFidSet::Size(void)
{
    return size;
}

/*====================================================================*/

// * Post: Return fidset's name.
//
// * Arguments: none  
//
string GIOFidSet::Name()
{
    return name;
}

/*====================================================================*/


// * Post: Return number of leads with fiducial values
//
// * Arguments: none
//
int GIOFidSet::NumFidLeads() 
{
    return fidDescArraySize;
}

/*====================================================================*/

// * Post: Return fidset's audit string. 
//
// * Arguments: none
//
string GIOFidSet::AuditString(void)
{
    return auditString;
}

/*====================================================================*/

// * Post: Return size of fidset's description array. 
//
// * Arguments: none
//
int GIOFidSet::DescArraySize(void)
{
    return fidDescArraySize;
}

/*====================================================================*/

// * Post: Return fidset's description array.  
//
// * Arguments: none
//
const short * GIOFidSet::DescArray(void)
{
    return fidDescArray;
}

/*====================================================================*/

// * Post: Return size of fidset's value and type array's. 
//
// * Arguments: none 
//
int GIOFidSet::ArraySize(void)
{
    return fidArraySize;
}

/*====================================================================*/

// * Post: Return fidset's value array.
//
// * Arguments: none 
//
const float * GIOFidSet::ValueArray(void)
{
    return fidValueArray;
}

/*====================================================================*/

// * Post: Return fidset's type array.
//
// * Arguments: none 
//
const short * GIOFidSet::TypeArray(void)
{
  return fidTypeArray;
}

/*====================================================================*/

// * Post: Return time series data file name.
//
// * Arguments: none
//
string  GIOFidSet::TsdfName(void){
  return tsdfname;

}

/*====================================================================*/

// * Post: Returns an array of the string names of all the fiducials
// in this set in the order they appear.  Repeats are included.
//
// * Arguments: none
//
//string * GIOFidSet::FidNames(){
//  return fidnames;
//}

/*====================================================================*/

// * Post: Read a single fid set from the specified tsdfc file and put 
// it into the current FS object.
// 
// Returns 0 if successful.
// Returns -1 if the tsdfc file couldn't be open opened.
// Returns -2 if data for given key couldn't be fetched.
//
// * Arguments:
// string tsdfcFileName -           Name of tsdfc file 
// string timeSeriesDataFileName -  Key for the gdbm file
//					 = Name of tsdf to find
// const int fsnum -                     Number of the fidset to read
//					within the record that matches
//					the key in timeSeriesDataFileName.
//					First fidset = 0 (not 1)
//
int GIOFidSet::ReadGIOFidSet( string tsdfcFileName, 
		    string timeSeriesDataFileName, const int fsnum )
{
 
  // get key and associated data from gdbm file 
  // a container file is just a gdbm file
  int result = 0;
  int filefidnum = 0;
  char *cdata_p, *enddata_p;
  datum key, data;


  //printf("In GIOFidSet::ReadGIOFidSet\n");
  // Open the tsdfc file - it's really just a gdbm file. 
  GDBM_FILE tsdfcFile = gdbm_open(const_cast<char*>(tsdfcFileName.c_str()), 
				    1024, GDBM_READER, 00644, 0);
  if (! tsdfcFile) {
    ReportError("GIOFidSet::Read", "could not open the file",
		    ERR_FILE, "");
    printf("ERROR: [GIOFidSet::Read] Could not open the file %s\n", const_cast<char*>(tsdfcFileName.c_str()));
        
    return( ERR_FILE );
  }
  //printf("Read: Opened file %s\n", tsdfcFileName);

  // Extract data for given key.
  key.dptr = const_cast<char*>(timeSeriesDataFileName.c_str());
  key.dsize = strlen(timeSeriesDataFileName.c_str());

  // this is the whole chuck of data for all the fiducial information
  // associated with this time series file
  data = gdbm_fetch(tsdfcFile, key);
  gdbm_close(tsdfcFile);
  if (! data.dptr) {
    printf("ERROR: [GIOFidSet::Read] - could not find the key %s\n", const_cast<char*>(timeSeriesDataFileName.c_str()));
    ReportError("GIOFidSet::Read", "could not find the key", ERR_FILE, "");
    return( ERR_FILE );
  }

  // Extract the fsnum^th fidset: the following loop quits 
  // if we skip over the end of the data block for this key.
  
  cdata_p = data.dptr;
  enddata_p = data.dptr + data.dsize - 3;
  //printf(" Fetched data block of size %xX with pointer from %xX to %xX\n", data.dsize, data.dptr, enddata_p );
  cdata_p = GetRecordInfo( cdata_p );
  //printf(" Read first record \n");

  for (filefidnum=0; filefidnum < fsnum; filefidnum++) {
    cdata_p += size;
    if ( cdata_p > enddata_p ) {
	    result = ERR_MEM;
	    ReportError( "GIOFidSet::Read", " skipped past end of data",
			 result, "");
	    return( result );
	}
	cdata_p = GetRecordInfo( cdata_p );
	//printf(" Read record #%d\n", filefidnum+1);
    }

 /*** This routine extracts the  actual fiducial values and loads up 
      the data structure.
 ***/
    cdata_p = ParseGIOFidSet( cdata_p );


    tsdfname = timeSeriesDataFileName;
    //printf("Returning from GIOFidSet::ReadGIOFidSet\n");


    return result;
}

/*======================================================================*/

// * Post: Parse the contents of a fiducial set record.  Returns an 
// updated value for *cdata_p as we walk through the file
//
// * Arguments: 
// char * cdata_p - pointer to the current place in the data record
//
char * GIOFidSet::ParseGIOFidSet( char *cdata_p )
{

  //printf("*********** Reading Fidset *************\n\n");

    // set type (short)
    
//     printf("| type = %d (%d bytes) |", type, sizeof(short));
//     printf("| version = %d (%d bytes) |",version, sizeof(short));
//     printf("| size = %d (%d bytes) |\n", size, sizeof(int));


    int fidnamesize, auditsize, fidcountsize, fidarraysize;

 /*** Read the fid set name ***/
    cdata_p = (char *)mfmemread((void *)&fidnamesize,sizeof(int),1,cdata_p,mfINT);
    
    //printf("| fidNameSize = %d (%d bytes) |", fidnamesize, sizeof(int));

    char * namePtr;
    namePtr = new char[fidnamesize+1];
    cdata_p = (char *)mfmemread((void *)namePtr,sizeof(char),fidnamesize,(void *)cdata_p,mfCHAR);  /* since calloc is used the last byte is automaticly 0 */

    namePtr[fidnamesize] = '\0';

    name = namePtr;
    
    delete [] namePtr;

    //printf("| name = %s (%d bytes) |", name.c_str(), fidnamesize * sizeof(char));
    //printf("\nFound fidset called %s\n of length %d\n", name.c_str(), fidnamesize );

    
 /*** Read the audit string ***/
    cdata_p = (char *)mfmemread((void *)&auditsize,sizeof(int),1,(void *)cdata_p,mfINT); 
    //printf("| auditSize = %d (%d bytes) |", auditsize, sizeof(int));

    char * auditStringPtr = new char[auditsize+1];
    cdata_p = (char *)mfmemread((void *)auditStringPtr,sizeof(char),auditsize,(void *)cdata_p,mfCHAR);  /* since calloc is used the last byte is automaticly 0 */

    auditStringPtr[auditsize] = '\0';
    auditString = auditStringPtr;
    delete [] auditStringPtr;

    //printf("| auditString = %s (%d bytes) |", auditString.c_str(), auditsize * sizeof(char));
    //printf(" Found audit string called %s\n of length %d\n", auditString.c_str(), auditsize );

 /*** Read the array that describes the number of fid values for each lead ***/
    cdata_p = (char *)mfmemread((void *)&fidcountsize,sizeof(int),1,(void *)cdata_p,mfINT);
    fidDescArraySize = fidcountsize;

    //printf("| descArraySize = %d (%d bytes) |", fidDescArraySize, sizeof(int));

    fidDescArray = new short[fidcountsize];
    cdata_p = (char *)mfmemread(fidDescArray,sizeof(short),fidcountsize,(void *)cdata_p,mfSHORT); 

//     printf("| fidDescArray ");
//     for(int k = 0; k < fidcountsize; k++){
//       printf(" %d", fidDescArray[k]);
//     }
//     printf(" (%d bytes) |", fidcountsize * sizeof(short));

//     printf(" Fid count is of size %d and here are some elements:\n", fidcountsize);

//     // Display the first and last five values.
//     for (int i=0; i<5; i++ ) {
// 	printf("Lead %3d has %d fid values\n", i+1, fidDescArray[i]);
//     }
//     for (int i=fidcountsize-5; i<fidcountsize; i++ ) {
// 	printf("Lead %3d has %d fid values\n", i+1, fidDescArray[i]);
//     }

 /*** Now the array of fid values. ***/
    cdata_p = (char *)mfmemread(&fidarraysize,sizeof(int),1,(void *)cdata_p,mfINT);
    fidArraySize = fidarraysize;

//     printf("| fidArraySize = %d (%d bytes) |", fidArraySize, sizeof(int));

    fidValueArray = new float[fidarraysize];
    cdata_p = (char *)mfmemread(fidValueArray,sizeof(float),fidarraysize,(void *)cdata_p,mfFLOAT); 

//     printf("| fidValueArray ");
//     for(int k = 0; k < fidarraysize; k++){
//       printf(" %f", fidValueArray[k]);
//     }
//     printf(" (%d bytes) |", fidarraysize * sizeof(float));    

 /*** Now the fid type array ***/
    fidTypeArray = new short[fidarraysize];
    cdata_p = (char *)mfmemread((void *)fidTypeArray,sizeof(short),fidarraysize,(void *)cdata_p,mfSHORT);	        

//     printf("| fidTypeArray ");
//     for(int k = 0; k < fidarraysize; k++){
//       printf(" %d", fidTypeArray[k]);
//     }
//     printf(" (%d bytes) |", fidarraysize * sizeof(short));
//     printf("\n\n*********** Done Reading Fidset *************\n\n");

//     printf(" Fid type array is of size %d and these elements:\n", fidarraysize);
//     // Display the first and last five values.
//     for (int i=0; i<5; i++ ) {
// 	printf("Lead %d is of fid type %d and value %7.1f\n", i+1, fidTypeArray[i], fidValueArray[i]);
//     }
//     for (int i=fidarraysize-5; i<fidarraysize; i++ ) {
// 	printf("Lead %d is of fid type %d and value %7.1f\n",  i+1, fidTypeArray[i], fidValueArray[i]);
//     }

//     printf("Returning from GIOFidSet::ParseGIOFidSet\n");
    return( cdata_p );
}

/*======================================================================*/

// * Post: Get the basic info from a gdbm record 
//   This consists of three values, the type, the version, and the size
//   of this record. Returns updated value of *cdata_p.
//
// * Arguments: 
// char *cdata_p -  a pointer to the memory containing the file
//		    We increment this pointer and pass it back for use
//		    in the calling routine.
//
char *GIOFidSet::GetRecordInfo( char *cdata_p )
{

    //printf("Entering GetRecordInfo cdata_p = %xX\n", cdata_p);
    cdata_p = (char *)mfmemread((void *)&type,sizeof(short),1,(void *)cdata_p,mfSHORT); 
    cdata_p = (char *)mfmemread((void *)&version,sizeof(short),1,(void *)cdata_p,mfSHORT); 
    cdata_p = (char *)mfmemread((void *)&size,sizeof(int),1,(void *)cdata_p,mfINT);  
    //printf("Leaving GetRecordInfo cdata_p = %xX"" and fidset type = %d and size = %xX\n", cdata_p, type, size);
    return( cdata_p );
}

/*====================================================================*/

// * Post: Find out how many fids sets there are in this file/key combo.
// Returns numfidsets  if successful.
// Returns ERR_FILE    in the case of an error
//
// * Arguments: 
// string tsdfcFileName -           Name of tsdfc file 
// string timeSeriesDataFileName -  Key for the gdbm file
//					 = Name of tsdf to find
//
int GIOFidSet::NumSets( string tsdfcFileName, 
		       string timeSeriesDataFileName )
{

    int numsets = 0;
    char *cdata_p, *enddata_p;
    datum key, data;


 /***   Open the tsdfc file - it's really just a gdbm file. ***/

    GDBM_FILE tsdfcFile = gdbm_open(const_cast<char*>(tsdfcFileName.c_str()), 
				    1024, GDBM_READER, 00644, 0);
    if (! tsdfcFile) {
	ReportError("FidSet::NumSets", "could not open the file",
		    ERR_FILE, "");
        printf("FidSet::NumSets could not open the file %s\n", const_cast<char*>(tsdfcFileName.c_str()));
	//return( ERR_FILE );
        return -1;
    }

    //printf("NumSets: Opened file %s\n", tsdfcFileName);

 /*** Now find the right key. ***/

    key.dptr = const_cast<char*>(timeSeriesDataFileName.c_str());
    key.dsize = strlen(const_cast<char*>(timeSeriesDataFileName.c_str()));
    data = gdbm_fetch(tsdfcFile, key);
    gdbm_close(tsdfcFile);
    if (! data.dptr) {
	ReportError("GIOFidSet::NumSets", "could not find the key",
		    ERR_FILE, "");
        printf("GIOFidSet::NumSets could not find the key %s\n", const_cast<char*>(timeSeriesDataFileName.c_str()));
	//return( ERR_FILE );
        return -2;
    }

 /*** Now run through the file and see how many fidsets there are. ***/

    cdata_p = data.dptr;
    enddata_p = data.dptr + data.dsize - 3;
    //printf(" Fetched data block of size %xX with pointer from %xX to %xX\n", data.dsize, data.dptr, enddata_p );
    numsets = 0;
    while ( cdata_p < enddata_p ) {
      //printf("Getting record info\n");
	cdata_p = GetRecordInfo( cdata_p );
        //printf("type = %d version = %d\n", type, version);
	if ( type == 1 && version == 0 ) {

	    numsets++;
	    // printf(" Read record #%d\n", numsets);
	}
	cdata_p += size;
	//printf(" Just increased data pointer to %x\n", cdata_p);
    }

    //printf("Numsets returning\n");
   
    return numsets;
}


/*====================================================================*/

// * Post: See if the type is a new one, or just a repeat of something we 
// already have in the fidtypes array.
//
// * Arguments: 
// short thetype - value of type to check
// short *thefidtypes - pointer to fid types array
// 
bool  GIOFidSet::NewFidType( short thetype, short *thefidtypes )
{

    int fidnum=0;

    for (fidnum=0; fidnum < numfidtypes; fidnum++ ) {
	if ( thetype == thefidtypes[fidnum] ) {
	    return FALSE;
	}
    }
    return TRUE;
}

/*======================================================================*/












