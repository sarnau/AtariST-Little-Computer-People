/*
 * hyber_roundtrip.c -- host-side smoke test for the HYBER save file.
 *
 * Copies DATA/HYBER into the CWD as "hyber", calls lcp_load() to parse
 * it into the PLAYER struct, then lcp_save() to write it back out and
 * verifies the two files are byte-identical.
 *
 * Endian note: on a little-endian host the individual short fields will
 * *print* byte-swapped ("wake=1536" instead of "wake=6") because the
 * PLAYER struct assumes ST-native big-endian, and no swap is applied
 * here.  What we verify is that the raw 128-byte block round-trips
 * bit-for-bit -- that's the contract lcp_save/lcp_load must honour on
 * the ST, and it's the property that carries the file across a real
 * game session.  A future test can add byteswap-aware field readers if
 * we want portable field-level assertions.
 *
 * Build: make hyber_test
 * Run:   from csrc/build/host/, execute ./hyber_test
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/types.h"
#include "../include/structs.h"

extern PLAYER   lcp;
extern short    lcp_load();
extern void     lcp_save();

int
main(argc, argv)
int     argc;
char ** argv;
{
        FILE *          f;
        unsigned char   orig[128];
        unsigned char   round[128];
        int             i;
        int             diff;

        (void) argc;
        (void) argv;

        f = fopen("../../../DATA/HYBER", "rb");
        if (f == NULL) { perror("open DATA/HYBER"); return 1; }
        if (fread(orig, 1, 128, f) != 128) {
                perror("read DATA/HYBER"); return 1;
        }
        fclose(f);

        f = fopen("hyber", "wb");
        if (f == NULL) { perror("open hyber"); return 1; }
        fwrite(orig, 1, 128, f);
        fclose(f);

        if (lcp_load() == 0) {
                fprintf(stderr, "lcp_load returned 0 (file missing?)\n");
                return 2;
        }
        printf("owner   = %.24s\n", lcp.owner_name);
        printf("resident= %.10s\n", lcp.character_name);
        printf("schedule (raw shorts, expect ST big-endian):\n");
        printf("  bedtime=%d wake=%d lunch=%d dinner=%d\n",
               lcp.bedtime_hour, lcp.wake_hour,
               lcp.lunch_hour, lcp.dinner_hour);

        lcp_save("hyber_roundtrip", 128, &lcp);

        f = fopen("hyber_roundtrip", "rb");
        if (f == NULL) { perror("open roundtrip"); return 3; }
        if (fread(round, 1, 128, f) != 128) {
                perror("read roundtrip"); return 3;
        }
        fclose(f);

        if (memcmp(orig, round, 128) == 0) {
                printf("ROUNDTRIP: byte-identical (128/128 bytes)\n");
                return 0;
        }
        diff = 0;
        for (i = 0; i < 128; i = i + 1)
                if (orig[i] != round[i])
                        diff = diff + 1;
        printf("ROUNDTRIP: %d bytes differ (of 128)\n", diff);
        return 4;
}
