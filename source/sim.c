/*
 * sim.c -- game-clock and needs simulation (simStep).
 * Called every 8 animation frames (~1 game-second).
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "calendar.h"
#include "events.h"
#include "globals.h"
#include "protos.h"

/* Advance the game clock and the resident's bodily needs.  Called
   every frame but acts only on every 8th (frameCount), counting those in
   t_sec; each time t_sec wraps at 60 (one game minute) it ticks the
   thirst, hunger, sickness and bathroom timers, may ring the phone
   (2% per minute between 08:00 and 21:59, outside the move-in), and
   steps t_min.  The hour rollover runs the mood cycle and t_hour; the
   day rollover runs resetDailyFlags and advances the date. */
void
simStep()
{
        /* No locals on purpose: every counter steps in place and every
           call result is consumed where it is produced. */
        if ((frameCount & 7) != 0)
                return;

        if (++t_sec != 60)
                return;

        t_sec = 0;

        /* Thirst tick */
        resident.thirst_timer--;
        if (resident.thirst_timer <= 0) {
                resident.thirst_timer = resident.thirst_timer_max;
                if (resident.thirst_level < NEED_SEVERE)
                        resident.thirst_level++;
                else
                        fallSick();
        }

        /* Hunger tick */
        resident.hunger_timer--;
        if (resident.hunger_timer <= 0) {
                resident.hunger_timer = resident.hunger_timer_max;
                if (resident.hunger_level < NEED_SEVERE)
                        resident.hunger_level++;
                else
                        fallSick();
        }

        /* Sickness progression / recovery */
        if (resident.sickness_level > SICKNESS_HEALTHY) {
                if (--resident.sickness_countdown == 0) {
                        resident.sickness_level += resident.sickness_direction;
                        if (resident.sickness_level == SICKNESS_HEALTHY)
                                setSkinColor();
                        else if (resident.sickness_level > SICKNESS_CRITICAL)
                                /* 1985 bug: `==` where `=` was meant, so
                                   the clamp never happens.  Kept on
                                   purpose: it is part of the original
                                   code. */
                                resident.sickness_level == SICKNESS_CRITICAL;
                        if (resident.sickness_level >= SICKNESS_MODERATE)
                                resident.happiness = MOOD_SAD;
                        if (resident.sickness_direction == DIR_IMPROVING)
                                resident.sickness_countdown = SICK_DELAY_IMPROVING;
                        else
                                resident.sickness_countdown = SICK_DELAY_WORSENING;
                }
        }

        /* Bathroom tick */
        resident.bathroom_timer--;
        if (resident.bathroom_timer <= 0) {
                resident.bathroom_timer = BATHROOM_TIMER_OFF;
                resident.bathroom_need = YES;
        }

        /* Random daytime phone call: 2% per second, 08:00-21:59 only */
        if (t_hour > 7 && t_hour < 22 &&
            rndRng(0, 100) < 2 &&
            phoneAnswered == NO &&
            movingIn == NO) {
                phoneRinging = YES;
                queueEvent(ACTION_EVENT_PHONE_CALL);
        }

        /* Clock advance: minute */
        if (++t_min != 60)
                return;

        t_min = 0;

        /* Happiness mood cycle -- suppressed while sick unless sad. */
        if (resident.sickness_level == SICKNESS_HEALTHY ||
            resident.happiness != MOOD_SAD) {
                if (--resident.happiness_duration_active == 0) {
                        resident.happiness += resident.happiness_direction;
                        if (resident.happiness <= MOOD_HAPPY) {
                                resident.happiness = MOOD_HAPPY;
                                resident.happiness_direction = DIR_WORSENING;
                        } else if (resident.happiness >= MOOD_SAD) {
                                resident.happiness = MOOD_SAD;
                                resident.happiness_direction = DIR_IMPROVING;
                        }
                        resident.happiness_duration_active =
                                resident.mood_duration[resident.happiness];
                }
        }

        /* Clock advance: hour */
        if (++t_hour != 24)
                return;

        t_hour = 0;
        resetDailyFlags();

        /* Calendar advance: day / month / year */
        if (daysInMonth(t_mon, t_year) == ++t_day) {
                t_day = 0;
                if (++t_mon == 12) {
                        t_mon = 0;
                        t_year++;
                }
        }
}
