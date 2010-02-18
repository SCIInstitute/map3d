/********** Function getline **********************************************/
/***
 	File: getline.c
  	Author: Rob MacLeod (well, actually K & R, pg 69
	Last update: Tue Oct 30 16:31:26 MST 1990 

 A function straight out of K & R to get a line of text data in 

  Input:
  	maxlen  int 	maximum number of characters to read in
  Output:
	line	char 	a string of inout text, temrinated with \0
  Returns:
	number of characters in the line 
***/
/************************************************************************/
#include<stdio.h>

int getline(char string[], int maxlen)
{
/************************************************************************/
    int i, c;
/************************************************************************/

    i = 0;
    while (--maxlen > 0 && (c = getchar()) != EOF && c != '\n')
	string[i++] = c;
    if (c == '\n')
	string[i++] = c;
    string[i] = '\0';
    return i;
}
	


