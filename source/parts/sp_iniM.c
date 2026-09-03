/*
 * parts/sp_iniM.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x6612, right after stpScrB). Files under parts/ are never
 * compiled standalone.
 */

void
sp_iniM(unused, mfdb, addr, width, height)
long    unused;
MFDB *  mfdb;
void *  addr;
short   width;
short   height;
{
        long    a;
        long    hi;

        a  = (long) addr;
        hi = a & 0xffff0000L;
        ((short *) mfdb)[0] = hi >> 16;
        ((short *) mfdb)[1] = a;
        mfdb->fd_w       = width;
        mfdb->fd_h       = height;
        mfdb->fd_wdwidth = width / 16;
        mfdb->fd_stand   = 0;
        mfdb->fd_nplanes = 4;
}
