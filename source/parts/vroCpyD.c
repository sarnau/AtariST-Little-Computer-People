/*
 * parts/vroCpyD.c -- shared body; LCP_STX puts vroCpyD in the 0x400c
 * object at 0x63cc, between hs_posXY and al_loal -- not with the VDI
 * bindings it wraps.  Files under parts/ are never compiled
 * standalone.
 */
/* addr: vroCpyD() (ROM 0xd8d2) -- discrete-argument vro_cpyfm. */
void
vroCpyD(handle, mode, src, dst, sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2)
short   handle;
short   mode;
long    src;
long    dst;
short   sx1;
short   sy1;
short   sx2;
short   sy2;
short   dx1;
short   dy1;
short   dx2;
short   dy2;
{
        /* Only a wrapper: it builds a pxy array on the stack and
           defers to the array-form vro_cpyfm in vdistx.c. */
        short   pxy[8];

        pxy[0] = sx1;
        pxy[1] = sy1;
        pxy[2] = sx2;
        pxy[3] = sy2;
        pxy[4] = dx1;
        pxy[5] = dy1;
        pxy[6] = dx2;
        pxy[7] = dy2;
        vro_cpyfm(handle, mode, pxy, src, dst);
}
