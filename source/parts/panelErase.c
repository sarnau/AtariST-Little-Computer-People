/*
 * panelErase: clear a rectangle via VDI v_bar.
 *
 * Its place in games.c, after the anagram helpers and far from
 * panelBegin, matters: closer, the panelBegin call would compile to a
 * shorter branch than the original's.
 */

void
panelErase(x1, y1, x2, y2)
short   x1;
short   y1;
short   x2;
short   y2;
{
        short   rect[4];

        rect[0] = x1;
        rect[1] = y1;
        rect[2] = x2;
        rect[3] = y2;
        panelBegin();
        v_bar(vdiHandle, rect);
        panelEnd();
}
