/*
 * parts/prCh.c -- shared body; LCP_ORG links it in renderx.c,
 * LCP_STX in the 0xdece object (0x16ede, immediately after strPr).  Files under parts/
 * are never compiled standalone.
 */

void
prCh(ch, x, y, color)
short   ch;
short   x;
short   y;
short   color;
{
        char    str[2];
        void *  saved_log;

        str[0] = (char) ch;
        str[1] = 0;

        saved_log = (void *) Logbase();
        Setscreen(g_dscp, (void *)-1L, -1L);
        vst_color(vdihnd, vdi_colt[color]);
        vswr_mode(vdihnd, MD_TRANS);
        v_gtext(vdihnd, x, y, str);
        vswr_mode(vdihnd, MD_REPLACE);
        Setscreen(saved_log, (void *)-1L, -1L);
}

/* rp_anim: sweep needle x=70..83 at y=42, 1px/frame, wrap at 0.
   If music playing and not browsing records, roll random VU LED (0..6)
   at y=47 and toggle lit/unlit (red if new mask overlaps g_ltpac, else black).
   g_ltlic/g_ltpac are 1985 shared-storage: also record-player state
   when no letter is being written.
   addr: rp_anim() */

void
rp_anim()
{
        unsigned short  rnd;
        short           col;

        if (g_ltlic >= 0)
                drwPixel(g_ltlic + 70, 42, COLOR_white);
        g_ltlic = g_ltlic - 2;
        if (g_ltlic < 0)
                g_ltlic = 13;
        drwPixel(g_ltlic + 70, 42, COLOR_black);

        if (mi_play == NO || g_rbact != NO)
                return;

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
}
