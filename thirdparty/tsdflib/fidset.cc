/*
 *
 * C++ METHODS (CC) FILE : fidset.cc
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
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include "gdbm.h"
#include "fi.h"
#include "cutil.h"
#include "fidset.h"
#include <string>
#include <map>

using std::string;

// ***************************************************************************
// ***************************************************************************
// *************************** Class FidSet **********************************
// ***************************************************************************
// ***************************************************************************


/*====================================================================*/

// * Post: Empty constructor.
//
// * Arguments: none
//
FidSet::FidSet(){
  //printf("In FidSet empty constructor\n");
  type = 0;
  version = 0;
  size = 0;
  name = "";
  auditString = "";
  tsdfname = "";
  fidDescArray = NULL;
  leadsArray = NULL;
  numfidleads = 0;
  descArraySize = 0;
  arrayIndex = -1;
  fidNamesUniqueArray = NULL;
  numFidTypesUnique = 0;
  numFidTypesInclusive = 0;              // number of fid types in the set, repeats included
  totalNumFids = 0;
  //fidNamesInclusiveArray = NULL;             // list of fid types by string in the set in the order they appear
                                // repeats included 
  
  //printf("End of FidSet empty constructor\n");
  
}

/*====================================================================*/

// * Post: Destructor.
// Checks to make sure that arrays are not NULL before deleting them.
//
// * Arguments: none
//
FidSet::~FidSet(){
  
  //printf("In FidSet destructor\n");
  
  // deallocate memory associated with leadsArray
  if(leadsArray != NULL){
    for(int i = 0; i < numfidleads; i++){
      if(leadsArray[i] != NULL){
        int numFids = fidDescArray[i];
        for(int j = 0; j < numFids; j++){
          delete leadsArray[i][j];
        }
        delete [] leadsArray[i];
      }
    }
    
    delete [] leadsArray;
  }

  // deallocate memory associated with fidDescArray
  if(fidDescArray != NULL){
    delete [] fidDescArray;
  }

  // deallocate memory associated with fidsMap
  DeleteFidsMap();

  //if(fidNamesInclusiveArray != NULL){
  //  delete [] fidNamesInclusiveArray;
  // }

  //printf("End of FidSet destructor\n");
  
}

/*====================================================================*/

// * Post: Copy Constructor
//
// * Arguments: none
//
FidSet::FidSet(const FidSet& f){
  //printf("In FidSet copy constructor\n");

  // only allocate new memory if there is info in the fidset
  if(f.numfidleads != 0){
     
    leadsArray = new FidPtr *[f.numfidleads];

    // fill in the leads map and the two dimensional leads array
    for(int i = 0; i < f.numfidleads; i++){

      int numFids = f.fidDescArray[i];
    
      leadsArray[i] = new FidPtr[numFids]; 

      for(int j = 0; j < numFids; j++){
        // copy values
        
        FidPtr oldFid = f.leadsArray[i][j];
        FidPtr thisFid = new Fid;
  
        *thisFid = *oldFid;

	// add FidPtr to array and hash
        leadsArray[i][j] = thisFid;

      }
    }

    numfidleads = f.numfidleads;            // number of leads that have fids.

    descArraySize = f.descArraySize;
    // copy fidDescArray
    if(descArraySize != 0){
      fidDescArray = new short[descArraySize];  
      for(int k = 0; k < descArraySize; k++){
        fidDescArray[k] = f.fidDescArray[k];
      }
    
    }

    //numFidTypesInclusive = f.numFidTypesInclusive;
    // copy fidNamesInclusiveArray
    //if(numFidTypesInclusive > 0){
    //  fidNamesInclusiveArray = new string[numFidTypesInclusive];
    //  for(int k = 0; k < numFidTypesInclusive; k++){
    //    fidNamesInclusiveArray[k] = f.fidNamesInclusiveArray[k];
    //  }

    //}
     
    fidOccurrenceMap = f.fidOccurrenceMap;
    InitializeFidsMap();
    
	
  }
  else{
    // empty fidset - no new memory 
    fidDescArray = NULL;
    leadsArray = NULL;
    fidNamesUniqueArray = NULL;
    //fidNamesInclusiveArray = NULL;
  }

  // copy all other values
  type = f.type;                 // fidset's type - should be 1.
  version = f.version;              // fidset's version - should be 0.
  size = f.size;
  name = f.name;                 // fidset name. 
  auditString = f.auditString; 		// fidset audit string. 
  tsdfname = f.tsdfname;             // name of associated tsdf file.  
  numFidTypesInclusive = f.numFidTypesInclusive;
  arrayIndex = f.arrayIndex;
  numfidleads = f.numfidleads;            // number of leads that have fids.
  descArraySize = f.descArraySize;
  numFidTypesUnique = f.numFidTypesUnique; 
  totalNumFids = f.totalNumFids;
  //printf("End of FidSet copy constructor\n");
}

