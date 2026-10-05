/* events.h -- the event queue filled by queueEvent and drained by nextEvent. */

#ifndef EVENTS_H
#define EVENTS_H

extern short eventQueue[];

extern void queueEvent();
extern short nextEvent();

#endif /* EVENTS_H */
