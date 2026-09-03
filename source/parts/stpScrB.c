/*
 * parts/stpScrB.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x6576, in the 0x400c object). Files under parts/ are never
 * compiled standalone.
 */

void
stpScrB()
{
        /* STX: the buffer size goes through a local that both arms of
           a vestigial if/else set to the same value, the pointer is
           aligned in the global itself, and only the MFDB extents are
           cleared. */
        unsigned short  size;   /* link #-12: three more slots follow */
        short           spare1;
        short           spare2;
        short           spare3;

        if (scr_scal == 1)
                size = 0xE800;
        else
                size = 0xE800;
        MFDB_A.fd_w = 0;
        MFDB_A.fd_h = 0;
        g_srptr = (void *) scrbufB;
        g_srptr = (void *) (((long) g_srptr + 0x200L) & ~0x1FFL);
        sp_iniM((long) (size >> 3), &mf_scrp, g_srptr,
                scr_scal * 0x140, scr_scal * 200);
        cpyScr(vdihnd, &mf_scrp);
}
