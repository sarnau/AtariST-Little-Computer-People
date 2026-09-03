/*
 * parts/erChr.c -- LCP_STX only; the 0x400c object carries it at
 * 0x72e6, right before fOpen.  Files under parts/ are never compiled
 * standalone.
 */

/* erChr: blank one 8x8 character cell whose baseline is (x, y).
   addr: 0x72e6 */

void
erChr(x, y, color)
short   x;
short   y;
short   color;
{
        plErCol(x, y - 7, x + 7, y, color);
}