/*====================================================================*/

// * Post: Overloaded assignment operator
// Checks to see that arrays aren't NULL before 
// deleting them.  Checks to see that the two objects are not 
// equal
//
// * Arguments:
// const FidSet& f = FidSet to make copy of 
//
const FidSet& FidSet::operator = (const FidSet& f){
  //printf("In FidSet assignment operator\n");
  // Delete any allocated memory in the current data structure
  if(this != &f){ 
    
    // deallocate memory associated with leadsArray
    if(leadsArray != NULL){
      for(int i = 0; i < numfidleads; i++){
        if(leadsArray[i] != NULL){
          int numFids = fidDescArray[i];
          for(int j = 0; j < numFids; j++){
            delete leadsArray[i][j];
          }
          delete [] leadsArray[i];
        }
      }
    
      delete [] leadsArray;
    }

    // deallocate memory associated with fidDescArray
    if(fidDescArray != NULL){
      delete [] fidDescArray;
    }

    // deallocate memory associated with fidsMap and fidNamesUniqueArray
    DeleteFidsMap();
  
//if(fidNamesInclusiveArray != NULL){
//     delete [] fidNamesInclusiveArray;
//  }


    // only allocate new memory if there is info in the fidset
    if(f.numfidleads != 0){
      // initialize arrays and copy contents

      leadsArray = new FidPtr *[f.numfidleads];

      for(int i = 0; i < f.numfidleads; i++){

        int numFids = f.fidDescArray[i];

        leadsArray[i] = new FidPtr [numFids];
        
        for(int j = 0; j < numFids; j++){
          // copy values

          FidPtr oldFid = f.leadsArray[i][j];
          FidPtr thisFid = new Fid;
          *thisFid = *oldFid;

	  // add FidPtr to array and hash
          leadsArray[i][j] = thisFid;
        }
      }

      numfidleads = f.numfidleads;            // number of leads that have fids.

      descArraySize = f.descArraySize;

      if(descArraySize != 0){
        fidDescArray = new short[descArraySize];
   
        for(int k = 0; k < descArraySize; k++){
   
          fidDescArray[k] = f.fidDescArray[k];
        }
      }

//    numFidTypesInclusive = f.numFidTypesInclusive;
      // copy fidNamesInclusiveArray
//    if(numFidTypesInclusive != 0){
//fidNamesInclusiveArray = new string[numFidTypesInclusive];
//for(int k = 0; k < numFidTypesInclusive; k++){
//	  fidNamesInclusiveArray[k] = f.fidNamesInclusiveArray[k];
//}

      //}

      // copy fidOccurrenceMap
      fidOccurrenceMap = f.fidOccurrenceMap;
      InitializeFidsMap();

    }
    else{
      // empty fidset
      fidDescArray = NULL;
      leadsArray = NULL;
      fidNamesUniqueArray = NULL; 
      //fidNamesInclusiveArray = NULL;
    }

    // copy all other values    
    type = f.type;                 // fidset's type - should be 1.
    version = f.version;              // fidset's version - should be 0.
    size = f.size;
    name = f.name;                 // fidset name. 
    auditString = f.auditString; 		// fidset audit string. 
    tsdfname = f.tsdfname;             // name of associated tsdf file.
    numfidleads = f.numfidleads;            // number of leads that have fids.
    arrayIndex = f.arrayIndex; 
    numFidTypesInclusive = f.numFidTypesInclusive;
    numfidleads = f.numfidleads;            // number of leads that have fids.
    descArraySize = f.descArraySize;
    numFidTypesUnique = f.numFidTypesUnique;
    totalNumFids = f.totalNumFids;
  }
  //printf("End of FidSet assignment operator\n");
  return *this;
 
}
     
