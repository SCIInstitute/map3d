/*
 *
 * C++ METHODS (CC) FILE : container.cc
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

#include "container.h"
#include "gdbmkeys.h"
#include "gdbm.h"
#include "fslist.h"
#include "fidset.h"
#include "giofidset.h"
#include <string.h>
#include <string>
#include <stdio.h>

using std::string;

// ****************************************************************************
// ****************************************************************************
// *************************** Class Container ********************************
// ****************************************************************************
// ****************************************************************************

/*======================================================================*/

// * Post: Empty Constructor for a Container object.  
//
// * Arguments:  
//
Container::Container(){
  //printf("In Container empty constructor\n");
  entryArray = NULL;
  numEntries = 0;  
  containerFileName = "";

  //printf("End of Container empty constructor\n");    
}

/*======================================================================*/

// * Post: Copy constructor
//
// * Arguments:  
// const Container& c - Container to make copy of
//
Container::Container(const Container& c){
  //printf("In Container copy constructor\n");
  numEntries = c.numEntries; 
  if(numEntries != 0){
    entryArray = new Entry *[numEntries];   
    for(int i = 0; i < numEntries; i++){
      PSList * tempPS;
      if((c.entryArray[i])->pslistType != "FSList"){
        printf("ERROR: [Container::Container] - Unrecognized parameter set type\n");
        exit(0);
      }
      if(c.entryArray[i]->paramList != NULL){
        tempPS = (c.entryArray[i]->paramList)->Clone();
      }
      else{
        tempPS = NULL;
      }
      // copy all contents
      Entry *tempEntry = new Entry;
      *tempEntry = *(c.entryArray[i]);
      tempEntry->paramList = tempPS;
      entryArray[i] = tempEntry;
      entryMap[tempEntry->entryName] = tempEntry; 
    }
  }
  else{
    entryArray = NULL;
  }  


  containerFileName = c.containerFileName;

  //printf("End of Container copy constructor\n");
}

/*======================================================================*/

// * Post: Copy constructor
//  Checks to see if arrays are NULL before deleting
// them.  Checks to see that the two objects are not 
// equal.
//
// * Arguments:  
// const Container& c - Container to make copy of
//
const Container& Container::operator = (const Container& c){
  //printf("In Container assignment operator\n");
  if(this != &c){
   
    if(entryArray != NULL){
      for(int i = 0; i < numEntries; i++){
        delete (entryArray[i]->paramList);
      }
      delete [] entryArray;
      
    } 

    numEntries = c.numEntries; 
    if(numEntries != 0){
      entryArray = new Entry *[numEntries];   
      for(int i = 0; i < numEntries; i++){
        PSList * tempPS;
        if((c.entryArray[i])->pslistType != "FSList"){
          printf("ERROR: [Container::operator=] Unrecognized paramet set type\n");
          exit(0);
        }
        tempPS = (c.entryArray[i]->paramList)->Clone();
        
    
        // copy all contents
        Entry *tempEntry = new Entry;
        *tempEntry = *(c.entryArray[i]);
        tempEntry->paramList = tempPS;
        entryArray[i] = tempEntry;
        entryMap[tempEntry->entryName] = tempEntry; 
      }
    }
    else{
      entryArray = NULL;
    }  


    containerFileName = c.containerFileName;
   
    
  }
  //printf("End of Container assignment operator\n");
  return *this;
}


/*======================================================================*/

// * Post: Destructor
// Checks to see if arrays are NULL before deleting
// them.
//
// * Arguments: none
//
Container::~Container(){
  
  //printf("In Container destructor\n");
  if(entryArray != NULL){
    //printf("deleting entryArray\n");
    for(int i = 0; i < numEntries; i++){
      delete (entryArray[i]->paramList);
    }
    delete [] entryArray;
  }
  //printf("End of Container destructor\n");
  
}

/*======================================================================*/

