/*
 * parts/rp_anim.c -- shared body; LCP_ORG links it in renderx.c,
 * LCP_STX in the 0xdece object (0x13aec, in the 0xdece object between drwPixel and a_toggt).  Files under parts/
 * are never compiled standalone.
 */
/* rp_anim: sweep needle x=70..83 at y=42, 1px/frame, wrap at 0.
   If music playing and not browsing records, roll random VU LED (0..6)
   at y=47 and toggle lit/unlit (red if new mask overlaps g_ltpac, else black).
   g_ltlic/g_ltpac are 1985 shared-storage: also record-player state
   when no letter is being written.
   addr: rp_anim() */

void
rp_anim()
{
#ifdef FAITHFUL
        unsigned short  rnd;
        short           col;
#else
        /* STX's frame is -10: the LED mask gets a local of its own,
           and the roll is a signed short.  (Named `bit` here because
           `rnd` is the global Random() wrapper in this build.) */
        short           bit;
        short           mask;
        short           col;
#endif

        if (g_ltlic >= 0)
                drwPixel(g_ltlic + 70, 42, COLOR_white);
#ifdef FAITHFUL
        g_ltlic = g_ltlic - 2;
#else
        g_ltlic -= 2;
#endif
        if (g_ltlic < 0)
                g_ltlic = 13;
        drwPixel(g_ltlic + 70, 42, COLOR_black);

        if (mi_play == NO || g_rbact != NO)
                return;

#ifdef FAITHFUL
        rnd = (unsigned short) Random();
        rnd = rnd & 7;
        if (rnd < 7) {
                g_ltpac = rec_ledt[rnd] ^
                                         g_ltpac;
                if ((rec_ledt[rnd] &
                     g_ltpac) == 0)
                        col = COLOR_black;
                else
                        col = COLOR_red;
                drwPixel(rnd * 2 + 66, 47, col);
        }
#else
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
#endif
}
