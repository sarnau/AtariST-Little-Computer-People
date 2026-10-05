/*
 * Sits between drawPixel and toggleTv.
 */
/* animRecPlayer: sweep needle x=70..83 at y=42, 1px/frame, wrap at 0.
   If music playing and not browsing records, roll random VU LED (0..6)
   at y=47 and toggle lit/unlit (red if new mask overlaps vuLeds, else black).
   needlePos/vuLeds are 1985 shared-storage: also record-player state
   when no letter is being written. */

void
animRecPlayer()
{
        /* The LED mask gets a local of its own, and the roll is a
           signed short, as in the original.  (Named `bit` because
           `rnd` is the global Random() wrapper.) */
        short           bit;
        short           mask;
        short           col;

        if (needlePos >= 0)
                drawPixel(needlePos + 70, 42, COLOR_white);
        needlePos -= 2;
        if (needlePos < 0)
                needlePos = 13;
        drawPixel(needlePos + 70, 42, COLOR_black);

        if (songPlaying == NO || organPlaying != NO)
                return;

        bit = (short) rnd() & 7;
        if (bit < 7) {
                mask = vuLedMasks[bit];
                vuLeds ^= mask;
                if ((vuLeds & mask) != 0)
                        col = COLOR_red;
                else
                        col = COLOR_black;
                drawPixel(bit * 2 + 66, 47, col);
        }
}
