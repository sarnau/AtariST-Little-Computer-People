/*
 * sim_tick.c -- host-side smoke test for gameSim1.
 *
 * Drives the sim for 24 game-hours (86400 game-seconds) from a known
 * starting state and asserts that clock, hunger, thirst and bathroom
 * progressions match what a pen-and-paper trace of sim.c predicts.
 *
 * This is a pure-logic test: no HYBER file, no file I/O.  The PLAYER
 * struct is populated with host-native short values (skipping the
 * usual big-endian file load) so we can reason about the counters
 * directly.  introSeq is asserted to suppress the random
 * daytime phone-call branch, keeping the test deterministic.
 *
 * Build: make sim_test
 * Run:   from source/build/host/, execute ./sim_test
 */

#include <stdio.h>
#include <string.h>

#include "../include/types.h"
#include "../include/structs.h"

extern PLAYER   lcp;
extern short    t_min;
extern short    t_hour;
extern short    date_day;
extern short    dt_mon;
extern short    dt_year;
extern short    ani_cnt;
extern short    g_secs;
extern BOOL16   ph_ans;
extern BOOL16   ph_call;
extern BOOL16   introSeq;
extern void     gameSim1();

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
        memset(&lcp, 0, sizeof(lcp));
        lcp.thirst_timer_max    = 30;   /* thirst rises every 30 min */
        lcp.thirst_timer        = 30;
        lcp.hunger_timer_max    = 45;   /* hunger rises every 45 min */
        lcp.hunger_timer        = 45;
        lcp.bathroom_timer_max  = 120;
        lcp.bathroom_timer      = 120;
        lcp.happiness           = 1;    /* MOOD_CONTENT   */
        lcp.happiness_initial_countdown  = 6;
        lcp.happiness_duration_happy     = 4;
        lcp.happiness_duration_content   = 6;
        lcp.happiness_duration_active    = 6;
        lcp.happiness_direction = 1;
        lcp.sickness_level      = 0;

        /* Sim entry conditions. */
        ani_cnt  = 0;    /* (counter & 7) == 0 -> tick */
        g_secs    = 0;
        t_min            = 0;
        t_hour              = 6;    /* 06:00:00 */
        date_day                = 1;
        dt_mon              = 0;
        dt_year               = 0;

        /* Suppress the random phone-call branch. */
        introSeq   = YES;
        ph_ans     = NO;
        ph_call  = NO;

        /* Drive 24 game-hours (86400 game-seconds). */
        for (i = 0; i < 86400L; i++)
                gameSim1();

        /* Clock should have advanced exactly 24h -- back to 06:00:00. */
        CHECK(t_hour == 6,   "t_hour != 6 after 86400 seconds");
        CHECK(t_min == 0, "t_min != 0 after 86400 seconds");
        CHECK(g_secs == 0, "g_secs != 0");

        /* Calendar should have rolled over exactly one day. */
        CHECK(date_day == 2, "date_day did not advance to 2");

        /* thirst_timer counts down each minute; after 24*60=1440 minutes
           with timer_max=30 it wraps 48 times.  Each wrap raises
           thirst_level (capped at 3 then triggers lcp_become_sick).
           Timer at end: 1440 % 30 == 0 so it resets to 30.            */
        CHECK(lcp.thirst_timer == 30, "thirst_timer end value wrong");
        CHECK(lcp.thirst_level >= 3,  "thirst_level should max out");

        /* hunger_timer: 1440 minutes with timer_max=45 = 32 wraps.
           At level 3 further wraps invoke lcp_become_sick which
           leaves level unchanged.                                     */
        CHECK(lcp.hunger_timer == 45, "hunger_timer end value wrong");
        CHECK(lcp.hunger_level >= 3,  "hunger_level should max out");

        /* bathroom_timer: 1440 min, timer_max=120 -> wraps 12 times.
           On first wrap bathroom_timer is set to 9999 and bathroom_need
           to YES, and stays that way.                                 */
        CHECK(lcp.bathroom_need == YES, "bathroom_need should be YES");

        /* Second run: 1 full game-hour from 07:00 with no need
           mutation, verifying pure clock advance.                     */
        memset(&lcp, 0, sizeof(lcp));
        lcp.thirst_timer        = 9999;
        lcp.thirst_timer_max    = 9999;
        lcp.hunger_timer        = 9999;
        lcp.hunger_timer_max    = 9999;
        lcp.bathroom_timer      = 9999;
        lcp.bathroom_timer_max  = 9999;
        lcp.happiness_duration_active = 9999;
        ani_cnt  = 0;
        g_secs    = 0;
        t_min            = 0;
        t_hour              = 7;
        introSeq   = YES;
        for (i = 0; i < 3600L; i++)
                gameSim1();
        CHECK(t_hour == 8,   "1-hour drive: t_hour != 8");
        CHECK(t_min == 0, "1-hour drive: t_min != 0");

        /* Sub-minute drive (30 seconds): only g_secs
           should advance; nothing else.                               */
        memset(&lcp, 0, sizeof(lcp));
        lcp.thirst_timer = lcp.thirst_timer_max = 9999;
        lcp.hunger_timer = lcp.hunger_timer_max = 9999;
        lcp.bathroom_timer = lcp.bathroom_timer_max = 9999;
        lcp.happiness_duration_active = 9999;
        ani_cnt = 0;
        g_secs = 0;
        t_min = 0;
        t_hour = 10;
        introSeq = YES;
        for (i = 0; i < 30L; i++)
                gameSim1();
        CHECK(g_secs == 30, "30-sec drive: counter wrong");
        CHECK(t_min == 0, "30-sec drive: minutes wrong");

        /* Non-tick frame: counter & 7 != 0 -> function must early-return. */
        g_secs = 42;
        ani_cnt = 3;      /* 3 & 7 == 3 != 0 */
        gameSim1();
        CHECK(g_secs == 42, "non-tick frame incremented counter");

        (void) thirst_hits; (void) hunger_hits;

        if (failures == 0) {
                printf("sim_tick: PASS  (all clock/needs progressions match)\n");
                return 0;
        }
        printf("sim_tick: FAIL  (%d assertion(s) failed)\n", failures);
        return 1;
}
