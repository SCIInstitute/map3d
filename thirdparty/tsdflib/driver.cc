/*
 * 
 * C++ DRIVER FILE : driver.cc
 * VERSION : 1.0
 * DESCRIPTION : This is the driver program for the tsdflib
 * AUTHOR(S)  : Rob Macleod and Jenny Simpson (simpson@cs.utah.edu) 
 * CREATED : 06/07/02
 * MODIFIED: 07/23/02
 * DOCUMENTATION: 
 * http://dante.sci.utah.edu/develop/simpson/scratch/documentation/tsdflib_doc.html
 *
 */ 
  
#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "container.h"
 
using namespace std;

/*======================================================================*/

// * Post:  Prints all of Container contents to standard out
void viewContainer(Container thisContainer){
  cout << "***********************************************************************\n";      
  int numEntries = thisContainer.numEntries;
  // list how many tsdf files are referenced
  cout << "There are " << numEntries << " time entries in this container file\n";

  for(int i = 0; i < numEntries; i++){
    
    string entryName = thisContainer.entryArray[i]->entryName;
    if(thisContainer.entryArray[i]->paramList != NULL){

      // Get FSList pointer    
      FSList *fsl = (FSList *)thisContainer.entryArray[i]->paramList;
   
      int numSets = fsl->NumSets();

      cout << "\n\nEntry " << i << "  is called " << entryName << " and has " << numSets << " parameter set(s)\n";

      for(int j = 0; j < numSets; j++){
        FidSet *myFS = fsl->fidSetsArray[j];

        int numLeads = myFS->numfidleads;
        cout << "\nFidset " << j << "\n";  
        cout << "Type: " << myFS->type << "\n";
        cout << "Version: " << myFS->version << "\n";
        cout << "Size: " << myFS->size << "\n";
        cout << "Name: " << myFS->name << "\n";                 // fidset name. 
        cout << "Audit String: " << myFS->auditString << "\n"; 		// fidset audit string. 
        cout << "Time Series File Name: " << myFS->tsdfname << "\n";
        cout << "Number of leads: " << myFS->numfidleads << "\n\n";            // number of leads that have fids.

        for(int k = 0; k < numLeads; k++){
        
          int numFids = myFS->fidDescArray[k];
          Fid ** fidArray = myFS->leadsArray[k];
          cout << "Lead number " << k << " has " << numFids << " fiducial(s)\n";
        
          for(int m = 0; m < numFids; m++){
          Fid * thisFid = fidArray[m];
            cout << "Fid number: " << m << " Type: " << thisFid->type << " Value: " << thisFid->value << " Name: " << thisFid->name << "\n"; 
          }
        
        } 
      }
    }
    else{
      cout << "Time Series " << i << " is called " << entryName << " and has 0 parameter sets\n";
    }
  }
  cout << "***********************************************************************\n";  
  
}

/*======================================================================*/

// * Post:  Changes a fiducial name
int addFidValueArray(){
  string tsdfcFileName1, tsdfcFileName2;
  string entryName, fidName, fsName;
  //int leadNum;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the tsdfc file to edit: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new tsdfc file to save edits to: ";
  cin >> tsdfcFileName2;
  cout << "Enter the time series name: ";
  cin >> entryName;
  cout << "Enter the fidset name: ";
  cin >> fsName;
  cout << "Enter new fiducial name: ";
  cin >> newFidName;

  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName1);

  // Error checking
  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  FSList *myFSL;
  if(thisContainer.entryMap.count(entryName)){
    // Get FSList pointer
    myFSL = (FSList *)thisContainer.entryMap[entryName]->paramList;
  }
  else{
    cout << "ERROR: Time series file not found\n";
    return -1;
  }

  if(!(myFSL->fidSetsMap.count(fsName))){
    cout << "ERROR: Invalid fiducial set\n";
    return -1;
  }

  // Get FidSet pointer
  FidSet *myFS = myFSL->fidSetsMap[fsName];

  //if(leadNum < 0 || leadNum >= myFS->numfidleads){
  // cout << "ERROR: Invalid lead number\n";
  // return -1;
  //}

  int numfidleads = myFS->numfidleads;

  float * values = new float[numfidleads];

  for(int i = 0; i < numfidleads; i++){
    values[i] = 888.0;
  }

  // change fid name
  if(myFS->AddFidValueArray(newFidName, values, numfidleads) == -1){
    cout <<"ERROR: There was an error when adding the fid type\n";
    return -1;
  }

  int ret = thisContainer.Write(tsdfcFileName2);

  thisContainer.Output();

  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  }

  cout << "***********************************************************************\n";

  return 0;

}

