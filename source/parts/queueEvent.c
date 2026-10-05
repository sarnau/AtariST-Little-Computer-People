/*
 * Must sit right after playDoorbell.
 */
/* Queues an outside event (a delivery, a phone call, the dog food key
   commands) for the resident: event is an ACTION_EVENT_* id, appended
   at the first free slot of the 10-entry queue eventQueue, which nextEvent
   empties from the front.  Ignored during the move-in cutscene and
   when the queue is already full. */
void
queueEvent(event)
short   event;
{
        short   index;

        if (movingIn != NO)
                return;
        if (eventQueue[9] != ACTION_NONE)
                return;                 /* queue full */

        /* The scan is a loop with an explicit break, as in the
           original. */
        for (index = 0; index < 10; index++)
                if (eventQueue[index] == ACTION_NONE)
                        break;
        eventQueue[index] = event;
}
