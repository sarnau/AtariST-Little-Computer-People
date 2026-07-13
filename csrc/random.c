/*
 * random.c -- bounded random-number helper.
 *
 * The 68k Random() XBIOS trap (function 17) returns a 24-bit unsigned
 * value in D0.  We mask to 15 bits and take a modulo to fold it into
 * the requested inclusive range.  Modulo bias is negligible for the
 * small (< 128) ranges the game uses.
 */

#include "types.h"
#include <osbind.h>             /* Alcyon: Random() macro -> trap #14 */

/* randomRange: return a uniform random integer in [low, high] inclusive.
   addr: randomRange() */

short
randomRange(low, high)
short   low;
short   high;
{
        unsigned short  r;

        r = (unsigned short) Random();
        return low + (short) (r & 0x7fff) % (short) (high - low + 1);
}