/*======================================================================*/

// * Post:  Deletes a fiducial from a set

int deleteFidValueArray(){
  string tsdfcFileName1, tsdfcFileName2;
  string entryName, fidName, fsName;
  int occNumber;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the tsdfc file to edit: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new tsdfc file to save edits to: ";
  cin >> tsdfcFileName2; 
  cout << "Enter the time series name: ";
  cin >> entryName;
  cout << "Enter the fidset name: ";
  cin >> fsName;
  cout << "Enter the fid name: ";
  cin >> fidName;
   cout << "Enter the occurrence number (enter 0 if unsure): ";
  cin >> occNumber;    

  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName1);

  // Error checking
  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  FSList *myFSL;
  if(thisContainer.entryMap.count(entryName)){
    // Get FSList pointer
    myFSL = (FSList *)thisContainer.entryMap[entryName]->paramList;
  }
  else{
    cout << "ERROR: Time series file not found\n";
    return -1;
  }

  if(!(myFSL->fidSetsMap.count(fsName))){
    cout << "ERROR: Invalid fiducial set\n";
    return -1;
  }

  // Get FidSet pointer
  FidSet *myFS = myFSL->fidSetsMap[fsName];
  
  // Delete Fid
  if(myFS->DeleteFidValueArray(fidName, occNumber) == -1){
    cout << "ERROR: There was an error when deleting the fiducial\n";
    return -1;
  }
  
  thisContainer.Output();
 
  int ret = thisContainer.Write(tsdfcFileName2);
 
  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 
	
  cout << "***********************************************************************\n";  
  
  return 0;  
  
}

/*======================================================================*/

// * Post: Adds a lead to a FidSet  

int addLead(){
  string tsdfcFileName1, tsdfcFileName2;
  string entryName, fidName, fsName;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the tsdfc file to edit: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new tsdfc file to save edits to: ";
  cin >> tsdfcFileName2; 
  cout << "Enter the time series name: ";
  cin >> entryName;
  cout << "Enter the fidset name: ";
  cin >> fsName;

  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName1);

   // Error checking
  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  FSList *myFSL;
  if(thisContainer.entryMap.count(entryName)){
    // Get FSList pointer
    myFSL = (FSList *)thisContainer.entryMap[entryName]->paramList;
  }
  else{
    cout << "ERROR: Time series file not found\n";
    return -1;
  }

  if(!(myFSL->fidSetsMap.count(fsName))){
    cout << "ERROR: Invalid fiducial set\n";
    return -1;
  }

  // Get FidSet pointer
  FidSet *myFS = myFSL->fidSetsMap[fsName];
 
  //int numFids = myFS->numFidTypesInclusive;

  // Add lead
  Fid * temp = NULL;

  myFS->AddLead(0, temp);

  thisContainer.Output();

  int ret = thisContainer.Write(tsdfcFileName2);

  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 

 
  cout << "***********************************************************************\n";  
  
  return 0;
}

/*======================================================================*/

// * Post:  Deletes a lead from a FidSet

int deleteLead(){
  string tsdfcFileName1, tsdfcFileName2;
  string entryName, fidName, fsName;
  int leadNum;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the tsdfc file to edit: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new tsdfc file to save edits to: ";
  cin >> tsdfcFileName2; 
  cout << "Enter the time series name: ";
  cin >> entryName;
  cout << "Enter the fidset name: ";
  cin >> fsName;
  cout << "Enter the lead number to be deleted: ";
  cin >> leadNum;  

  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName1);
  
  // Error checking
  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  FSList *myFSL;
  if(thisContainer.entryMap.count(entryName)){
    // Get FSList pointer
    myFSL = (FSList *)thisContainer.entryMap[entryName]->paramList;
  }
  else{
    cout << "ERROR: Time series file not found\n";
    return -1;
  }

  if(!(myFSL->fidSetsMap.count(fsName))){
    cout << "ERROR: Invalid fiducial set\n";
    return -1;
  }

  // Get FidSet pointer
  FidSet *myFS = myFSL->fidSetsMap[fsName];

  if(myFS->DeleteLead(leadNum) == -1){
    cout << "ERROR: Delete lead failed\n";
    return -1;
  }

  thisContainer.Output();

  int ret = thisContainer.Write(tsdfcFileName2);
 
  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 
	
  cout << "***********************************************************************\n";  
  
  return 0;

}


