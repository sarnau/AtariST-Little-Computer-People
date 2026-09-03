/*
 * parts/getEv.c -- shared body; LCP_STX puts getEv in the sprite
 * object at 0x16002, right after putEv.  Files under parts/ are never
 * compiled standalone.
 */
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
