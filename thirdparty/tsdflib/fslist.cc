/*
 *
 * C++ METHODS (CC) FILE : fslist.cc
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

#include "fslist.h"
#include "giofidset.h"
#include "myfile.h"
#include <string>
#include <map>

using std::string;

// ****************************************************************************
// ****************************************************************************
// *************************** Class FSList ***********************************
// ****************************************************************************
// ****************************************************************************

/*====================================================================*/

// * Post: Empty Constructor.
//
// * Arguments: none
//
FSList::FSList(){
  //printf("In FSList empty constructor\n");
  containerFileName = "";
  TSFileName = "";
  fidSetsArray = NULL; 
  numPS = 0;
  arrayIndex = -1; // not yet in the array

  //printf("End of FSList empty constructor\n");
}

/*====================================================================*/

// * Post: Copy constructor.
//
// * Arguments:
// const FSList& f - FSList to make a copy of 
//
FSList::FSList(const FSList& f){
  
  //printf("In FSList copy constructor\n");
  numPS = f.numPS;

  if(numPS != 0){
    
    fidSetsArray = new FidSet *[numPS];
    for(int i = 0; i < numPS; i++){
      FidSet * tempFS = new FidSet;
     
      // copy the contents
      
      *tempFS = *f.fidSetsArray[i];
 
      
      fidSetsArray[i] = tempFS;
      fidSetsMap[tempFS->name] = tempFS;
    }
  }
  else{
    fidSetsArray = NULL;
  }
  
  containerFileName = f.containerFileName;
  TSFileName = f.TSFileName;
  arrayIndex = f.arrayIndex;  

  //printf("End of FSList copy constructor\n");
}

/*====================================================================*/

// * Post: Destructor.
// Checks to make sure that fidSetsArray isn't NULL
// before deleting
//
// * Arguments: none
//
FSList::~FSList(){
  
  //printf("In FSList destructor\n");
  if(fidSetsArray != NULL){
    //printf("Deleting fidSetsArray\n";
    for(int i = 0; i < numPS; i++){
      delete fidSetsArray[i];
    }
    delete [] fidSetsArray;
    fidSetsArray = NULL;
  }
  //printf("End of FSList destructor\n");
  
}

/*====================================================================*/

// * Post: Overloaded assignment operator.
// Checks to see if arrays are NULL before deleting
// them.   Checks to see that the two objects are not 
// equal.
//
// * Arguments: 
// const FSList& f - FSlist to make a copy of
//
const FSList& FSList::operator = (const FSList& f){
  //printf("In FSList assignment operator\n");
  if(this != &f){ 

    if(fidSetsArray != NULL){
      for(int i = 0; i < numPS; i++){
        delete fidSetsArray[i];
      }
      delete [] fidSetsArray;
      fidSetsArray = NULL;
    }
    // get number of parameter sets
    numPS = f.numPS;

    if(numPS != 0){     
      fidSetsArray = new FidSet *[numPS];
      for(int i = 0; i < numPS; i++){
        FidSet * tempFS = new FidSet;
     
        *tempFS = *f.fidSetsArray[i];
 
        fidSetsArray[i] = tempFS;
        fidSetsMap[tempFS->name] = tempFS;
      }
    }
    else{
      fidSetsArray = NULL;
    }

    containerFileName = f.containerFileName;
    TSFileName = f.TSFileName;
    arrayIndex = f.arrayIndex;  
  
  }
  //printf("End of FSList assignment operator\n");
  return *this;
}

/*====================================================================*/

// * Post: Makes a clone of this FSList and returns a pointer to it.
// This is needed since this in an inheriting class.
//
// * Arguments: none
//
PSList * FSList::Clone(){
  //printf("In FSList::Clone\n");
  //PSList * f = new FSList;
  FSList * f = new FSList;

  f->numPS = numPS;

  if(numPS != 0){
    
    f->fidSetsArray = new FidSet *[numPS];
    for(int i = 0; i < numPS; i++){
      FidSet * tempFS = new FidSet;
     
      // copy the contents
      
      *tempFS = *fidSetsArray[i];
 
      
      f->fidSetsArray[i] = tempFS;
      f->fidSetsMap[tempFS->name] = tempFS;
    }
  }
  else{
    f->fidSetsArray = NULL;
  }
  
  f->containerFileName = containerFileName;
  f->TSFileName = TSFileName;
  f->arrayIndex = arrayIndex;
 
  //printf("End of FSList::Clone\n");

  return f;
}

/*====================================================================*/

