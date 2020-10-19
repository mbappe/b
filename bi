#!/bin/sh -x

# This lengthy regression test script has the structure required to be
# used by git bisect run.

REGRESS=${1:-"regress"}

# Use -Wno-override-init with gcc for "-DAUGMENT_TYPE -DJUMP_TABLE".
# Use -Wno-initializer-overrides with clang for "-DAUGMENT_TYPE -DJUMP_TABLE".
# Unfortunately, with clang, -Wno-initializer-overrides is cancelled out
# by -Wextra if -Wextra occurs after -Wno_initializer-overrides on the
# command line.
CCA=clang
WFLAGSA_A=-Wno-initializer-overrides
CCB=gcc
WFLAGSA_B=-Wno-override-init
CC=$CCA
WFLAGSA=$WFLAGSA_A
export CC WFLAGSA

# How do we tell if AVX2 is supported by the cpu?
# On Linux: grep avx2 /etc/cpuinfo
# On Mac: sysctl -a | grep machdep.cpu.leaf7_features | grep AVX2
# On Windows: ?
if [ `uname` = Linux ]
then
    # echo Linux
    if grep avx2 /etc/cpuinfo
    then
        MAVX2=-mavx2
    fi
else # [ `uname` = Linux ]
    # echo Not Linux
    if sysctl -a | grep AVX2
    then
        MAVX2=-mavx2
    fi
fi # [ `uname` = Linux ] else

make clean default && ${REGRESS}
if [ $? != 0 ]; then echo "non-zero exit"; exit 1; fi

: \
&& DEFINES="-DDEBUG" make clean default \
&& ${REGRESS} \
&& : "test for nPopCntLoop > anListPopCntMax[nBLLoop] in Splay" \
&& DEFINES="-DcnListPopCntMax16=8 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMax16=8 -DSPLAY_WITH_INSERT -DDEBUG" \
    make clean default \
&& ${REGRESS} \
&& DEFINES="-DDS_4_WAY -DDEBUG" make clean default \
&& ${REGRESS} \
&& : "64-way" \
&& DEFINES="-DDSPLIT_16 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DDS_8_WAY -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DDS_16_WAY -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DOLD_HK_128 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_PARALLEL_HK_128_64 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_NEW_NEXT -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_LOCATE_GE_AFTER_LOCATE_EQ -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_LOCATE_GE_KEY_X -DDEBUG" make clean default \
&& ${REGRESS} \
&& CC_MFLAGS=$MAVX2 DEFINES="-DPARALLEL_256 -DDEBUG" make clean default \
&& ${REGRESS} \
&& MALLOC_ALIGNMENT=32 CC_MFLAGS=$MAVX2 DEFINES="-DPARALLEL_256 -DDEBUG" \
    make clean default \
&& ${REGRESS} \
&& DEFINES="-DDEBUG_ALL -DFULL_DUMP" make clean default \
&& CC=$CCB WFLAGSA=$WFLAGSA_B make clean default \
&& CC=$CCB WFLAGSA=$WFLAGSA_B DEFINES="-DDEBUG" make clean default \
&& CC=$CCB WFLAGSA=$WFLAGSA_B DEFINES="-DDEBUG_ALL -DFULL_DUMP" \
    make clean default \
&& DEFINES="-DPARALLEL_SEARCH_WORD -DNO_PSPLIT_PARALLEL -DDEBUG" \
    make clean default \
&& ${REGRESS} \
&& : "Default for Judy1 is REVERSE_SORT, FILL_W_BIG_KEY, NO_EK_CALC_POP" \
&& DEFINES="-DNO_REVERSE_SORT_EMBEDDED_KEYS -DDEBUG" \
    make clean default \
&& ${REGRESS} \
&& : "_RETURN_NULL_TO_INSERT_AGAIN is default for JudyL" \
&& DEFINES="-DDEBUG -D_RETURN_NULL_TO_INSERT_AGAIN" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DPOP_CNT_MAX_IS_KING -DcnBitsInD1=6 \
-DDEBUG" \
    make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DNO_EMBED_KEYS -DcnBitsInD1=6 -DDEBUG" \
    make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DPOP_CNT_MAX_IS_KING -DDEBUG" \
    make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DNO_EMBED_KEYS -DDEBUG" \
     make clean default \
&& ${REGRESS} \
&& DEFINES="-DCHECK_TYPE_FOR_EBM -DAUGMENT_TYPE_8_PLUS_4 -DcnBitsInD1=6 \
-DDEBUG" \
    make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DPOP_CNT_MAX_IS_KING -DcnBitsInD1=6 \
-DAUGMENT_TYPE_8_PLUS_4 -DNO_UNPACK_BM_VALUES -DDEBUG" \
    make clean default \
&& ${REGRESS} \
&& : "_LNX is default for JudyL" \
&& DEFINES="-DDEBUG -D_LNX" make clean default \
&& ${REGRESS} \
&& : "DUMMY_REMOTE_LNX is undefined in bedefines.h for JudyL" \
&& DEFINES="-DDEBUG -DDUMMY_REMOTE_LNX" make clean default \
&& ${REGRESS} \
&& DEFINES="-DQP_PLN -DDEBUG" make clean default \
&& ${REGRESS} \
&& :
if [ $? != 0 ]; then echo "non-zero exit"; exit 1; fi

