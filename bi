#!/bin/sh -x

# Regression test with various key ifdef combinations.
# Turn off default features we like to turn off when trying to turn on or debug
# new functionality, individually. Those that simplify the data structure.
# Turn on non-default features that change the structure.
# Change algorithms without changing the structure.
# Non-default combinations.
# Test basic builds with both compilers.
# Test basic 32-bit builds.
# Targeted bug-fix or code capability regression tests.

# Wish we could do more to regression test DEBUG_ALL. It might not take too
# long to run regress with DEBUG_ALL without FULL_DUMP.
# FULL_DUMP will probably take a lot longer. I guess we should measure.

# This lengthy regression test script has the structure required to be used by
# git bisect run, i.e. exit status indicates success or failure.
# But running it takes too long for it to be used in that way.
# We should make sure Check and Time and regress also have that structure.

# Put date in output.
date

REGRESS=${1:-"regress"}

# Use "OFLAGS=-O0 nohup bi true" to make sure all cases build (without
# optimization). It's quicker than optimizing.
# May need -Wno-psabi on Linux.
CC=${CC:-"cc"}
MAKE=make

# How do we tell if AVX2 is supported by the cpu?
# On Linux: grep avx2 /proc/cpuinfo
# On Mac: sysctl -a | grep machdep.cpu.leaf7_features | grep AVX2
# On Windows: ?
if [ `uname` = Linux ]
then
    # echo Linux
    if lscpu | grep avx2
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

# man uname on Mac indicates make uses uname -p to set the MACHINE_ARCH
# variable, e.g. "arm"
MACHINE_ARCH=`uname -p`

: \
&& ${MAKE} clean default && ${REGRESS} \
&& DEFINES="-DREGRESS -DDEBUG_ALL -DFULL_DUMP" ${MAKE} clean default \
&& DEFINES="-DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DLIBCMALLOC -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DUSE_DLMALLOC_DEFAULT_SIZES -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_USE_BM_SW -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_EMBED_KEYS -DREGRESS -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_SKIP_LINKS -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_BITMAP -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMax64=0 -DcnListPopCntMaxDl1=0 -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_REMOTE_LNX -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& : "Done with biggest hammers" \
&& DEFINES="-DNO_UNPACK_BM_VALUES -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_PACK_BM_VALUES -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& : '"DEFINES="-DSEPARATE_T_NULL -DREGRESS -DDEBUG" ${MAKE} clean default' \
&& DEFINES="-DNO_SKIP_TO_BM_SW -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_SKIP_TO_BITMAP -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_COMPRESSED_LISTS -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_EK_XV -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_SW_POP_IN_WR_HB -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_GPC_ALL_SKIP_TO_SW_CASES -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnSwCnts=1 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnSwCnts=0 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnLogBmlfBitsPerCnt=15 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& : "turn off 2-digit bitmap conversion in Judy1" \
&& DEFINES="-Dcn2dBmMaxWpkPercent=0 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& : "Combinations" \
&& : "Algorithms" \
&& DEFINES="-DNO_NEW_NEXT -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& : "Non-default features" \
&& : "test for nPopCntLoop > anListPopCntMax[nBLLoop] in Splay" \
&& DEFINES="-DcnListPopCntMax16=8 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_COUNT_2_PREFIX -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_COUNT_2_PREFIX -DCOUNT_2 -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DDS_4_WAY -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& : "64-way" \
&& DEFINES="-DDSPLIT_16 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DDS_8_WAY -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DDS_8_WAY -DNO_LOCATE_GE_KEY_X -DDEBUG_LOCATE_GE \
-DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DDS_16_WAY -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DBMLF_INTERLEAVE -DNO_NEW_NEXT -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DOLD_HK_128 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_PARALLEL_HK_128_64 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_LOCATE_GE_AFTER_LOCATE_EQ -DDEBUG_LOCATE_GE \
-DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_LOCATE_GE_KEY_X -DDEBUG_LOCATE_GE -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& : "-mavx2 is default in Makefile if Linux and the cpu supports avx2" \
&& : "PARALLEL_256 is default if PSPLIT_PARALLEL || PARALLEL_SEARCH_WORD" \
&& DEFINES="-mno-avx -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-mno-sse4.2 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
:

# We don't support parallel searching on ARM Mac yet.
# supported yet.
if [ ${MACHINE_ARCH} != "arm" ]
then
    : \
    && DEFINES="-DPARALLEL_128 -DREGRESS -DDEBUG" ${MAKE} clean default \
    && ${REGRESS} \
    && DEFINES="-mno-sse4.2 -DPARALLEL_128 -DREGRESS -DDEBUG" \
        ${MAKE} clean default \
    && ${REGRESS} \
    && DEFINES="-DPARALLEL_SEARCH_WORD -DNO_PSPLIT_PARALLEL \
                -DREGRESS -DDEBUG" \
        ${MAKE} clean default \
    && ${REGRESS} \
    && DEFINES="-DDOUBLE_DOWN -DUSE_LOWER_XX_SW -DUSE_XX_SW_ONLY_AT_DL2 \
                -DNO_USE_BM_SW -DcnListPopCntMax64=48 \
                -DcnListPopCntMaxDl1=256 -DDEBUG" \
        ${MAKE} clean default \
    && ${REGRESS} \
    && DEFINES="-DUSE_XX_SW_ONLY_AT_DL2 -DcnListPopCntMax64=64 \
                -DREGRESS -DDEBUG" \
       ${MAKE} clean default \
    && ${REGRESS} \
    :
