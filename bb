
#!/bin/sh

# Regression test script.

set -x # turn echo on

cp b b.bb

# TIME=time

# B="b.bb -1l"
# B="b.bb -1lm"
# B="b.bb -1"
# B="b.bb -1ld"
# B="b.bb -1d"
  B="b.bb -1dgi"

# -x does wait for context switch; useful for timing with small pops
# -l skips small-pop del/ins loop

for SFLAG in "" 1 -7 -1 7
do
# -D (mirror the key after it is generated) has the effect of increasing
# the expanse (-B option) to the full expanse (32 or 64) despite the use
# of -B.  The -B in that case just makes sure all keys are a long ways from
# each other.
# Memory usage blows up with cnBitsPerWord=64 and small cnListPopCntMax.
# I think Doug is going to change the behavior of -D so that it respects -B
# but we have to do something in the meantime.
for DFLAG in "" # -D
do
for BFLAG in "" -B15
do

    if [ "$SFLAG" != "" ]
    then
        CMD="${TIME} $B -s0 -S$SFLAG"
    else
        CMD="${TIME} $B"
    fi

    if ! $CMD $DFLAG $BFLAG -n50000
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

