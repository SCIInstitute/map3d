#include "fi.h"
#include <stdio.h>

void PrintFI(FiducialInfo fi)
{
    printf("name = %s\n", fi.name);
    printf("type = %d\n", fi.type);
    printf("label = %s\n", fi.label);
}

void main()
{
    FiducialInfo fi;
    int result;
    FI_Init(1);
    FI_First(&fi);
    PrintFI(fi);
    while (FI_Next(&fi)) {
        PrintFI(fi);
    }

    fi.name = FI_PON_NAME;
    result = FI_GetInfoByName(&fi);
    printf("result, type = %d %s %d\n", result, fi.name, fi.type);
    fi.type = FI_PON;
    result = FI_GetInfoByType(&fi);
    printf("result, name = %d %s\n", result, fi.name);
}
