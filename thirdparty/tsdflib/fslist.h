/*
 *
 * C++ HEADER FILE : fslist.h
 * VERSION : 2.0 
 * DESCRIPTION : This is a list of fidsets which is stored as both a hash
 * and an array of fidsets.  
 * Individual fidsets are referenced in the hash by a unique id.  One of these
 * objects is stored for each time series in the tsdfc and is called more 
 * generally a parameter list.  It inherits the PSList class.
 *
 * AUTHOR(S)  : Jenny Simpson (simpson@cs.utah.edu) 
 * CREATED : 06/07/02
 * MODIFIED: 07/15/02
 * DOCUMENTATION:
 * http://dante.sci.utah.edu/develop/simpson/scratch/documentation/tsdflib_doc.html 
 *
*/

#ifndef FSLIST_H
#define FSLIST_H

#include "pslist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fslist.h"
#include "fidset.h"
#include "gdbm.h"
#include "cutil.h"
#include "giofidset.h"
#include <string>
#include <map>

using std::string;

// ****************************************************************************
// ****************************************************************************
// *************************** Class FSList ***********************************
// ****************************************************************************
// ****************************************************************************

typedef map <string, FidSet *, less<string> > FSMap;

class FSList: public PSList {
public:

  // Empty constructor
  FSList();

  // Copy Constructor
  FSList(const FSList& f);

  // Overloaded assignment operator
  const FSList& operator = (const FSList& f);

  // Destructor
  ~FSList();
  
  //Reads all data for a given time series and puts information 
  // into appropriate data structures.
  // This is designed to be *used only once* for each FSList object in order 
  // to initialize it.
  int Read(string tsdfcFileName, string tsdfName);
   
  // Examine all parameter sets (fiducial and other) to find out their type
  int ExamineParamSets();

  // Returns raw data version of fslist.
  datum RawData();

  // Outputs the contents of this fiducial set list
  void Output();

  // returns number of parameter sets in the list
  int NumSets();

  // Returns the name of the tsdf file associated with this FSList
  string GetName(); 
 
  // Sets the arrayIndex member variable to i. 
  void SetArrayIndex(int i);

  // Returns the array index of this FSList in the entryArray in 
  // the Container object, which is one level up in this data structure.
  int GetArrayIndex();

  //Sets the name of the tsdf file associated with this FSList 
  // to the specified name
  void SetName(string tsdfName); 

  // Makes a clone of this FSList and returns a pointer to it.
  // This is needed since this in an inheriting class.
  PSList * Clone();    

  //Returns the type of this parameter set list, which is FSList
  string GetType();

  // Editing Functions
  
  // Adds a FidSet to this FSList.  This new FidSet must have a 
  // different name from all the other FidSets in this FSList.
  int AddFidSet(FidSet fs);

  // Deletes the FidSet with the given name from this FSList
  int DeleteFidSet(string name);

  // Change the name of one of the FidSets in this FSList from oldName to newName.
  int ChangeFSName(string oldName, string newName);

  // Member variables
  
  FSMap fidSetsMap; /* hash whose keys are the unique id (string name) of individual fiducial sets and whose corresponding values are pointers to fiducial sets of type FidSet.
		*/ 

  FidSet ** fidSetsArray; // array of pointers to FidSets

  string containerFileName; // container file name
  string TSFileName; // time series file name
  int numPS; // number of parameter sets (fiducial sets) contained in the list
  int arrayIndex; // index of fslist in entryArray 
  int totalNumPS; // number of parameter sets (fiducial and other) found with the tsdf entry  
  short * allParamTypes; // list of all id values for parameter sets encountered
};

#endif

