// * Post: Testing function that outputs the values of the entire 
// container file tree structure starting with the container file name
// and going down to the individual fiducial values.  Assumes we're 
// dealing with fiducial sets.
//
// * Arguments: none
//
void Container::Output(){
  printf("***********************************************************************\n");      
  
  // list how many tsdf files are referenced
  printf("\nThere are %d entries in this container file\n\n", numEntries);

  for(int i = 0; i < numEntries; i++){

    printf("-----------------------------------------------------------------------\n");    

    string entryName = entryArray[i]->entryName;
    if(entryArray[i]->paramList != NULL){
    
      // get FSList pointer
      FSList *fsl = (FSList *)entryArray[i]->paramList;
   
      // get the number of fiducial sets in this FSList
      int numSets = fsl->NumSets();

      printf("\nEntry %d is called %s and has %d parameter set(s)\n\n", i, const_cast<char*>(entryName.c_str()), numSets);

      // loop through all fiducial sets
      for(int j = 0; j < numSets; j++){
        printf("-----------------------------------------------------------------------\n");  
        // get FidSet pointer
        FidSet *myFS = fsl->fidSetsArray[j];

        // get the number of leads for a given fiducial set
        int numLeads = myFS->numfidleads;
        printf("\nFidset %d\n", j);  
        printf("Type: %d\n", myFS->type);
        printf("Version: %d\n", myFS->version);
        printf("Size: %d\n", myFS->size);
        printf("Name: %s\n", const_cast<char*>(myFS->name.c_str()));                 // fidset name. 
        printf("Audit String: %s\n", const_cast<char*>(myFS->auditString.c_str())); 		// fidset audit string. 
        printf("Time Series File Name: %s\n", const_cast<char*>(myFS->tsdfname.c_str()));
        printf("Number of leads: %d\n\n", myFS->numfidleads);            // number of leads that have fids.

        printf("Here is a listing of fiducials organized by lead number:\n\n");

        for(int k = 0; k < numLeads; k++){
        
          int numFids = myFS->fidDescArray[k];

          // get array of Fid pointers for this lead
          Fid ** fidArray = myFS->leadsArray[k];
          printf("Lead number %d has %d fiducial(s)\n", k, numFids);
             
          for(int m = 0; m < numFids; m++){
            // get Fid pointer
            Fid * thisFid = fidArray[m];
            printf("Fid number: %d Type: %d Value: %f Name: %s\n", m, thisFid->type, thisFid->value, const_cast<char*>(thisFid->name.c_str())); 
          }
         
        } 

        printf("\nHere is a listing of fiducials organized by fiducial name:\n");

        for(int i = 0; i < myFS->numFidTypesUnique; i++){
          string fidName = myFS->fidNamesUniqueArray[i];
          printf("\nFid name: %s\n\n", const_cast<char*>(fidName.c_str()));
          int numFids = myFS->fidOccurrenceMap[fidName];

          for(int j = 0; j < numFids; j++){
            printf("Occurrence %d:\n", j);
            for(int k = 0; k < myFS->numfidleads; k++){
              if(myFS->fidsMap[fidName][j][k] != NULL){
                printf("Lead %d: %f\n", k, (myFS->fidsMap[fidName][j][k])->value);
              }
              else{
                printf("Lead %d: %s\n", k, "NULL");
              }
            }
            printf("\n");
          }
         
        }
        //printf("-----------------------------------------------------------------------\n");   
         
      }

      
    }
    else{
      printf("Time Series %d is called %s and has 0 parameter sets\n", i, const_cast<char*>(entryName.c_str()));
    }
    //printf("-----------------------------------------------------------------------\n");    
  }
  printf("***********************************************************************\n");  

}

/*======================================================================*/

