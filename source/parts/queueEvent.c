/*
 * Must sit right after playDoorbell.
 *
 * Included by stx_u3.c; never compiled on its own.
 */
/* Queues an outside event (a delivery, a phone call, the dog food key
   commands) for the resident: event is an ACTION_EVENT_* id, appended
   at the first free slot of the 10-entry queue g_trel, which nextEvent
   empties from the front.  Ignored during the move-in cutscene and
   when the queue is already full. */
void
queueEvent(event)
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
