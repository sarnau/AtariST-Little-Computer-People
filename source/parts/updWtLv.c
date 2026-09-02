/*
 * parts/updWtLv.c -- shared body; LCP_ORG links it in render.c,
 * LCP_STX in the 0xdece object (0x122fa, immediately after a_drink).  Files under parts/
 * are never compiled standalone.
 */

void
updWtLv(val)
short   val;
{
        short   y;
        RECT16  rect;

        rect.x1 = 146;
        rect.y1 = 174;
        rect.x2 = 159;
        rect.y2 = 174;

        if (val == 0) {
                /* Draw filled portion (colour 0x0D). */
                y = lcp_watr;
                sc_sdtb();
                while (y != 0) {
                        rect.y1 = 174 - (y - 1);
                        rect.y2 = rect.y1;
                        vsl_color(vdihnd, vdi_colt[0xd]);
                        v_pline(vdihnd, 2, &rect.x1);
                        y = y - 1;
                }
                sc_sdtf();

                /* Draw empty portion (colour 0x0C). */
                y = lcp_watr;
                sc_sdtb();
                while (y < 10) {
                        rect.y1 = 174 - y;
                        rect.y2 = rect.y1;
                        vsl_color(vdihnd, vdi_colt[0xc]);
                        v_pline(vdihnd, 2, &rect.x1);
                        y = y + 1;
                }
                sc_sdtf();
                return;
        }

        if (val < 0) {
                /* Drain -val steps. */
                while (lcp_watr != 0 && val != 0) {
                        rect.y1 = 174 - (lcp_watr - 1);
                        rect.y2 = rect.y1;
                        sc_sdtb();
                        vsl_color(vdihnd, vdi_colt[0xc]);
                        v_pline(vdihnd, 2, &rect.x1);
                        sc_sdtf();
                        gameTick(4);
                        lcp_watr = lcp_watr - 1;
                        val = val + 1;
                }
                return;
        }

        /* Fill val steps (capped at 10). */
        while (val != 0 && lcp_watr < 11 &&
               (lcp_watr + 1) < 11) {
                rect.y1 = 174 - lcp_watr;
                lcp_watr = lcp_watr + 1;
                rect.y2 = rect.y1;
                sc_sdtb();
                vsl_color(vdihnd, vdi_colt[0xd]);
                v_pline(vdihnd, 2, &rect.x1);
                sc_sdtf();
                val = val - 1;
        }
}