/*====================================================================*/

// * Post: Copies info from the GIOFidSet structure from Rob's original 
// tsdflib into the FidSet object
// This is designed to be *used only once* for each FidSet object in order
// to initialize it.
// 
// Returns 0 if successful, -1 otherwise
//
// * Arguments:  
// GIOFidSet *giofs - Pointer to GIOFidSet whose contents are copied
// into this FidSet
//
// Note: Debugging statements have been left in code in case of future 
// use (knock on wood...)
//
int FidSet::Read(GIOFidSet *giofs){
  //printf("****** In FidSet::Read ***************\n");
  if(giofs == NULL){
    printf("ERROR: [FidSet::Read] Cannot read NULL pointer\n");
    return -1;  
  }

  // fill in basic information

  type = giofs->Type();                 // fidset's type - should be 1.

  //printf("| type = %d (%d bytes) |", type, sizeof(short));

  version = giofs->Version();              // fidset's version - should be 0.

  //printf("| version = %d (%d bytes) |", version, sizeof(short));

  size = giofs->Size();

  //printf("| size = %d (%d bytes) |", size, sizeof(int));
  
  name = giofs->Name();

  //printf("| fidNameSize = %d (%d bytes) |", strlen(giofs->Name()), sizeof(int));
  //printf("| name = %s (%d bytes) |", const_cast<char*>(name.c_str()), strlen(giofs->Name()) * sizeof(char));

  auditString = giofs->AuditString();
  
  //printf("| auditSize = %d (%d bytes) |", strlen(giofs->AuditString()), sizeof(int));
  //printf("| auditString = %s (%d bytes) |", const_cast<char*>(auditString.c_str()), strlen(giofs->AuditString()) * sizeof(char));

  tsdfname = giofs->TsdfName();

  totalNumFids = giofs->ArraySize();
 
  numfidleads = descArraySize = giofs->DescArraySize();	       // Return size of fidset's 
  
  //printf("| descArraySize = %d (%d bytes) |", descArraySize, sizeof(int));
 
  fidDescArray = new short[descArraySize];	// the fid descrip array. 
  
  const short *tempFDA = giofs->DescArray();
 
  for(int i = 0; i < descArraySize; i++){                         
    fidDescArray[i] = tempFDA[i];
  }

  //printf("| fidDescArray ");
  for(int k = 0; k < descArraySize; k++){
    //printf(" %d", fidDescArray[k]);
  }
  //printf(" (%d bytes) |", descArraySize * sizeof(short));

//printf("Before inclusive\n");
  numFidTypesInclusive = giofs->ArraySize();
// printf("numFidTypesInclusive = %d", numFidTypesInclusive);
//string * fidnames = giofs->FidNames();

//printf("After fidnames\n");
  // copy fidNamesInclusiveArray
//if(numFidTypesInclusive > 0){
//  fidNamesInclusiveArray = new string[numFidTypesInclusive];
//  printf("Before loop\n");
//  for(int k = 0; k < numFidTypesInclusive; k++){
//    printf("fidnames[k] = %s\n", const_cast<char *>((fidnames[k]).c_str()));
//    fidNamesInclusiveArray[k] = fidnames[k];
//  }

//}

// printf("After inclusive\n");
  //printf("| numFidTypesInclusive = %d (%d bytes) |", numFidTypesInclusive, sizeof(int));

  numfidleads = giofs->NumFidLeads();
 
  // initialize leadsArray
  leadsArray = new FidPtr *[numfidleads];

  //leadsMap = new FidMap[numfidleads];
 
  // initialize temporary pointers to value and type arrays
  const float *fidValueArray = giofs->ValueArray();
  const short  *fidTypeArray = giofs->TypeArray(); 

  //printf("| fidValueArray  ");
  for(int k = 0; k < numFidTypesInclusive; k++){
    //printf(" %f", fidValueArray[k]);
  }
  //printf(" (%d bytes) |", numFidTypesInclusive * sizeof(float));

  //printf("| fidTypeArray  ");
  for(int k = 0; k < numFidTypesInclusive; k++){
    //printf(" %d", fidTypeArray[k]);
  }
  //printf(" (%d bytes) |\n\n", numFidTypesInclusive * sizeof(short));

  //for each entry in descriptor array, fill in corresponding  
  //   leads hash table with corresponding fiducials 

  int index = 0;
  for(int i = 0; i < numfidleads; i++){
    //printf("Getting lead %d\n", i);
    int numFids = fidDescArray[i];
    
    FidOccurrenceMap tempFOM; 

    // initialize leads array
    leadsArray[i] = new FidPtr[numFids];

    for(int j = 0; j < numFids; j++){
    
      // Create FidPtr object
      FidPtr thisFid = new Fid;
      thisFid->type = fidTypeArray[index + j];  // integer type of fiducial
      //printf("Getting Fid %d fidTypeArray[%d] = %d\n", j, index + j, thisFid->type);

      thisFid->value = fidValueArray[index + j]; // value of fiducial 
      //printf("Getting Fid %d fidValueArray[%d] = %f\n", j, index + j, thisFid->value);

      // get name of fid

      thisFid->name = GetFidName(thisFid->type);
      if(thisFid->name == ""){
        return -1;
      }

      // if the fid isn't already in the map
      if(!tempFOM.count(thisFid->name)){
        //add it with an occurrence of 1
	tempFOM[thisFid->name] = 1;
      }
      else{
        tempFOM[thisFid->name] += 1;
      }  

      //thisFid->arrayIndex = j;

      // insert fiducial into hash (map)
      //(leadsMap[i])[thisFid->name] = thisFid;

      // insert fiducial into leads array
      leadsArray[i][j] = thisFid;
      
    }
   
    // update fidOccurrenceMap

    // for each fid in tempFOM
    // need to replace this code with iterator code
    for(int b = 0; b < numFids; b++){
      // Create FidPtr object
      FidPtr thisFid = new Fid;
      thisFid->type = fidTypeArray[index + b];  // integer type of fiducial
            
      // get name of fid
      string fidName = GetFidName(thisFid->type);
      if(fidName == ""){
        return -1;
      }

      // if fidOccurrenceMap doesn't have an entry or if this value is greater, replace 
      if(!fidOccurrenceMap.count(fidName) || (tempFOM[fidName] > fidOccurrenceMap[fidName])){
        fidOccurrenceMap[fidName] = tempFOM[fidName];
      }
    }

    index += numFids;
  }
 
  InitializeFidsMap();

  //printf("****** End of FidSet::Read ***************\n");
  return 0;
}

