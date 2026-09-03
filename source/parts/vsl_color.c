/*
 * parts/vsl_color.c -- shared body.  the VDI binding module is ordered
 * differently in the two revisions, so vdistx.c includes these in
 * LCP_STX order and vdiown.c in LCP_ORG's.
 * Files under parts/ are never compiled standalone.
 */

void
vsl_color(handle, index)
short   handle;
short   index;
{
        /* STX assigns intin first and returns intout[0]; LCP_ORG
           fills contrl first and returns nothing. */
#ifdef FAITHFUL
        contrl[0] = 17;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = index;
        vdi_go();
#else
        intin[0]  = index;
        contrl[0] = 17;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
#endif
}
