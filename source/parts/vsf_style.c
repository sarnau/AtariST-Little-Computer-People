/*
 * parts/vsf_style.c -- shared body.  the VDI binding module is ordered
 * differently in the two revisions, so vdistx.c includes these in
 * LCP_STX order and vdiown.c in LCP_ORG's.
 * Files under parts/ are never compiled standalone.
 */

void
vsf_style(handle, style)
short   handle;
short   style;
{
#ifdef FAITHFUL
        contrl[0] = 24;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = style;
        vdi_go();
#else
        intin[0]  = style;
        contrl[0] = 24;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
#endif
}