/*======================================================================*/

// * Post:  Adds a FidSet to an FSList
int addFidSet(){
  string tsdfcFileName1, tsdfcFileName2, tsdfcFileName3;
  string entryName, fidName, fsName;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the tsdfc file to copy FidSet from: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the existing tsdfc file to add FidSet to: ";
  cin >> tsdfcFileName2;
  cout << "Enter the name of the new tsdfc file to save FidSet to: ";
  cin >> tsdfcFileName3; 
  cout << "Enter the time series name: ";
  cin >> entryName;
  cout << "Enter the name of the fidset from file 1 to add to file 2: ";
  cin >> fsName;

  Container container1, container2;

  int retVal = container1.Read(tsdfcFileName1);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }
  if(!(container1.entryMap.count(entryName))){
    cout << "ERROR: Time Series file not found\n";
    return -1;
  }
  if(container1.entryMap[entryName]->paramList == NULL){
    cout << "ERROR: This time series has no parameter set list\n";
    return -1;
  }  

  retVal = container2.Read(tsdfcFileName2);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  if(!(container2.entryMap.count(entryName))){
    cout << "ERROR: Time Series file not found\n";
    return -1;
  }

  if(container2.entryMap[entryName]->paramList == NULL){
    cout << "ERROR: This time series has no parameter set list\n";
    return -1;
  }

  FidSet thisFS = *(((FSList *)container1.entryMap[entryName]->paramList)->fidSetsMap[fsName]);
  if(((FSList *)container2.entryMap[entryName]->paramList)->AddFidSet(thisFS) == -1){
    cout << "ERROR: Adding fiducial set failed\n";
    return -1;
  }

  int ret = container2.Write(tsdfcFileName3);
 
  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 

  cout << "***********************************************************************\n";  
  
  return 0;

}

/*======================================================================*/

// * Post: Deletes a FidSet from an FSList  
int deleteFidSet(){
  string tsdfcFileName1, tsdfcFileName2;
  string entryName, fidName, fsName;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the tsdfc file to edit: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new tsdfc file to save edits to: ";
  cin >> tsdfcFileName2; 
  cout << "Enter the time series name: ";
  cin >> entryName;
  cout << "Enter the name of the fidset to delete: ";
  cin >> fsName;

  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName1);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  if(!(thisContainer.entryMap.count(entryName))){
    cout << "ERROR: Time Series file not found\n";
    return -1;
  }

  if(((FSList *)thisContainer.entryMap[entryName]->paramList)->DeleteFidSet(fsName) == -1){
    cout << "ERROR: Deletion of fiducial set failed\n";
    return -1;
  }

  int ret = thisContainer.Write(tsdfcFileName2);
 
  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 
	
  cout << "***********************************************************************\n";  
  
  return 0;
  
}

/*======================================================================*/

// * Post:  Change the name (unique identifier) of a FidSet
int changeFSName(){
  string tsdfcFileName1, tsdfcFileName2;
  string entryName, fidName, fsOld, fsNew;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the tsdfc file to edit: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new tsdfc file to save edits to: ";
  cin >> tsdfcFileName2; 
  cout << "Enter the time series name: ";
  cin >> entryName;
  cout << "Enter the old name of the fidset: ";
  cin >> fsOld;
  cout << "Enter the new name of the fidset: ";
  cin >> fsNew;

  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName1);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  if(!(thisContainer.entryMap.count(entryName))){
    cout << "ERROR: Time Series file not found\n";
    return -1;
  }

  if(((FSList *)thisContainer.entryMap[entryName]->paramList)->ChangeFSName(fsOld, fsNew) == -1){
    cout << "ERROR: Changing fiducial set name failed\n";
    return -1;
  }

  int ret = thisContainer.Write(tsdfcFileName2);
 
  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 
	
  cout << "***********************************************************************\n";  
 
  return 0;

}

