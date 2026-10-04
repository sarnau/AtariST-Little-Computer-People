/*
 * Included by stx_u3.c; never compiled on its own.
 */
/* Pop the head of the action queue g_trel: returns ACTION_NONE when
   the queue is empty, otherwise the first entry, shifting the other
   nine up one slot and freeing the last.  chk_actT and games.c hand
   the result to execEv. */
short
getEv()
{
        /* Declaration order (index first) and testing the queue head
           in place both match the original. */
        short   index;
        short   result;

        if (g_trel[0] == ACTION_NONE)
                return ACTION_NONE;
        result = g_trel[0];

        for (index = 1; index < 10; index++)
                g_trel[index - 1] = g_trel[index];
        g_trel[9] = ACTION_NONE;
        return result;
}
