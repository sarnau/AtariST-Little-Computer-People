/*
 * parts/printChar.c -- included by stx_u3.c; never compiled on its own.
 * Draws one character into the g_dscp screen; must follow printString.
 */

void
printChar(ch, x, y, color)
short   ch;
short   x;
short   y;
short   color;
{
        char    str[2];
        void *  saved_log;

        str[0] = ch;
        str[1] = 0;

        saved_log = (void *) Logbase();
        Setscreen(g_dscp, (void *)-1L, -1);
        vst_color(vdihnd, vdi_colt[color]);
        vswr_mode(vdihnd, MD_TRANS);
        v_gtext(vdihnd, x, y, str);
        vswr_mode(vdihnd, MD_REPLACE);
        Setscreen(saved_log, (void *)-1L, -1);
}
