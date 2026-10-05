/*
 * Fill an MFDB for a 4-plane buffer.  The address is stored as two
 * words through a (short *) cast, high half first, as the original
 * does.
 */

void
initMfdb(unused, mfdb, addr, width, height)
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
