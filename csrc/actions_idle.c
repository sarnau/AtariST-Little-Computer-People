/*
 * actions_idle.c -- short "no-walk" idle / gesture handlers.
 *
 * All share the same shape: pick a pair of animation states, tick
 * through them for a short duration, return to STATE_STAND_SIDE_VIEW.
 * No walking, no world state mutation, no sound (except toggle_tv).
 *
 * addr: a_wandi(), a_peeka(),
 *       a_pacen(), a_toggt(), a_sleep()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    triggered_event_list[];
extern short    lcp_x;
extern short    lcp_y;
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;
extern short    g_hsfra;
extern short    g_wtx;
extern short    g_wty;
extern short    PLAYER_STATE_ARRAY[];
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern short    lcp_tv_on;
extern short    lcp_on_stairs_flag;
extern short    get_floor_number_from_y();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    floor_center_y_coords[];
extern short    randomRange();                  /* random.c */
extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     sf_sele();
extern void     tt_on();
extern void     tt_off();

/* a_wandi: two-state shrug idle.
   addr: a_wandi() */

void
a_wandi()
{
        PLAYER_STATE_ARRAY[0]  = STATE_IDLE_SHRUG_START;
        PLAYER_STATE_ARRAY[1]  = STATE_IDLE_SHRUG_HOLD;
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_wait_head_reach_target();

        lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(2);
        lcp_state = PLAYER_STATE_ARRAY[1]; game_tick_and_animate(5);
        lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(2);
        lcp_state = STATE_STAND_SIDE_VIEW; game_tick_and_animate(0);
}

/* a_peeka: 6-tick look-away with head frame 2.
   addr: a_peeka() */

void
a_peeka()
{
        short   saved_frame;

        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_wait_head_reach_target();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;
        g_hsfra      = 2;
        game_tick_and_animate(6);

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        game_tick_and_animate(0);
}

/* a_pacen: 15-frame side-shift alternation.
   addr: a_pacen() */

void
a_pacen()
{
        short   i;

        PLAYER_STATE_ARRAY[0]  = STATE_PACE_SHIFT_LEFT;
        PLAYER_STATE_ARRAY[1]  = STATE_PACE_SHIFT_RIGHT;
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

/* a_toggt: flip the TV state.  Both tt_on and tt_off
   handle their own SFX_TV_CLICK.
   addr: a_toggt() */

void
a_toggt()
{
        if (lcp_tv_on == NO)
                tt_on();
        else
                tt_off();
}

/* a_sleep: lie in bed, snore, optionally forever (value == -1 is
   the copy-protection punishment path).  On value == -1 the resident
   first walks to the current floor's center Y before lying down.
   addr: a_sleep() */

void
a_sleep(value)
short   value;
{
        short   duration;
        short   i;
        short   floor;

        PLAYER_STATE_ARRAY[0] = STATE_SLEEP_BREATHE_IN;
        PLAYER_STATE_ARRAY[1] = STATE_SLEEP_BREATHE_OUT;

        if (lcp_on_stairs_flag != NO)
                return;

        if (value == -1) {
                g_wtx = lcp_x;
                floor = get_floor_number_from_y(lcp_y);
                g_wty = floor_center_y_coords[floor - 1];
                if (lcp_walk_to_destination() != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                lcp_wait_head_reach_target();
        }

        duration = randomRange(7, 15);
        if (value != -1)
                duration = value;

        i = 0;
        while (i < duration &&
               triggered_event_list[0] == ACTION_NONE) {
                lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(1);
                lcp_state = PLAYER_STATE_ARRAY[1]; game_tick_and_animate(0);
                sf_sele(SFX_SNORING, 3L);
                game_tick_and_animate(1);
                lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(1);
                i = i + 1;
        }

        if (value == -1) {
                lcp_state = STATE_STAND_SIDE_VIEW;
                game_tick_and_animate(0);
        }
}
