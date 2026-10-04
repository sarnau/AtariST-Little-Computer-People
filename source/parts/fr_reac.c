/*
 * Sits ahead of main.
 *
 * Included by stx_u1.c; never compiled on its own.
 */
/* Decompress LETTER.TXT into out_buf for the letter writer.
   File layout: a short holding the uncompressed size + 0x11 header
   bytes, then the 15 most common bytes, then a nibble stream where
   nibbles 0..14 pick one of those bytes and 15 escapes to a literal
   byte.  outsize is the *uncompressed* byte count (10496 for
   LETTER.TXT). */
void
fr_reac(filename, out_buf, outsize)
char *          filename;
unsigned char * out_buf;
short           outsize;
{
        /* Seven locals, declared in this order.  There is no copy of
           the original buffer pointer, so the Mfree at the end frees
           the pointer the loop has already advanced.  1985 code, kept
           on purpose. */
        short           flag;
        short           nibble;
        short           count;
        short           word_index;
        short           fsize;
        unsigned char * fbuffer;
        short           filehandle;

        filehandle = fOpen(filename, RMODE_RD);
        /* The size word is read straight into the short. */
        fr_read(filehandle, 2L, &fsize);

        fbuffer = (unsigned char *) Malloc((long) (fsize - 0x11));
        if (fbuffer == (unsigned char *) 0)
                er_nomem();

        fr_read(filehandle, 0xfL, comp_tok);
        fr_read(filehandle, (long) (fsize - 0x11), fbuffer);

        flag = 1;
        for (count = 0; count < outsize; count++) {
                if (flag != 0) {
                        nibble = (*fbuffer >> 4) & 0x0f;
                } else {
                        nibble = *fbuffer & 0x0f;
                        fbuffer++;
                }
                flag = (flag != 0) ? 0 : 1;

                if (nibble != 0xf) {
                        *out_buf = comp_tok[nibble];
                        out_buf++;
                } else {
                        /* Escape: the next 2 nibbles are a literal. */
                        nibble = 0;
                        for (word_index = 0; word_index < 2;
                             word_index++) {
                                nibble = nibble << 4;
                                if (flag != 0) {
                                        nibble |= (*fbuffer >> 4) & 0x0f;
                                } else {
                                        nibble |= *fbuffer & 0x0f;
                                        fbuffer++;
                                }
                                flag = (flag != 0) ? 0 : 1;
                        }
                        *out_buf = nibble;
                        out_buf++;
                }
        }

        Fclose(filehandle);
        Mfree(fbuffer);
}