// * Post: Takes the name of a container file and 
// parses its contents into the underlying data structure to initialize the 
// object. 
// This is designed to be *used only once* for each Container object in order
// to initialize it.
//
// Returns 0 if successful, -1 if read fails.
//
// * Arguments: 
// string containerFile - name of container file. This can be a tsdfc 
// or dfc file.
//
int Container::Read(string containerFile){
  //printf("In Container::Read\n");
  containerFileName = containerFile;

  // first, get all time series info
  GDBMKeys gkeys;
  
  numEntries = gkeys.ReadKeys(containerFile);
 
  if(numEntries < 0){
    numEntries = 0;
    printf("ERROR: [Container::Read] There was a problem reading the container file\n");
    return -1;
  }

  // initialize tsdf files array
  entryArray = new Entry *[numEntries];  

  int retVal;

  // read all tsdf file names from gdbm file
  // loop through all time series    
  for(int i = 0; i < numEntries; i++){
 
    string entryName = gkeys.ReturnKey(i); 
    
    // initialize corresponding PSList
    PSList * thisPSList;
  
    // parameter set list type temporarily hard coded to fiducial
    // this list will ignore non-fiducial sets
    // once other parameter set types are introduced a generalized
    // version of this class will need to be written
    string pslistType = "FSList";

    // check pslist type to make sure we're dealing with fiducials    
    if(pslistType == "FSList"){
      thisPSList = new FSList;
    }
    else{
      printf("ERROR: [Container::Read] Unrecognized parameter set type\n");
      return -1;
    }
    
    // fill in the PSList information
    retVal = thisPSList->Read(containerFile, entryName);

    thisPSList->SetArrayIndex(i);

    // check to see if read failed
    if(retVal == -1 || retVal == -2){
      printf("ERROR: [Container::Read] There was a problem reading the container file\n");
      return -1;
    }   

    // create Entry object
    Entry * timeSeries = new Entry;
    timeSeries->entryName = entryName;
    timeSeries->pslistType = pslistType;

    timeSeries->paramList = thisPSList;
    timeSeries->arrayIndex = i;

    // add tsdf and PSList to hash and array
    entryArray[i] = timeSeries;

    entryMap[entryName] = timeSeries;  

  }  

  //printf("End of Container::Read\n");
  return 0;
}

/*======================================================================*/

// * Post: Writes current data structure to a container (tsdfc or dfc) file
// Will only write to a new file, not to an existing one.
//
// Return 0 if successful
// Returns -1 if the write fails
//
// * Arguments: 
// string containerFile - This can be a tsdfc or dfc file.
//
int Container::Write(string containerFile){

  int ret = 0;

  // check to see if container file already exists.  If so, return -1
  // the GDBM_WRCREAT option should take care of this, but it doesn't for some
  // reason
  if(FILE *input = fopen(const_cast<char*>(containerFile.c_str()), "rb")){
    fclose(input);
    printf("ERROR: [Container::Write] Attempting to write to existing file\n");
    return -1; 
  }

  // open the gdbm file
  GDBM_FILE dbf =  gdbm_open(const_cast<char*>(containerFile.c_str()), 1024, GDBM_WRCREAT, 00644,0 );

  if(dbf != NULL){
    // for each time series, get raw data and write to container

    //printf("Writing - numEntries = %d\n", numEntries);
    for(int i = 0; i < numEntries; i++){
      //printf("Writing PSList " << i << "\n");

      datum content;
      if(entryArray[i]->paramList != NULL){

        FSList fsl = *((FSList *)(entryArray[i]->paramList));
        content = fsl.RawData();
       
      }
      else{
        char *temp = new char;
        *temp = 0; 
        content.dptr = temp;
        content.dsize = 1;
        delete temp;
      }

      string k = entryArray[i]->entryName;
    
      //printf("key = " << k << "\n");

      datum key = { const_cast<char*>(k.c_str()), k.size() };
   
      ret = gdbm_store(dbf, key, content, GDBM_REPLACE);

      if(ret == -1){
        printf("ERROR: [Container::Write] Container Write failed\n");
        return -1;
      }

      // deallocate this memory
      if(entryArray[i]->paramList != NULL){
        delete [] content.dptr;
      }
    }
    gdbm_close(dbf);
  }
  else{
    printf("Error: Container::Write - Failed to open gdbm file\n");
    return -1;
  }

  return 0;
}

/*======================================================================*/