/*======================================================================*/

// * Post:  Change the name (tsdfc/dfc file name) of a Container entry
int changeEntryName(){
  string tsdfcFileName1, tsdfcFileName2;
  string tsdfOld, tsdfNew, fidName, fsOld, fsNew;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the tsdfc file to edit: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new tsdfc file to save edits to: ";
  cin >> tsdfcFileName2; 
  cout << "Enter the old time series name: ";
  cin >> tsdfOld;
  cout << "Enter the new time series name: ";
  cin >> tsdfNew;

  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName1);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  if(thisContainer.ChangeEntryName(tsdfOld, tsdfNew) == -1){
    cout << "ERROR: Name change failed\n";
    return -1;
  }

  int ret = thisContainer.Write(tsdfcFileName2);
      
  if(ret == 0){
     cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 
	
  cout << "***********************************************************************\n";  
  
  return 0;
}


/*======================================================================*/

// * Post:  Add a tsdf and its fiducial info to the Container
int addTSDFEntry(){
  string tsdfcFileName1, tsdfcFileName2;
  string entryName, fidName, fsName;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the tsdfc file to get Entry from: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new tsdfc file to Entry to: ";
  cin >> tsdfcFileName2; 
  cout << "Enter the name of the time series from file 1 to add to file 2: ";
  cin >> entryName;

  Container container1, container2;

  int retVal = container1.Read(tsdfcFileName1);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  if(!(container1.entryMap.count(entryName))){
    cout << "ERROR: Time Series file not found\n";
    return -1;
  }
  
  //retVal = container2.Read(tsdfcFileName2);

  //if(retVal == -1){
  //  cout << "ERROR: Container read failed\n";
    //return -1;
  //}

  PSList *thisPSL = container1.entryMap[entryName]->paramList;
  if(container2.AddTSDFEntry(entryName, thisPSL, "FSList") == -1){
    cout << "ERROR: Add entry failed\n";
    return -1;
  }

  int ret = container2.Write(tsdfcFileName2);

  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 

  cout << "***********************************************************************\n";  
  
  return 0;
}

/*======================================================================*/

// * Post:  Add tsdfc/dfc file name to a dfc file
int addEntry(){
  string tsdfcFileName1, tsdfcFileName2;
  string entryName, fidName, fsName;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the container file to edit: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new container file to  save edits to: ";
  cin >> tsdfcFileName2; 
  cout << "Enter the name of the new entry: ";
  cin >> entryName;

  Container container1, container2;

  int retVal = container1.Read(tsdfcFileName1);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    //return -1;
  }

  //retVal = container2.Read(tsdfcFileName2);

  // if(retVal == -1){
  //  cout << "ERROR: Container read failed\n";
    //return -1;
  //}

  if(container2.AddEntry(entryName) == -1){
    cout << "ERROR: Add entry failed\n";
    return -1;
  }

  int ret = container2.Write(tsdfcFileName2);
        
  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 



  cout << "***********************************************************************\n";  
  
  return 0;
}

/*======================================================================*/

// * Post:  Deletes an entry from a container file
int deleteEntry(){

  string tsdfcFileName1, tsdfcFileName2;
  string entryName, fidName, fsName;
  string oldFidName, newFidName;

  cout << "***********************************************************************\n";
  cout << "Enter the name of the container file to edit: ";
  cin >> tsdfcFileName1;
  cout << "Enter the name of the new container file to save edits to: ";
  cin >> tsdfcFileName2; 
  cout << "Enter the name of the entry to delete: ";
  cin >> entryName;
 

  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName1);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  thisContainer.DeleteEntry(entryName);

  int ret = thisContainer.Write(tsdfcFileName2);
        
  if(ret == 0){
     cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 
	
  cout << "***********************************************************************\n";  
 
  return 0;
}

/*======================================================================*/

