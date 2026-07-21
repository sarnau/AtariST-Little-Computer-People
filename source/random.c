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
        unsigned short  r;

        r = (unsigned short) Random();
        return low + (short) (r & 0x7fff) % (short) (high - low + 1);
}
