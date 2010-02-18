/*
 * numseq.c
 *
 * Sometime in the 1990s by Ted Dustman
 */

#include "numseq.h"
#include <limits.h>
#include <stdlib.h>

#define MAXRANGES 100
#define UNDEFINED -1

/* Structure representing a continuous range of numbers */
typedef struct {
    int lower;			/* Lower end of range */
    int upper;			/* Upper end of range */
    int countUp;		/* If true count up from lower to upper
				   else count down */
} range;

/* A number sequence structure.  It is a collection of ranges. */
struct numseq_struct {
    int curRange;		/* Index into current range */
    int lastRange;		/* Index of last range */
    int curNum;			/* Current # of current range */
    int lastNum;		/* Last # of current range */
    int numNums;		/* Total # of numbers */
    range ranges[MAXRANGES];	/* Collection of ranges */
    int low, high;              /* The low and high values in the
                                   sequence. */
};

/* Get an int from the string pointed to by *str.  Update *str to
 * point to next character to be processed.
 */
static int GetInt(const char **str)
{
    int inum;
    for (inum = 0; (**str >= '0') && (**str <= '9'); (*str)++) {
	inum = 10 * inum + **str - '0';
    }
    return inum;
}

/* Update the given char pointer until it does not point to a ' ' or a 
 * '\t' character.
 */
static void EatWhiteSpace(const char **cpp)
{
    while (**cpp == ' ' || **cpp == '\t') (*cpp)++;
}

/* Create a new number sequence structure.
 */
NumSeq
NumSeqCreate(void)
{
    return (NumSeq) malloc(sizeof(struct numseq_struct));
}

/* Delete a number sequence structure */
void NumSeqDestroy(NumSeq ns)
{
    free(ns);
}

/* Convert string representation of a number sequence into a
 * 'numseq_struct'.  Return 0 if parsed correctly, nozero otherwise.
 * Once you have created a NumSeq you may call NumSeqParse as many times as
 * you like.
 */
int NumSeqParse(NumSeq ns, const char *numSeqStr)
{
    int state, inum, irange;
    char c;

    if (!numSeqStr) {
	goto abort;
    }

    EatWhiteSpace(&numSeqStr);
    if ((numSeqStr[0] == '\0') || (numSeqStr[0] == '\n')) {
        goto abort;
    }

    for (irange = 0; irange < MAXRANGES; ++irange) {
	ns->ranges[irange].upper = UNDEFINED;
	ns->ranges[irange].countUp = TRUE;
    }
    irange = 0;

    /* Get first number of first range */
    inum = GetInt(&numSeqStr);
    if (inum) {
	ns->ranges[irange].lower = inum;
	state = 1;
    } else {
	goto abort;
    }

    while (state != 5) {
	switch (state) {
	case 1:                 /* Looking for number separator or end of */
                                /* string  */
	    c = *numSeqStr++;
	    switch (c) {
	    case '-':
		state = 2;
		break;
	    case ',':
            case ' ':
            case '\t':
                EatWhiteSpace(&numSeqStr);
		state = 4;
		break;
	    case '.':
                EatWhiteSpace(&numSeqStr);
		state = 6;
		break;
	    case '\0':
            case '\n':
		state = 5;
		break;
	    default:
		goto abort;
	    }
	    break;

	case 2:		/* Get second number of the range */
	    inum = GetInt(&numSeqStr);
	    if (inum) {
		state = 3;
		if (inum < abs(ns->ranges[irange].lower)) {
		    ns->ranges[irange].countUp = FALSE;
		}
		ns->ranges[irange].upper = inum;
	    } else {
		goto abort;
	    }
	    break;

	case 3:                 /* Looking for range separator or end of */
                                /* string */
	    c = *numSeqStr++;
	    switch (c) {
	    case ',':
            case ' ':
            case '\t':
                EatWhiteSpace(&numSeqStr);
		state = 4;
		break;
	    case '.':
                EatWhiteSpace(&numSeqStr);
		state = 6;
		break;
	    case '\0':
            case '\n':
		state = 5;
		break;
	    default:
		goto abort;
	    }
	    break;

	case 4:                 /* Get first number of new range */
	case 6:
	    inum = GetInt(&numSeqStr);
	    if (inum) {
		if (++irange == MAXRANGES) {
		    goto abort;
		}
		if (state == 6) {
		    inum = -inum;
		}
		ns->ranges[irange].lower = inum;
		state = 1;
	    } else {
		goto abort;
	    }
	    break;
	}
    }

    ns->lastRange = irange;
    ns->curRange = 0;

    /* Fixup ranges that have only one number. */
    for (irange = 0; irange <= ns->lastRange; ++irange) {
	if (ns->ranges[irange].upper == UNDEFINED) {
	    ns->ranges[irange].upper = abs(ns->ranges[irange].lower);
	}
    }

    /* Count total number of numbers in the sequence */
    ns->numNums = 0;
    ns->low = INT_MAX;
    ns->high = INT_MIN;
    for (irange = 0; irange <= ns->lastRange; ++irange) {
	int high, low;
	low = abs(ns->ranges[irange].lower);
	high = ns->ranges[irange].upper;
	if (low > high) {
	    int l = low;
	    low = high;
	    high = l;
	}
	ns->numNums += high - low + 1;
        if (low < ns->low) {
            ns->low = low;
        }
        if (high > ns->high) {
            ns->high = high;
        }
    }

    /* Get ready for first call to NumSeqNext() */
    ns->curNum = ns->ranges[0].lower;
    ns->lastNum = ns->ranges[0].upper;

    return 0;

  abort:
    ns->curRange = 0;
    ns->lastRange = -1;		/* Force NumSeqNext() to always return
				   FALSE. */
    return 1;
}