for lvl in "-DNO_LVL_IN_WR_HB -DDEFAULT_SKIP_TO_SW" \
           "-DNO_LVL_IN_WR_HB -DALL_SKIP_TO_SW_CASES" \
            -DLVL_IN_PP
do
for jt in "" -DJUMP_TABLE
do
    : \
    && DEFINES="$lvl $jt -DDEBUG" make clean default \
    && ${REGRESS} \
    && :
    if [ $? != 0 ]; then echo "non-zero exit"; exit 1; fi
done
done

: \
&& : 'Regression test bug fix in 204826.' \
&& DEFINES="-DNO_USE_BM_SW -DcnBitsInD1=6 -DcnBitsInD2=10 -DNO_REMOTE_LNX \
-DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& : 'Regression test fix of BM SW with cnBitsInD2 != cnBitsPerDigit.' \
&& DEFINES="-DNO_BM_SW_CNT_IN_WR -DcnBitsInD1=6 -DcnBitsInD2=10 \
-DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_LVL_IN_WR_HB -DDEFAULT_SKIP_TO_SW -DPOP_WORD -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DDEBUG -DNO_EK_XV" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMax64=0 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_EMBED_KEYS -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DSKIP_TO_BITMAP -DcnListPopCntMax64=64 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMax64=16 -DcnListPopCntMaxDl1=16 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DSPLAY_WITH_INSERT -DDOUBLE_DOWN -DUSE_LOWER_XX_SW \
-DcnListPopCntMax64=16 -DNO_USE_BM_SW -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DSPLAY_WITH_INSERT -DDOUBLE_DOWN -DUSE_LOWER_XX_SW \
-DNO_EMBED_KEYS -DcnListPopCntMax64=16 -DNO_USE_BM_SW -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DDOUBLE_DOWN -DUSE_LOWER_XX_SW -DcnListPopCntMax64=64 \
-DNO_EMBED_KEYS -DNO_USE_BM_SW -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DDOUBLE_DOWN -DUSE_LOWER_XX_SW -DcnListPopCntMax64=64 \
-DNO_USE_BM_SW -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DDOUBLE_DOWN -DcnListPopCntMax64=64 -DNO_EMBED_KEYS \
-DNO_USE_BM_SW -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DDOUBLE_DOWN -DcnListPopCntMax64=64 \
-DNO_USE_BM_SW -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DXX_LISTS -DUSE_LOWER_XX_SW -DcnListPopCntMax64=64 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DBM_IN_LINK -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES=-DSEARCHMETRICS make clean default \
&& ${REGRESS} \
&& DEFINES="-URAMMETRICS -DSEARCHMETRICS" make clean default \
&& ${REGRESS} \
&& DEFINES="-DUSE_XX_SW_ONLY_AT_DL2 -DcnListPopCntMax64=64 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_EMBED_KEYS -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_BITMAP -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_COMPRESSED_LISTS -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DLVL_IN_PP -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_SKIP_LINKS -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DPP_IN_LINK -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=16	-Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=15	-Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=14 -Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=13 -Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=12 -Dcn2dBmMaxWpkPercent=0 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=11 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=10 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=9 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=8 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=7 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=6 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=6 -DNO_ALLOW_EMBEDDED_BITMAP -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=5 -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=5 -DNO_ALLOW_EMBEDDED_BITMAP -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=4 -DcnListPopCntMax64=16 -DNO_EK_XV -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=3 -DcnListPopCntMax64=8 -DNO_EK_XV -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=3 -DcnListPopCntMax64=8 -DNO_EK_XV \
-DNO_ALLOW_EMBEDDED_BITMAP -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& : "cnListPopCntMax64=8 fails" \
&& DEFINES="-DcnBitsPerDigit=2 -DcnListPopCntMax64=16 -DNO_EK_XV -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& : "cnListPopCntMax64=2 fails" \
&& DEFINES="-DcnBitsPerDigit=1 -DcnListPopCntMax64=4 -DNO_EK_XV -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DUSE_XX_SW -DcnListPopCntMax64=64 -DNO_SKIP_LINKS -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DUSE_XX_SW -DcnListPopCntMax64=64 -DNO_SKIP_TO_BITMAP -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DPOP_WORD -DDEBUG" make clean default \
&& ${REGRESS} \
&& DEFINES="-DPOP_CNT_MAX_IS_KING -DcnListPopCntMax64=1 -DDEBUG" \
   make clean default \
&& DEFINES="-DGUARDBAND" make clean default \
&& ${REGRESS} \
&& DEFINES="-DGUARDBAND -DcnGuardWords=3" make clean default \
&& ${REGRESS} \
&& DEFINES="-DPOP_WORD_IN_LINK -DNO_USE_BM_SW \
-DSKIP_TO_BITMAP -DcnListPopCntMax64=64 -DDEBUG" \
   make clean default \
&& ${REGRESS} \
&& DEFINES="-DSEARCHMETRICS -DDEBUG_ALL" make clean default \
&& DEFINES="-URAMMETRICS -DSEARCHMETRICS -DDEBUG_ALL" make clean default \
&& BPW=32 make clean default \
&& BPW=32 DEFINES=-DDEBUG make clean default \
&& BPW=32 DEFINES="-DDEBUG_ALL -DFULL_DUMP" make clean default \
&& BPW=32 CC=$CCB WFLAGSA=$WFLAGSA_B make clean default \
&& BPW=32 CC=$CCB WFLAGSA=$WFLAGSA_B DEFINES=-DDEBUG make clean default \
&& BPW=32 CC=$CCB WFLAGSA=$WFLAGSA_B DEFINES="-DDEBUG_ALL -DFULL_DUMP" \
    make clean default \
&& BPW=32 DEFINES="-DALL_SKIP_TO_SW_CASES" make clean default \
&& BPW=32 DEFINES="-DALL_SKIP_TO_SW_CASES -DDEBUG" make clean default \
&& BPW=32 DEFINES="-DALL_SKIP_TO_SW_CASES -DDEBUG_ALL -DFULL_DUMP" \
    make clean default \
&& BPW=32 CC=$CCB WFLAGSA=$WFLAGSA_B DEFINES="-DALL_SKIP_TO_SW_CASES" \
    make clean default \
&& BPW=32 CC=$CCB WFLAGSA=$WFLAGSA_B \
    DEFINES="-DALL_SKIP_TO_SW_CASES -DDEBUG" make clean default \
&& BPW=32 CC=$CCB WFLAGSA=$WFLAGSA_B \
    DEFINES="-DALL_SKIP_TO_SW_CASES -DDEBUG_ALL -DFULL_DUMP" \
    make clean default \
&& :
if [ $? != 0 ]; then echo "non-zero exit"; exit 1; fi

