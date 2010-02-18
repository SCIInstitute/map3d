/*
 *
 * C++ HEADER FILE : container.h
 * VERSION : 2.0
 * DESCRIPTION : This object contains the same information as container (tsdfc/dfc) file.  It takes
 * a container file and parses its data into various underlying data structures which can
 * then be passed to other applications.  It stores the underlying data in two
 * formats: an array and a hash of time series file name and their corresponding
 * parameter lists.  These pairs correspond to the key/value pairs found in the
 * container files, which are in reality gdbm files.  In this case, the key is the name of
 * a tsdf/tsdfc/dfc file and the value is a
 * parameter list (PSlist).  Usually, the parameter list will consist of a list of
 * parameter sets of type FidSet.
 *
 * AUTHOR(S)  : Jenny Simpson (simpson@cs.utah.edu) 
 * CREATED : 06/07/02
 * MODIFIED: 07/15/02
 * DOCUMENTATION: 
 * http://dante.sci.utah.edu/develop/simpson/scratch/documentation/tsdflib_doc.html
*/

#include <string.h>
#include <iostream>
#include "pslist.h"
#include <string>
#include "fslist.h"
#include "fidset.h"
#include "paramset.h"
#include "gdbmkeys.h"
#include "gdbm.h"
#include "cutil.h"
#include "giofidset.h"

using std::string;

// Entry struct -- consists of a container file name and its corresponding 
// parameter list. Right now the parameter list is hard coded to be a fiducial
// list 
struct Entry{

  // Member variables
  string entryName; // identifying name for this entry
  // this could be a tsdf, tsdfc, or dfc name, depending on
  // whether this is a tsdfc or dfc container 

  PSList * paramList; // pointer to parameter set list (FSList)
  string pslistType; // type of parameter set list 
  // right now, the only type of parmeter set list accepted is FSList
 
  int arrayIndex; // index of this entry in the entryArray
  
};


#ifndef CONTAINER_H
#define CONTAINER_H


// ****************************************************************************
// ****************************************************************************
// *************************** Class Container ********************************
// ****************************************************************************
// ****************************************************************************

typedef map <string, Entry *, less<string> > EntryMap;

class Container {
public:

  // Empty Constructor
  Container();

  // Copy Constructor
  Container(const Container& c);

  // Overloaded assignment operator
  const Container& operator = (const Container& c);

  // Destructor
  ~Container();

  // Reads in the info from a gdbm (.tsdfc) container file and parses it into the 
  // various data structures.
  // This is designed to be *used only once* for each Container object in order
  // to initialize it.
  int Read(string containerFile);

  // Writes current data structure to a *new* tsdfc file
  int Write(string containerFile); 

  // Testing function that outputs all container file information as ASCII to STDOUT
  void Output();

  // Editing functions

  // Adds an tsdf and its associated parameter set list to the container
  int AddTSDFEntry(string timeSeries, PSList *psl, string pslistType);

  // Adds a container file as an entry to the Container.  This 
  // is meant to be used to add entries (tsdfc or dfc file names)
  // to a dfc file
  int AddEntry(string entryName);

  // Deletes a container file entry from the container.
  int DeleteEntry(string name);

  // Changes the identifying name of an entry.
  int ChangeEntryName(string oldName, string newName);

  // Gets the path for the tsdf files pointed to in the tsdfc file.
  string GetTSDFFilePath(string fileName, string localPath);

  // Member variables

  EntryMap entryMap; /* hash whose keys are the string names of individual tsdf files and 
                   whose corresponding values are objects of type Entry.  For example, the Entry for a tsdf name foo.tsdf would be accessed using the following syntax: Entry * ePtr = entryMap["foo.tsdf"];
		*/ 

  
  Entry ** entryArray;  // array of key/value pairs.   
  int numEntries;  // number of time series in container and in the entryArray
               
  string containerFileName;

};

#endif







