/*
 * Included by stx_u2.c; never compiled on its own.
 */


/* Draws and changes the water tank level lcp_watr, one horizontal
   line per unit at x 146..159 counting up from y 174.  val 0 redraws
   the whole tank (blue up to the level, light grey above it, at boot);
   a positive val adds that many units, stopping at WATER_MAX (the
   Ctrl-W key); a negative val drains -val units one at a time with a
   short pause each (the resident drinking). */
void
updateWaterTank(val)
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
                beginDraw();
                while (y-- != 0) {
                        pts[1] = 174 - y;
                        pts[3] = pts[1];
                        vsl_color(vdihnd, vdi_colt[COLOR_blue]);
                        v_pline(vdihnd, 2, pts);
                }
                endDraw();

                /* Draw empty portion (colour 0x0C). */
                y = lcp_watr;
                beginDraw();
                while (y++ < WATER_MAX) {
                        pts[1] = 174 - (y - 1);
                        pts[3] = pts[1];
                        vsl_color(vdihnd, vdi_colt[COLOR_lt_grey]);
                        v_pline(vdihnd, 2, pts);
                }
                endDraw();
                return;
        }

        if (val > 0) {
                /* Fill val steps (capped at 10). */
                while (val != 0 && lcp_watr <= WATER_MAX) {
                        if (++lcp_watr > WATER_MAX) {
                                lcp_watr--;
                                break;
                        }
                        pts[1] = 174 - (lcp_watr - 1);
                        pts[3] = pts[1];
                        beginDraw();
                        vsl_color(vdihnd, vdi_colt[COLOR_blue]);
                        v_pline(vdihnd, 2, pts);
                        endDraw();
                        val--;
                }
        } else {
                /* Drain -val steps. */
                while (lcp_watr != 0 && val != 0) {
                        pts[1] = 174 - (lcp_watr - 1);
                        pts[3] = pts[1];
                        beginDraw();
                        vsl_color(vdihnd, vdi_colt[COLOR_lt_grey]);
                        v_pline(vdihnd, 2, pts);
                        endDraw();
                        gameTick(4);
                        lcp_watr--;
                        val++;
                }
        }
}
