/*
 * Included by stx_u3.c; never compiled on its own.
 */
/* Pop the head of the action queue eventQueue: returns ACTION_NONE when
   the queue is empty, otherwise the first entry, shifting the other
   nine up one slot and freeing the last.  chooseAction and games.c hand
   the result to runEvent. */
short
nextEvent()
{
        /* Declaration order (index first) and testing the queue head
           in place both match the original. */
        short   index;
        short   result;

        if (eventQueue[0] == ACTION_NONE)
                return ACTION_NONE;
        result = eventQueue[0];

        for (index = 1; index < 10; index++)
                eventQueue[index - 1] = eventQueue[index];
        eventQueue[9] = ACTION_NONE;
        return result;
}
