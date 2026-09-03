/*
 * sim.c -- game-clock and needs simulation (gameSim1).
 * Called every 8 animation frames (~1 game-second).
 * addr: gameSim1()
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

void
gameSim1()
{
        /* STX has NO locals (frame -4): every counter steps in place
           and every call result is consumed where it is produced. */
        if ((ani_cnt & 7) != 0)
                return;

        if (++g_secs != 60)
                return;

        g_secs = 0;

        /* Thirst tick */
        lcp.thirst_timer--;
        if (lcp.thirst_timer <= 0) {
                lcp.thirst_timer = lcp.thirst_timer_max;
                if (lcp.thirst_level < 3)
                        lcp.thirst_level++;
                else
                        lcp_sick();
        }

        /* Hunger tick */
        lcp.hunger_timer--;
        if (lcp.hunger_timer <= 0) {
                lcp.hunger_timer = lcp.hunger_timer_max;
                if (lcp.hunger_level < 3)
                        lcp.hunger_level++;
                else
                        lcp_sick();
        }

        /* Sickness progression / recovery */
        if (lcp.sickness_level > 0) {
                if (--lcp.sickness_countdown == 0) {
                        lcp.sickness_level += lcp.sickness_direction;
                        if (lcp.sickness_level == SICKNESS_HEALTHY)
                                lcp_upal();
                        else if (lcp.sickness_level > 4)
                                /* `==` where the author meant `=`: the
                                   clamp never happens, and the binary
                                   carries the discarded comparison.
                                   Preserved as written. */
                                lcp.sickness_level == 4;
                        if (lcp.sickness_level >= 2)
                                lcp.happiness = MOOD_SAD;
                        if (lcp.sickness_direction == DIR_IMPROVING)
                                lcp.sickness_countdown = 5;
                        else
                                lcp.sickness_countdown = 60;
                }
        }

        /* Bathroom tick */
        lcp.bathroom_timer--;
        if (lcp.bathroom_timer <= 0) {
                lcp.bathroom_timer = 9999;
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
                        if (lcp.happiness <= 0) {
                                lcp.happiness = MOOD_HAPPY;
                                lcp.happiness_direction = DIR_WORSENING;
                        } else if (lcp.happiness >= 2) {
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