/*====================================================================*/

// * Post: Initializes the fiducial map
//
// * Arguments: 
// none
//
void FidSet::InitializeFidsMap(){
  //printf("In InitializeFidsMap\n");
  // this number will need to be increased if the number
  // of fid types in the fi library exceeds 50
  
  fidNamesUniqueArray = new string[50];
  
  numFidTypesUnique = 0;

  // for each lead
  for(int i = 0; i < numfidleads; i++){
    // for each fid
    int numFids = fidDescArray[i];
    for(int j = 0; j < numFids; j++){
      
      // if fidsMap[fid] not initialized, allocate memory, set values to NULL
      Fid * thisFid = leadsArray[i][j];
      string fidName = thisFid->name;
      if(!fidsMap.count(fidName)){
        int maxNumOccurrences = fidOccurrenceMap[fidName]; // temporarily hard coded
        fidsMap[fidName] = new Fid **[maxNumOccurrences];
        for(int k = 0; k < maxNumOccurrences; k++){
          fidsMap[fidName][k] = new Fid *[numfidleads];
          for(int m = 0; m < numfidleads; m++){
            fidsMap[fidName][k][m] = NULL;
          }
        }

        fidNamesUniqueArray[numFidTypesUnique] = fidName;
        numFidTypesUnique++;
      }
     
      // find the first occurrence of this fid for which the lead is NULL
      // drop this fid in that slot
      int occurrence = 0;
      while(fidsMap[fidName][occurrence][i] != NULL){
        occurrence++;
      }

      fidsMap[fidName][occurrence][i] = thisFid;       
    }
  } 
  //printf("End of InitializeFidsMap\n");

}

