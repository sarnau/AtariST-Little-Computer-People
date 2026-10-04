/*
 * Must sit right after p_dobls.
 *
 * Included by stx_u3.c; never compiled on its own.
 */
void
putEv(event)
short   event;
{
        short   index;

        if (introSeq != NO)
                return;
        if (g_trel[9] != ACTION_NONE)
                return;                 /* queue full */

        /* The scan is a loop with an explicit break, as in the
           original. */
        for (index = 0; index < 10; index++)
                if (g_trel[index] == ACTION_NONE)
                        break;
        g_trel[index] = event;
}
