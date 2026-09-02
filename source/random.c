/* random.c -- bounded random-number helper. */

#include "types.h"
#include <osbind.h>             /* Alcyon: Random() macro -> trap #14 */
#include "random.h"

/* addr: rndRng() */
short
rndRng(low, high)
short   low;
short   high;
{
#ifdef FAITHFUL
        unsigned short  r;

        r = (unsigned short) Random();
        return low + (short) (r & 0x7fff) % (short) (high - low + 1);
#else
        /* STX: link #-8 -- r plus a slot that is never written. */
        short   r;
        short   result;

        r = Random();
        r &= 0x7fff;
        result = low + r % (high - low + 1);
        return result;
#endif
}