// * Post: Reads all data for a given time series and puts information 
// into appropriate underlying data structures.  Ignores any non-fiducial 
// sets encountered, but outputs a warning message.  This is designed to 
// be *used only once* for each FSList object in order to initialize it.
// 
// Returns 0 if successful, -1 otherwise.
//
// * Arguments:  
// string tsdfcFileName - name of container tsdfc file 
// string tsdfName - name of time series data file in tsdfc file
//
int FSList::Read(string tsdfcFileName, string tsdfName){
 
  containerFileName = tsdfcFileName;

  TSFileName = tsdfName;
   
  // declare Rob's type of fidset
  GIOFidSet tempFS;
 
  // calculate number of sets
  // numPS = tempFS.NumSets(const_cast<char*>(containerFileName.c_str()), const_cast<char*>(TSFileName.c_str()));
  ExamineParamSets();
  
  if(numPS == -1){
    printf("ERROR: [FSList::Read] There was a problem reading the container file\n");
    return -1;
  }


  fidSetsArray = new FidSet *[numPS];
  
  // loop through all fid sets for a given time series
  for(int i = 0; i < totalNumPS; i++){
    // load fid set info into FidSet object
    if(allParamTypes[i] == 1){    
      GIOFidSet tempFidSet;
  
      int retVal = tempFidSet.ReadGIOFidSet(const_cast<char*>(containerFileName.c_str()), const_cast<char*>(TSFileName.c_str()), i);
      if(retVal == -1 || retVal == -2){
        printf("ERROR: [FSList::Read] There was a problem reading the file or its contents\n");
        return retVal;
      }

      FidSet * regFidSet = new FidSet;

      // convert GIOFidSet to FidSet
      if(regFidSet->Read(&tempFidSet) == -1){
        printf("ERROR: [FSList::Read] FidSet read failed\n");
        return -1;
      }

      regFidSet->arrayIndex = i;
    
      fidSetsMap[regFidSet->name] = regFidSet;
    
      fidSetsArray[i] = regFidSet;
    }
    else{
      printf("WARNING: [FSList::Read] - Encountered non-fiducial parameter set.  Ignoring\n");
    }   
  }

  return 0;
}

/*====================================================================*/

// * Post: Examine all parameter sets (fiducial and other) to find out their type 
// Initializes the numPS, totalNumPS, and allParamTypes member variables
//
// Returns totalNumPS if successfull, -1 otherwise
//
// * Arguments: none
//
int FSList::ExamineParamSets(){
  int tmpSize = 50;
  allParamTypes = new short[tmpSize];
  
  short type, version;
  int size;

  totalNumPS = 0;
  numPS = 0;
  char *cdata_p, *enddata_p;
  datum key, data;


  /***   Open the tsdfc file - it's really just a gdbm file. ***/

  GDBM_FILE tsdfcFile = gdbm_open(const_cast<char*>(containerFileName.c_str()),
				  1024, GDBM_READER, 00644, 0);
  if (! tsdfcFile) {
    ReportError("[FidSet::NumSets]", "could not open the file",
		ERR_FILE, "");
    printf("ERROR: [FidSet::NumSets] Could not open the file %s\n", const_cast<char*>(containerFileName.c_str()));
    //return( ERR_FILE );
    return -1;
  }

  //printf("NumSets: Opened file %s\n", tsdfcFileName);

  /*** Now find the right key. ***/

  key.dptr = const_cast<char*>(TSFileName.c_str());
  key.dsize = strlen(const_cast<char*>(TSFileName.c_str()));
  data = gdbm_fetch(tsdfcFile, key);
  gdbm_close(tsdfcFile);
  if (! data.dptr) {
    ReportError("[GIOFidSet::NumSets]", "could not find the key",
		ERR_FILE, "");
    printf("ERROR: [GIOFidSet::NumSets] Could not find the key %s\n", const_cast<char*>(TSFileName.c_str()));
    //return( ERR_FILE );
    return -1;
  }
  
  /*** Now run through the file and see how many fidsets there are. ***/

  cdata_p = data.dptr;
  enddata_p = data.dptr + data.dsize - 3;
  //printf(" Fetched data block of size %xX with pointer from %xX to %xX\n", data.dsize, data.dptr, enddata_p );
  //numsets = 0;
  while ( cdata_p < enddata_p ) {
    //printf("Getting record info\n");
    //cdata_p = GetRecordInfo( cdata_p );

    //printf("Entering GetRecordInfo cdata_p = %xX\n", cdata_p);
    cdata_p = (char *)mfmemread((void *)&type,sizeof(short),1,(void *)cdata_p,mfSHORT); 
    cdata_p = (char *)mfmemread((void *)&version,sizeof(short),1,(void *)cdata_p,mfSHORT); 
    cdata_p = (char *)mfmemread((void *)&size,sizeof(int),1,(void *)cdata_p,mfINT); 

    //printf("type = %d version = %d size = %d\n", type, version, size);
    if ( type == 1 && version == 0 ) {

      numPS++;
      // printf(" Read record #%d\n", numsets);
    }
    else{
      printf("WARNING: [FSList::ExamineParamSets] Encountered non-fiducial parameter set. Ignoring\n");
    }
    allParamTypes[totalNumPS] = type;
    totalNumPS++;

    cdata_p += size;
    //printf(" Just increased data pointer to %x\n", cdata_p);
  }

  //printf("Numsets returning\n");
  return totalNumPS;

}
  
