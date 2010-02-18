#include "numseq.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    NumSeq ns;
    int num, n, *na, i;

    if (argc != 2) {
	printf("Usage: nstest number-sequence-string\n");
        return 1;
    }

    ns = NumSeqCreate();

    if (NumSeqParse(ns, argv[1])) {
	printf("? Illegal sequence\n");
        return 1;
    }

    printf("Low, High = %d,%d\n", NumSeqLow(ns), NumSeqHigh(ns));
    
    printf("Retrieve numbers via NumSeqNext()...\n");
    n = NumSeqN(ns);
    printf("There are %d numbers in the sequence\n", n);
    while (NumSeqNext(ns, &num)) {
        printf("n = %d\n", num);
    }

    printf("Retrieve numbers via NumSeqArray()...\n");
    NumSeqArray(ns, &na, &n);
    printf("n=%d\n", n);
    for (i=0; i<n; i++) {
        printf("na[%d]=%d\n", i, na[i]);
    }
    free(na);

    printf("Resetting...\n");
    NumSeqReset(ns);
    printf("Retrieve numbers via NumSeqNext()...\n");
    while (NumSeqNext(ns, &num)) {
        printf("n = %d\n", num);
    }

    printf("Testing \\n termination...\n");
    argv[1][strlen(argv[1])] = '\n';
    if (NumSeqParse(ns, argv[1])) {
	printf("? Illegal sequence\n");
        return 1;
    }
    printf("Retrieve numbers via NumSeqNext()...\n");
    n = NumSeqN(ns);
    printf("There are %d numbers in the sequence\n", n);
    while (NumSeqNext(ns, &num)) {
        printf("n = %d\n", num);
    }

    NumSeqDestroy(ns); 
}