/*====================================================================*/

// * Post: Deletes the fiducial map
//
// * Arguments:
// none
//
void FidSet::DeleteFidsMap(){
  //printf("In DeleteFidsMap\n");

  // deallocate memory associated with fidsMap
  for(int i = 0; i < numFidTypesUnique; i++){
    string fidName = fidNamesUniqueArray[i];

    if(fidOccurrenceMap.count(fidName) && fidsMap.count(fidName)){
      int numOccurrences = fidOccurrenceMap[fidName];
      for(int j = 0; j < numOccurrences; j++){
        delete [] fidsMap[fidName][j];
      }
      delete [] fidsMap[fidName];
    }
    else{
      printf("ERROR: [FidSet::~FidSet] FidOccurrenceMap not initialized properly\n");
    }
  }  

  // clear the fidsMap
  fidsMap.clear();

  numFidTypesUnique = 0;
  if(fidNamesUniqueArray != NULL){
    delete [] fidNamesUniqueArray;
  }



  //printf("End of DeleteFidsMap\n");

}

/*====================================================================*/

// * Post: Outputs the contents of this FidSet
//
// * Arguments: 
// none
//
void FidSet::Output(){
  printf("-----------------------------------------------------------------------\n");  
 
  // get the number of leads for a given fiducial set
  int numLeads = numfidleads;
   
  printf("\nType: %d\n", type);
  printf("Version: %d\n", version);
  printf("Size: %d\n", size);
  printf("Name: %s\n", const_cast<char*>(name.c_str()));                 // fidset name. 
  printf("Audit String: %s\n", const_cast<char*>(auditString.c_str())); 		// fidset audit string. 
  printf("Time Series File Name: %s\n", const_cast<char*>(tsdfname.c_str()));
  printf("Number of leads: %d\n\n", numfidleads);            // number of leads that have fids.

  printf("Here is a listing of fiducials organized by lead number:\n\n");

  for(int k = 0; k < numLeads; k++){
        
    int numFids = fidDescArray[k];

    // get array of Fid pointers for this lead
    Fid ** fidArray = leadsArray[k];
    printf("Lead number %d has %d fiducial(s)\n", k, numFids);
             
    for(int m = 0; m < numFids; m++){
      // get Fid pointer
      Fid * thisFid = fidArray[m];
      printf("Fid number: %d Type: %d Value: %f Name: %s\n", m, thisFid->type, thisFid->value, const_cast<char*>(thisFid->name.c_str())); 
    }
         
  } 

  printf("\nHere is a listing of fiducials organized by fiducial name:");

  for(int i = 0; i < numFidTypesUnique; i++){
    string fidName = fidNamesUniqueArray[i];
    printf("\n\nFid %s:\n", const_cast<char*>(fidName.c_str()));
    for(int j = 0; j < fidOccurrenceMap[fidName]; j++){
      printf("Occurrence %d:\n", j);
      for(int k = 0; k < numfidleads; k++){
        if(fidsMap[fidName][j][k] != NULL){
          printf("Lead %d: %f\n", k, (fidsMap[fidName][j][k])->value);
        }
        else{
          printf("Lead %d: %s\n", k, "NULL");
        }
      }
      printf("\n");
    }
   
  }
  

  printf("-----------------------------------------------------------------------\n");  
}

