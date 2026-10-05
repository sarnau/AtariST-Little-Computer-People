/*
 * ai.c -- AI decision engine and event dispatcher.
 * chooseAction: ~1 Hz priority ladder -> runAction(nextAction).
 * runEvent: dispatch deferred events from FIFO.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "protos.h"
#include "events.h"
#include "globals.h"

/* runEvent: dispatch a single deferred event to its handler.
   inEvent guards recursion; sleeper is forced out of bed first.
   Food-delivery drops silently if the 3-bit food-count is already 4. */

void
runEvent(event)
short   event;
{
        inEvent = YES;

        if (resident.is_sleeping != NO)
                getInOutOfBed();

        /* The arms are in the original's source order (BOOK_DELIVERY
           first, DOG_FOOD last), which decides the code layout; keep it.
           The phone handler is passed 0. */
        switch (event) {
        case ACTION_EVENT_BOOK_DELIVERY:
                bookDelivery();
                break;
        case ACTION_EVENT_RECORD_DELIVERY:
                recordDelivery();
                break;
        case ACTION_EVENT_FOOD_DELIVERY:
                if (((resident.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD) == FOOD_PACKS_MAX)
                        break;
                foodDelivery();
                break;
        case ACTION_EVENT_PHONE_CALL:
                /* answerPhone takes no parameter; the 0 is pushed and
                   ignored, but the push is part of the original code. */
                answerPhone(0);
                break;
        case ACTION_NOD_OK:
                nodOk();
                break;
        case ACTION_EVENT_DOG_FOOD:
                dogFoodDelivery();
                break;
        }

        inEvent = NO;
}

