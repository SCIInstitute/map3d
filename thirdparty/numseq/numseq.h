#ifndef NUMSEQ_H
#define NUMSEQ_H
#ifdef __cplusplus
extern "C" {
#endif

/*
 * numseq.h
 *
 * Sometime in the 1990s by Ted Dustman
 */

typedef struct numseq_struct *NumSeq;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Create/Destroy a number sequence */
NumSeq NumSeqCreate(void);
void NumSeqDestroy(NumSeq);

/* Convert string representation of number sequence into internal 
 * magic form. Returns non-zero if string is an invalid number
 * sequence.
 */
int NumSeqParse(NumSeq, const char *);

/* Return numbers in sequence until no more. FALSE means no more. */
int NumSeqNext(NumSeq, int*);

/* Start the sequence over */
void NumSeqReset(NumSeq);

/* Return the total number of numbers in the sequence */
int NumSeqN(NumSeq);

/* Return an array that contains the entire number sequence.
 * Also return the number of numbers in the sequence.
 * The caller owns the returned array.
 */
void NumSeqArray(NumSeq, int**, int*);

/* Return the lowest number in the sequence */
int NumSeqLow(NumSeq);

/* Return the highest number in the sequence */
int NumSeqHigh(NumSeq);

#ifdef __cplusplus
}
#endif
#endif