/*====================================================================*/

// * Post: Given a numerical fiducial type, retrieve its corresponding 
// string fiducial name.
//
// Returns "FI_UNKNOWN" if type is not recognized and outputs error message
//
// * Arguments: 
// int type - numerical type of the fiducial. These are predefined
// integers.  The list of assigned fiducial numbers and names can be
// found in the library fi.h
//
string FidSet::GetFidName(int type){

  FiducialInfo fidinfo;
  FI_Init(0);
  fidinfo.type = type;
  if(FI_GetInfoByType( &fidinfo )){
   
    char * tempName = new char [strlen(fidinfo.name)+1];
    strcpy(tempName, fidinfo.name );
    tempName[strlen(fidinfo.name)] = '\0';
    string name = tempName;
    delete [] tempName;
    return name;
  }
  else{
    printf("WARNING: [FidSet::GetFidName] - Unknown fiducial type\n");
    return "FI_UNKNOWN";
  }
  //printf("End of FidSet::GetFidName\n");
}

/*====================================================================*/

// * Post: Given a string fiducial name, retrieve its corresponding 
// numerical fiducial type.
//
// Returns -1 if name is not recognized as valid fiducial and outputs
// error message.  
//
// * Arguments: 
// string name - string name of the fiducial. These are predefined
// strings.  The list of assigned fiducial numbers and names can be
// found in the library fi.h
// 
int FidSet::GetFidType(string name){

  FiducialInfo fidinfo;
  FI_Init(0);
  fidinfo.name = const_cast<char*>(name.c_str());
  if(FI_GetInfoByName( &fidinfo )){
    return fidinfo.type;
  }
  else{
    printf("WARNING: [FidSet::GetFidType] - Unknown fiducial name\n");
    return -1;
  }
}

/*====================================================================*/

// ***************************************************************************
// ************************* Editing Functions *******************************
// ***************************************************************************

/*====================================================================*/

// * Post: Appends lead with corresponding fiducials to the FidSet.
// The new lead is assigned to be the last lead in the set.  It has the 
// highest lead number.  All Fids in the list must be existing Fids in 
// the FidSet.  If you want to add a new fiducial type, do so using the
// AddFidValueArray function.
//
// Returns the lead index if successfull, -1 otherwise
//
// * Arguments:
// int numFids - number of fiducials that are to be associated with this
// lead.  This is the length of the arrayOfFids.  This should correspond
// to the number of fiducials that the other leads have.
// Fid * arrayOfFids - array of Fids for this new lead
//
int FidSet::AddLead(int numFids, Fid * arrayOfFids){
  // add to leadsArray
  
  // expand leadsArray and fidDescArray by one
  numfidleads++;
  descArraySize++;

  FidPtr ** oldLeadsArray = leadsArray;
  short * oldFidDescArray = fidDescArray;

  leadsArray = new FidPtr *[numfidleads];
  fidDescArray = new short[descArraySize];

  for(int i = 0; i < numfidleads-1; i++){
    leadsArray[i] = oldLeadsArray[i];
    fidDescArray[i] = oldFidDescArray[i];

  }
  if(numFids > 0){ 
    leadsArray[numfidleads-1] = new FidPtr[numFids];
  }
  else{
    leadsArray[numfidleads-1] = NULL;
  }
  fidDescArray[descArraySize-1] = numFids;

  delete [] oldLeadsArray;
  delete [] oldFidDescArray;

  // copy Fids
  for(int k = 0; k < numFids; k++){
    FidPtr temp = new Fid;
    *temp = arrayOfFids[k];
    leadsArray[numfidleads-1][k] = temp;    
  }

  totalNumFids += numFids; 
  // delete old fidsMap, initialize new one based on new leadsArray
  DeleteFidsMap();
  InitializeFidsMap();

  return 0;
}

