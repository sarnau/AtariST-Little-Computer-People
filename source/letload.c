/*
 * letload.c -- decompress LETTER.TXT into g_ltlp[] for a_writl().
 *
 * On-disk format:
 *   +0    short   uncompressed_size + 0x11 header bytes
 *   +2    byte    comp_tok[15]  (15 most common bytes)
 *   +17   ...     compressed body (nibble stream; 15 = literal byte escape)
 *
 * Fidelity note: the 1985 code passes the *advanced* fbuffer to Mfree;
 * Alcyon's allocator tolerated that, modern free(3) traps.  We stash
 * the original pointer in fbuffer_orig.
 *
 * addr: fr_reac(), fl_ltpl()
 */

#include "types.h"
#include "enums.h"
#include <osbind.h>
#include "alerts.h"
#include "globals.h"
#include "letload.h"
#include "save.h"


/* outsize is the *uncompressed* byte count (10496 for LETTER.TXT).
   addr: fr_reac() */
void
fr_reac(filename, out_buf, outsize)
char *          filename;
unsigned char * out_buf;
short           outsize;
{
        short           filehandle;
        unsigned short  common_word;
        unsigned char * fbuffer;
        unsigned char * fbuffer_orig;   /* stash for Mfree; loop advances fbuffer */
        short           fsize;
        short           word_index;
        short           count;
        unsigned short  nibble;
        unsigned short  flag;

        {
                unsigned char   sizebuf[2];

                filehandle = fOpen(filename, 0);
                fr_read(filehandle, 2L, sizebuf);
                /* On-disk size word is big-endian (68k native).
                   Reassemble explicitly so the loader works on any
                   host endian; on the ST the assembly is a no-op. */
                fsize = ((short) sizebuf[0] << 8) | sizebuf[1];
        }
        fbuffer = (unsigned char *) Malloc((long) (fsize - 0x11));
        fbuffer_orig = fbuffer;
        if (fbuffer == (unsigned char *) 0)
                er_nomem();

        fr_read(filehandle, 0xfL, comp_tok);
        fr_read(filehandle, (long) (fsize - 0x11), fbuffer);

        flag = 1;
        for (count = 0; count < outsize; count = count + 1) {
                if (flag == 0) {
                        nibble = (unsigned short) *fbuffer;
                        fbuffer = fbuffer + 1;
                } else {
                        nibble = (unsigned short) ((*fbuffer >> 4) & 0x0f);
                }
                nibble = nibble & 0xf;
                flag = (flag == 0) ? 1 : 0;

                if (nibble == 0xf) {
                        /* Escape: next 2 nibbles are a literal byte. */
                        nibble = 0;
                        for (word_index = 0; word_index < 2;
                             word_index = word_index + 1) {
                                if (flag == 0) {
                                        common_word = (unsigned short) *fbuffer;
                                        fbuffer = fbuffer + 1;
                                } else {
                                        common_word = (unsigned short)
                                                ((*fbuffer >> 4) & 0x0f);
                                }
                                nibble = ((common_word & 0xf) | (nibble << 4));
                                flag = (flag == 0) ? 1 : 0;
                        }
                        *out_buf = (unsigned char) nibble;
                } else {
                        *out_buf = comp_tok[nibble];
                }
                out_buf = out_buf + 1;
        }

        Fclose(filehandle);
        Mfree(fbuffer_orig);
}

/* addr: fl_ltpl() */
void
fl_ltpl()
{
        char *  i;
        short   linecount;

        fr_reac("letter.txt",
                             (unsigned char *) g_lttx,
                             10496);

        i = g_lttx;
        for (linecount = 0; linecount < 360; linecount = linecount + 1) {
                g_ltlp[linecount] = i;

                /* Advance past the line body.  Ghidra's compare is a
                   signed-char > 31 (any byte 0x80+ counts as terminator
                   under signed extension); the loop pre-increments once
                   so we walk at least one byte.  LETTER.TXT ships pure
                   7-bit ASCII, so signed vs unsigned is equivalent for
                   the shipped data, but match Ghidra literally. */
                do {
                        i = i + 1;
                } while (*i > 31);

                /* Skip the terminator run. */
                while (*i < ' ')
                        i = i + 1;
        }
}
