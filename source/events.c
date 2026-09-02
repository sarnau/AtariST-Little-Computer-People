/* events.c -- deferred event queue drained by chk_actT(). */

#include "types.h"
#include "enums.h"
#include "ai.h"
#include "events.h"
#include "globals.h"

short   g_trel[10] = {
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE,
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE
};

/* putEv -> parts/putEv.c (STX: 0x15fb4, after p_dobls). */
#ifdef FAITHFUL
#include "parts/putEv.c"
#endif

/* addr: getEv() */
short
getEv()
{
#ifdef FAITHFUL
        short   result;
        short   index;

        result = g_trel[0];
        if (result == ACTION_NONE)
                return ACTION_NONE;

        for (index = 1; index < 10; index = index + 1)
#else
        /* STX: index first, and the queue head tested in place. */
        short   index;
        short   result;

        if (g_trel[0] == ACTION_NONE)
                return ACTION_NONE;
        result = g_trel[0];

        for (index = 1; index < 10; index++)
#endif
                g_trel[index - 1] = g_trel[index];
        g_trel[9] = ACTION_NONE;
        return result;
}
