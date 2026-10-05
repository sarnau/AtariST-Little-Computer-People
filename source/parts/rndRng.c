/* Returns a random number from low to high inclusive: the XBIOS
   Random() value is masked to 15 bits and reduced modulo the range. */
short
rndRng(low, high)
short   low;
short   high;
{
        /* Random number in [low, high].  Keep both locals: they are
           the original's, and the frame depends on them. */
        short   r;
        short   result;

        r = Random();
        r &= 0x7fff;
        result = low + r % (high - low + 1);
        return result;
}
