/*
 * Draws one character into the stripBuf screen; must follow
 * printString.
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
        Setscreen(stripBuf, (void *)-1L, -1);
        vst_color(vdiHandle, colorPens[color]);
        vswr_mode(vdiHandle, MD_TRANS);
        v_gtext(vdiHandle, x, y, str);
        vswr_mode(vdiHandle, MD_REPLACE);
        Setscreen(saved_log, (void *)-1L, -1);
}