/*====================================================================*/

// * Post: Returns raw data version of fslist.  This raw data
// is used as the data part of the key/data pair in a gdbm file.
//
// * Arguments: none
// 
// Note: datum contains two members:
// a char * and the integer size of the data pointed to.
//
datum FSList::RawData(){

  
  int totalSize = 0;

  int * allSizes = new int[numPS];
  int overhead = sizeof(short) + sizeof(short) + sizeof(int);

  // for each fidset, calculate amount of memory needed
  for(int i = 0; i < numPS; i++){

    FidSet fs = *(fidSetsArray[i]);
    
    
    int nameStringSize = fs.name.length();
    int auditStringSize = fs.auditString.length();
    int fidDescArraySize = fs.descArraySize;
    int totalNumFids = fs.totalNumFids;

    int fidSize =  sizeof(int) + sizeof(char)*nameStringSize +
         sizeof(int) + sizeof(char)*auditStringSize +
         sizeof(int) + sizeof(short)*fidDescArraySize +
         sizeof(int) + (sizeof(float)+sizeof(short))*totalNumFids; 

    int size = overhead + fidSize;
    allSizes[i] = fidSize;

    totalSize += size;

  }


  char * head = new char[totalSize];
  char * cdataP = head;

  for(int i = 0; i < numPS; i++){

    FidSet fs = *(fidSetsArray[i]);

    //printf("*********** Writing Fidset *************\n\n");
    int nameStringSize = fs.name.length();
    int auditStringSize = fs.auditString.length();
    int fidDescArraySize = fs.descArraySize;
    //int numFidTypesInclusive = fs.numFidTypesInclusive;

    // set type (short)
    memcpy(cdataP, &fs.type, sizeof(short));
    cdataP += sizeof(short);
    //printf("| type = %d (%d bytes) |", fs.type, sizeof(short));
    
    // set version (short)
    memcpy(cdataP, &fs.version, sizeof(short));
    cdataP += sizeof(short);
    //printf("| version = %d (%d bytes) |", fs.version, sizeof(short));

    // set size (int)
    memcpy(cdataP, &allSizes[i], sizeof(int));
    cdataP += sizeof(int);
    //printf("| size = %d (%d bytes) |", allSizes[i], sizeof(int));

    // set fidnamesize (int)
    memcpy(cdataP, &nameStringSize, sizeof(int));
    cdataP += sizeof(int);
    //printf("| fidNameSize = %d (%d bytes) |", nameStringSize, sizeof(int));

    // set name (char[])

    memcpy(cdataP,  const_cast<char*>(fs.name.c_str()), nameStringSize * sizeof(char));
    cdataP += nameStringSize * sizeof(char);
    //printf("| name = %s (%d bytes) |", const_cast<char*>(fs.name.c_str()), nameStringSize * sizeof(char));

    // set auditsize (int)
    memcpy(cdataP, &auditStringSize, sizeof(int));
    cdataP += sizeof(int);
    //printf("| auditSize = %d (%d bytes) |", auditStringSize, sizeof(int));

    // set auditString (char[])

    memcpy(cdataP, const_cast<char*>(fs.auditString.c_str()), auditStringSize * sizeof(char));
    cdataP += auditStringSize * sizeof(char);
    //printf("| auditString = %s (%d bytes) |", const_cast<char*>(fs.auditString.c_str()), auditStringSize * sizeof(char));

    // set fidcountsize (int)
    memcpy(cdataP, &fidDescArraySize, sizeof(int));
    cdataP += sizeof(int);
    //printf("| descArraySize = %d (%d bytes) |", fidDescArraySize, sizeof(int));

    // set fidDescArray (short[])
    memcpy(cdataP, fs.fidDescArray, fidDescArraySize * sizeof(short));
    cdataP += fidDescArraySize * sizeof(short);
    
    //printf("| fidDescArray ");
    for(int k = 0; k < fidDescArraySize; k++){
      //printf(" %d", fs.fidDescArray[k]);
    }
    //printf(" (%d bytes) |", fidDescArraySize * sizeof(short));

    // set fidarraysize (int)
    memcpy(cdataP, &fs.totalNumFids, sizeof(int));
    cdataP += sizeof(int);
    //printf("| totalNumFids = %d (%d bytes) |", fs.totalNumFids, sizeof(int));

    // set fidValueArray (float[])
    //printf("| fidValueArray  ");
    for(int j = 0; j < fs.numfidleads; j++){
      for(int k = 0; k < fs.fidDescArray[j]; k++){  
        //printf("writing Fid %d with value %f\n", k, fs.leadsArray[j][k]->value); 
        memcpy(cdataP, &(fs.leadsArray[j][k]->value), sizeof(float));
        cdataP += sizeof(float);
       
      }
    }
    //printf(" (%d bytes) |", fs.totalNumFids* sizeof(float));

    //printf("| fidTypeArray  ");
    // set fidTypeArray (short[])
    for(int j = 0; j < fs.numfidleads; j++){
      for(int k = 0; k < fs.fidDescArray[j]; k++){ 
        //printf("writing Fid of type " << fs.leadsArray[j][k]->type << "\n"; 
        memcpy(cdataP,  &(fs.leadsArray[j][k]->type), sizeof(short));
        cdataP += sizeof(short); 
        //printf(" %d", fs.leadsArray[j][k]->type);
      }
    }
    //printf(" (%d bytes) |", fs.totalNumFids* sizeof(short));
    //printf("\n\n*********** Done Writing Fidset *************\n\n");
  }

  delete [] allSizes;

  if((head+totalSize*sizeof(char)) != (cdataP )){
    printf("ERROR: [FSList::RawData] Something isn't adding up!!!\n");
  
  }
  
  datum rd = {head, totalSize};
  
  return rd;

}


