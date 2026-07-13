/*
 * letter_load.c -- decompress LETTER.TXT and index it into
 *                  letter_line_ptr[] for action_write_letter().
 *
 * file_read_compressed: nibble-based token decoder.  The on-disk file
 * format is:
 *   +0    short   uncompressed_size + 0x11 header bytes
 *   +2    byte    compression_tokens[15]  (15 most common bytes)
 *   +17   ...     compressed body
 *
 * Body decode: read 4 bits at a time (high nibble of each byte first,
 * then the low nibble; state kept in `flag`).  For each nibble:
 *   nibble in 0..14  ->  emit compression_tokens[nibble]
 *   nibble == 15     ->  read two more nibbles for a literal 8-bit byte
 *
 * file_load_letter_template: after decompression, letter_txt_content
 * is a stream of newline-terminated (well, control-char-terminated)
 * strings.  We walk it exactly 360 times, recording the start of each
 * line into letter_line_ptr[i] and then skipping past the terminator
 * run (any bytes < ' ') to the start of the next line.
 *
 * Fidelity note: the 1985 code passes the *advanced* fbuffer to
 * GEMDOS Mfree at the end -- Alcyon's ST allocator tolerated freeing
 * from anywhere inside the block, but modern free(3) traps on that.
 * We stash the original pointer in fbuffer_orig and free that.  The
 * visible behaviour is unchanged.
 *
 * addr: file_read_compressed(), file_load_letter_template()
 */

#include "types.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern void     file_read();
extern short    file_open();
extern void     error_not_enough_memory();

/* file_read_compressed: decode a token-compressed file into outbuffer.
   outsize is the *uncompressed* byte count (10496 for LETTER.TXT).
   addr: file_read_compressed() */

void
file_read_compressed(filename, outbuffer, outsize)
char *          filename;
unsigned char * outbuffer;
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

                filehandle = file_open(filename, 0);
                file_read(filehandle, 2L, sizebuf);
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
                error_not_enough_memory();

        file_read(filehandle, 0xfL, compression_tokens);
        file_read(filehandle, (long) (fsize - 0x11), fbuffer);

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
                        *outbuffer = (unsigned char) nibble;
                } else {
                        *outbuffer = compression_tokens[nibble];
                }
                outbuffer = outbuffer + 1;
        }

        _gemdos(GEMDOS_Fclose, (long) filehandle,    0L, 0L);
        _gemdos(GEMDOS_Mfree,  (long) fbuffer_orig,  0L, 0L);
}

/* file_load_letter_template: decompress LETTER.TXT into
   letter_txt_content and populate the 360-entry letter_line_ptr[]
   line-start table.  Called by action_write_letter after the buffer
   is allocated.

   Line terminator: any control byte (< ' ', which is 0x20).  There may
   be multiple consecutive terminators (typical is CR+LF); we skip past
   the whole run before recording the next line's start.

   addr: file_load_letter_template() */

void
file_load_letter_template()
{
        char *  i;
        short   linecount;

        file_read_compressed("letter.txt",
                             (unsigned char *) letter_txt_content,
                             10496);

        i = letter_txt_content;
        for (linecount = 0; linecount < 360; linecount = linecount + 1) {
                letter_line_ptr[linecount] = i;

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
