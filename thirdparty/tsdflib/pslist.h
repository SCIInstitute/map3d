/*
 *
 * C++ HEADER FILE : pslist.h
 * VERSION : 1.0
 * DESCRIPTION : 
 * 
 * AUTHOR(S)  : Rob Macleod and Jenny Simpson (simpson@cs.utah.edu) 
 * CREATED : 06/07/02
 * MODIFIED: 
 * DOCUMENTATION: 
 *
*/

#ifndef PSLIST_H
#define PSLIST_H


#include <string>
#include "fidset.h"

using std::string;

// ****************************************************************************
// ****************************************************************************
// *************************** Class PSList ***********************************
// ****************************************************************************
// ****************************************************************************

class PSList {
public:
  
  virtual int Read(string tsdfcFileName, string tsdfName) = 0;
  virtual int NumSets() = 0; // returns number of parameter sets in the list
  virtual void SetArrayIndex(int i) = 0;
  virtual int GetArrayIndex() = 0;
  virtual string GetName() = 0;
  virtual void SetName(string tsdfName) = 0; 
  virtual PSList * Clone() = 0;  
  virtual string GetType() = 0; 
  
  int arrayIndex;

};

#endif 




