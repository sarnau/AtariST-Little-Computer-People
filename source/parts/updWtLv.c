/*
 * parts/updWtLv.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x122fa, immediately after a_drink). Files under parts/ are
 * never compiled standalone.
 */


void
updWtLv(val)
short   val;
{
        short   pts[4];
        short   y;

        pts[0] = 146;
        pts[1] = 174;
        pts[2] = pts[0] + 13;
        pts[3] = pts[1];

        if (val == 0) {
                /* Draw filled portion (colour 0x0D). */
                y = lcp_watr;
                sc_sdtb();
                while (y-- != 0) {
                        pts[1] = 174 - y;
                        pts[3] = pts[1];
                        vsl_color(vdihnd, vdi_colt[0xd]);
                        v_pline(vdihnd, 2, pts);
                }
                sc_sdtf();

                /* Draw empty portion (colour 0x0C). */
                y = lcp_watr;
                sc_sdtb();
                while (y++ < 10) {
                        pts[1] = 174 - (y - 1);
                        pts[3] = pts[1];
                        vsl_color(vdihnd, vdi_colt[0xc]);
                        v_pline(vdihnd, 2, pts);
                }
                sc_sdtf();
                return;
        }

        if (val > 0) {
                /* Fill val steps (capped at 10). */
                while (val != 0 && lcp_watr <= 10) {
                        if (++lcp_watr > 10) {
                                lcp_watr--;
                                break;
                        }
                        pts[1] = 174 - (lcp_watr - 1);
                        pts[3] = pts[1];
                        sc_sdtb();
                        vsl_color(vdihnd, vdi_colt[0xd]);
                        v_pline(vdihnd, 2, pts);
                        sc_sdtf();
                        val--;
                }
        } else {
                /* Drain -val steps. */
                while (lcp_watr != 0 && val != 0) {
                        pts[1] = 174 - (lcp_watr - 1);
                        pts[3] = pts[1];
                        sc_sdtb();
                        vsl_color(vdihnd, vdi_colt[0xc]);
                        v_pline(vdihnd, 2, pts);
                        sc_sdtf();
                        gameTick(4);
                        lcp_watr--;
                        val++;
                }
        }
}
