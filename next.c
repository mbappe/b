// This program attempts to cause Judy to create Bitmap-Branches at levels
// 3, 4 and 5 with one JP pointing to a Leaf-2 and seven JPs with a single
// immediate in them.
// And full uncompressed branches at level 6 where all JPs point to a
// BB5.
// The -UEASY_FOR_PSPLIT variant creates Leaf-2's designed to foil psplit
// search.
// The -DEASY_FOR_PSPLIT variant creates evenly distributed Leaf-2's.

// Judy uncompresses bitmap branches when their population exceeds 135
// (and the whole tree population exceeds 750).
// This allows us to create a max-pop Judy1 Leaf2 of 100 keys
// plus 7 JPs with one immediate each in a BB3,
// then 7 more JPs in a BB4,
// then 7 more JPs in a BB5,
//
// then 7 more JPs in a BB6,
// then 7 more JPs in a BB7
// then one uncompressed branch at the top.
// But that would result in only 256 Leaf2s.
// If we do top three levels of uncompressed branches it would be
// 16 million Leaf2s and a total population of 121*2^24 = 2,030,043,136.
// Top four would be 114 * 2^32.
// Top two would be 128 * 2^16.
// Top one would be 135 * 2^8.
// Top five would be 107 * 2^40.

#include <stdlib.h>
#include <stdio.h>

#if defined(JUDYL)
  #define L2_POP  52 // max pop for JudyL L2 is 52
#else // defined(JUDYL)
  #define L2_POP  100 // max pop for Judy1 L2 is 100
#endif // defined(JUDYL)

unsigned long next(unsigned long current);

unsigned long
next(unsigned long current)
{
    static struct {
        int bStarted; // this machine has been started
        int bContinued;
        unsigned long prev; // previous key for this machine
        unsigned long phase; // current phase for this machine
        unsigned long low; // low bits for this phase
        unsigned long level; // shift amount of low bits for this phase
        unsigned long hi; // high bits for this phase
    } machines[4], *pMachine = &machines[0];

    if ( ! pMachine->bStarted ) {
startNewMachine:
        //printf("start new machine %ld\n", pMachine - machines);
        pMachine->bStarted = 1;
    } else {
        if (current != pMachine->prev) {
            // look for a different machine to continue
            for (unsigned i = 0;
                 i < sizeof(machines) / sizeof(machines[0]); i++)
            {
                pMachine = &machines[i];
                if (pMachine->bStarted) {
                    if (pMachine->prev == current) {
            //printf("continue different machine %ld\n", pMachine - machines);
                        pMachine->bContinued = 1;
                        goto gotMachine;
                    }
                }
            }
            // look for a new machine to start
            for (unsigned i = 0;
                 i < sizeof(machines) / sizeof(machines[0]); i++)
            {
                pMachine = &machines[i];
                if (!pMachine->bStarted) {
                    goto startNewMachine;
                }
            }
            // look for a uncontinued machine to start
            for (unsigned i = 0;
                 i < sizeof(machines) / sizeof(machines[0]); i++)
            {
                pMachine = &machines[i];
                if (!pMachine->bContinued) {
                    //printf("restart machine %ld\n", pMachine - machines);
                    goto restartMachine;
                }
            }
            //printf("all out of machines\n");
            exit(1);
        } else {
            // printf("continue current machine %ld\n", pMachine - machines);
            // continue current machine
        }
    }

gotMachine:

    if (current == 0) {
restartMachine:
        // restart machine
        pMachine->phase = 0;
#if defined(EASY_FOR_PSPLIT)
        pMachine->low = L2_POP / 2;
        pMachine->hi = -1UL; // hi will be incremented before first use
#else // defined(EASY_FOR_PSPLIT)
        pMachine->low = L2_POP;
        pMachine->hi = -1UL; // hi will be incremented before first use
#endif // defined(EASY_FOR_PSPLIT)
        //pMachine->level = 8;
    }

    switch (pMachine->phase) {
    case 0: goto phase0;
    case 1: goto phase1;
    }

    // Generate the numbers that will go in the Leaf-2s.
    // One Leaf-2 for every Bitmap-Branch-3.
    // 0x00|00|00|00|00|00|xx|xx L2
    // 0x00|00|01|00|00|00|xx|xx L2
    // 0x00|00|02|00|00|00|xx|xx L2
    // ...
    // 0xff|ff|ff|00|00|00|xx|xx L2
    //printf("Leaf-2s:\n");
#if defined(EASY_FOR_PSPLIT)
    while (pMachine->low < 1<<16)
#else // defined(EASY_FOR_PSPLIT)
    while (pMachine->low > 0/*L2_POP-2*/)
#endif // defined(EASY_FOR_PSPLIT)
    {
        //printf("pMachine->low 0x%0lx\n", pMachine->low);
        pMachine->hi = 0;
        while (pMachine->hi < 1<<20/*2*/) {
            //printf("pMachine->hi 0x%0lx\n", pMachine->hi);
            pMachine->prev = (pMachine->hi << 40) + pMachine->low;
            // Judy1LHTime can't handle zero.
            if (pMachine->prev == 0) { return next(pMachine->prev); }
            return pMachine->prev;
phase0:;
            ++pMachine->hi;
        }
#if defined(EASY_FOR_PSPLIT)
        pMachine->low += (1<<16) / L2_POP;
#else // defined(EASY_FOR_PSPLIT)
        --pMachine->low; // bunched at one end to foil psplit
#endif // defined(EASY_FOR_PSPLIT)
    }
    //printf("phase0 done\n");

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
    //printf("Immediates in Bitmap-Branches:\n");
    pMachine->phase = 1;
    pMachine->hi = 0;
    while (pMachine->hi < 1<<20/*2*/) {
        pMachine->level = 16;
        while (pMachine->level < 40/*32*/) {
            pMachine->low = 1;
            while (pMachine->low < 8/*3*/) {
                pMachine->prev
                    = (pMachine->hi<<40) + (pMachine->low << pMachine->level);
                // Judy1LHTime can't handle zero.
                if (pMachine->prev == 0) { return next(pMachine->prev); }
                return pMachine->prev;
phase1:;
                ++pMachine->low;
            }
            pMachine->level += 8;
        }
        ++pMachine->hi;
    }
    //printf("phase1 done\n");

    exit(0);
    goto restartMachine;
}

int
main()
{
    unsigned long current0;
    unsigned long current1;
    unsigned long current0saved;

    current0 = 0;
    for (;;) {
        printf("0x%016lx\n", current0 = next(current0));
    }

    current0 = 0;
    printf("insert tit\n");
    for (int i = 0; i < 4; i++) {
        printf("0x%016lx\n", current0 = next(current0));
    }
    current0 = 0;
    printf("insert\n");
    for (int i = 0; i < 4; i++) {
        printf("0x%016lx\n", current0 = next(current0));
    }
    for (int j = 0; j < 8; j++) {
        printf("j %d\n", j);
        current1 = 0;
        printf("lookup tit\n");
        for (int i = 0; i < 4; i++) {
            printf("0x%016lx\n", current1 = next(current1));
        }
        current1 = 0;
        printf("lookup\n");
        for (int i = 0; i < 4; i++) {
            printf("0x%016lx\n", current1 = next(current1));
        }
        current0saved = current0;
        printf("insert tit 0x%lx\n", current0);
        for (int i = 0; i < 4; i++) {
            printf("0x%016lx\n", current0 = next(current0));
        }
        current0 = current0saved;
        printf("insert 0x%lx\n", current0);
        for (int i = 0; i < 4; i++) {
            printf("0x%016lx\n", current0 = next(current0));
        }
    }
}