for sfw in "" -DNO_SEARCH_FROM_WRAPPER
do
for qpln in "" -DQP_PLN
do
for augtype in "" "-DNO_AUGMENT_TYPE_8 -DAUGMENT_TYPE" \
                  "-DNO_AUGMENT_TYPE_8 -DAUGMENT_TYPE -DMASK_TYPE" \
                  "-DAUGMENT_TYPE_8_PLUS_4 -DcnBitsInD1=4 -DNO_EK_XV" \
                  "-DAUGMENT_TYPE_8_PLUS_4 -DcnBitsInD1=11" \
                  "-DAUGMENT_TYPE_8_PLUS_4 -DcnBitsInD2=6" \
                  "-DAUGMENT_TYPE_8_PLUS_4 -DcnBitsInD2=10" \
                  "-DAUGMENT_TYPE_8_PLUS_4 -DcnBitsInD3=7" \
                  "-DAUGMENT_TYPE_8_PLUS_4 -DcnBitsInD3=9" \
                  "-DMASK_TYPE"
do
for allcases in "" -DALL_SKIP_TO_SW_CASES -DDEFAULT_SKIP_TO_SW
do
for jt in "" -DJUMP_TABLE
do
    # Pare down the number of tests.
    if [ "$sfw" != "" -o "$qpln" != "" ]; then
        if [ "$augtype" != "" ]; then
        if [ "$augtype" != "-DAUGMENT_TYPE" ]; then
        if [ "$augtype" != "-DAUGMENT_TYPE_8" ]; then
        if [ "$augtype" != "-DAUGMENT_TYPE_8_PLUS_4 -DcnBitsInD1=4" ]; then
            continue
        fi
        fi
        fi
        fi
    fi
    : \
    && DEFINES="$sfw $qpln $augtype $allcases $jt -DDEBUG" make clean default \
    && ${REGRESS} \
    && :
    if [ $? != 0 ]; then echo "non-zero exit"; exit 1; fi
done
done
done
done
done

echo
echo "=== SUCCESS! ==="
echo

#BPW=32 make clean default
#CC=clang make clean default
#DEFINES=-URAMMETRICS make clean default
#DEFINES=-DSEARCHMETRICS make clean default
#DEFINES=-DDEBUG_ALL make clean default
#DEFINES=-DLVL_IN_PP make clean default
#DEFINES=-DPP_IN_LINK make clean default
#DEFINES=-DNO_SKIP_LINKS make clean default
#BPW=32 CC=clang DEFINES="-DDEBUG -DNO_SKIP_LINKS" make clean default
#BPW=32 DEFINES="-DSEARCHMETRICS -DDEBUG -UNO_BITMAP" make clean default
#trap 'if [ $? -eq 0 ]; then exit 0; else exit 1; fi' EXIT
#${REGRESS}32-6fd7cd

