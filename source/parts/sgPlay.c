/*
 * Included by stx_u4.c; never compiled on its own.
 */
/* sgPlay: load a .sng/.org from disk (10-byte Music Studio 2.0 header,
   then up to 20000 bytes of sequence data) and hand it to mq_inis. */


void
sgPlay(filename)
char *  filename;
{
        /* `unused` is never written but must stay: removing it changes
           the compiled code. */
        short           fhnd;
        short           unused;
        unsigned char   temp[10];
        _DTA *   dta_ptr;

        mi_slop = YES;
        mi_varR          = YES;

        if (mi_play != NO) {
                mq_inis(mi_sbuf, g_momap);
                while (mi_play != NO)
                        ;
        }
        if (mi_sbuf != (char *) 0) {
                Mfree(mi_sbuf);
                mi_sbuf = (char *) 0;
        }

        Fsfirst(filename, F_NORMAL);
        dta_ptr = (_DTA *) Fgetdta();
        mi_sbuf = (char *) Malloc(dta_ptr->d_length);
        if (mi_sbuf == (char *) 0)
                er_nomem();

        fhnd = fOpen(filename, RMODE_RD);
        if (fhnd >= 0) {
                fr_read(fhnd, 10L, temp);
                fr_read(fhnd, 20000L, mi_sbuf);
                Fclose(fhnd);
        }
        mq_inis(mi_sbuf, g_momap);
}
