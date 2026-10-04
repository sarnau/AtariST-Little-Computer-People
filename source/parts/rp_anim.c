/*
 * Sits between drwPixel and a_toggt.
 *
 * Included by stx_u2.c; never compiled on its own.
 */
/* rp_anim: sweep needle x=70..83 at y=42, 1px/frame, wrap at 0.
   If music playing and not browsing records, roll random VU LED (0..6)
   at y=47 and toggle lit/unlit (red if new mask overlaps g_ltpac, else black).
   g_ltlic/g_ltpac are 1985 shared-storage: also record-player state
   when no letter is being written. */

void
rp_anim()
{
        /* The LED mask gets a local of its own, and the roll is a
           signed short, as in the original.  (Named `bit` because
           `rnd` is the global Random() wrapper.) */
        short           bit;
        short           mask;
        short           col;

        if (g_ltlic >= 0)
                drwPixel(g_ltlic + 70, 42, COLOR_white);
        g_ltlic -= 2;
        if (g_ltlic < 0)
                g_ltlic = 13;
        drwPixel(g_ltlic + 70, 42, COLOR_black);

        if (mi_play == NO || g_rbact != NO)
                return;

        bit = (short) rnd() & 7;
        if (bit < 7) {
                mask = rec_ledt[bit];
                g_ltpac ^= mask;
                if ((g_ltpac & mask) != 0)
                        col = COLOR_red;
                else
                        col = COLOR_black;
                drwPixel(bit * 2 + 66, 47, col);
        }
}
