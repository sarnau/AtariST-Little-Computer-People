/*
 * parts/fr_reac.c -- shared body; LCP_ORG links it in letload.c,
 * LCP_STX in the 0xdece object (0x53b8, in the 0x400c object ahead of main).  Files under parts/
 * are never compiled standalone.
 */
/* outsize is the *uncompressed* byte count (10496 for LETTER.TXT).
   addr: fr_reac() */
void
fr_reac(filename, out_buf, outsize)
char *          filename;
unsigned char * out_buf;
short           outsize;
{
        /* STX's frame is -20: seven locals in this order, and no
           `fbuffer_orig` -- the Mfree at the end frees the pointer the
           loop has already advanced. */
        short           flag;
        short           nibble;
        short           count;
        short           word_index;
        short           fsize;
        unsigned char * fbuffer;
        short           filehandle;

        filehandle = fOpen(filename, 0);
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