/* Retrieve the next number in the sequence and put it in 'num'.  Return
 * FALSE when no more numbers remain.
 */
int NumSeqNext(NumSeq ns, int *num)
{
    if (ns->curRange <= ns->lastRange) {
	*num = ns->curNum;
	ns->curNum = abs(ns->curNum);
	if (ns->curNum == ns->lastNum) {
	    if (++ns->curRange < MAXRANGES) {
		ns->curNum = ns->ranges[ns->curRange].lower;
		ns->lastNum = ns->ranges[ns->curRange].upper;
	    }
	} else {
	    if (ns->ranges[ns->curRange].countUp) {
		++ns->curNum;
	    } else {
		--ns->curNum;
	    }
	}
    } else {
	*num = 0;
	return FALSE;
    }

    return TRUE;
}

/* Reset the number sequence so that the next time you call NumSeqNext
 * you will get the first number in the sequence.
 */
void NumSeqReset(NumSeq ns)
{
    ns->curRange = 0;
    ns->curNum = ns->ranges[0].lower;
    ns->lastNum = ns->ranges[0].upper;
}

/* Return the total number of numbers in the sequence.
 */
int NumSeqN(NumSeq ns)
{
    return ns->numNums;
}

/* Allocate an array large enough to hold all numbers in the given
 * number sequence and then copy all numbers to this array.  Also
 * return the number of numbers in the sequence.
 */
void NumSeqArray(NumSeq ns, int **a, int *n)
{
    int num, i;
    *a = (int *) 0;
    if (ns->numNums) {
	*a = (int *) malloc(ns->numNums * sizeof(int));
    }
    if (!(ns->numNums && *a)) {
	return;
    }
    NumSeqReset(ns);
    i = 0;
    while (NumSeqNext(ns, &num)) {
	(*a)[i++] = num;
    }
    *n = ns->numNums;
    NumSeqReset(ns);
}

/* Return the lowest number in the sequence */
int NumSeqLow(NumSeq ns)
{
    return ns->low;
}

/* Return the highest number in the sequence */
int NumSeqHigh(NumSeq ns)
{
    return ns->high;
}