// * Post:  Gets the path for the tsdf files pointed to in the tsdfc 
// file.  Walks through the series of possible locations of the file, 
// and then returns the first successful hit.  The returned string
// is then what would be used to call the graphicsio openfile_ command.
// Returns an empty string if the file is not found. 
// The user is responsible to set the TSDF_DATA_PATH environment 
// variable if it is to be used to provide directories for searching.
//
// Searches the following location for the full path to the actual
// location of fileName:
// 1)  Same directory as tsdfc file
// 2)  Local path passed to function
// 3)  Absolute path(s) in the TSDF_DATA_PATH environment variable
//
// * Arguments:
// string fileName - base filename from the tsdfc file 
// string localPath - any local path set within the calling program.
//
string Container::GetTSDFFilePath(string fileName, string localPath){
  // search the following locations for the full path to the actual
  // location of fileName
  // 1)  Same directory as tsdfc file
  // 2)  Local path passed to function
  // 3)  Absolute path(s) in the TSDF_DATA_PATH environment variable

  // check current directory  
  string thisDir = getenv("PWD");
 
  string fullPath = thisDir + "/" + fileName;

  if (FILE *input = fopen(const_cast<char*>(fullPath.c_str()), "rb")) {
    fclose(input);
    return thisDir;   
  }  
  
  // check localPath passed in by user

  fullPath = thisDir + "/" + localPath + "/" + fileName;
   
  if(FILE *input = fopen(const_cast<char*>(fullPath.c_str()), "rb")){
    fclose(input);
    return (thisDir + "/" + localPath);
  }

  // check all paths in environment variable

  if(getenv("TSDF_DATA_PATH") == NULL){
    printf("ERROR: [Container::GetTSDFFileName] TSDF_DATA_PATH environment variable not set.\n");
    return "";
  }

  string envVar = getenv("TSDF_DATA_PATH");

  //split environment variable into individual paths

  string thisPath;
  int pos = envVar.find(":");
 
  while((pos = envVar.find(":")) >= 0){
    thisPath = envVar.substr(0, pos);
    envVar = envVar.substr(pos+1);
    fullPath = thisPath + "/" +  fileName;

    if(FILE *input = fopen(const_cast<char*>(fullPath.c_str()), "rb")){
      fclose(input);
      return envVar;
    }
  }
 
  fullPath = envVar + "/" + fileName;

  if(FILE *input = (fopen(const_cast<char*>(fullPath.c_str()), "rb"))){
    fclose(input);
    return envVar;
  }

  printf("ERROR: [Container::GetTSDFFileName] - Didn't find file\n");
  return "";
}


/*======================================================================*/

// ***************************************************************************
// ************************* Editing Functions *******************************
// ***************************************************************************

/*====================================================================*/

// * Post: Adds an tsdf and its associated parameter set list to the
// container
//
// Returns 0 if successful
// Returns -1 if there is already an entry for this time series
// or if the pslistType is something other than "FSList"
//
// * Arguments:
// string entryName -  Time series data file (tsdf) name.
// PSList *psl - Pointer to a parameter set list to be associated with
// the tsdf file.  
// string pslistType - Type of parameter set list.  For now, this should
// always be set to "FSList".
//
int Container::AddTSDFEntry(string entryName, PSList *psl, string pslistType){
  //printf("In Container::AddTSDFEntry\n");
  
  if(entryMap.count(entryName)){
    printf("ERROR: [Container::AddTSDFEntry] There is already an entry for this time series\n");
    return -1;
  }
  if(pslistType != "FSList"){
    printf("ERROR: [Container::AddTSDFEntry] Unrecognized parameter set list type. This should be set to \"FSList\"\n");
    return -1;
  }


  PSList * newPSL; 

  if(psl != NULL){

    newPSL = psl->Clone();
  

    newPSL->SetArrayIndex(numEntries);
    newPSL->SetName(entryName);
 
  }
  else{
    printf("WARNING: [Container::AddTSDFEntry] NULL PSList\n");
    newPSL = new FSList;
  }


  // create new array
  Entry ** newTsdfFilesArray = new Entry *[numEntries+1]; // array of Entry

  // copy old array
  for(int i = 0; i < numEntries; i++){
    Entry * temp = new Entry;
    *temp = *entryArray[i];
    temp->paramList = (entryArray[i]->paramList)->Clone();
    newTsdfFilesArray[i] = temp;
    
  }

  Entry * tempEntry= new Entry;
  tempEntry->paramList = newPSL;
  tempEntry->entryName = entryName;
  tempEntry->pslistType = pslistType;  
  tempEntry->arrayIndex = numEntries;
 
  newTsdfFilesArray[numEntries] = tempEntry;

  // clean up old memory
  if(entryArray != NULL){
    //printf("deleting entryArray\n";
    for(int i = 0; i < numEntries; i++){
      delete (entryArray[i]->paramList);
    }
    delete [] entryArray;
  } 
  
  // set new values
  entryArray = newTsdfFilesArray;

  // add to map
  entryMap[entryName] = tempEntry;

  numEntries++;
  
  
  //printf("End of Container::AddTSDFEntry\n");

  return 0;
}