// * Post:  Writes the contents of a Container to a file
int writeFile(){
  cout << "***********************************************************************\n";

  string tsdfcFileName1, tsdfcFileName2;
    
  cout << "Enter the name of the container file to copy: ";
  cin >> tsdfcFileName1;
  
  cout << "Enter the name of the new container file to copy to: ";
  cin >> tsdfcFileName2;
     
  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName1);
 
  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }
	
  int ret = thisContainer.Write(tsdfcFileName2);
 
  if(ret == 0){
    cout << "Write was successful\n";
  }
  else{
    cout << "ERROR: Write failed\n";
  } 
 
  return ret;        
}

/*======================================================================*/

// * Post:  View all info in a container file 
int view(){
  cout << "***********************************************************************\n"; 
  string tsdfcFileName;
  cout << "Enter the name of the container file: ";
  cin >> tsdfcFileName;
  
  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1; 
  }
	
  thisContainer.Output();
  
  cout << "***********************************************************************\n";
	  
  return 0;

}

/*======================================================================*/

// * Post:  View all info for a time series
int viewTS(){
  
  string entryName, tsdfcFileName, fidName;

  cout << "***********************************************************************\n"; 
  cout << "Enter the container file name: ";
  cin >> tsdfcFileName;
  cout << "Enter the time series name: ";
  cin >> entryName;
 
  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }
 
  FSList *fsl;
  if(thisContainer.entryMap.count(entryName)){
    if(thisContainer.entryMap[entryName]->paramList != NULL){
      // Get FSList pointer
      fsl = (FSList *)thisContainer.entryMap[entryName]->paramList;
    }
    else{
      cout << "Time series " << entryName << " has 0 parameter sets\n";
      return 0;
    }
  }
  else{
    cout << "ERROR: Time series file not found\n";
    return -1;
  }

  fsl->Output();

  cout << "***********************************************************************\n";  
  return 0;
}



/*======================================================================*/

// * Post:  View all info for one FidSet
int changeType(){
  Container c;
  c.Read("ant.tsdfc");

  ((FSList *)c.entryMap["ant400.tsdf"]->paramList)->fidSetsMap["LsdRec"]->type = 2;
  c.Write("newType.tsdfc");
  return 0;
  
}



/*======================================================================*/

// * Post:  View all info for one FidSet
int viewFS(){
  
  string entryName, tsdfcFileName, fidName, fsName;

  cout << "***********************************************************************\n"; 
  cout << "Enter the container file name: ";
  cin >> tsdfcFileName;
  cout << "Enter the time series name: ";
  cin >> entryName;
  cout << "Enter the fidset name: ";
  cin >> fsName;
  
  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName);

  // Error checking
  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  FSList *myFSL;
  if(thisContainer.entryMap.count(entryName)){
    // Get FSList pointer
    myFSL = (FSList *)thisContainer.entryMap[entryName]->paramList;
  }
  else{
    cout << "ERROR: Time series file not found\n";
    return -1;
  }

  if(!(myFSL->fidSetsMap.count(fsName))){
    cout << "ERROR: Invalid fiducial set\n";
    return -1;
  }

  // Get FidSet pointer
  FidSet *myFS = myFSL->fidSetsMap[fsName];
  myFS->Output();


  cout << "***********************************************************************\n";  
  return 0;
}

/*======================================================================*/

// * Post:  View all info for one fiducial type
int viewFid(){
  
  string entryName, tsdfcFileName, fidName, fsName;
  //int leadNum;
  //float fidVal;

  cout << "***********************************************************************\n"; 
  cout << "Enter the container file name: ";
  cin >> tsdfcFileName;
  cout << "Enter the time series name: ";
  cin >> entryName;
  cout << "Enter the fidset name: ";
  cin >> fsName;
  //cout << "Enter the lead number: ";
  //cin >> leadNum;
  cout << "Enter the fid name: ";
  cin >> fidName;

  Container thisContainer;

  int retVal = thisContainer.Read(tsdfcFileName);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }

  
  FSList *myFSL;
  if(thisContainer.entryMap.count(entryName)){
    // Get FSList pointer
    myFSL = (FSList *)thisContainer.entryMap[entryName]->paramList;
  }
  else{
    cout << "ERROR: Time series file not found\n";
    return -1;
  }


  if(!(myFSL->fidSetsMap.count(fsName))){
    cout << "ERROR: Invalid fiducial set\n";
    return -1;
  }

  // Get FidSet pointer
  FidSet *myFS = myFSL->fidSetsMap[fsName];
  
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
  printf("\n");
 
  cout << "***********************************************************************\n"; 
  return 0;
}


