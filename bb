
#!/bin/sh

# Regression test script.

set -x # turn echo on

cp b b.bb

if [ $1 = -q ]
then
    SLIST='1'
    NOPT=-n10000
else
    SLIST='1 -7 -1 7'
    DLIST=-D
    BLIST=-B16
    NOPT=-n50000
fi

# TIME=time

# B="b.bb -1l"
# B="b.bb -1lm"
# B="b.bb -1"
# B="b.bb -1ld"
# B="b.bb -1d"
  B="b.bb -1dgi"

# -x does wait for context switch; useful for timing with small pops
# -l skips small-pop del/ins loop

for SFLAG in "" $SLIST
do
# -D (mirror the key after it is generated) has the effect of increasing
# the expanse (-B option) to the full expanse (32 or 64) despite the use
# of -B.  The -B in that case just makes sure all keys are a long ways from
# each other.
# Memory usage blows up with cnBitsPerWord=64 and small cnListPopCntMax.
# I think Doug is going to change the behavior of -D so that it respects -B
# but we have to do something in the meantime.
for DFLAG in "" $DLIST # -D (-D doesn't respect -B)
do
# Can't use -B15 with -n50000 because we get a duplicate key.  Will have
# to wait for a fix for Judy1LHTime.
for BFLAG in "" $BLIST
do
    if [ "$SFLAG" != "" ]
    then
        CMD="${TIME} $B -s0 -S$SFLAG"
    else
        CMD="${TIME} $B"
    fi

    if ! $CMD $DFLAG $BFLAG $NOPT
    then
        set +x # turn echo off
        echo
        echo Failure!
        echo
        exit 1
    fi
done
done
done

set +x # turn echo off

echo
echo Success.
echo