fi

: \
&& DEFINES="-DPARALLEL_64 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& MALLOC_ALIGNMENT=32 DEFINES="-DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& : "Default for Judy1 is REVERSE_SORT, FILL_W_BIG_KEY, NO_EK_CALC_POP" \
&& DEFINES="-DNO_REVERSE_SORT_EMBEDDED_KEYS -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& : "_RETURN_NULL_TO_INSERT_AGAIN is default for JudyL" \
&& DEFINES="-DREGRESS -DDEBUG -D_RETURN_NULL_TO_INSERT_AGAIN" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DPOP_CNT_MAX_IS_KING -DcnBitsInD1=6 \
-DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DNO_EMBED_KEYS -DcnBitsInD1=6 \
-DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DPOP_CNT_MAX_IS_KING -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DNO_EMBED_KEYS -DREGRESS -DDEBUG" \
     ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DCHECK_TYPE_FOR_EBM -DAUGMENT_TYPE_8_PLUS_4 -DcnBitsInD1=6 \
-DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMaxDl1=0 -DPOP_CNT_MAX_IS_KING -DcnBitsInD1=6 \
-DAUGMENT_TYPE_8_PLUS_4 -DNO_UNPACK_BM_VALUES -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& : "_LNX is default for JudyL" \
&& DEFINES="-DREGRESS -DDEBUG -D_LNX" ${MAKE} clean default \
&& ${REGRESS} \
&& : "DUMMY_REMOTE_LNX is undefined in bedefines.h for JudyL" \
&& DEFINES="-DREGRESS -DDEBUG -DDUMMY_REMOTE_LNX" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DQP_PLN -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& :
if [ $? != 0 ]; then echo "non-zero exit"; exit 1; fi

for lvl in "-DNO_LVL_IN_WR_HB -DDEFAULT_SKIP_TO_SW" \
           "-DNO_LVL_IN_WR_HB -DALL_SKIP_TO_SW_CASES"
do
for jt in "" -DJUMP_TABLE
do
    : \
    && DEFINES="$lvl $jt -DREGRESS -DDEBUG" ${MAKE} clean default \
    && ${REGRESS} \
    && :
    if [ $? != 0 ]; then echo "non-zero exit"; exit 1; fi
done
done

