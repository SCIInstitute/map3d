c -*- fortran -*-
c 
c fi_f.h
c
c Some useful declarations and constants for use with the fi library.
c

      integer fi_finit, fi_fdone, fi_finitglcolormapmode,
     $    fi_finitglrgbmode, fi_fgetnumtypes, fi_fnametotype,
     $    fi_ftypetoname, fi_fisvalidtype, fi_fisvalidname,
     $    fi_fgetlabel, fi_fgetxcolorname, 
     $    fi_fgetrgbvalues, fi_fgetglindex, fi_ffirst, fi_fnext,
     $    fi_ffirst_r, fi_fnext_r

      integer FI_PON/0/, FI_POFF/1/, FI_QON/2/, FI_RPEAK/3/,
     $    FI_SOFF/4/,FI_STOFF/5/, FI_TPEAK/6/,
     $    FI_TOFF/7/, FI_ACTPLUS/8/, FI_ACTMINUS/9/,
     $    FI_ACT/10/, FI_RECPLUS/11/, FI_RECMINUS/12/ , FI_REC/13/,
     $    FI_REF/14/, FI_JPT/15/, FI_BL/16/

      character FI_PON_NAME*3/'pon'/, FI_POFF_NAME*4/'poff'/,
     $    FI_QON_NAME*3/'qon'/, FI_RPEAK_NAME*5/'rpeak'/,
     $    FI_SOFF_NAME*4/'soff'/,
     $    FI_STOFF_NAME*5/'stoff'/, FI_TPEAK_NAME*5/'tpeak'/,
     $    FI_TOFF_NAME*4/'toff'/,
     $    FI_ACTPLUS_NAME*7/'actplus'/, FI_ACTMINUS_NAME*8/'actminus'/,
     $    FI_ACT_NAME*3/'act'/, FI_RECPLUS_NAME*7/'recplus'/,
     $    FI_RECMINUS_NAME*8/'recminus'/ , FI_REC_NAME*3/'rec'/,
     $    FI_REF_NAME*3/'ref'/, FI_JPT_NAME*3/'jpt'/,
     $    FI_BL_NAME/'baseline'/

      common /fi_symbols/FI_PON, FI_POFF, FI_QON, FI_RPEAK, FI_SOFF,
     $    FI_STOFF, FI_TPEAK, FI_TOFF, FI_ACTPLUS, FI_ACTMINUS,
     $    FI_ACT, FI_RECPLUS, FI_RECMINUS, FI_REC, FI_REF, FI_JPT,
     $    FI_PON_NAME, FI_POFF_NAME, FI_QON_NAME, FI_RPEAK_NAME,
     $    FI_SOFF_NAME, FI_STOFF_NAME, FI_TPEAK_NAME, FI_TOFF_NAME,
     $    FI_ACTPLUS_NAME, FI_ACTMINUS_NAME, FI_ACT_NAME,
     $    FI_RECPLUS_NAME, FI_RECMINUS_NAME, FI_REC_NAME,
     $    FI_REF_NAME, FI_JPT_NAME, FI_BL_NAME