/*======================================================================*/

// * Post:  Get the full path to a container entry file
int getPath(){
  
  string entryName, containerFileName;

  cout << "***********************************************************************\n"; 
  cout << "Enter the container file name: ";
  cin >> containerFileName;
  cout << "Enter the entry name: ";
  cin >> entryName;

  Container thisContainer;

  int retVal = thisContainer.Read(containerFileName);

  if(retVal == -1){
    cout << "ERROR: Container read failed\n";
    return -1;
  }
  

  if(thisContainer.entryMap.count(entryName)){
    cout << entryName << " is located in " << thisContainer.GetTSDFFilePath(entryName, "") << "\n";
  }
  else{
    cout << "ERROR: Entry not found in container file\n";
    return -1;
  }

  return 0;
}


/*======================================================================*/

// * Post:  Old debugging code for viewing a single fiducial
int viewOldFid(){
  /*
  // fidset test code
  
  // read all tsdf file names from gdbm file

  cout << "***********************************************************************\n";
  string tsdfcFileName;
  cout << "Enter the name of the tsdfc file: ";
  cin >> tsdfcFileName;
  
  //cout << "You entered " << tsdfcFileName << "\n";

  GDBMKeys gkeys;
  //cout << "***before gkeys.ReadKeys\n";
  
  //const char * tempTsdfc = stoc(tsdfcFileName);  
  int numEntries = gkeys.ReadKeys(tsdfcFileName);
  //delete [] tempTsdfc;

  cout << "There are " << numEntries << " time series in this container file\n";
 
  //cout << "***before outer loop\n";

  // loop through all time series    
  for(int i = 0; i < numEntries; i++){


    string entryName = gkeys.ReturnKey(i);

    GIOFidSet testFS;
    
    int numSets = testFS.NumSets(const_cast<char*>(tsdfcFileName.c_str()), const_cast<char*>(entryName.c_str()));
   
    cout << "Time Series " << i << " is called " << entryName << " and has " << numSets << " parameter sets\n";
    
    for(int j = 0; j < numSets; j++){
      testFS.ReadGIOFidSet(const_cast<char*>(tsdfcFileName.c_str()), const_cast<char*>(entryName.c_str()), j);
      //cout << "before declaration of myFS\n";
      FidSet myFS;
      myFS.Read(&testFS); 
      //cout << "after declaration of myFS\n";
      int numLeads = myFS.numfidleads;
      cout << "\nFidset " << j << "\n";  
      cout << "Type: " << myFS.type << "\n";
      cout << "Version: " << myFS.version << "\n";
      cout << "Name: " << myFS.name << "\n";                 // fidset name. 
      cout << "Audit String: " << myFS.auditString << "\n"; 		// fidset audit string. 
      cout << "Time Series File Name: " << myFS.tsdfname << "\n";
      cout << "Number of leads: " << myFS.numfidleads << "\n\n";            // number of leads that have fids.
      
      for(int k = 0; k < numLeads; k++){
        int numFids = myFS.fidDescArray[k];
        FidPtr * fidArray = myFS.leadsArray[k];
        cout << "Lead number " << k << " has " << numFids << " fiducials\n";
        //myFS.leadsHash[k].Output();
        for(int m = 0; m < numFids; m++){
          FidPtr thisFid = fidArray[m];
          cout << "Fid number: " << m << " Type: " << thisFid->type << " Value: " << thisFid->value << " Name: " << thisFid->name << "\n"; 
        }
      } 
       
    }
    //delete[] tempTsdf;
    //delete[] tempTsdfc;
  }  

  string tsfName;
  int fsNum, leadNum;
  string fidName;
  float fidVal;

  cout << "Enter the time series name: ";
  cin >> tsfName;
  cout << "Enter the fidset number: ";
  cin >> fsNum;
  cout << "Enter the lead number: ";
  cin >> leadNum;
  cout << "Enter the fid name: ";
  cin >> fidName;

  GIOFidSet testFS;

  
  //const char * tempTsdf = stoc(tsfName);
  testFS.ReadGIOFidSet(const_cast<char*>(tsdfcFileName.c_str()), const_cast<char*>(tsfName.c_str()), fsNum);
  FidSet myFS;
  myFS.Read(&testFS); 

  //fidVal = testFid.value;
  fidVal = (*(myFS.leadsMap[leadNum][fidName])).value;

  cout << "Time Series: " << tsfName << " Fidset #: " << fsNum << " Lead #: " << leadNum << " Fid name: " << fidName << " Fid value: " << fidVal << "\n";
  
  
  //delete [] tempTsdfc;
  //delete []  tempTsdf;

  */
  return 0;
}

