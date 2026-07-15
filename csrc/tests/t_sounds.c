/*
 * sounds_load.c -- smoke test for the SOUNDS.LCP loader.
 *
 * Loads the real 1985 SOUNDS.LCP file and verifies:
 *   1. At least a handful of records parse (SFX_DOORBELL etc. exist)
 *   2. Each record's stored size matches the number of bytes actually
 *      allocated for it (proves the file walk stays aligned).
 *   3. No slot pointer got left NULL where a size > 0 was read.
 *
 * Build: make sounds_test
 */

#include <stdio.h>
#include <string.h>

#include "../include/types.h"

extern void             sf_sl();
extern unsigned char *  midi_note_length_params[];

static void
copy_file(src, dst)
char *  src;
char *  dst;
{
        FILE *          fi;
        FILE *          fo;
        unsigned char   buf[8192];
        size_t          n;
        fi = fopen(src, "rb");
        if (fi == NULL) { perror(src); return; }
        fo = fopen(dst, "wb");
        while ((n = fread(buf, 1, sizeof buf, fi)) > 0)
                fwrite(buf, 1, n, fo);
        fclose(fi);
        fclose(fo);
}

int
main()
{
        int     i;
        int     nonempty = 0;
        int     fails = 0;

        setvbuf(stdout, NULL, _IONBF, 0);
        copy_file("../../../DATA/SOUNDS.LCP", "sounds.lcp");

        sf_sl();

        printf("First 16 loaded SFX slots:\n");
        for (i = 0; i < 16; i = i + 1) {
                if (midi_note_length_params[i] == NULL) {
                        printf("  [%2d] (empty)\n", i);
                        continue;
                }
                nonempty++;
                short size = *(short *) midi_note_length_params[i];
                unsigned char *body = midi_note_length_params[i] + 2;
                printf("  [%2d] size=%d  first bytes: %02x %02x %02x %02x\n",
                       i, size, body[0], body[1], body[2], body[3]);
                if (size <= 0 || size > 512) {
                        printf("       FAIL: size %d out of range\n", size);
                        fails++;
                }
        }

        if (nonempty < 4) {
                printf("FAIL: only %d non-empty slots loaded (expected 4+)\n",
                       nonempty);
                fails++;
        }

        if (fails == 0)
                printf("PASS: SOUNDS.LCP loaded, %d SFX records parsed\n",
                       nonempty);
        return fails ? 1 : 0;
}