/*====================================================================*/

// * Post: Adds a container file as an entry to the Container.  This 
// is meant to be used to add entries (tsdfc or dfc file names)
// to a dfc file
//
// Returns 0 if successful
// Returns -1 if there is a duplicate entry
//
// * Arguments:
// string entryName - name of container file (tsdfc or dfc) to add
// to this dfc Container 
// 
int Container::AddEntry(string entryName){

 if(entryMap.count(entryName)){
    printf("ERROR: [Container::AddDFCEntry] There is already an entry for this time series\n");
    return -1;
  }


  PSList * newPSL = new FSList; // this is an arbitrary place holder 
    
  newPSL->SetArrayIndex(numEntries);
  newPSL->SetName(entryName);
 

  // create new array
  Entry ** newTsdfFilesArray = new Entry *[numEntries+1]; // array of Entry

  // copy old array
  for(int i = 0; i < numEntries; i++){
    Entry * temp = new Entry;
    *temp = *entryArray[i];
    temp->paramList = (entryArray[i]->paramList)->Clone();
    newTsdfFilesArray[i] = temp;
    
  }

  Entry * tempEntry= new Entry;
  tempEntry->paramList = newPSL;
  tempEntry->entryName = entryName;
  tempEntry->pslistType = "none";  
  tempEntry->arrayIndex = numEntries;
 
  newTsdfFilesArray[numEntries] = tempEntry;

  // clean up old memory
  if(entryArray != NULL){
    //printf("deleting entryArray\n";
    for(int i = 0; i < numEntries; i++){
      delete (entryArray[i]->paramList);
    }
    delete [] entryArray;
  } 
  
  // set new values
  entryArray = newTsdfFilesArray;

  // add to map
  entryMap[entryName] = tempEntry;

  numEntries++;
  return 0;
}

/*====================================================================*/

// * Post:  Deletes a container file entry from the container.
//
// * Arguments:
// string name - name of entry to delete.  If deleting from a tsdfc
// file, this would be a tsdf name.  If deleting from a dfc file, this
// would be either a tsdfc or dfc file name.
//
int Container::DeleteEntry(string name){
  if(!(entryMap.count(name))){
    printf("ERROR: [Container::DeleteEntry] Entry doesn't exist\n");
    return -1; 
  }  

  // deallocate memory allociated with this FidSet
  PSList * psl = entryMap[name]->paramList;
  int deleteIndex = psl->GetArrayIndex();
  delete psl;


  entryMap.erase(name); 

  for(int i = deleteIndex; i < numEntries-1; i++){
    entryArray[i] = entryArray[i+1];
    entryArray[i]->paramList->SetArrayIndex(i);
  }

  numEntries--;
  
  return 0;
  
}

/*====================================================================*/

// * Post: Changes the identifying name of an entry. 
//
// Returns 0 if successful
// Returns -1 if the entry doesn't exist
//
// * Arguments: 
// string oldName - old entry name.  If the container is a tsdfc file,
// this would be a tsdf name.  If the container is a dfc file, this 
// would be either a tsdfc or dfc file name.
// string newName - new entry name.  If the container is a tsdfc file,
// this would be a tsdf name.  If the container is a dfc file, this 
// would be either a tsdfc or dfc file name.
//
int Container::ChangeEntryName(string oldName, string newName){
  if(!entryMap.count(oldName)){
    printf("ERROR: [Container::ChangeEntryDFName] Entry doesn't exist\n");
    return -1;
  }  

  Entry * ts = entryMap[oldName];
  PSList * psl = ts->paramList;
  psl->SetName(newName);
  ts->entryName = newName;

  entryMap.erase(oldName);
  entryMap[newName] = ts;
  
  return 0;
}

/*====================================================================*/















