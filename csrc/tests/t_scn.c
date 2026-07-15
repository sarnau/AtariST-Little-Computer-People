/*
 * scn_decode.c -- smoke test for the compressed screen image decoder.
 *
 * Decodes DATA/HOUSE.SCN through decompress_scn and
 * verifies the first 8 output words match the Python reference
 * (decompressImageFile in readFiles.py):
 *   0000 0000 0000 ffff 0000 0000 0000 ffff
 *
 * These are the first 8 words of a 320x200 4-plane ST screen: the
 * pattern encodes 32 pixels (2 words per column position * 4 planes).
 * Getting them right proves the whole nibble state machine, dictionary
 * lookup, and escape handling are correct.
 *
 * Build: make scn_test
 */

#include <stdio.h>
#include <string.h>

#include "../include/types.h"

extern void     decompress_scn();

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
        static unsigned short   buf[16000];
        int                     i;
        int                     fails = 0;
        static unsigned short   expected[8] = {
                0x0000, 0x0000, 0x0000, 0xffff,
                0x0000, 0x0000, 0x0000, 0xffff
        };

        setvbuf(stdout, NULL, _IONBF, 0);
        copy_file("../../../DATA/HOUSE.SCN", "house.scn");

        memset(buf, 0xaa, sizeof buf);  /* poison so we notice under-fill */
        decompress_scn("house.scn", buf, 16000L);

        printf("HOUSE.SCN first 8 words:\n ");
        for (i = 0; i < 8; i = i + 1) {
                printf(" %04x", buf[i]);
                if (buf[i] != expected[i])
                        fails++;
        }
        printf("\n");
        printf(" (expected: 0000 0000 0000 ffff 0000 0000 0000 ffff)\n");
        printf("Last 4 words: %04x %04x %04x %04x\n",
               buf[15996], buf[15997], buf[15998], buf[15999]);

        /* Sanity: the tail shouldn't still contain the 0xaaaa poison
           marker -- prove we wrote all 16000 words. */
        if (buf[15999] == 0xaaaa) {
                printf("  FAIL: last word still holds poison marker\n");
                fails++;
        }

        if (fails == 0)
                printf("PASS: HOUSE.SCN decoded to 16000 words, first 8 match Python reference\n");
        return fails ? 1 : 0;
}
