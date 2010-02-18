/*
 *
 * C++ METHODS (CC) FILE : gdbmkeys.cc
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

#include "gdbmkeys.h"
#include "gdbm.h"
#include "cutil.h"
#include <stdlib.h>
#include <string.h>
#include <string>

using std::string;

// ****************************************************************************
// ****************************************************************************
// *************************** Class GDBMKeys *********************************
// ****************************************************************************
// ****************************************************************************

/*====================================================================*/

// * Post: Default constructor. 
// Checks to make sure that size is > 0.  Exits
// with an error message if size <= 0.
//
// * Arguments: 
// int size - Initial size of the database. Default is 10.
GDBMKeys::GDBMKeys( int size ) 
{   // Constructor
  //printf("In GDBMKeys constructor\n");
    if ( size <= 0 ) {
	ReportError("GDBMKeys::Constructor",
		    " Bad GDBMKeys size", 0, "" );
	exit(0);
    }
    keys = new string[size];
    maxnumkeys = size;
    numkeys = 0;
    //printf("End of GDBMKeys constructor\n");

}

/*====================================================================*/

// * Post: Destructor
// Checks to see if pointer is NULL before deleting
//
// * Arguments: none
//
GDBMKeys::~GDBMKeys() 
{     // Destructor
  //printf("In GDBMKeys destructor\n");
  
  if(keys != NULL){
    delete [] keys;
  }
  
  numkeys = 0;
  
  //printf("End of GDBMKeys destructor\n");
}

/*====================================================================*/

// * Post: Grow the array of keys by 50%.
// Checks to make sure that keys isn't NULL
//
// * Arguments: none
//
void GDBMKeys::Grow()
{
  if(keys != NULL){
    string * oldkeys = keys;
    int oldmaxnumkeys = maxnumkeys;
    maxnumkeys += maxnumkeys/2 + 1;
    keys = new string[maxnumkeys];
    for ( int knum=0; knum < oldmaxnumkeys; knum++ )
    {
	keys[knum] = oldkeys[knum];
    }
    for ( int knum = oldmaxnumkeys; knum < maxnumkeys; knum++ )
    {
	keys[knum] = "";

    }
    
    // Clean up old memory that we do not need.
    delete [] oldkeys;
  }  
}


/*====================================================================*/

// * Post: Load a single key string in the next entry in the key vector.
// If there isn't room for the key, grow the array
//
// * Arguments: 
// string keystring - string name of key to load
//
void GDBMKeys::Load_Key(string keystring )
{

    if ( ++numkeys > maxnumkeys ) 
	Grow();
    keys[numkeys-1] = keystring;
   
}

/*====================================================================*/

// * Post: Load a single key string to a particular entry in the key vector. 
// Checks to see if key number is out of bounds.  Reports an error if 
// it is.
//
// * Arguments: 
// string keystring - string name of key to load
// int keynum - index in key vector where key is to be loaded
//
void GDBMKeys::Load_Key(string keystring, int keynum )
{

    if ( keynum >= numkeys )
    {
	ReportError("GDBMKeys::Load_Key","key number out of bounds", 
		    ERR_MISC, "");
	exit(ERR_MISC);
    }

    keys[keynum] = keystring;

}

/*====================================================================*/

// * Post: Overloaded [] operator
// Checks to make sure index is >= 0. Reports an error if not.
//
// * Arguments: 
// int index - index of key to retrieve
//
string GDBMKeys::operator[](int index)
{
    char errstring[40];
    if ( index < 0 ) 
    {
	sprintf( errstring, "Index = %d\n", index);
	ReportError("GDBMKeys::operator[]","Bad index", ERR_MISC, errstring );
	exit(ERR_MISC);
    }
    return( keys[index] );
}

/*====================================================================*/

// * Post: Read list of keys from a gdbm file.
// Returns N>0 if successful, where N = number of keys.
// Returns -1 if the gdbm file could not be opened.
//
// * Arguments: 
// string gdbmFileName - name of gdbm file 
//
int GDBMKeys::ReadKeys(string gdbmFileName )
{

    int numkeys = 0;
    string keyString;
    datum key, nextkey; // datum = ptr to data + data size 

    //char * tempFN = (char *)stoc(gdbmFileName);
    GDBM_FILE gdbmFile = gdbm_open( const_cast<char*>(gdbmFileName.c_str()), 1024, GDBM_READER, 00644, 0);

    //delete [] tempFN;
    
    if (gdbmFile) 
    {
	//printf("File opened successfuly--read first key\n");
        key = gdbm_firstkey(gdbmFile);
	numkeys=0;
	//printf("Got first key\n");
        
        while (key.dptr) 
	{
            char *tempKeyString = new char[key.dsize + 1];
            memcpy(tempKeyString, key.dptr, key.dsize);
            
            tempKeyString[key.dsize] = '\0';
            keyString = tempKeyString;
            
            Load_Key( keyString );
	  
            delete [] tempKeyString;
            nextkey = gdbm_nextkey(gdbmFile, key);
            free(key.dptr);
            key = nextkey;
	    numkeys++;
        }
        gdbm_close(gdbmFile);
        
    } else {
        numkeys = -1;
    }
    
    
    return numkeys;
}


/*====================================================================*/

// * Post: Write a key to a given gdbm file
// Returns -1 if write fails, 0 otherwise
//
// * Arguments: 
// string gdbmFileName - name of gdbm file
// datum key - datum (raw data) version of key
// datum content - datum (raw data) version of content
//
int GDBMKeys::WriteKey(string gdbmFileName, datum key, datum content)
{
  if(content.dptr == NULL){
    printf("WARNING: [GDBMKeys::WriteKey] Inserting empty content into gdbm\n");

  }
  GDBM_FILE dbf;

  dbf  =  gdbm_open(const_cast<char*>(gdbmFileName.c_str()), 1024, GDBM_WRCREAT, 00644,0 );

  if(dbf){

    int ret = gdbm_store(dbf, key, content, GDBM_REPLACE);
    gdbm_close(dbf);
    
    if(ret != 0){
      printf("ERROR: [GDBMKeys::WriteKey] Error trying to add key/value to gdbm\n");

      return -1;
    }


    string keyString;
    char *tempKeyString = new char[key.dsize + 1];
    memcpy(tempKeyString, key.dptr, key.dsize);
            
    tempKeyString[key.dsize] = '\0';
    keyString = tempKeyString;
    Load_Key( keyString );

    numkeys++;
  }
  else{
    printf("ERROR: [GDBMKeys::WriteKey] - Failed to open gdbm file\n");
    return -1;
  }

  return 0;
}

/*====================================================================*/

// * Post: Return the ith key in the list. 
// Checks to make sure that keynum is less than
// maxnumkeys
//
// * Arguments: 
// int keynum - index of key to return
// 
string GDBMKeys::ReturnKey( int keynum )
{
  if(keynum < maxnumkeys){

    return( keys[keynum] );
  }
  else{
    printf("ERROR: [GDBMKeys::ReturnKey] Invalid key number\n");
    return "";
  }
}

/*====================================================================*/













