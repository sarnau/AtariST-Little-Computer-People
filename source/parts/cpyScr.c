/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* cpyScr: vro_cpyfm the physbase screen into pdesMFDB.
   Source MFDB_A.fd_addr=NULL is VDI "device screen" -- reads visible
   video RAM.  Mode ALL_WHITE (=0) irrelevant on ST with fd_addr=NULL. */

void
cpyScr(handle, pdesMFDB)
short   handle;
MFDB *  pdesMFDB;
{
        short   points[8];

        points[0] = 0;
        points[1] = 0;
        /* The MFDB extents are read as unsigned, as in the original;
           the casts reproduce that. */
        points[2] = (unsigned short) pdesMFDB->fd_w - 1;
        points[3] = (unsigned short) pdesMFDB->fd_h - 1;
        points[4] = 0;
        points[5] = 0;
        points[6] = (unsigned short) pdesMFDB->fd_w - 1;
        points[7] = (unsigned short) pdesMFDB->fd_h - 1;
        vro_cpyfm(handle, ALL_WHITE, points, MFDB_A, pdesMFDB);
}
