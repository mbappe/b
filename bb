
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

   ${TIME} $B -s0 -S3          -n100000 \
&& ${TIME} $B -s0 -S1  -D      -n100000 \
&& ${TIME} $B -s0 -S-1         -n100000 \
&& ${TIME} $B -s0 -S-3 -D      -n100000 \
&& ${TIME} $B             -B15 -n100000 \
&& ${TIME} $B                  -n100000 \
&& ${TIME} $B -s0 -S-3         -n100000 \
&& ${TIME} $B -s0 -S-1 -D      -n100000 \
&& ${TIME} $B -s0 -S1          -n100000 \
&& ${TIME} $B -s0 -S3  -D      -n100000 \
#&& ${TIME} $B -s0 -s1          -n-1 \
#&& ${TIME} $B -s0 -s3  -D      -n-1 \

#&& ${TIME} $B -s0 -S3          -n-1 \
#&& ${TIME} $B -s0 -s-1         -n-1 \
#&& ${TIME} $B -s0 -S-3         -n-1
#&& ${TIME} $B                  -n-1 \

STATUS=$?

set +x # turn echo off

if [ $STATUS -eq 0 ]
then
    echo
    echo Success.
    echo
else
    echo
    echo Failure!
    echo
fi

