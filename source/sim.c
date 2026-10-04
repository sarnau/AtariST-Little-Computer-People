/*
 * sim.c -- game-clock and needs simulation (gameSim1).
 * Called every 8 animation frames (~1 game-second).
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "calendar.h"
#include "events.h"
#include "globals.h"
#include "health.h"
#include "random.h"
#include "renderx.h"
#include "sim.h"

/* Advance the game clock and the resident's bodily needs.  Called
   every frame but acts only on every 8th (ani_cnt), counting those in
   g_secs; each time g_secs wraps at 60 (one game minute) it ticks the
   thirst, hunger, sickness and bathroom timers, may ring the phone
   (2% per minute between 08:00 and 21:59, outside the move-in), and
   steps t_min.  The hour rollover runs the mood cycle and t_hour; the
   day rollover runs daily_rs and advances the date. */
void
gameSim1()
{
        /* No locals on purpose: every counter steps in place and every
           call result is consumed where it is produced. */
        if ((ani_cnt & 7) != 0)
                return;

        if (++g_secs != 60)
                return;

        g_secs = 0;

        /* Thirst tick */
        lcp.thirst_timer--;
        if (lcp.thirst_timer <= 0) {
                lcp.thirst_timer = lcp.thirst_timer_max;
                if (lcp.thirst_level < NEED_SEVERE)
                        lcp.thirst_level++;
                else
                        lcp_sick();
        }

        /* Hunger tick */
        lcp.hunger_timer--;
        if (lcp.hunger_timer <= 0) {
                lcp.hunger_timer = lcp.hunger_timer_max;
                if (lcp.hunger_level < NEED_SEVERE)
                        lcp.hunger_level++;
                else
                        lcp_sick();
        }

        /* Sickness progression / recovery */
        if (lcp.sickness_level > SICKNESS_HEALTHY) {
                if (--lcp.sickness_countdown == 0) {
                        lcp.sickness_level += lcp.sickness_direction;
                        if (lcp.sickness_level == SICKNESS_HEALTHY)
                                lcp_upal();
                        else if (lcp.sickness_level > SICKNESS_CRITICAL)
                                /* 1985 bug: `==` where `=` was meant, so
                                   the clamp never happens.  Kept on
                                   purpose: it is part of the original
                                   code. */
                                lcp.sickness_level == SICKNESS_CRITICAL;
                        if (lcp.sickness_level >= SICKNESS_MODERATE)
                                lcp.happiness = MOOD_SAD;
                        if (lcp.sickness_direction == DIR_IMPROVING)
                                lcp.sickness_countdown = SICK_DELAY_IMPROVING;
                        else
                                lcp.sickness_countdown = SICK_DELAY_WORSENING;
                }
        }

        /* Bathroom tick */
        lcp.bathroom_timer--;
        if (lcp.bathroom_timer <= 0) {
                lcp.bathroom_timer = BATHROOM_TIMER_OFF;
                lcp.bathroom_need = YES;
        }

        /* Random daytime phone call: 2% per second, 08:00-21:59 only */
        if (t_hour > 7 && t_hour < 22 &&
            rndRng(0, 100) < 2 &&
            ph_ans == NO &&
            introSeq == NO) {
                ph_call = YES;
                putEv(ACTION_EVENT_PHONE_CALL);
        }

        /* Clock advance: minute */
        if (++t_min != 60)
                return;

        t_min = 0;

        /* Happiness mood cycle -- suppressed while sick unless sad. */
        if (lcp.sickness_level == SICKNESS_HEALTHY ||
            lcp.happiness != MOOD_SAD) {
                if (--lcp.happiness_duration_active == 0) {
                        lcp.happiness += lcp.happiness_direction;
                        if (lcp.happiness <= MOOD_HAPPY) {
                                lcp.happiness = MOOD_HAPPY;
                                lcp.happiness_direction = DIR_WORSENING;
                        } else if (lcp.happiness >= MOOD_SAD) {
                                lcp.happiness = MOOD_SAD;
                                lcp.happiness_direction = DIR_IMPROVING;
                        }
                        lcp.happiness_duration_active =
                                (&lcp.happiness_initial_countdown)
                                        [lcp.happiness];
                }
        }

        /* Clock advance: hour */
        if (++t_hour != 24)
                return;

        t_hour = 0;
        daily_rs();

        /* Calendar advance: day / month / year */
        if (daysInMo(dt_mon, dt_year) == ++date_day) {
                date_day = 0;
                if (++dt_mon == 12) {
                        dt_mon = 0;
                        dt_year++;
                }
        }
}
