/*
 * parts/erChr.c -- included by stx_u1.c, right before fOpen; never
 * compiled on its own.
 */

/* Blank one 8x8 character cell whose baseline is (x, y). */

void
erChr(x, y, color)
short   x;
short   y;
short   color;
{
        plErCol(x, y - 7, x + 7, y, color);
}
