/* Draws the picture on the TV set in the living room: five short
   vertical lines in the given colour.  tvNoise calls it every tick
   with a random colour while the TV is on, and tvOff blanks it in
   white. */
void
drawTvPicture(color)
short   color;
{
        short   i;

        for (i = 0; i < 5; i++)
                drawLine(i + 44, 51 - (i >> 1), i + 44, 57 - (i >> 1), color);
}
