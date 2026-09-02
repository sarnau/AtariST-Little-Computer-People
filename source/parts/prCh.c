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

#ifdef FAITHFUL
        str[0] = (char) ch;
#else
        str[0] = ch;
#endif
        str[1] = 0;

        saved_log = (void *) Logbase();
#ifdef FAITHFUL
        Setscreen(g_dscp, (void *)-1L, -1L);
#else
        Setscreen(g_dscp, (void *)-1L, -1);
#endif
        vst_color(vdihnd, vdi_colt[color]);
        vswr_mode(vdihnd, MD_TRANS);
        v_gtext(vdihnd, x, y, str);
        vswr_mode(vdihnd, MD_REPLACE);
#ifdef FAITHFUL
        Setscreen(saved_log, (void *)-1L, -1L);
#else
        Setscreen(saved_log, (void *)-1L, -1);
#endif
}
