/*
 * sim_tick.c -- host-side smoke test for simStep.
 *
 * Drives the sim for 24 game-hours (86400 game-seconds) from a known
 * starting state and asserts that clock, hunger, thirst and bathroom
 * progressions match what a pen-and-paper trace of sim.c predicts.
 *
 * This is a pure-logic test: no HYBER file, no file I/O.  The PLAYER
 * struct is populated with host-native short values (skipping the
 * usual big-endian file load) so we can reason about the counters
 * directly.  movingIn is asserted to suppress the random
 * daytime phone-call branch, keeping the test deterministic.
 *
 * Build: make sim_test
 * Run:   from source/build/host/, execute ./sim_test
 */

#include <stdio.h>
#include <string.h>

#include "../include/types.h"
#include "../include/structs.h"
#include "../include/enums.h"

extern PLAYER   resident;
extern short    t_min;
extern short    t_hour;
extern short    t_day;
extern short    t_mon;
extern short    t_year;
extern short    frameCount;
extern short    t_sec;
extern BOOL16   phoneAnswered;
extern BOOL16   phoneRinging;
extern BOOL16   movingIn;
extern void     simStep();

static int      failures = 0;

#define CHECK(cond, msg) do {                                           \
        if (!(cond)) {                                                  \
                fprintf(stderr, "FAIL %s:%d  %s\n",                     \
                        __FILE__, __LINE__, msg);                       \
                failures++;                                             \
        }                                                               \
} while (0)

int
main(argc, argv)
int     argc;
char ** argv;
{
        long    i;
        short   thirst_hits;
        short   hunger_hits;

        (void) argc;
        (void) argv;

        /* Zero the PLAYER, then set known starting values. */
        memset(&resident, 0, sizeof(resident));
        resident.thirst_timer_max    = 30;   /* thirst rises every 30 min */
        resident.thirst_timer        = 30;
        resident.hunger_timer_max    = 45;   /* hunger rises every 45 min */
        resident.hunger_timer        = 45;
        resident.bathroom_timer_max  = 120;
        resident.bathroom_timer      = 120;
        resident.happiness           = MOOD_CONTENT;
        resident.mood_duration[MOOD_HAPPY]    = 6;
        resident.mood_duration[MOOD_CONTENT]  = 4;
        resident.mood_duration[MOOD_SAD]      = 6;
        resident.happiness_duration_active    = 6;
        resident.happiness_direction = DIR_WORSENING;
        resident.sickness_level      = SICKNESS_HEALTHY;

        /* Sim entry conditions. */
        frameCount  = 0;    /* (counter & 7) == 0 -> tick */
        t_sec    = 0;
        t_min            = 0;
        t_hour              = 6;    /* 06:00:00 */
        t_day                = 1;
        t_mon              = 0;
        t_year               = 0;

        /* Suppress the random phone-call branch. */
        movingIn   = YES;
        phoneAnswered     = NO;
        phoneRinging  = NO;

        /* Drive 24 game-hours (86400 game-seconds). */
        for (i = 0; i < 86400L; i++)
                simStep();

        /* Clock should have advanced exactly 24h -- back to 06:00:00. */
        CHECK(t_hour == 6,   "t_hour != 6 after 86400 seconds");
        CHECK(t_min == 0, "t_min != 0 after 86400 seconds");
        CHECK(t_sec == 0, "t_sec != 0");

        /* Calendar should have rolled over exactly one day. */
        CHECK(t_day == 2, "t_day did not advance to 2");

        /* thirst_timer counts down each minute; after 24*60=1440 minutes
           with timer_max=30 it wraps 48 times.  Each wrap raises
           thirst_level (capped at 3 then triggers lcp_become_sick).
           Timer at end: 1440 % 30 == 0 so it resets to 30.            */
        CHECK(resident.thirst_timer == 30, "thirst_timer end value wrong");
        CHECK(resident.thirst_level >= NEED_SEVERE,  "thirst_level should max out");

        /* hunger_timer: 1440 minutes with timer_max=45 = 32 wraps.
           At level 3 further wraps invoke lcp_become_sick which
           leaves level unchanged.                                     */
        CHECK(resident.hunger_timer == 45, "hunger_timer end value wrong");
        CHECK(resident.hunger_level >= NEED_SEVERE,  "hunger_level should max out");

        /* bathroom_timer: 1440 min, timer_max=120 -> wraps 12 times.
           On first wrap bathroom_timer is set to 9999 and bathroom_need
           to YES, and stays that way.                                 */
        CHECK(resident.bathroom_need == YES, "bathroom_need should be YES");

        /* Second run: 1 full game-hour from 07:00 with no need
           mutation, verifying pure clock advance.                     */
        memset(&resident, 0, sizeof(resident));
        resident.thirst_timer        = 9999;
        resident.thirst_timer_max    = 9999;
        resident.hunger_timer        = 9999;
        resident.hunger_timer_max    = 9999;
        resident.bathroom_timer      = 9999;
        resident.bathroom_timer_max  = 9999;
        resident.happiness_duration_active = 9999;
        frameCount  = 0;
        t_sec    = 0;
        t_min            = 0;
        t_hour              = 7;
        movingIn   = YES;
        for (i = 0; i < 3600L; i++)
                simStep();
        CHECK(t_hour == 8,   "1-hour drive: t_hour != 8");
        CHECK(t_min == 0, "1-hour drive: t_min != 0");

        /* Sub-minute drive (30 seconds): only t_sec
           should advance; nothing else.                               */
        memset(&resident, 0, sizeof(resident));
        resident.thirst_timer = resident.thirst_timer_max = 9999;
        resident.hunger_timer = resident.hunger_timer_max = 9999;
        resident.bathroom_timer = resident.bathroom_timer_max = 9999;
        resident.happiness_duration_active = 9999;
        frameCount = 0;
        t_sec = 0;
        t_min = 0;
        t_hour = 10;
        movingIn = YES;
        for (i = 0; i < 30L; i++)
                simStep();
        CHECK(t_sec == 30, "30-sec drive: counter wrong");
        CHECK(t_min == 0, "30-sec drive: minutes wrong");

        /* Non-tick frame: counter & 7 != 0 -> function must early-return. */
        t_sec = 42;
        frameCount = 3;      /* 3 & 7 == 3 != 0 */
        simStep();
        CHECK(t_sec == 42, "non-tick frame incremented counter");

        (void) thirst_hits; (void) hunger_hits;

        if (failures == 0) {
                printf("sim_tick: PASS  (all clock/needs progressions match)\n");
                return 0;
        }
        printf("sim_tick: FAIL  (%d assertion(s) failed)\n", failures);
        return 1;
}
