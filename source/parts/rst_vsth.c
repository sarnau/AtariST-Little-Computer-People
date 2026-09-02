/*
 * parts/rst_vsth.c -- shared body; LCP_ORG links it in gfx_prim.c,
 * LCP_STX in the 0xdece object (0x761e, in the games object right before initVdi).  Files under parts/
 * are never compiled standalone.
 */
void
rst_vsth()
{
        short   ta, tb, tc, td;
#ifdef FAITHFUL
        vst_height(vdihnd, sv_vqta[7], &td, &tc, &tb, &ta);
#else
        vst_height(vdihnd, sv_vqta[7], &ta, &tb, &tc, &td);
#endif
}
