/*
 * abathrm.c -- hygiene handlers.
 *
 * All three share the bathroom-sink / shower-cubicle walk-and-animate
 * pattern, without persistent world-state updates (unlike toilet or
 * kitchen).  Water-running SFX is toggled on entry and stopped on exit.
 *
 * addr: a_takes(), a_brust(), a_washh()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    g_trel[];
extern short    lcp_x;
extern short    lcp_y;
extern short    g_hatas;
extern short    g_hamod;
extern BOOL16   g_actif;
extern short    g_wtx;
extern short    g_wty;
extern short    PLAYER_STATE_ARRAY[];
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern short    g_sfplf;
extern short    g_sfpli;
extern void     house_get_position_xy();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_selaf[];
extern short    g_seslm[];
extern short    randomRange();                  /* random.c */
#include <osbind.h>             /* Random() */

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     sp_sprs();
extern void     sp_upds();
extern void     sf_sele();
extern void     sf_so();
extern void     sp_ssco();

/* a_takes: enter the shower cubicle, randomly alternate
   scrub / wash blocks for 20..25 cycles, exit.  Head-anim mode gets a
   dedicated HEAD_ANIM_SHOWER so the head bobs left/right in step.
   addr: a_takes() */

void
a_takes()
{
        short   result;
        short   count;
        short   pick;

        house_get_position_xy(POS_MID_SHOWER_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        house_get_position_xy(POS_MID_SHOWER_INSIDE,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_walk_to_destination();

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_SHOWER_STAND;
        lcp_x = lcp_x - 8;
        lcp_y = lcp_y - 23;
        g_hatas = 8;
        lcp_wait_head_reach_target();
        g_hamod = HEAD_ANIM_SHOWER;

        count = randomRange(20, 25);
        while (count != 0) {
                pick = randomRange(0, 1);
                if (pick == 0) {
                        lcp_state = STATE_SHOWER_SCRUB_LEFT;  game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_SCRUB_RIGHT; game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_SCRUB_LEFT;  game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_SCRUB_RIGHT; game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_STAND;       game_tick_and_animate(4);
                } else {
                        lcp_state = STATE_SHOWER_WASH_LEFT;   game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_WASH_RIGHT;  game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_WASH_LEFT;   game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_WASH_RIGHT;  game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_STAND;       game_tick_and_animate(4);
                }
                count = count - 1;
        }

        lcp_state = STATE_STAND_FACING_SCREEN;
        lcp_y = lcp_y + 29;
        game_tick_and_animate(2);
        house_get_position_xy(POS_MID_SHOWER_DOOR,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();
        g_hamod = HEAD_ANIM_DISABLED;
        g_actif = NO;
}

/* a_brust: 24..35 cycle brush loop.  The "toothbrush"
   sprite is actually SPRITE_STUDY_DOOR_FRAME (id 6) repositioned above
   the resident's head, alternating between two X positions.
   addr: a_brust() */

void
a_brust()
{
        unsigned short  brush_cycles;
        short           result;
        short           x_left;
        short           x_right;

        brush_cycles = (unsigned short) randomRange(24, 35);
        house_get_position_xy(POS_MID_BATHROOM_SINK,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        g_hamod = HEAD_ANIM_DISABLED;
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_BRUSH_TEETH;
        g_hatas = 10;
        lcp_y = lcp_y - 2;
        lcp_wait_head_reach_target();

        g_selaf[SPRITE_STUDY_DOOR_FRAME] = SPRITE_BEHIND_LCP;
        sp_sprs(SPRITE_STUDY_DOOR_FRAME);
        x_left  = lcp_x + 8;
        x_right = lcp_x + 12;
        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
        g_sepey[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = lcp_y - 24;

        while (brush_cycles != 0) {
                if (((brush_cycles - 1) & 1) == 0)
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_right;
                else
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
                game_tick_and_animate(0);
                brush_cycles = brush_cycles - 1;
        }

        g_selaf[SPRITE_STUDY_DOOR_FRAME] = SPRITE_HIDDEN;
        sp_upds();
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        lcp_y = lcp_y + 2;
        game_tick_and_animate(0);
}

/* a_washh: sink + water + 4..127 random wash cycles picking
   from 3 hand-position states.  Stops water on any interruption.
   addr: a_washh() */

void
a_washh()
{
        short           result;
        unsigned short  rnd;
        unsigned short  val;
        unsigned short  last_pick;
        short           counter;

        PLAYER_STATE_ARRAY[0] = STATE_WASH_HANDS_CENTER;
        PLAYER_STATE_ARRAY[1] = STATE_WASH_HANDS_LEFT;
        PLAYER_STATE_ARRAY[2] = STATE_WASH_HANDS_RIGHT;

        house_get_position_xy(POS_MID_BATHROOM_SINK,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        rnd = (unsigned short) Random();
        sf_sele(SFX_WATER_RUNNING, 10000L);

        counter   = 0;
        last_pick = 0;
        while (counter < (short) ((rnd & 0x7f) | 4) &&
               g_trel[0] == ACTION_NONE) {
                val = (unsigned short) Random();
                while ((val & 3) == last_pick)
                        val = (unsigned short) Random();
                val = val & 3;
                last_pick = val;
                if (val == 3)
                        lcp_state = PLAYER_STATE_ARRAY[1];
                else
                        lcp_state = PLAYER_STATE_ARRAY[val];
                lcp_facing_direction = (val == 3) ? FACING_LEFT : FACING_RIGHT;
                game_tick_and_animate(1);
                counter = counter + 1;
        }

        if (g_sfplf != NO &&
            g_sfpli == SFX_WATER_RUNNING)
                sf_so();

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* a_driwa: fill / drink a glass (carried_object
   pre-selected by the caller).  Runs the same 3-position hand-shift
   loop as a_washh but scoped to lower amplitudes (bit 0x1f
   instead of 0x7f), so it plays for ~4..35 ticks instead of ~4..127.
   The `value` argument is the SPRITE_ID of the object being carried
   (typically SPRITE_GLASS).
   addr: a_driwa() */

void
a_driwa(value)
short   value;
{
        unsigned short  rnd;
        unsigned short  pick;
        unsigned short  last_pick;
        short           counter;

        PLAYER_STATE_ARRAY[0] = STATE_WASH_HANDS_CENTER;
        PLAYER_STATE_ARRAY[1] = STATE_WASH_HANDS_LEFT;
        PLAYER_STATE_ARRAY[2] = STATE_WASH_HANDS_RIGHT;

        sp_ssco(value);
        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();
        g_selaf[value] = SPRITE_HIDDEN;
        sp_upds();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        rnd = (unsigned short) Random();
        sf_sele(SFX_WATER_RUNNING, 10000L);

        last_pick = 0;
        for (counter = 0;
             counter < (short) ((rnd & 0x1f) | 4);
             counter = counter + 1) {
                pick = (unsigned short) Random();
                while ((pick & 3) == last_pick)
                        pick = (unsigned short) Random();
                pick = pick & 3;
                last_pick = pick;
                if (pick == 3)
                        lcp_state = PLAYER_STATE_ARRAY[1];
                else
                        lcp_state = PLAYER_STATE_ARRAY[pick];
                lcp_facing_direction = (pick == 3) ? FACING_LEFT : FACING_RIGHT;
                game_tick_and_animate(1);
        }

        if (g_sfplf != NO &&
            g_sfpli == SFX_WATER_RUNNING)
                sf_so();

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}
