/*
 * letload.c -- decompress LETTER.TXT and index it into
 *                  g_ltlp[] for a_writl().
 *
 * fr_reac: nibble-based token decoder.  The on-disk file
 * format is:
 *   +0    short   uncompressed_size + 0x11 header bytes
 *   +2    byte    comp_tok[15]  (15 most common bytes)
 *   +17   ...     compressed body
 *
 * Body decode: read 4 bits at a time (high nibble of each byte first,
 * then the low nibble; state kept in `flag`).  For each nibble:
 *   nibble in 0..14  ->  emit comp_tok[nibble]
 *   nibble == 15     ->  read two more nibbles for a literal 8-bit byte
 *
 * fl_ltpl: after decompression, g_lttx
 * is a stream of newline-terminated (well, control-char-terminated)
 * strings.  We walk it exactly 360 times, recording the start of each
 * line into g_ltlp[i] and then skipping past the terminator
 * run (any bytes < ' ') to the start of the next line.
 *
 * Fidelity note: the 1985 code passes the *advanced* fbuffer to
 * GEMDOS Mfree at the end -- Alcyon's ST allocator tolerated freeing
 * from anywhere inside the block, but modern free(3) traps on that.
 * We stash the original pointer in fbuffer_orig and free that.  The
 * visible behaviour is unchanged.
 *
 * addr: fr_reac(), fl_ltpl()
 */

#include "types.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern char *   g_lttx;
extern char *   g_ltlp[];
extern unsigned char comp_tok[];
#include <osbind.h>

extern void     fr_read();
extern short    fOpen();
extern void     er_nomem();

/* fr_reac: decode a token-compressed file into out_buf.
   outsize is the *uncompressed* byte count (10496 for LETTER.TXT).
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
        fbuffer = (unsigned char *) _gemdos(GEMDOS_Malloc,
                                            (long) (fsize - 0x11),
                                            0L, 0L);
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

        _gemdos(GEMDOS_Fclose, filehandle,    0L, 0L);
        _gemdos(GEMDOS_Mfree,  (long) fbuffer_orig,  0L, 0L);
}

/* fl_ltpl: decompress LETTER.TXT into
   g_lttx and populate the 360-entry g_ltlp[]
   line-start table.  Called by a_writl after the buffer
   is allocated.

   Line terminator: any control byte (< ' ', which is 0x20).  There may
   be multiple consecutive terminators (typical is CR+LF); we skip past
   the whole run before recording the next line's start.

   addr: fl_ltpl() */

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

                /* Advance past the line body -- anything with a
                   printable byte value.  0x21 = '!' matches the
                   original comparison (> 31 means >= 32; the loop
                   pre-increments once so we walk at least one byte). */
                do {
                        i = i + 1;
                } while ((unsigned char) *i > 31);

                /* Skip the terminator run. */
                while ((unsigned char) *i < ' ')
                        i = i + 1;
        }
}
