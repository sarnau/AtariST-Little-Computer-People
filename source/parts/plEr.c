/*
 * plEr: clear a rectangle via VDI v_bar.
 *
 * Its place in games.c, after the anagram helpers and far from
 * initVdi, matters: closer, the initVdi call would compile to a
 * shorter branch than the original's.
 *
 * Included by games.c; never compiled on its own.
 */

void
plEr(x1, y1, x2, y2)
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
        initVdi();
        v_bar(vdihnd, rect);
        exitVdi();
}
