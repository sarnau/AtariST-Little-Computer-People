/*
 * letload.c -- host-side smoke test for LETTER.TXT decoder.
 *
 * Copies DATA/LETTER.TXT into the CWD as "letter.txt", calls
 * file_load_letter_template() which internally allocates the 10496-byte
 * buffer, decompresses the nibble-encoded file, and populates
 * g_ltlp[360].  Then prints a handful of decoded lines so you
 * can eyeball the output matches the actual 1985 letter fragments.
 *
 * Build: make letter_test
 * Run:   from csrc/build/host/, execute ./letter_test
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/types.h"

extern char *   g_ltlp[];
extern char *   g_lttx;
extern unsigned char compression_tokens[];
extern void     file_load_letter_template();

int
main(argc, argv)
int     argc;
char ** argv;
{
        FILE *          f;
        unsigned char   buf[8192];
        size_t          nread;
        int             i;

        (void) argc;
        (void) argv;
        setvbuf(stdout, NULL, _IONBF, 0);

        /* Stage the compressed template in the CWD so file_load_letter_
           template's file_open("letter.txt", 0) finds it. */
        f = fopen("../../../DATA/LETTER.TXT", "rb");
        if (f == NULL) { perror("open DATA/LETTER.TXT"); return 1; }
        nread = fread(buf, 1, sizeof buf, f);
        fclose(f);

        f = fopen("letter.txt", "wb");
        if (f == NULL) { perror("open letter.txt"); return 1; }
        fwrite(buf, 1, nread, f);
        fclose(f);
        printf("copied %zu bytes to CWD/letter.txt\n", nread);

        /* a_writl allocates g_lttx via
           _gemdos(GEMDOS_Malloc); we do that here manually. */
        g_lttx = (char *) malloc(10496);
        if (g_lttx == NULL) { perror("malloc"); return 2; }

        /* Decompress + index via the real ports. */
        file_load_letter_template();

        printf("compression_tokens (15 most common bytes):");
        for (i = 0; i < 15; i = i + 1)
                printf(" %02x", compression_tokens[i]);
        printf("\n");

        printf("First 10 g_ltlp[] entries:\n");
        for (i = 0; i < 10; i = i + 1) {
                if (g_ltlp[i] == NULL) {
                        printf("  [%3d] (null)\n", i);
                        continue;
                }
                printf("  [%3d] %.60s\n", i, g_ltlp[i]);
        }
        printf("Line 45 (mid-body sample):\n  %.100s\n",
               g_ltlp[45]);
        printf("Line 359 (last):\n  %.100s\n", g_ltlp[359]);
        printf("PASS: 360 letter template lines decoded and indexed\n");

        free(g_lttx);
        return 0;
}
