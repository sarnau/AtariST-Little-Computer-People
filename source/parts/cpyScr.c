/*
 * parts/cpyScr.c -- shared body; LCP_STX links it in the 0xdece object
 * (0x64fa, in the 0x400c object). Files under parts/ are never
 * compiled standalone.
 */
/* cpyScr (Ghidra 0x164FA): vro_cpyfm the physbase screen into pdesMFDB.
   Source MFDB_A.fd_addr=NULL is VDI "device screen" -- reads visible
   video RAM.  Mode ALL_WHITE (=0) irrelevant on ST with fd_addr=NULL.
   addr: cpyScr() */

void
cpyScr(handle, pdesMFDB)
short   handle;
MFDB *  pdesMFDB;
{
        short   points[8];

        points[0] = 0;
        points[1] = 0;
        /* STX's own MFDB has unsigned extents (clr.w before the
           load at every use). */
        points[2] = (unsigned short) pdesMFDB->fd_w - 1;
        points[3] = (unsigned short) pdesMFDB->fd_h - 1;
        points[4] = 0;
        points[5] = 0;
        points[6] = (unsigned short) pdesMFDB->fd_w - 1;
        points[7] = (unsigned short) pdesMFDB->fd_h - 1;
        vro_cpyfm(handle, ALL_WHITE, points, &MFDB_A, pdesMFDB);
}
