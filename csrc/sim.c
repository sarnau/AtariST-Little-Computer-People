/*
 * sim.c -- game-clock and needs simulation (game_simulate_one_second).
 *
 * Called every 8 animation frames (~1 game-second).  Advances thirst,
 * hunger, sickness, bathroom, mood, and the wall clock; triggers a
 * random daytime phone call.  All logic is verified against the Ghidra
 * decompile of game_simulate_one_second at addr 0x??? in LCP.PRG (see
 * plate comment for exhaustive per-field notes).
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    animation_tick_counter;
extern short    game_seconds_counter;           /* 0..59 game-seconds  */
extern short    time_minutes;
extern short    time_hours;
extern short    date_day;
extern short    date_month;
extern short    date_year;
extern PLAYER   lcp;                            /* the resident LCP */
extern BOOL16   phone_answered_flag;
extern BOOL16   phone_call_active_flag;
extern BOOL16   intro_sequence_active;
extern short    randomRange();                  /* random.c */
extern void     lcp_become_sick();              /* health.c  */
extern void     lcp_update_palette_colors();    /* render.c  */
extern void     daily_reset_action_flags();     /* ai.c      */
extern short    days_in_month();                /* calendar.c*/
extern void     put_event_to_list();            /* ai.c      */
/* game_simulate_one_second: called every 8 animation frames (~1 game-second).
   Updates all time-dependent PLAYER state:
     Thirst: thirst_timer-- each minute. At 0 -> thirst_level++ (max 3, then sickness)
     Hunger: hunger_timer-- each minute. At 0 -> hunger_level++ (max 3, then sickness)
     Sickness: sickness_countdown-- each minute. At 0 -> level changes by
               sickness_direction. Level 0 = recovered (restores palette).
               Level >1 = forces happiness=2.
               Recovery rate: 5 min/step. Worsening rate: 60 min/step.
     Bathroom: bathroom_timer-- each minute. At 0 -> bathroom_need=1, timer=9999.
     Phone: 2% chance per second of random phone call, 8AM-10PM only.
     Happiness: hourly cycle. happiness_duration_active-- each hour. At 0 ->
                happiness shifts by happiness_direction. Reverses at bounds
                (0 or 2). Overridden by sickness.
     Clock: seconds->minutes->hours->days->months->years with proper calendar.

   addr: game_simulate_one_second() */

void
game_simulate_one_second()
{
        short   random_val;
        short   days_this_month;

        if ((animation_tick_counter & 7) != 0)
                return;

        game_seconds_counter = game_seconds_counter + 1;
        if (game_seconds_counter != 60)
                return;

        game_seconds_counter = 0;

        /* Thirst tick */
        lcp.thirst_timer = lcp.thirst_timer - 1;
        if (lcp.thirst_timer < 1) {
                lcp.thirst_timer = lcp.thirst_timer_max;
                if (lcp.thirst_level < 3)
                        lcp.thirst_level = lcp.thirst_level + NEED_MILD;
                else
                        lcp_become_sick();
        }

        /* Hunger tick */
        lcp.hunger_timer = lcp.hunger_timer - 1;
        if (lcp.hunger_timer < 1) {
                lcp.hunger_timer = lcp.hunger_timer_max;
                if (lcp.hunger_level < 3)
                        lcp.hunger_level = lcp.hunger_level + NEED_MILD;
                else
                        lcp_become_sick();
        }

        /* Sickness progression / recovery */
        if (lcp.sickness_level > 0) {
                lcp.sickness_countdown = lcp.sickness_countdown - 1;
                if (lcp.sickness_countdown == 0) {
                        lcp.sickness_level = lcp.sickness_level +
                                             lcp.sickness_direction;
                        if (lcp.sickness_level == SICKNESS_HEALTHY)
                                lcp_update_palette_colors();
                        if (lcp.sickness_level > 1)
                                lcp.happiness = MOOD_SAD;
                        if (lcp.sickness_direction == DIR_IMPROVING)
                                lcp.sickness_countdown = 5;
                        else
                                lcp.sickness_countdown = 60;
                }
        }

        /* Bathroom tick */
        lcp.bathroom_timer = lcp.bathroom_timer - 1;
        if (lcp.bathroom_timer < 1) {
                lcp.bathroom_timer = 9999;
                lcp.bathroom_need = YES;
        }

        /* Random daytime phone call: 2% per second, 08:00-21:59 only */
        if (time_hours > 7 && time_hours < 22) {
                random_val = randomRange(0, 100);
                if (random_val < 2 &&
                    phone_answered_flag == NO &&
                    intro_sequence_active == NO) {
                        phone_call_active_flag = YES;
                        put_event_to_list(ACTION_EVENT_PHONE_CALL);
                }
        }

        /* Clock advance: minute */
        time_minutes = time_minutes + 1;
        if (time_minutes != 60)
                return;

        time_minutes = 0;

        /* Happiness mood cycle -- suppressed while sick unless already sad.
           The three-entry lookup table happiness_initial_countdown /
           happiness_duration_happy / happiness_duration_content lives at
           struct offsets 0x2A..0x2F; original code indexed via
           (&lcp.happiness_initial_countdown)[happiness]. */
        if ((lcp.sickness_level == SICKNESS_HEALTHY ||
             lcp.happiness != MOOD_SAD)) {
                lcp.happiness_duration_active =
                        lcp.happiness_duration_active - 1;
                if (lcp.happiness_duration_active == 0) {
                        lcp.happiness = lcp.happiness +
                                        lcp.happiness_direction;
                        if (lcp.happiness < 1) {
                                lcp.happiness = MOOD_HAPPY;
                                lcp.happiness_direction = DIR_WORSENING;
                        } else if (lcp.happiness > 1) {
                                lcp.happiness = MOOD_SAD;
                                lcp.happiness_direction = DIR_IMPROVING;
                        }
                        lcp.happiness_duration_active =
                                (&lcp.happiness_initial_countdown)
                                        [lcp.happiness];
                }
        }

        /* Clock advance: hour */
        time_hours = time_hours + 1;
        if (time_hours != 24)
                return;

        time_hours = 0;
        daily_reset_action_flags();

        /* Calendar advance: day / month / year */
        days_this_month = days_in_month(date_month, date_year);
        date_day = date_day + 1;
        if (days_this_month == date_day) {
                date_day = 0;
                date_month = date_month + 1;
                if (date_month == 12) {
                        date_month = 0;
                        date_year = date_year + 1;
                }
        }
}