/*======================================================================*/

// * Post:  Old code for viewing the info is a FidSet
int viewOldFS(){
  /*
  // ************************OLD TEST CODE***********************************

  //cout << "In read\n";

  // fidset test code
  
  // read all tsdf file names from gdbm file

   cout << "***********************************************************************\n";
    string tsdfcFileName;
        cout << "Enter the name of the tsdfc file: ";
        cin >> tsdfcFileName;
  
        //cout << "You entered " << tsdfcFileName << "\n";
  // first, get all time series info
  
  GDBMKeys gkeys;
  //cout << "***before gkeys.ReadKeys\n";
  
  
  int numEntries = gkeys.ReadKeys(tsdfcFileName);


  cout << "There are " << numEntries << " time series in this container file\n";
 

  for(int i = 0; i < numEntries; i++){

    string entryName = gkeys.ReturnKey(i);

    //cout << "********************Before readlist\n";

    FSList fsl;
    fsl.Read(tsdfcFileName, entryName);
    //cout << "********************After readlist\n";
    int numSets = fsl.numPS;

    cout << "Time Series " << i << " is called " << entryName << " and has " << numSets << " parameter sets\n";

    //fsl.fidSetsHash.Output();
    FidSet myFS;

    for(int j = 0; j < numSets; j++){
      
      //cout << "*****Initializing myFS\n";
      myFS = *(fsl.fidSetsArray[j]);
      //cout << "*****myFS initialized\n";

      int numLeads = myFS.numfidleads;
      cout << "\nFidset " << j << "\n";  
      cout << "Type: " << myFS.type << "\n";
      cout << "Version: " << myFS.version << "\n";
      cout << "Name: " << myFS.name << "\n";                 // fidset name. 
      cout << "Audit String: " << myFS.auditString << "\n"; 		// fidset audit string. 
      cout << "Time Series File Name: " << myFS.tsdfname << "\n";
      cout << "Number of leads: " << myFS.numfidleads << "\n\n";            // number of leads that have fids.

      for(int k = 0; k < numLeads; k++){
        cout << "about to access fidDescArray\n";
        int numFids = myFS.fidDescArray[k];
        FidPtr * fidArray = myFS.leadsArray[k];
        cout << "Lead number " << k << " has " << numFids << " fiducials\n";
        
        //myFS.leadsHash[k].Output();
        for(int m = 0; m < numFids; m++){
          FidPtr thisFid = fidArray[m];
          cout << "Fid number: " << m << " Type: " << thisFid->type << " Value: " << thisFid->value << " Name: " << thisFid->name << "\n"; 
        }
        
      } 
      
      
    }
    

  }

  
 
  string tsfName;
  int fsNum, leadNum;
 
  string fidName;
  float fidVal;

  cout << "\nEnter the time series name: ";
  cin >> tsfName;
  cout << "Enter the fidset number: ";
  cin >> fsNum;
  cout << "Enter the lead number: ";
  cin >> leadNum;
  cout << "Enter the fid name: ";
  cin >> fidName;

  //cout << "********************Before readlist\n";

  FSList testFS;
  testFS.Read(tsdfcFileName, tsfName);
 
  //cout << "********************After readlist\n";
  FidSet myFS = *(testFS.fidSetsArray[fsNum]);


  fidVal = (myFS.leadsMap[leadNum][fidName])->value;
  cout << "\n\nTime Series: " << tsfName << "\n";
  cout << "Fidset #: " << fsNum << "\n";
  cout << "Lead #: " << leadNum << "\n";
  cout << "Fid name: " << fidName << "\n";
  cout << "Fid value: " << fidVal << "\n";
  
  */
  return 0;
    
}


// ****************************************************************************
// ****************************************************************************
// ********************************* Main *************************************
// ****************************************************************************
// ****************************************************************************


