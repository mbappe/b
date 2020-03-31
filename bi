#!/bin/sh -x

# This lengthy regression test script has the structure required to be
# used by git bisect run.

: \
&& make clean default \
&& regress \
&& DEFINES="-DDEBUG" make clean default \
&& regress \
&& DEFINES="-DQP_PLN -DDEBUG" make clean default \
&& regress \
&& :

for lvl in "-DNO_LVL_IN_WR_HB -DDEFAULT_SKIP_TO_SW" \
           "-DNO_LVL_IN_WR_HB -DALL_SKIP_TO_SW_CASES" \
            -DLVL_IN_PP
do
for jt in "" -DJUMP_TABLE
do
    : \
    && DEFINES="$lvl $jt -DDEBUG" make clean default \
    && regress \
    && :
done
done

: 'regression test bug fix in 204826' \
&& DEFINES="-DNO_USE_BM_SW -DcnBitsInD1=6 -DcnBitsInD2=10 -DNO_REMOTE_LNX" \
   make clean default \
&& regress \
&& DEFINES="-DNO_LVL_IN_WR_HB -DDEFAULT_SKIP_TO_SW -DPOP_WORD -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DDEBUG -DNO_EK_XV" make clean default \
&& regress \
&& DEFINES="-DcnListPopCntMax64=0 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DNO_EMBED_KEYS -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DSKIP_TO_BITMAP -DcnListPopCntMax64=64 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnListPopCntMax64=16 -DcnListPopCntMaxDl1=16 -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DSPLAY_WITH_INSERT -DDOUBLE_DOWN -DUSE_LOWER_XX_SW \
-DcnListPopCntMax64=16 -DNO_USE_BM_SW -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DSPLAY_WITH_INSERT -DDOUBLE_DOWN -DUSE_LOWER_XX_SW \
-DNO_EMBED_KEYS -DcnListPopCntMax64=16 -DNO_USE_BM_SW -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DDOUBLE_DOWN -DUSE_LOWER_XX_SW -DcnListPopCntMax64=64 \
-DNO_EMBED_KEYS -DNO_USE_BM_SW -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DDOUBLE_DOWN -DUSE_LOWER_XX_SW -DcnListPopCntMax64=64 \
-DNO_USE_BM_SW -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DDOUBLE_DOWN -DcnListPopCntMax64=64 -DNO_EMBED_KEYS \
-DNO_USE_BM_SW -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DDOUBLE_DOWN -DcnListPopCntMax64=64 \
-DNO_USE_BM_SW -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DXX_LISTS -DUSE_LOWER_XX_SW -DcnListPopCntMax64=64 -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DBM_IN_LINK -DDEBUG" make clean default \
&& regress \
&& NO_SM=1 make clean default \
&& regress \
&& NO_SM=1 NO_RM=1 make clean default \
&& regress \
&& DEFINES="-DUSE_XX_SW_ONLY_AT_DL2 -DcnListPopCntMax64=64 -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DNO_EMBED_KEYS -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DNO_BITMAP -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DNO_COMPRESSED_LISTS -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DLVL_IN_PP -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DNO_SKIP_LINKS -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DPP_IN_LINK -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=16	-Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=15	-Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=14 -Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=13 -Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=12 -Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=11 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=10 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=9 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=8 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=7 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=6 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=6 -DNO_ALLOW_EMBEDDED_BITMAP -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=5 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=5 -DNO_ALLOW_EMBEDDED_BITMAP -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=4 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=3 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=3 -DNO_ALLOW_EMBEDDED_BITMAP -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=2 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DcnBitsPerDigit=1 -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DUSE_XX_SW -DcnListPopCntMax64=64 -DNO_SKIP_LINKS -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DUSE_XX_SW -DcnListPopCntMax64=64 -DNO_SKIP_TO_BITMAP -DDEBUG" \
   make clean default \
&& regress \
&& DEFINES="-DPOP_WORD -DDEBUG" make clean default \
&& regress \
&& DEFINES="-DPOP_CNT_MAX_IS_KING -DcnListPopCntMax64=1 -DDEBUG" \
   make clean default \
&& DEFINES="-DGUARDBAND" make clean default \
&& regress \
&& DEFINES="-DGUARDBAND -DcnGuardWords=3" make clean default \
&& regress \
&& DEFINES="-DDEBUG_ALL" make clean default \
&& NO_SM=1 DEFINES="-DDEBUG_ALL" make clean default \
&& NO_SM=1 NO_RM=1 DEFINES="-DDEBUG_ALL" make clean default \
&& BPW=32 DEFINES="-DDEFAULT_SKIP_TO_SW -DDEBUG" make clean default \
&& BPW=32 DEFINES="-DALL_SKIP_TO_SW_CASES -DDEBUG_ALL" make clean default \
&& :

for sfw in "" -DNO_SEARCH_FROM_WRAPPER
do
for qpln in "" -DQP_PLN
do
for augtype in "" "-DAUGMENT_TYPE" \
                  "-DAUGMENT_TYPE                  -DMASK_TYPE" \
                  "-DAUGMENT_TYPE                  -DAUGMENT_TYPE_NOT" \
                  "-DAUGMENT_TYPE -DAUGMENT_TYPE_8" \
                  "-DAUGMENT_TYPE -DAUGMENT_TYPE_8 -DMASK_TYPE" \
                  "-DAUGMENT_TYPE -DAUGMENT_TYPE_8 -DAUGMENT_TYPE_NOT"
do
for allcases in "" -DALL_SKIP_TO_SW_CASES -DDEFAULT_SKIP_TO_SW
do
for jt in "" -DJUMP_TABLE
do
    : \
    && DEFINES="$sfw $qpln $augtype $allcases $jt -DDEBUG" make clean default \
    && regress \
    && :
done
done
done
done
done

#BPW=32 make clean default
#CC=clang make clean default
#NO_RM=0 make clean default
#NO_SM=0 make clean default
#DEFINES=-DDEBUG_ALL make clean default
#DEFINES=-DLVL_IN_PP make clean default
#DEFINES=-DPP_IN_LINK make clean default
#DEFINES=-DNO_SKIP_LINKS make clean default
#BPW=32 CC=clang DEFINES="-DDEBUG -DNO_SKIP_LINKS" make clean default

#BPW=32 NO_SM=1 DEFINES="-DDEBUG -UNO_BITMAP" make clean default
#trap 'if [ $? -eq 0 ]; then exit 0; else exit 1; fi' EXIT
#regress32-6fd7cd

