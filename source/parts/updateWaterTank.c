/*
 * Included by stx_u2.c; never compiled on its own.
 */


/* Draws and changes the water tank level waterLevel, one horizontal
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
                y = waterLevel;
                beginDraw();
                while (y-- != 0) {
                        pts[1] = 174 - y;
                        pts[3] = pts[1];
                        vsl_color(vdiHandle, colorPens[COLOR_blue]);
                        v_pline(vdiHandle, 2, pts);
                }
                endDraw();

                /* Draw empty portion (colour 0x0C). */
                y = waterLevel;
                beginDraw();
                while (y++ < WATER_MAX) {
                        pts[1] = 174 - (y - 1);
                        pts[3] = pts[1];
                        vsl_color(vdiHandle, colorPens[COLOR_lt_grey]);
                        v_pline(vdiHandle, 2, pts);
                }
                endDraw();
                return;
        }

        if (val > 0) {
                /* Fill val steps (capped at 10). */
                while (val != 0 && waterLevel <= WATER_MAX) {
                        if (++waterLevel > WATER_MAX) {
                                waterLevel--;
                                break;
                        }
                        pts[1] = 174 - (waterLevel - 1);
                        pts[3] = pts[1];
                        beginDraw();
                        vsl_color(vdiHandle, colorPens[COLOR_blue]);
                        v_pline(vdiHandle, 2, pts);
                        endDraw();
                        val--;
                }
        } else {
                /* Drain -val steps. */
                while (waterLevel != 0 && val != 0) {
                        pts[1] = 174 - (waterLevel - 1);
                        pts[3] = pts[1];
                        beginDraw();
                        vsl_color(vdiHandle, colorPens[COLOR_lt_grey]);
                        v_pline(vdiHandle, 2, pts);
                        endDraw();
                        gameTick(4);
                        waterLevel--;
                        val++;
                }
        }
}
