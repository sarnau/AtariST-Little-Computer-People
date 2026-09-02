/*
 * parts/plEr.c -- plEr's body, shared between configurations.
 *
 * The two revisions place it differently inside the games object:
 * LCP_ORG right after mg_stp, LCP_STX at 0x86e0, past the anagram
 * helpers (ag_intr 0x7f84) -- and that distance is what makes its
 * initVdi call a bsr.w instead of a bsr.s.  Files under parts/ are
 * never compiled standalone.
 */

/* plEr: clear a rectangle via VDI v_bar.
   addr: plEr() */

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
