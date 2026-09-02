/*
 * parts/putEv.c -- shared body; LCP_ORG links it in events.c,
 * LCP_STX in the 0xdece object (0x15fb4, after p_dobls).  Files under parts/
 * are never compiled standalone.
 */
/* addr: putEv() */
void
putEv(event)
short   event;
{
        short   index;

        if (introSeq != NO)
                return;
        if (g_trel[9] != ACTION_NONE)
                return;                 /* queue full */

#ifdef FAITHFUL
        for (index = 0;
             index < 10 && g_trel[index] != ACTION_NONE;
             index = index + 1)
                ;
#else
        /* STX splits the scan into a loop with an explicit break. */
        for (index = 0; index < 10; index++)
                if (g_trel[index] == ACTION_NONE)
                        break;
#endif
        g_trel[index] = event;
}
