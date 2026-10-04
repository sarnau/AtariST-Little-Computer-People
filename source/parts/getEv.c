/*
 * Included by stx_u3.c; never compiled on its own.
 */
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
