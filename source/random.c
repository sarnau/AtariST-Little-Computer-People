/* random.c -- bounded random-number helper. */

#include "types.h"
#include <osbind.h>             /* Alcyon: Random() macro -> trap #14 */
#include "random.h"

/* rndRng -> parts/rndRng.c (STX: 0x74fc, in the minigame object right after mg_wkev). */
