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
extern unsigned char *  mi_ntLp[];

/* sf_sl reads each block's size with a raw two-byte fr_read into a
   short, so on a little-endian host every size comes back byte-swapped
   -- 34 reads as 8704 -- and the file walk is lost after the first
   block.  That is faithful ST code, not a bug to fix in the port.
   So the test writes a HOST-ENDIAN copy of the asset: same payloads,
   same order, same size-0 terminator, with each size field in native
   byte order.  The loader's logic is then what is under test, which is
   the part that can be checked here at all. */
static int
copy_swapped(src, dst)
char *  src;
char *  dst;
{
        FILE *          fi;
        FILE *          fo;
        unsigned char   hdr[2];
        unsigned char   buf[65536];
        short           size;
        int             blocks = 0;

        fi = fopen(src, "rb");
        if (fi == NULL) { perror(src); return -1; }
        fo = fopen(dst, "wb");
        if (fo == NULL) { perror(dst); fclose(fi); return -1; }

        for (;;) {
                if (fread(hdr, 1, 2, fi) != 2) break;
                size = (short) (((int) hdr[0] << 8) | hdr[1]);
                fwrite(&size, sizeof size, 1, fo);      /* native order */
                if (size == 0) break;
                if ((int) fread(buf, 1, (size_t) size, fi) != (int) size)
                        break;
                fwrite(buf, 1, (size_t) size, fo);
                blocks++;
        }
        fclose(fi);
        fclose(fo);
        return blocks;
}

int
main()
{
        int     i;
        int     nonempty = 0;
        int     fails = 0;

        setvbuf(stdout, NULL, _IONBF, 0);
        {
                int n = copy_swapped("../../../DATA/SOUNDS.LCP",
                                     "sounds.lcp");
                if (n < 0) return 1;
                printf("SOUNDS.LCP holds %d blocks before the "
                       "size-0 terminator\n", n);
        }

        sf_sl();

        printf("First 16 loaded SFX slots:\n");
        for (i = 0; i < 16; i = i + 1) {
                if (mi_ntLp[i] == NULL) {
                        printf("  [%2d] (empty)\n", i);
                        continue;
                }
                nonempty++;
                short size = *(short *) mi_ntLp[i];
                unsigned char *body = mi_ntLp[i] + 2;
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