/*====================================================================*/

// * Post: Deletes a lead and all of its fiducials from the FidSet.
// 
// Returns 0 if successful
// Returns -1 if the lead number is not valid
//
// * Arguments:
// int leadNum - index of lead to delete.  Must be in the range of
// 0 <= leadNum < numfidleads.  Remember that the first lead would be
// lead 0, not lead 1.
//
int FidSet::DeleteLead(int leadNum){

  // delete lead array from leadsArray 

  if(leadNum < 0 || leadNum >= numfidleads){
    printf("ERROR: [FidSet::DeleteLead] Specified lead does not exist.\n");
    return -1;
  }

  int numFids = fidDescArray[leadNum];

  // deallocate memory associated with this lead
  if(leadsArray[leadNum] != NULL){
    
    for(int j = 0; j < numFids; j++){
      delete leadsArray[leadNum][j];
    }
    delete [] leadsArray[leadNum];
  }
 
  // move over entries for leadsArray
  for(int i = leadNum; i < numfidleads-1; i++){
    leadsArray[i] = leadsArray[i+1];
   
    fidDescArray[i] = fidDescArray[i+1];
  } 
  
  // decrement pertinent values
  numfidleads--;            // number of leads that have fids.
  totalNumFids -= numFids;
  descArraySize--; // size of fiducial descriptor array

  // delete old fidsMap, initialize new one based on new leadsArray
  DeleteFidsMap();
  InitializeFidsMap();

  UpdateSize();

  return 0;

}


/*====================================================================*/

// * Post:  Add a new occurrence of a fid with associated leads values
//
// Returns 0 if successfull, -1 otherwise
//
// Returns -1 if values array is NULL or if numValues is not equal to
// numfidleads
//
// * Arguments: 
// string fidName - name of fiducial 
// float *values - array of fiducial values where the array indices 
// correspond directly to the lead numbers the values belong to 
// int numValues - number of values, i.e. number of leads.  This *must*
// match the number of leads in the rest of the fidset
//
int FidSet::AddFidValueArray(string fidName, float *values, int numValues){

  //printf("In FidSet::AddFidValueArray\n");
  if(numValues != numfidleads){
    printf("ERROR: [FidSet::AddFidValueArray] Trying to add fiducial with wrong number of leads\n");
    return -1;
  }

   // for each value (for each lead)
  for(int i = 0; i < numValues; i++){
    if(values[i] != NULL){
      // make a Fid object
      Fid *f = new Fid;
      f->name = fidName;
      f->type = GetFidType(fidName);
      f->value = values[i];
    
      // add Fid to appropriate leads array
      FidPtr * oldArray = leadsArray[i];

      // increment the number of fiducials for this lead
      fidDescArray[i] += 1;

      int numFids = fidDescArray[i];
      leadsArray[i] = new FidPtr[numFids];

      for(int j = 0; j < numFids-1; j++){
        leadsArray[i][j] = oldArray[j];
      } 

      leadsArray[i][numFids-1] = f; 
      delete [] oldArray;

      totalNumFids++;
    }  
  }    
    
  // expand fidNamesInclusiveArray by 1, add new fiducial name
  numFidTypesInclusive++;
//string * oldInclusiveArray = fidNamesInclusiveArray;
// fidNamesInclusiveArray = new string[numFidTypesInclusive];
 
//for(int k = 0; k < numFidTypesInclusive-1; k++){
//   fidNamesInclusiveArray[k] = oldInclusiveArray[k];
// }

//fidNamesInclusiveArray[numFidTypesInclusive-1] = fidName;
// delete [] oldInclusiveArray;

  DeleteFidsMap();

  // increment the fidOccurenceMap since we now have an additional occurrence of this fiducial type
  if(fidOccurrenceMap.count(fidName)){
    fidOccurrenceMap[fidName] += 1;
  }
  else{
    fidOccurrenceMap[fidName] = 1;
  }


  InitializeFidsMap();

  UpdateSize();

  //printf("End of FidSet::AddFidValueArray\n");
  return 0;
}

