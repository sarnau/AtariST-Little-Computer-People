/* Like the other TV routines it carries a 10-short point buffer, of
   which v_pline only uses the first two points.  The declaration order
   of the locals fixes the original's stack frame; keep it. */

void
tvPattern()
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
                        xs = tvBar0X;
                        ys = tvBar0Y;
                }
                if (pattern == 1) {
                        xs = tvBar1X;
                        ys = tvBar1Y;
                }
                if (pattern == 2) {
                        xs = tvBar2X;
                        ys = tvBar2Y;
                }
                if (pattern == 3) {
                        xs = tvBar3X;
                        ys = tvBar3Y;
                }

                for (i = 0; i <= rnd; i++) {
                        pts[0] = xs[i];
                        pts[1] = ys[i];
                        pts[2] = pts[0] + 3;
                        pts[3] = pts[1];
                        beginDraw();
                        vsl_color(vdiHandle,
                                  colorPens[
                                    tvBarColor[pattern]]);
                        v_pline(vdiHandle, 2, pts);
                        endDraw();
                        gameTick(1);
                }
        }
}