int main(int argc, char **argv){

  cout << "Choose one of the following options:\n"
       << "\nViewing options:\n"
       << "\t 'a' -- View all container file information\n"
       << "\t 'b' -- View information for one time series\n"
       << "\t 'c' -- View information for one parameter set (i.e. fiducial set)\n"
       << "\t 'd' -- View information for an individual fiducial\n"
       << "\nEditing (write and rewrite) options:\n"
       << "Note: These are only a few of the options for editing\n"
       << "\t 'e' -- Read tsdfc file and write to new tsdfc file\n"
       << "\t 'f' -- Add fiducial and corresponding leads values to a set\n"
       << "\t 'g' -- Delete fiducial and corresponding leads values from a set\n"
    //<< "\t 'h' -- Change fiducial type (name)\n"
       << "\t 'i' -- Add lead to fiducial set\n"
       << "\t 'j' -- Delete lead from fiducial set\n"
       << "\t 'k' -- Add FidSet to an FSList\n"
       << "\t 'm' -- Delete FidSet from an FSList\n"
       << "\t 'n' -- Change FidSet name\n"
       << "\t 'o' -- Add tsdf entry\n"
       << "\t 'p' -- Add container entry\n"
       << "\t 'r' -- Delete Entry\n"
       << "\t 's' -- Change entry name\n"       
       << "\t 't' -- Get path to entry (tsdf)\n"
       << "\nOther options:\n" 
       << "\t 'l' -- List choices\n"
       << "\t 'q' -- Quit\n\n";
       

  char option = '0';
  
  while(option != 'q'){
    cout << "Enter choice: ";
    cin >> option;
     
    switch(option){
    
      case 'a':
        view();
        break;
      case 'b':
        viewTS();
        break;
      case 'c':
        viewFS();
        break;
      case 'd':
        viewFid();
        break;
      case 'e':
        writeFile();
        break;
      case 'f':
        addFidValueArray();
        break;
      case 'g':
        deleteFidValueArray();
        break;
      case 'h':
        //changeFidName();
        break;
      case 'i':
        addLead();
        break;
      case 'j':
        deleteLead();
        break;
      case 'k':
        addFidSet();
        break;
      case 'm':
        deleteFidSet();
        break;
      case 'n':
        changeFSName();
        break;
      case 'p':
        addEntry();
        break;
      case 's':
        changeEntryName();
        break;
      case 'o':
        addTSDFEntry();
        break;
      case 'r':
        deleteEntry();
        break;
      case 't':
        getPath();
        break;
      case 'q':
        exit(0);
        break;
      case 'l':
        cout << "Choose one of the following options:\n"
             << "\nViewing options:\n"
             << "\t 'a' -- View all container file information\n"
             << "\t 'b' -- View information for one time series\n"
             << "\t 'c' -- View information for one parameter set (i.e. fiducial set)\n"
             << "\t 'd' -- View information for an individual fiducial\n"
             << "\nEditing (write and rewrite) options:\n"
             << "Note: These are only a few of the options for editing\n"
             << "\t 'e' -- Read tsdfc file and write to new tsdfc file\n"
	     << "\t 'f' -- Add fiducial and corresponding leads values to a set\n"
             << "\t 'g' -- Delete fiducial and corresponding leads values from a set\n"
	  //<< "\t 'h' -- Change fiducial type (name)\n"
	     << "\t 'i' -- Add lead to fiducial set\n"
	     << "\t 'j' -- Delete lead from fiducial set\n"
             << "\t 'k' -- Add FidSet to an FSList\n"
             << "\t 'm' -- Delete FidSet from an FSList\n"
             << "\t 'n' -- Change FidSet name\n"
	     << "\t 'o' -- Add tsdf entry\n"
             << "\t 'p' -- Add container entry\n"
             << "\t 'r' -- Delete Entry\n"
             << "\t 's' -- Change entry name\n"
             << "\t 't' -- Get path to entry (tsdf)\n"
             << "\nOther options:\n" 
             << "\t 'l' -- List choices\n"
             << "\t 'q' -- Quit\n\n";
        break;
      default:
        cout << "Sorry, that option does not exist.\n";
    }
  }
  
  
   
  return 0;
}