: \
&& : 'Regression test bug fix in 204826.' \
&& DEFINES="-DNO_USE_BM_SW -DcnBitsInD1=6 -DcnBitsInD2=10 -DNO_REMOTE_LNX \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& : 'Regression test fix of BM SW with cnBitsInD2 != cnBitsPerDigit.' \
&& DEFINES="-DNO_BM_SW_CNT_IN_WR -DcnBitsInD1=6 -DcnBitsInD2=10 \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DNO_LVL_IN_WR_HB -DDEFAULT_SKIP_TO_SW -DPOP_WORD \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DSKIP_TO_BITMAP -DcnListPopCntMax64=64 -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnListPopCntMax64=16 -DcnListPopCntMaxDl1=16 -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DSPLAY_WITH_INSERT -DDOUBLE_DOWN -DUSE_LOWER_XX_SW \
-DcnListPopCntMax64=16 -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DSPLAY_WITH_INSERT -DDOUBLE_DOWN -DUSE_LOWER_XX_SW \
-DNO_EMBED_KEYS -DcnListPopCntMax64=16 -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& : 'Found a long-standing SPLAY_WITH_INSERT bug with the following test' \
&& DEFINES="-DSPLAY_WITH_INSERT -DDOUBLE_DOWN -DUSE_LOWER_XX_SW \
-DNO_EMBED_KEYS -DcnListPopCntMax64=24 -DcnListPopCntMax16=12 \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DRIGID_XX_SW -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnLogBmSwLinksPerBit=3 -DcnListPopCntMax=32 -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnLogBmSwLinksPerBit=1 -DcnListPopCntMax=32 -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DUSE_BM_SW_BM_IN_WR_OR_LNX -DcnListPopCntMax=32 -DREGRESS \
-DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DDOUBLE_DOWN -DUSE_LOWER_XX_SW -DcnListPopCntMax64=64 \
-DNO_EMBED_KEYS -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DDOUBLE_DOWN -DUSE_LOWER_XX_SW -DcnListPopCntMax64=64 \
-DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DDOUBLE_DOWN -DcnListPopCntMax64=64 -DNO_EMBED_KEYS \
-DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DDOUBLE_DOWN -DcnListPopCntMax64=64 -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& ${REGRESS} \
&& : "XX_LISTS without DOUBLE_DOWN doesn't result in any shared lists" \
&& DEFINES="-DXX_LISTS -DUSE_LOWER_XX_SW -DcnListPopCntMax64=64 \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DBM_IN_LINK -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES=-DSEARCHMETRICS ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-URAMMETRICS -DSEARCHMETRICS" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=16 -Dcn2dBmMaxWpkPercent=0 -UREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=15 -Dcn2dBmMaxWpkPercent=0 -UREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& : "${REGRESS}" \
&& DEFINES="-DcnBitsPerDigit=14 -Dcn2dBmMaxWpkPercent=0 -UREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=13 -Dcn2dBmMaxWpkPercent=0 -UREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=12 -Dcn2dBmMaxWpkPercent=0 -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=11 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=10 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=9 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=8 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=7 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=6 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=6 -DNO_ALLOW_EMBEDDED_BITMAP -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=5 -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=5 -DNO_ALLOW_EMBEDDED_BITMAP -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=4 -DcnListPopCntMax64=16 -DNO_EK_XV \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=3 -DcnListPopCntMax64=8 -DNO_EK_XV \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DcnBitsPerDigit=3 -DcnListPopCntMax64=8 -DNO_EK_XV \
-DNO_ALLOW_EMBEDDED_BITMAP -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& : "cnListPopCntMax64=8 fails" \
&& DEFINES="-DcnBitsPerDigit=2 -DcnListPopCntMax64=16 -DNO_EK_XV \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& : "cnListPopCntMax64=2 fails" \
&& DEFINES="-DcnBitsPerDigit=1 -DcnListPopCntMax64=4 -DNO_EK_XV \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DUSE_XX_SW -DcnListPopCntMax64=64 -DNO_SKIP_LINKS \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DUSE_XX_SW -DcnListPopCntMax64=64 -DNO_SKIP_TO_BITMAP \
-DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DPOP_WORD -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DPOP_CNT_MAX_IS_KING -DcnListPopCntMax64=1 -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& DEFINES="-DGUARDBAND" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DGUARDBAND -DcnGuardWords=3" ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DPOP_WORD_IN_LINK -DNO_USE_BM_SW \
-DSKIP_TO_BITMAP -DcnListPopCntMax64=64 -DREGRESS -DDEBUG" \
   ${MAKE} clean default \
&& ${REGRESS} \
&& DEFINES="-DSEARCHMETRICS -DREGRESS -DDEBUG_ALL" ${MAKE} clean default \
&& DEFINES="-URAMMETRICS -DSEARCHMETRICS -DREGRESS -DDEBUG_ALL" \
    ${MAKE} clean default \
&& DEFINES="-DNO_BITMAP -DNO_EMBED_KEYS -DNO_USE_BM_SW -DNO_SKIP_LINKS \
-DNO_COMPRESSED_LISTS -DNO_PSPLIT_PARALLEL -DNO_PARALLEL_SEARCH_WORD \
-DGUARDBAND -DTESTCOUNTACCURACY \
-DDEBUG_LOCATE_GE -DREGRESS -DDEBUG_ALL -DFULL_DUMP -DDEBUG -DEBUG_LOOKUP" \
    ${MAKE} clean default \
&& DEFINES="-DNO_SEARCH_FROM_WRAPPER -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& :
if [ $? != 0 ]; then echo "non-zero exit"; exit 1; fi

if [ `uname` != "Darwin" ]
then

: \
&& BPW=32 DEFINES="-DDEBUG_LOCATE_GE -DREGRESS -DDEBUG" ${MAKE} clean default \
&& ${REGRESS} \
&& BPW=32 ${MAKE} clean default \
&& ${REGRESS} \
&& BPW=32 DEFINES="-DNO_BITMAP -DNO_EMBED_KEYS -DNO_USE_BM_SW -DNO_SKIP_LINKS \
-DNO_COMPRESSED_LISTS -DNO_PSPLIT_PARALLEL -DNO_PARALLEL_SEARCH_WORD \
-DGUARDBAND -DTESTCOUNTACCURACY \
-DDEBUG_LOCATE_GE -DREGRESS -DDEBUG_ALL -DFULL_DUMP -DDEBUG -DEBUG_LOOKUP" \
    ${MAKE} clean default \
&& BPW=32 ${MAKE} clean default \
&& BPW=32 DEFINES="-DREGRESS -DDEBUG" ${MAKE} clean default \
&& BPW=32 DEFINES="-DREGRESS -DDEBUG_ALL -DFULL_DUMP" ${MAKE} clean default \
&& BPW=32 DEFINES="-DALL_SKIP_TO_SW_CASES" ${MAKE} clean default \
&& BPW=32 DEFINES="-DALL_SKIP_TO_SW_CASES -DREGRESS -DDEBUG" \
    ${MAKE} clean default \
&& BPW=32 DEFINES="-DALL_SKIP_TO_SW_CASES -DREGRESS -DDEBUG_ALL -DFULL_DUMP" \
    ${MAKE} clean default \
&& BPW=32 DEFINES="-DNO_LOCATE_GE_KEY_X -DREGRESS -DDEBUG_ALL -DFULL_DUMP" \
    ${MAKE} clean default \
&& :
if [ $? != 0 ]; then echo "non-zero exit"; exit 1; fi

fi # Darwin

echo
echo "=== SUCCESS! ==="
echo

