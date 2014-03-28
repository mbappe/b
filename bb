
#!/bin/sh

# Regression test script.

set -x # turn echo on

cp b b.bb

# TIME=time

  B="b.bb -1l"
# B="b.bb -1lm"
# B="b.bb -1"
# B="b.bb -1ld"
# B="b.bb -1d"
# B="b.bb -1dgi"

# -x does wait for context switch; useful for timing with small pops
# -l skips small-pop del/ins loop

for SFLAG in 1 7 -1 -7 ""
do
for DFLAG in -D ""
do
for BFLAG in -B15 ""
do
    if [ "$SFLAG" != "" ]
    then
        X="${TIME} $B -s0 -S$SFLAG"
    else
        X="${TIME} $B"
    fi

    if ! $X $DFLAG $BFLAG -n20000
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

