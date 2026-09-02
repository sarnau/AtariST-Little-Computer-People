/*
 * parts/vst_h20.c -- shared body; LCP_ORG links it in gfx_prim.c,
 * LCP_STX in the 0xdece object (0x75dc, in the games object between mg_stp and rst_vsth).  Files under parts/
 * are never compiled standalone.
 */
void
vst_h20()
{
        short   ta, tb, tc, td;
        vqt_attributes(vdihnd, sv_vqta);
        /* STX passes the four out-pointers in declaration order. */
        vst_height(vdihnd, 20, &ta, &tb, &tc, &td);
}
