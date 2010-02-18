/*
 *
 * C++ HEADER FILE : gdbmkeys.h
 * VERSION : 2.0
 * DESCRIPTION :  Class that manages the keys within a Gnu DataBase Manager file.
 * It knows nothing about internal contents of the file, just the basic
 * structure of keys and associated block of data.
 * It is meant to be able to read and write keys.
 * AUTHOR(S)  : Rob Macleod (modified slightly by Jenny Simpson) 
 * CREATED : 
 * MODIFIED: 
 * DOCUMENTATION:  
 * http://dante.sci.utah.edu/develop/simpson/scratch/documentation/tsdflib_doc.html 
 *
*/

#ifndef GDBMKEYS_H
#define GDBMKEYS_H

#include <string>
#include "gdbm.h" 

using std::string;

// ****************************************************************************
// ****************************************************************************
// *************************** Class GDBMKeys *********************************
// ****************************************************************************
// ****************************************************************************

class GDBMKeys {

  
public:

  // Default constructor
  GDBMKeys( int size = 10);

  // Destructor 
  ~GDBMKeys();

  // Grow the array of keys by 50%. 
  void Grow();
  
  // Load a single key string in the next entry in the key vector. 
  void Load_Key(string keyString); 

  // Load a single key string to a particular entry in the key vector.
  void Load_Key(string keyString, int keynum); 

  // Returns number of keys in gdbm
  int NumKeys() { return numkeys; };

  // Overloaded [] operator
  string operator[](int index);

  // Read list of keys from a gdbm file.
  int ReadKeys(string gdbmFileName );

  // Write a key to a given gdbm file
  int WriteKey(string gdbmFileName, datum key, datum content);

  // Return the ith key in the list.  
  string ReturnKey( int keynum );

  //  Member Variables 
  string *keys;    // array of strings
  int numkeys;    // number of keys stored
  int maxnumkeys; // size of the keys array


};

#endif




