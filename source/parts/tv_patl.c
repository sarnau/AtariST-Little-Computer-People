/*
 * parts/tv_patl.c -- shared body; LCP_STX links it in the 0xdece
 * object at 0x13204, before tv_boul. Files under parts/ are never
 * compiled standalone.
 */
/* addr: tv_patl() (ROM 0xd53c).  As in tv_boul, v_pline count=2 with
   one initialised point at &x: point 2 reads the rnd local and the
   saved frame pointer -- the ROM's exact layout, reproduced here
   (-2 rnd, -4 y, -6 x, -8 i, -10 pattern, -14 xs, -18 ys). */

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
