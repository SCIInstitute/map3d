/*
 *
 * C++ HEADER FILE : fidset.h
 * VERSION : 2.0
 * DESCRIPTION : This data structure handles one set of fiducials values, e.g., one
 * set of activations times or recovery times.  It is essentially designed to
 * be an interface to Rob's class with only the member variables pertinent
 * to a fidset.  A fiducial set is an example of one type of parameter set that
 * can be associated with a tsdf in a tsdfc file.  Currently, it is the most
 * common type of parameter set and the only one specifically implemented in
 * this library.
 *
 * The individual fiducials in the set are grouped by the lead that they
 * correspond to.
 * 
 * A FidSet contains an array of hashes of all fiducials as well as a
 * two-dimensional array of all fiducials indexed by lead number.  These
 * two data structures contain the same information but allow it to be
 * accessed differently.
 *
 * AUTHOR(S)  : Jenny Simpson (simpson@cs.utah.edu) 
 * CREATED : 06/07/02
 * MODIFIED: 07/15/02
 * DOCUMENTATION: 
 * http://dante.sci.utah.edu/develop/simpson/scratch/documentation/tsdflib_doc.html
*/


#ifndef FIDSET_H
#define FIDSET_H

#include "paramset.h"
#include "giofidset.h"
#include <string> 
#include <map>

using std::string;
using std::map;
using std::less;

// Fid struct.  Contains the information for an individual fiducial
struct Fid {
  short type;  // integer type of fiducial
  float value; // value of fiducial 
  string name;  // string name of fiducial
  //int arrayIndex; // index of fid in leadsArray
};


// ***************************************************************************
// ***************************************************************************
// *************************** Class FidSet **********************************
// ***************************************************************************
// ***************************************************************************

typedef Fid * FidPtr;
typedef map <string, FidPtr **, less<string> > FidPtrMap;
typedef map <string, int, less<string> > FidOccurrenceMap;

class FidSet: public ParamSet {
public:
  // Empty constructor
  FidSet();
 
  // Destructor
  ~FidSet();
 
  // Copy constructor
  FidSet(const FidSet& f);

  // Overloaded assignment operator
  const FidSet& operator = (const FidSet& f);
  
  // Fills FidSet with info from GIOFidSet
  // This is designed to be *used only once* for each FidSet object in order
  // to initialize it. 
  int Read(GIOFidSet *giofs);

  // Outputs the contents of this FidSet
  void Output();

  // Given a numerical fiducial type, retrieve its corresponding 
  // string fiducial name.
  string GetFidName(int type);

  // Given a string fiducial name, retrieve its corresponding 
  // numerical fiducial type.
  int GetFidType(string name);

  // Editing functions

  // Appends specified number of leads to the fidset 
  //int AddLeads(int numLeads);

  // Appends lead with corresponding fiducials to the fidset
  int AddLead(int numFids, Fid * arrayOfFids);

  // Deletes a lead and all of its fiducials from the FidSet
  int DeleteLead(int leadNum);

  // Adds a fiducial with the given info to the lead with index leadNum.
  //int AddFid(string name, float value, int leadNum);  

  // Deletes the fiducial with the given name from the lead with index leadNum.
  //int DeleteFid(string name, int leadNum, int occurrence);

  // Add a new occurrence of a fid with associated leads values
  int AddFidValueArray(string fidName, float *values, int numValues);

  // Delete an occurrence of a fid along with its associated leads values
  int DeleteFidValueArray(string fidName, int occurrence);

  // Changes the fiducial name in the lead with index leadNum
  // from oldName to newName. 
  //int ChangeFidName(string oldName, string newName, int leadNum);

  // Updates and returns the size (in bytes) of this FidSet.
  int UpdateSize();

  // Member variables
  
  // map referenced by fid name whose values are two dimensional arrays of
  // pointers to fids 
  // see documentation for diagram
  FidPtrMap fidsMap; 

  // hash of the maximum number of occurrences of each type of fiducial, referenced
  // by fiducial name
  FidOccurrenceMap fidOccurrenceMap; 

  FidPtr ** leadsArray;  // array of arrays of Fid pointers for each lead

  short type;                 // fidset's type - should be 1.
  short version;              // fidset's version - should be 0.
  int size;                    // number of bytes contained in the fidset
  string name;                 // fidset name. 
  string auditString; 		// fidset audit string. 
  string tsdfname;             // name of associated tsdf file.
  int numfidleads;            // number of leads that have fids.
  short * fidDescArray;   // fiducial descriptor array.
                          // this is an array of length descArraySize=numfidleads with an 
                          // entry for each lead.
                          // the value at each index is the number of fiducials for the 
                          // lead corresponding to that index.
  int descArraySize; // size of fiducial descriptor array
  int arrayIndex; // index of fidset in fidSetsArray
  int numFidTypesUnique; // number of unique fiducial types
  string * fidNamesUniqueArray; // array of all fiducial names without repeats
  int numFidTypesInclusive;              // number of fid types in the set, repeats included
  int totalNumFids; // total number of fiducials (of any type) in the set
  //string *fidNamesInclusiveArray;  //list of fid types by string in the set in the order they appear
                                // repeats included 

private:
  // Initialize the data structure for viewing info by fiducial name
  void InitializeFidsMap();

  // Deletes the fiducial map
  void DeleteFidsMap();

};

#endif












