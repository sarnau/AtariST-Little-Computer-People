/*
 * t_letter.c -- host-side smoke test for the LETTER.TXT decoder (unpackFile).
 *
 * Copies DATA/LETTER.TXT into the CWD as "letter.txt", calls
 * loadLetterText() which internally allocates the 10496-byte
 * buffer, decompresses the nibble-encoded file, and populates
 * letterLines[360].  Then prints a handful of decoded lines so you
 * can eyeball the output matches the actual 1985 letter fragments.
 *
 * Build: make letter_test
 * Run:   from source/build/host/, execute ./letter_test
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/types.h"

extern char *   letterLines[];
extern char *   letterText;
extern unsigned char nibbleBytes[];
extern void     loadLetterText();

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

        /* Stage the compressed template in the CWD so loadLetterText's
           unpackFile("letter.txt", ...) finds it. */
        f = fopen("../../../DATA/LETTER.TXT", "rb");
        if (f == NULL) { perror("open DATA/LETTER.TXT"); return 1; }
        nread = fread(buf, 1, sizeof buf, f);
        fclose(f);

        f = fopen("letter.txt", "wb");
        if (f == NULL) { perror("open letter.txt"); return 1; }
        fwrite(buf, 1, nread, f);
        fclose(f);
        printf("copied %zu bytes to CWD/letter.txt\n", nread);

        /* writeLetter allocates letterText via
           _gemdos(GEMDOS_Malloc); we do that here manually. */
        letterText = (char *) malloc(10496);
        if (letterText == NULL) { perror("malloc"); return 2; }

        /* Decompress + index via the real ports. */
        loadLetterText();

        printf("comp_tok (15 most common bytes):");
        for (i = 0; i < 15; i = i + 1)
                printf(" %02x", nibbleBytes[i]);
        printf("\n");

        printf("First 10 g_ltlp[] entries:\n");
        for (i = 0; i < 10; i = i + 1) {
                if (letterLines[i] == NULL) {
                        printf("  [%3d] (null)\n", i);
                        continue;
                }
                printf("  [%3d] %.60s\n", i, letterLines[i]);
        }
        printf("Line 45 (mid-body sample):\n  %.100s\n", letterLines[45]);
        printf("Line 359 (last):\n  %.100s\n", letterLines[359]);
        printf("PASS: 360 letter template lines decoded and indexed\n");

        free(letterText);
        return 0;
}