/*====================================================================*/

// * Post:  Delete an occurrence of a fid along with its associated leads values
//
// Returns 0 if successfull, -1 otherwise
//
// * Arguments: 
// string fidName - name of fiducial
// int occurrence - occurrence number (which occurrence of the fiducial
// do you want to delete)
//
int FidSet::DeleteFidValueArray(string fidName, int occurrence){

  // decrement the fidOccurenceMap since we now have one less occurrence of this fiducial type
  if(!fidOccurrenceMap.count(fidName) || fidOccurrenceMap[fidName] <= 0 || occurrence >= fidOccurrenceMap[fidName]){
    printf("ERROR: [FidSet::DeleteFidValueArray] This fiducial type or this occurrence was not found\n");
    return -1;
  }

  // for each lead
  for(int i = 0; i < numfidleads; i++){
   
    // find the appropriate Fid *
    int k = 0;
    int occ = -1;

    int numFids = fidDescArray[i];
    while(occ < occurrence && k < numFids){
      if(leadsArray[i][k]->name == fidName){
        occ++;
      }
      k++;
    }
    
    // if there was a fid of this type for this lead
    if(occ == occurrence){
      // k is the index of the Fid to delete
      k--;
      totalNumFids--;     

      delete leadsArray[i][k];
      // delete the Fid and move over all the values in the array
      for(int m = k; m < numFids-1; m++){
        leadsArray[i][m] = leadsArray[i][m+1];
      }

      // decrement the fidDescArray
      fidDescArray[i] -= 1;
    }
  }

 
  // remove from the fidNamesInclusiveArray
  numFidTypesInclusive--;              // number of fid types in the set, repeats included
 
  
  // find the appropriate string and move all values over
  // find the appropriate Fid name
//int k = 0;
// int occ = -1;

//int numFids = numFidTypesInclusive+1;
//  while(occ < occurrence && k < numFids){
//  if(fidNamesInclusiveArray[k] == fidName){
//     occ++;
//   }
//   k++;
// }

  // if the appropriate fid was found
//if(occ == occurrence){
    // k is the index of the Fid to delete
//  k--;

    // move over all the values in the array
//  for(int m = k; m < numFids-1; m++){
//     fidNamesInclusiveArray[m] = fidNamesInclusiveArray[m+1];
//   }
// }
// else{
//   printf("ERROR: [FidSet::DeleteFidValueArray] - Fiducial not found in list\n");
//   return -1;
// }

  DeleteFidsMap();

  // decrement the fidOccurenceMap since we now have one less occurrence of this fiducial type
  fidOccurrenceMap[fidName] -= 1;

  InitializeFidsMap();
 
  UpdateSize();

  return 0;
}

/*====================================================================*/

// * Post: Updates and returns the size (in bytes) of this FidSet.
//
// * Arguments: none
//
int FidSet::UpdateSize(){
  int overhead = sizeof(short) + sizeof(short) + sizeof(int);

  int nameStringSize = name.length();
  int auditStringSize = auditString.length();


  int fidSize =  sizeof(int) + sizeof(char)*nameStringSize +
    sizeof(int) + sizeof(char)*auditStringSize +
    sizeof(int) + sizeof(short)*descArraySize +
    sizeof(int) + (sizeof(float)+sizeof(short))*totalNumFids;

  size = overhead + fidSize;
  return size;
}

/*====================================================================*/










