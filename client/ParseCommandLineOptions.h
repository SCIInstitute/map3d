/* ParseCommandLineOptions.h */

#ifndef PARSECOMMANDLINEOPTIONS_H
#define PARSECOMMANDLINEOPTIONS_H

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

//#include "map3d-struct.h"       /* for Global_Input */
struct Global_Input;

/****************************************
*
* Parsing macros
*
****************************************/

/* the option currently under scrutiny */
#define OPTION_CUR argv[curargindex]

/* start the options list with this macro */
#define OPTION_START() \
  int curargindex = 1;\
while (curargindex<argc) { bool a = false; if (a) {}  // always be false here

/* finish the options list with this macro */
#define OPTION_END(CODE) \
  else {\
  CODE\
		curargindex++;\
}}

/* use this macro to handle case when only one option */
#define OPTION_ONLY(CODE) \
  else if (argc==2) {\
		CODE\
    curargindex+=1;\
  }

/* use this macro to copy the given TAG's only argument into FIELD */
#define OPTION_1(TAG,FIELD) \
  else if (strcmp(TAG,argv[curargindex])==0) {\
		if (argc<=curargindex+1) option_error(TAG,1);\
    option_copy(argv[curargindex+1],FIELD);\
    curargindex+=2;\
  }

/* same as OPTION_1 but for TAG's with 2 arguments */
#define OPTION_2(TAG,F1,F2) \
  else if (strcmp(TAG,argv[curargindex])==0) {\
		if (argc<=curargindex+2) option_error(TAG,2);\
    option_copy(argv[curargindex+1],F1);\
    option_copy(argv[curargindex+2],F2);\
    curargindex+=3;\
  }

/* same as OPTION_1 but for TAG's with 3 arguments */
#define OPTION_3(TAG,F1,F2,F3) \
  else if (strcmp(TAG,argv[curargindex])==0) {\
		if (argc<=curargindex+3) option_error(TAG,3);\
    option_copy(argv[curargindex+1],F1);\
    option_copy(argv[curargindex+2],F2);\
    option_copy(argv[curargindex+3],F3);\
    curargindex+=4;\
  }

/* same as OPTION_1 but for TAG's with 4 arguments */
#define OPTION_4(TAG,F1,F2,F3,F4) \
  else if (strcmp(TAG,argv[curargindex])==0) {\
		if (argc<=curargindex+4) option_error(TAG,4);\
    option_copy(argv[curargindex+1],F1);\
    option_copy(argv[curargindex+2],F2);\
    option_copy(argv[curargindex+3],F3);\
    option_copy(argv[curargindex+4],F4);\
    curargindex+=5;\
  }

/* similiar to OPTION_1, but we copy SIZE elements into an array */
#define OPTION_ARRAY(TAG, SIZE, dest) \
  else if (strcmp(TAG, argv[curargindex]) == 0) {\
    if (argc<=curargindex+SIZE) option_error(TAG,SIZE);\
    for (int cp = 0; cp < SIZE; cp++)\
      option_copy(argv[curargindex+1+cp],dest[cp]);\
    curargindex+=SIZE+1;\
  }

/* use this macro to set FIELD to VALUE when TAG has no arguments */
#define OPTION_VALUE(TAG,FIELD,VALUE) \
  else if (strcmp(TAG,argv[curargindex])==0) {\
		FIELD = VALUE;\
    curargindex+=1;\
  }

/* use this macro to run special handling CODE when TAG
is encountered which has INC arguments */
#define OPTION_CODE(TAG,INC,CODE) \
  else if (strcmp(TAG,argv[curargindex])==0) {\
		if (argc<=curargindex+INC) option_error(TAG,INC);\
    CODE\
    curargindex+=INC+1;\
  }

/* use this macro to copy argument NUM to FIELD.
Use inside CODE section of OPTION_CODE or OPTION_ONLY */
#define OPTION_COPY(NUM,FIELD) \
  {\
    if (argc<=curargindex+NUM) option_error("",NUM);\
    option_copy(argv[curargindex+NUM],FIELD);\
  }

/****************************************
*
* Parsing helper functions
*
****************************************/

void option_copy(char *, char *&);
void option_copy(char *, char &);
void option_copy(char *, unsigned char &c);
void option_copy(char *, short &);
void option_copy(char *, int &);
void option_copy(char *, bool &);
void option_copy(char *, long &);
void option_copy(char *, float &);
void option_copy(char *, double &);
void option_error(const char *, int);

/*************************************
*
* Parsing functions
*
*************************************/

#ifdef __cplusplus
extern "C"
{
#endif

  int ParseCommandLineOptions(int argc, char **argv, Global_Input & globalInput);

#ifdef __cplusplus
}
#endif

#endif
