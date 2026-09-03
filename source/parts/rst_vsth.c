/*
 * parts/rst_vsth.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x761e, in the games object right before initVdi). Files
 * under parts/ are never compiled standalone.
 */
void
rst_vsth()
{
        short   ta, tb, tc, td;
        vst_height(vdihnd, sv_vqta[7], &ta, &tb, &tc, &td);
}
