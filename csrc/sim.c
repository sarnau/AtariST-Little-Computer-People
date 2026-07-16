/*
 * sim.c -- game-clock and needs simulation (gameSim1).
 *
 * Called every 8 animation frames (~1 game-second).  Advances thirst,
 * hunger, sickness, bathroom, mood, and the wall clock; triggers a
 * random daytime phone call.  All logic is verified against the Ghidra
 * decompile of gameSim1 at addr 0x??? in LCP.PRG (see
 * plate comment for exhaustive per-field notes).
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    ani_cnt;
extern short    g_secs;           /* 0..59 game-seconds  */
extern short    t_min;
extern short    t_hour;
extern short    date_day;
extern short    dt_mon;
extern short    dt_year;
extern PLAYER   lcp;                            /* the resident LCP */
extern BOOL16   ph_ans;
extern BOOL16   ph_call;
extern BOOL16   introSeq;
extern short    rndRng();                  /* random.c */
extern void     lcp_sick();              /* health.c  */
extern void     lcp_upal();    /* render.c  */
extern void     daily_rs();     /* ai.c      */
extern short    daysInMo();                /* calendar.c*/
extern void     putEv();            /* ai.c      */
/* gameSim1: called every 8 animation frames (~1 game-second).
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

   addr: gameSim1() */

void
gameSim1()
{
        short   random_val;
        short   days_this_month;

        if ((ani_cnt & 7) != 0)
                return;

        g_secs = g_secs + 1;
        if (g_secs != 60)
                return;

        g_secs = 0;

        /* Thirst tick */
        lcp.thirst_timer = lcp.thirst_timer - 1;
        if (lcp.thirst_timer < 1) {
                lcp.thirst_timer = lcp.thirst_timer_max;
                if (lcp.thirst_level < 3)
                        lcp.thirst_level = lcp.thirst_level + NEED_MILD;
                else
                        lcp_sick();
        }

        /* Hunger tick */
        lcp.hunger_timer = lcp.hunger_timer - 1;
        if (lcp.hunger_timer < 1) {
                lcp.hunger_timer = lcp.hunger_timer_max;
                if (lcp.hunger_level < 3)
                        lcp.hunger_level = lcp.hunger_level + NEED_MILD;
                else
                        lcp_sick();
        }

        /* Sickness progression / recovery */
        if (lcp.sickness_level > 0) {
                lcp.sickness_countdown = lcp.sickness_countdown - 1;
                if (lcp.sickness_countdown == 0) {
                        lcp.sickness_level = lcp.sickness_level +
                                             lcp.sickness_direction;
                        if (lcp.sickness_level == SICKNESS_HEALTHY)
                                lcp_upal();
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
        if (t_hour > 7 && t_hour < 22) {
                random_val = rndRng(0, 100);
                if (random_val < 2 &&
                    ph_ans == NO &&
                    introSeq == NO) {
                        ph_call = YES;
                        putEv(ACTION_EVENT_PHONE_CALL);
                }
        }

        /* Clock advance: minute */
        t_min = t_min + 1;
        if (t_min != 60)
                return;

        t_min = 0;

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
        t_hour = t_hour + 1;
        if (t_hour != 24)
                return;

        t_hour = 0;
        daily_rs();

        /* Calendar advance: day / month / year */
        days_this_month = daysInMo(dt_mon, dt_year);
        date_day = date_day + 1;
        if (days_this_month == date_day) {
                date_day = 0;
                dt_mon = dt_mon + 1;
                if (dt_mon == 12) {
                        dt_mon = 0;
                        dt_year = dt_year + 1;
                }
        }
}
