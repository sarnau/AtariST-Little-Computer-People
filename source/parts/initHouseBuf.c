/*
 * Included by stx_u1.c; never compiled on its own.
 */

/* Sets up the off-screen picture buffer at boot: g_srptr becomes
   scrbufB rounded up to a 512-byte boundary, mf_scrp describes it as a
   320x200 (scaled by scr_scal) bitmap, MFDB_A's NULL address names the
   physical screen, and copyScreen copies what is on screen into the new
   buffer. */
void
initHouseBuf()
{
        /* The buffer size goes through a local that both arms of a
           vestigial if/else set to the same value, and the pointer is
           aligned in the global itself.  The spare locals are unused.
           All of this is the original's shape and must stay. */
        unsigned short  size;
        short           spare1;
        short           spare2;
        short           spare3;

        if (scr_scal == 1)
                size = 0xE800;
        else
                size = 0xE800;
        /* fd_addr = NULL, written as its two halves; copyScreen passes the
           same address.  It is the NULL address that means "the device
           screen" to the VDI. */
        MFDB_A[0] = 0;
        MFDB_A[1] = 0;
        g_srptr = (void *) scrbufB;
        g_srptr = (void *) (((long) g_srptr + 0x200L) & ~0x1FFL);
        initMfdb((long) (size >> 3), &mf_scrp, g_srptr,
                scr_scal * 0x140, scr_scal * 200);
        copyScreen(vdihnd, &mf_scrp);
}
