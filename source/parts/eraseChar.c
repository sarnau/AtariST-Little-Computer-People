/* Blank one 8x8 character cell whose baseline is (x, y). */

void
eraseChar(x, y, color)
short   x;
short   y;
short   color;
{
        eraseRectColor(x, y - 7, x + 7, y, color);
}
