
#!/bin/sh

# Regression test script.

set -x # turn echo on

cp t b.bb

#B="b.bb -1l"
#B="b.bb -1lm"
#B="b.bb -1"
#B="b.bb -1ld"
 B="b.bb -1d"

   time $B  -s0 -S3          -n100000 \
&& time $B -s0 -S1  -D      -n100000 \
&& time $B -s0 -S-1         -n100000 \
&& time $B -s0 -S-3 -D      -n100000 \
&& time $B             -B15 -n100000 \
&& time $B                  -n100000 \
&& time $B -s0 -S-3         -n100000 \
&& time $B -s0 -S-1 -D      -n100000 \
&& time $B -s0 -S1          -n100000 \
&& time $B -s0 -S3  -D      -n100000 \
&& time $B -s0 -s1          -n-1 \
&& time $B -s0 -s3  -D      -n-1 \

#&& time $B -s0 -S3          -n-1 \
#&& time $B -s0 -s-1         -n-1 \
#&& time $B -s0 -S-3         -n-1
#&& time $B                  -n-1 \

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

echo '\a\a\a\a\a\a\a\a\a'

