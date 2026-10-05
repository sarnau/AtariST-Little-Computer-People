/* tick_tables.h -- extern declarations for tick_tables.c. */

#ifndef TICK_TABLES_H
#define TICK_TABLES_H

#include "types.h"

extern short clockFrames[];
extern short alarmFrames[];
extern short phoneFrames[];
extern short fireFrames[];
extern short bowlFrames[];
extern short patSprites[];
extern short patLastSprite;
extern BOOL16 alarmSounding;
extern short ringCountdown;

#endif /* TICK_TABLES_H */
