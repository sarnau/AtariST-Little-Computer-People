/*
 * events.c -- deferred event queue.
 *
 * g_trel[] is a 10-slot FIFO of ACTION_ID values that
 * chk_actT() drains at the top of its priority
 * ladder.  Events are queued from timer callbacks (phone_call in sim.c),
 * keyboard shortcuts (Ctrl+F food delivery, Ctrl+R record delivery, ...)
 * and the doorbell handler.
 *
 * addr: putEv(), g_trel[]
 */

#include "types.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern BOOL16   introSeq;
extern void     putEv();            /* ai.c      */
extern short    getEv();          /* events.c  */
extern void     chk_actT();/* ai.c      */
/* Deferred-event FIFO.  Compact-empty: the queue is "empty" when
   g_trel[9] holds ACTION_NONE (the sentinel used to test
   for a full queue).  putEv refuses to append while the
   intro sequence is playing so the queued events don't fire against
   uninitialised state. */
short   g_trel[10] = {
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE,
        ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE
};

/* putEv: append `event` to the deferred-event FIFO if there
   is room and the intro sequence is not active.  Uses a linear scan for
   the first ACTION_NONE slot -- fine for a 10-slot queue.

   addr: putEv() */

void
putEv(event)
short   event;
{
        short   index;

        if (introSeq != NO)
                return;
        if (g_trel[9] != ACTION_NONE)
                return;                 /* queue full */

        for (index = 0;
             index < 10 && g_trel[index] != ACTION_NONE;
             index = index + 1)
                ;
        g_trel[index] = event;
}

/* getEv: pop the head of the deferred-event FIFO.  Shifts
   the tail down one slot and pads with ACTION_NONE.  Returns ACTION_NONE
   when the queue is empty.

   addr: getEv() */

short
getEv()
{
        short   result;
        short   index;

        result = g_trel[0];
        if (result == ACTION_NONE)
                return ACTION_NONE;

        for (index = 1; index < 10; index = index + 1)
                g_trel[index - 1] = g_trel[index];
        g_trel[9] = ACTION_NONE;
        return result;
}
