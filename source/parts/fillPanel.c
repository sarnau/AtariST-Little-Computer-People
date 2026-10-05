/* Paint the top panel of the drawing buffer: stripBuf is set to
   stripStore rounded up to a 512-byte boundary (panelBegin later makes it
   the logical screen), then rows 0..max_y-2 are filled -- with the
   letter-paper colour (paperRow) for a short panel, the two-plane
   stripe (stripeRow) for one of 70 rows or more -- and row maxY-1 is a
   black separator (blackRow).  27 rows for the letter and game menu,
   77 for the minigames (mgSetup). */
void
fillPanel(maxY)
short   maxY;
{
        short   y;

        /* Align the buffer up to a 512-byte boundary. */
        stripBuf = (void *) stripStore;
        stripBuf = (void *) (((long) stripBuf + 512L) & ~511L);

        for (y = 0; y < maxY - 1; y++) {
                if (maxY < 70)
                        paperRow(stripBuf, y);
                else
                        stripeRow(stripBuf, y);
        }
        blackRow(stripBuf, maxY - 1);
}
