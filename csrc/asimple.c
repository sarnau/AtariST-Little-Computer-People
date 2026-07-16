/*
 * asimple.c -- short idle / gesture actions.
 *
 * Ports for do_action() handlers that don't need walking to a specific
 * house position and involve mostly head/body animation with sound.
 *
 * addr: a_wakfa(), a_hello(), a_yawas(),
 *       a_nodh(), a_petd(), a_calld()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern BOOL16   intro_sequence_active;
extern short    g_trel[];
extern BOOL16   ctrl_a_alarm_pressed_flag;
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;
extern short    g_hsfra;
extern long     g_sfret;
extern BOOL16   g_actif;
extern BOOL16   dog_pettable_flag;
extern short    g_wtx;
extern short    g_wty;
extern short    PLAYER_STATE_ARRAY[];
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern void     house_get_position_xy();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    randomRange();                  /* random.c */
extern void     do_action();                    /* actions.c */
extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     p_sftvc();
extern void     p_sfgrt();
extern void     p_sfspe();
extern void     p_sfhnd();
extern void     sf_so();

/* a_wakfa: Ctrl+A path.  Walks to the bedroom alarm,
   faces right, silences the alarm and clears the pressed flag.
   addr: a_wakfa() */

void
a_wakfa()
{
        short   result;

        house_get_position_xy(POS_MID_BEDROOM_WALK,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result == 0) {
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                ctrl_a_alarm_pressed_flag = NO;
        }
}

/* a_hello: face-forward wave with a random 20-40 head sequence.
   Interruptible via the deferred-event queue.
   addr: a_hello() */

void
a_hello()
{
        short   wave_count;
        short   pick;
        short   prev_pick;
        short   saved_frame;
        short   wait;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_wait_head_reach_target();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        wave_count = randomRange(20, 40);
        prev_pick  = 0;
        pick       = 0;
        while (wave_count != 0) {
                while (pick == prev_pick)
                        pick = randomRange(0, 2);
                prev_pick = pick;

                if (pick == 0) {
                        g_hsfra = 5;
                        p_sftvc();
                } else if (pick == 1) {
                        g_hsfra = 6;
                        if (randomRange(0, 1) == 0)
                                p_sfgrt();
                        else
                                p_sfspe();
                } else {
                        g_hsfra = 4;
                        p_sfhnd();
                }
                wait = randomRange(1, 2);
                game_tick_and_animate(wait);
                g_sfret = (long) wait;
                wave_count = wave_count - 1;
        }

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        game_tick_and_animate(0);
}

/* a_yawas: 15-frame idle yawn.
   addr: a_yawas() */

void
a_yawas()
{
        short   i;

        PLAYER_STATE_ARRAY[0]  = STATE_YAWN_MOUTH_OPEN;
        PLAYER_STATE_ARRAY[1]  = STATE_YAWN_STRETCH_ARMS;
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_wait_head_reach_target();

        for (i = 0; i < 15; i = i + 1) {
                lcp_state = PLAYER_STATE_ARRAY[i & 1];
                game_tick_and_animate(1);
        }
        lcp_state = STATE_STAND_SIDE_VIEW;
        game_tick_and_animate(0);
}

/* a_nodh: 3-frame nod with SFX.
   addr: a_nodh() */

void
a_nodh()
{
        short   saved_frame;

        PLAYER_STATE_ARRAY[0]  = STATE_WALK_FRAME_3_STEP;
        PLAYER_STATE_ARRAY[1]  = STATE_WALK_FRAME_4;
        PLAYER_STATE_ARRAY[2]  = STATE_WALK_FRAME_5;
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_wait_head_reach_target();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        g_hsfra = PLAYER_STATE_ARRAY[0];
        game_tick_and_animate(1);
        g_hsfra = PLAYER_STATE_ARRAY[1];
        game_tick_and_animate(1);
        g_hsfra = PLAYER_STATE_ARRAY[2];
        game_tick_and_animate(2);

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        game_tick_and_animate(0);
}

/* a_petd: call the dog if not already pettable, then wait
   100..200 frames (10 during intro) or until a new event queues.
   addr: a_petd() */

extern void a_calld();

void
a_petd()
{
        short   ticks;

        g_actif = YES;
        if (dog_pettable_flag == NO)
                a_calld();
        g_actif = NO;

        ticks = randomRange(100, 200);
        if (intro_sequence_active != NO)
                ticks = 10;

        do {
                ticks = ticks - 1;
                if (ticks == 0)
                        break;
                game_tick_and_animate(0);
        } while (g_trel[0] == ACTION_NONE);

        dog_pettable_flag = NO;
        lcp_state         = STATE_STAND_SIDE_VIEW;
        game_tick_and_animate(0);
}

/* a_calld: walk to POS_BTM_DOG_FOOD, crouch, set dog_pettable_flag.
   Real Ghidra behaviour -- see previous session for the derivation.
   addr: a_calld() */

void
a_calld()
{
        short   result;

        house_get_position_xy(POS_BTM_DOG_FOOD, &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        lcp_facing_direction   = FACING_RIGHT;
        g_hatas = 8;
        lcp_wait_head_reach_target();
        lcp_state = STATE_CROUCH_DOWN;
        game_tick_and_animate(5);
        dog_pettable_flag = YES;
}
