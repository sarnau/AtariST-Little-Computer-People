/*
 * Included by stx_u2.c; never compiled on its own.
 */
/* Like the other TV routines it carries a 10-short point buffer, of
   which v_pline only uses the first two points.  The declaration order
   of the locals fixes the original's stack frame; keep it. */

void
tv_patl()
{
        short *         xs;
        short *         ys;
        short           rnd;
        short           pattern;
        short           i;
        short           pts[10];

        for (pattern = 0; pattern < 4; pattern++) {
                rnd = Random() & 7;
                if (pattern == 0) {
                        xs = g_tp0xc;
                        ys = g_tp0yc;
                }
                if (pattern == 1) {
                        xs = g_tp1xc;
                        ys = g_tp1yc;
                }
                if (pattern == 2) {
                        xs = g_tp2xc;
                        ys = g_tp2yc;
                }
                if (pattern == 3) {
                        xs = g_tp3xc;
                        ys = g_tp3yc;
                }

                for (i = 0; i <= rnd; i++) {
                        pts[0] = xs[i];
                        pts[1] = ys[i];
                        pts[2] = pts[0] + 3;
                        pts[3] = pts[1];
                        sc_sdtb();
                        vsl_color(vdihnd,
                                  vdi_colt[
                                    g_tpcoi[pattern]]);
                        v_pline(vdihnd, 2, pts);
                        sc_sdtf();
                        gameTick(1);
                }
        }
}