/*====================================================================*/

// * Post: Outputs all FSList contents
//
// * Arguments: none
//
void FSList::Output(){
  printf("-----------------------------------------------------------------------\n");    
   
  printf("\nTime Series %s has %d parameter sets\n\n", const_cast<char*>(TSFileName.c_str()), numPS);

  for(int j = 0; j < numPS; j++){
    printf("-----------------------------------------------------------------------\n"); 

    // Get FidSet pointer
    FidSet *myFS = fidSetsArray[j];

    int numLeads = myFS->numfidleads;
    printf("\nFidset %d\n", j);  
    printf("Type: %d\n", myFS->type);
    printf("Version: %d\n", myFS->version);
    printf("Name: %s\n", const_cast<char*>(myFS->name.c_str()));                 // fidset name.
    printf("Audit String: %s\n", const_cast<char*>(myFS->auditString.c_str())); 		// fidset audit string. 
    printf("Time Series File Name: %s\n", const_cast<char*>(myFS->tsdfname.c_str()));
    printf("Number of leads: %d\n\n", myFS->numfidleads);            // number of leads that have fids.

    printf("Here is a listing of fiducials organized by lead number:\n\n");

    for(int k = 0; k < numLeads; k++){
        
      int numFids = myFS->fidDescArray[k];

      // Get array of Fid pointers
      Fid ** fidArray = myFS->leadsArray[k];
      printf("Lead number %d has %d fiducials\n", k, numFids);
        
      for(int m = 0; m < numFids; m++){
        // Get Fid pointer
        Fid * thisFid = fidArray[m];
        printf("Fid number: %d Type: %d Value: %f Name: %s\n", m, thisFid->type, thisFid->value, const_cast<char*>(thisFid->name.c_str())); 
      }
        
    } 

    printf("\nHere is a listing of fiducials organized by fiducial name:");

    for(int i = 0; i < myFS->numFidTypesUnique; i++){
      string fidName = myFS->fidNamesUniqueArray[i];
      printf("\n\nFid %s:\n", const_cast<char*>(fidName.c_str()));
      for(int j = 0; j < myFS->fidOccurrenceMap[fidName]; j++){
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
  printf("-----------------------------------------------------------------------\n");  
}

/*====================================================================*/

// * Post: Returns number of parameter sets in the list 
//
// * Arguments: none
//
int FSList::NumSets(){
  return numPS;
}

/*====================================================================*/

// * Post: Returns the name of the tsdf file associated with this FSList 
//
// * Arguments: none
//
string FSList::GetName(){
  return TSFileName;
}

/*====================================================================*/

// * Post: Sets the name of the tsdf file associated with this FSList 
// to the specified name 
//
// * Arguments:
// string tsdfName - new time series data file name to be assigned to
// this FSList
//
void FSList::SetName(string tsdfName){
  TSFileName = tsdfName;
}
  
/*====================================================================*/

// * Post: Sets the arrayIndex member variable to i.  
//
// * Arguments:
// int i - array index of this FSList in the entryArray in the Container 
// object, which is one level up in this data structure. 
//
void  FSList::SetArrayIndex(int i){
  arrayIndex = i;
}

/*====================================================================*/

// * Post: Returns the array index of this FSList in the entryArray in 
// the Container object, which is one level up in this data structure.
//
// * Arguments:
//
int FSList::GetArrayIndex(){
  return arrayIndex;
}

/*====================================================================*/

// * Post: Returns the type of this parameter set list, which is FSList
//
// * Arguments: none
//
string FSList::GetType(){
  return "FSList";
} 

/*====================================================================*/

// ***************************************************************************
// ************************* Editing Functions *******************************
// ***************************************************************************

/*====================================================================*/

// * Post: Adds a FidSet to this FSList.  This new FidSet must have a 
// different name from all the other FidSets in this FSList.
//
// Returns 0 if successful
// Returns -1 if a FidSet with this name already exists 
// 
// * Arguments:
// FidSet fs - FidSet to be added.  This could be a FidSet from another 
// FSList or even another container file.
//
int FSList::AddFidSet(FidSet fs){
 
  if(fidSetsMap.count(fs.name)){
    printf("ERROR: [FSList::AddFidSet] FidSet with this name already exists\n");
    return -1;  
  }

  FidSet * newFS = new FidSet;
  *newFS = fs;


  newFS->arrayIndex = numPS;
  // add to map
  fidSetsMap[fs.name] = newFS;

  // create new array
  FidSet ** newFidSetsArray = new FidSet *[numPS+1]; // array of fidsets

  // copy old array
  for(int i = 0; i < numPS; i++){
    newFidSetsArray[i] = new FidSet;
    *(newFidSetsArray[i]) = *(fidSetsArray[i]);

  }

  newFidSetsArray[numPS] = newFS;

  // clean up old memory
  if(fidSetsArray != NULL){
    //printf("Deleting fidSetsArray\n");
    for(int i = 0; i < numPS; i++){
      delete fidSetsArray[i];
    }
    delete [] fidSetsArray;
    fidSetsArray = NULL;
  }
  
  // set new values
  fidSetsArray = newFidSetsArray;

  numPS++;

  return 0;
}

/*====================================================================*/

// * Post: Deletes the FidSet with the given name from this FSList
// 
// Returns 0 if successful
// Returns -1 if the specified fiducial set doesn't exist  
//
// * Arguments:
// string name - name (unique identifier) of FidSet to be removed
//
int FSList::DeleteFidSet(string name){

  if(!fidSetsMap.count(name)){
    printf("ERROR: [FSList::DeleteFidSet] Specified fiducial set doesn't exist\n");
    return -1;
  }

  // deallocate memory allociated with this FidSet
  FidSet * temp = fidSetsMap[name];
  int deleteIndex = temp->arrayIndex;
  delete temp;


  fidSetsMap.erase(name); 

  for(int i = deleteIndex; i < numPS-1; i++){
    fidSetsArray[i] = fidSetsArray[i+1];
    fidSetsArray[i]->arrayIndex--;
  }

  numPS--;
 
  return 0;
}

/*====================================================================*/

// * Post: Change the name of one of the FidSets in this FSList from 
// oldName to newName.
//
// Returns 0 if successful
// Returns -1 if a FidSet with the specified name doesn't exist
// 
// Note: The reason that this function is in the FSList class instead 
// of the Fidset class is so that the fidSetsMap map (hash) can be 
// accurately updated in the FSList.  You can edit the name of the FidSet 
// directly, but its hash location won't be updated.
//
// * Arguments:
// string oldName - old name of FidSet to be changed
// string newName - new name to replace old name
//
int FSList::ChangeFSName(string oldName, string newName){

  if(!fidSetsMap.count(oldName)){
    printf("ERROR: [FSList::ChangeFSName] Specified fiducial set doesn't exist\n");
    return -1;
  }

  FidSet * f = fidSetsMap[oldName];
  f->name = newName;

  fidSetsMap.erase(oldName);
  fidSetsMap[newName] = f;

  return 0; 

}


/*====================================================================*/










