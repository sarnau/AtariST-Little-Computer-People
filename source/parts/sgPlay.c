/*
 * parts/sgPlay.c -- shared body; LCP_STX puts it at 0xd9ea, the first
 * function of the 0xdece object, ahead of sf_irqp.  Files under
 * parts/ are never compiled standalone.
 */
/* sgPlay: load a .sng/.org from disk (10-byte Music Studio 2.0 header,
   then up to 20000 bytes of sequence data) and hand it to mq_inis.
   addr: sgPlay() */


void
sgPlay(filename)
char *  filename;
{
        /* STX: link #-22 -- fhnd at -2, an unwritten slot at -4,
           temp at -14 and dta_ptr at -18. */
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

        Fsfirst(filename, 0);
        dta_ptr = (_DTA *) Fgetdta();
        mi_sbuf = (char *) Malloc(dta_ptr->d_length);
        if (mi_sbuf == (char *) 0)
                er_nomem();

        fhnd = fOpen(filename, 0);
        if (fhnd >= 0) {
                fr_read(fhnd, 10L, temp);
                fr_read(fhnd, 20000L, mi_sbuf);
                Fclose(fhnd);
        }
        mq_inis(mi_sbuf, g_momap);
}
