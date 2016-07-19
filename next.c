// This program creates Bitmap-Branches at levels 3, 4 and 5 with one
// JP pointing to a Leaf-2 of 114 keys and seven JPs with a single
// immediate in them.
// And full uncompressed branches at level 6 where all JPs point to a
// BB5.
// The commented-out variant creates Leaf-2's designed to foil psplit
// search.
// The uncommented-out variant creates evenly distributed Leaf-2's.
#include <stdlib.h>
#include <stdio.h>

unsigned long next(unsigned long current);

unsigned long
next(unsigned long current)
{
    static int bStarted = 0;
    static unsigned long first;
    static unsigned long phase;
    static unsigned long low;
    static unsigned long hi;
    static unsigned long level;

    if (bStarted == 0) {
        bStarted = 1;
        first = current;
    }

    if (current == first) {
        phase = 0;
        low = 0;
        hi = -1UL;
        level = 8;
    }

    switch (phase) {
    case 0: goto phase0;
    case 1: goto phase1;
    case 2: goto phase2;
    case 3: goto phase3;
    }

    // Generate the numbers that will go in the Leaf-2s.
    // One Leaf-2 for every JP pointing to a Bitmap-Branch-3.
    // 0x00|00|00|00|00|00|xx|xx L2
    // 0x00|00|01|00|00|00|xx|xx L2
    // 0x00|00|02|00|00|00|xx|xx L2
    // ...
    // 0xff|ff|ff|00|00|00|xx|xx L2
    // printf("Leaf-2s:\n");
    //for (low = 113; low <= 113; --low)
    for (low = 0; low < 1<<16; low += (1<<16) / 113)
    {
//printf("low 0x%0lx\n", low);
        for (hi = 0; hi < 1<<20; ++hi) {
//printf("hi 0x%0lx\n", hi);
            unsigned long num = (hi << 40) + low;
            if (num == 0) { continue; } // Judy1LHTime can't handle zero.
            return num;
phase0:;
//printf("phase0 hi 0x%0lx\n", hi);
        }
//printf("phase0 low 0x%0lx\n", low);
    }
//printf("phase0 done\n");
    phase = 1;
    // Generate the numbers that will go in the Bitmap-Branch immediates.
    // JP 0 is used for Leaf-2.
    // 0x00|00|00|00|00|01|00|00 BB3IMM
    // 0x00|00|00|00|00|07|00|00 BB3IMM
    // 0x00|00|00|00|01|00|00|00 BB4IMM
    // 0x00|00|00|00|07|00|00|00 BB4IMM
    // 0x00|00|00|01|00|00|00|00 BB5IMM
    // 0x00|00|00|07|00|00|00|00 BB5IMM
    // 0x00|00|01|00|00|00|00|00 BB3IMM
    // 0x00|00|01|00|00|07|00|00 BB3IMM
    // 0x00|00|01|00|01|00|00|00 BB4IMM
    // 0x00|00|01|00|07|00|00|00 BB4IMM
    // 0x00|00|01|01|00|00|00|00 BB5IMM
    // 0x00|00|01|07|00|00|00|00 BB5IMM
    // ...
    // 0xff|ff|ff|00|00|00|00|00 BB3IMM
    // 0xff|ff|ff|00|00|07|00|00 BB3IMM
    // 0xff|ff|ff|00|01|00|00|00 BB4IMM
    // 0xff|ff|ff|00|07|00|00|00 BB4IMM
    // 0xff|ff|ff|01|00|00|00|00 BB5IMM
    // 0xff|ff|ff|07|00|00|00|00 BB5IMM
    // printf("Immediates in Bitmap-Branches:\n");
    for (hi = 0; hi < 1<<20; ++hi) {
        for (level = 16; level <= 32; level += 8) {
            for (low = 1; low <= 7; ++low) {
                return (hi<<40) + (low << level);
phase1:;
            }
        }
    }
    //exit(0);
    phase = 2;
    //for (low = 113; low <= 113; --low)
    for (low = 0; low < 1<<16; low += (1<<16) / 113)
    {
        for (hi = 1<<20; hi < 1<<24; ++hi) {
            unsigned long num = (hi << 40) + low;
            return num;
phase2:;
        }
    }
    phase = 3;
    for (hi = 1<<20; hi < 1<<24; ++hi) {
        for (level = 16; level <= 32; level += 8) {
            for (low = 1; low <= 7; ++low) {
                return (hi<<40) + (low << level);
phase3:;
            }
        }
    }
    exit(0);
}

int
main()
{
    unsigned long current = 0;
    for (;;) { printf("0x%016lx\n", current = next(current)); }
}

