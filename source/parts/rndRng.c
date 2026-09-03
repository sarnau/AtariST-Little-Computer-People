/*
 * parts/rndRng.c -- shared body; LCP_STX links it in the 0xdece object
 * (0x74fc, in the minigame object right after mg_wkev). Files under
 * parts/ are never compiled standalone.
 */
/* addr: rndRng() */
short
rndRng(low, high)
short   low;
short   high;
{
        /* STX: link #-8 -- r plus a slot that is never written. */
        short   r;
        short   result;

        r = Random();
        r &= 0x7fff;
        result = low + r % (high - low + 1);
        return result;
}
