/*
 * ahouse.c -- walk-and-interact action handlers.
 *
 * Ports for actions that walk somewhere in the house, play an
 * interaction animation with SFX, and update world state.
 *
 * addr: a_readn(), a_gioob(),
 *       a_dance(), a_drink(), a_uset(),
 *       a_wakum(), a_gotbn()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern BOOL16   intro_sequence_active;
extern short    g_trel[];
extern short    lcp_x;
extern short    lcp_y;
extern BOOL16   ctrl_a_alarm_pressed_flag;
extern short    lcp_water_level;
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;
extern BOOL16   g_actif;
extern short    g_wtx;
extern short    g_wty;
extern short    PLAYER_STATE_ARRAY[];
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern void     a_getd();
extern short    lcp_toilet_door_open;
extern short    lcp_record_playing;
extern short    g_obidt;
extern short    g_obi09;
extern short    g_obi10;
extern BOOL16   midi_is_playing;
extern void     house_get_position_xy();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_lcyof;
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_selaf[];
extern short    g_seslm[];
extern short    randomRange();                  /* random.c */
#include <osbind.h>             /* Random() */

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     sp_ssco();
extern void     sp_sprs();
extern void     sp_upds();
extern void     sf_sele();
extern void     od_draw();
extern void     tt_on();
extern void     tt_off();
extern void     update_water_level_bar();
extern void     lcp_check_recovery();
extern void     a_driwa();
extern void     a_lists();
extern void     a_wakfa();
extern void     a_takes();
extern void     a_brust();
extern void     a_opcbc();
extern void     a_eatm();
extern void     a_clotd();
extern void     a_kitcc();
extern void     hide_lcp_sprites();
extern void     show_lcp_sprites();

/* a_readn: armchair + TV + 200-frame reading loop.
   addr: a_readn() */

void
a_readn()
{
        short           result;
        unsigned short  rnd;
        short           t;

        PLAYER_STATE_ARRAY[0] = STATE_READ_PAPER_HOLD;
        PLAYER_STATE_ARRAY[1] = STATE_READ_PAPER_TURN_PAGE;
        tt_on();
        house_get_position_xy(POS_TOP_ARMCHAIR,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        g_hamod         = HEAD_ANIM_READING;
        lcp_facing_direction   = FACING_LEFT;
        lcp_state              = STATE_SIT_IN_ARMCHAIR;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER;
        lcp_wait_head_reach_target();
        lcp_y = lcp_y + 8;

        t = 0;
        while (t < 200 && g_trel[0] == ACTION_NONE) {
                lcp_facing_direction = FACING_LEFT;
                lcp_state            = PLAYER_STATE_ARRAY[0];
                rnd = (unsigned short) Random();
                if ((rnd & 0xf) == 5)
                        lcp_state = PLAYER_STATE_ARRAY[1];
                game_tick_and_animate(1);
                t = t + 1;
        }

        lcp_y = lcp_y - 8;
        lcp_facing_direction = FACING_LEFT;
        lcp_state = STATE_SIT_IN_ARMCHAIR;
        game_tick_and_animate(2);
        tt_off();
}

/* a_gioob: undress and lie down, or reverse.
   addr: a_gioob() */

void
a_gioob()
{
        short   result;

        PLAYER_STATE_ARRAY[0] = STATE_UNDRESS_AT_BED;
        PLAYER_STATE_ARRAY[1] = STATE_LIE_DOWN_GETTING_IN;
        PLAYER_STATE_ARRAY[2] = STATE_LIE_DOWN_IN_BED;

        if (lcp.is_sleeping == NO) {
                house_get_position_xy(POS_MID_BED,
                                      &g_wtx, &g_wty);
                result = lcp_walk_to_destination();
                if (result != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_IDLE;
                g_hatas = 10;
                lcp_wait_head_reach_target();
                lcp.is_sleeping = YES;
                lcp_x = lcp_x - 10;
                lcp_facing_direction = FACING_RIGHT;
                lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(2);
                lcp_x = lcp_x - 8;
                lcp_state = PLAYER_STATE_ARRAY[1]; game_tick_and_animate(2);
                lcp_x = lcp_x - 2;
                lcp_state = PLAYER_STATE_ARRAY[2]; game_tick_and_animate(2);
        } else {
                lcp_facing_direction = FACING_RIGHT;
                lcp_x = lcp_x + 10;
                lcp_state = STATE_LIE_DOWN_GETTING_IN; game_tick_and_animate(2);
                lcp_x = lcp_x + 10;
                lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(2);
                lcp.is_sleeping = NO;
                lcp_state              = STATE_STAND_IDLE;
                g_hatas = 10;
                lcp_wait_head_reach_target();
                game_tick_and_animate(2);
        }
}

/* a_dance: turn on the record player if needed, then step-shift
   until the song ends or the event queue interrupts.
   addr: a_dance() */

void
a_dance()
{
        short   result;
        short   i;

        PLAYER_STATE_ARRAY[0] = STATE_DANCE_STEP_LEFT;
        PLAYER_STATE_ARRAY[1] = STATE_DANCE_STEP_RIGHT;

        if (lcp_record_playing == NO) {
                g_actif = YES;
                a_lists();
        }
        g_actif = NO;

        house_get_position_xy(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        g_wty = g_wty + 8;
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_wait_head_reach_target();

        i = 0;
        while (midi_is_playing != NO) {
                i = i + 1;
                lcp_state = PLAYER_STATE_ARRAY[i & 1];
                if (g_trel[0] != ACTION_NONE)
                        break;
                game_tick_and_animate(2);
        }

        lcp_state = STATE_STAND_SIDE_VIEW;
        game_tick_and_animate(0);
}

/* a_drink: sink -> glass -> tap -> drink -> reset thirst.
   addr: a_drink() */

void
a_drink()
{
        short   result;

        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        g_actif = YES;
        sp_ssco(SPRITE_GLASS);
        house_get_position_xy(POS_BTM_WATER_TAP,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();

        g_selaf[SPRITE_GLASS] = SPRITE_HIDDEN;
        sp_upds();
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        if (lcp_water_level != 0) {
                lcp_state = STATE_BEND_DOWN;
                lcp_facing_direction = FACING_RIGHT;
                game_tick_and_animate(0);
                update_water_level_bar(-3);
                g_hamod = HEAD_ANIM_DISABLED;
                lcp_state = STATE_DRINK_FROM_GLASS;
                game_tick_and_animate(16);
                lcp_state = STATE_STAND_FACING_SCREEN;
                lcp_y = lcp_y + 1;
                game_tick_and_animate(3);
                a_driwa(3);
        }

        lcp.thirst_level = NEED_SATISFIED;
        lcp.thirst_timer = lcp.thirst_timer_max;
        lcp_check_recovery();
        g_selaf[SPRITE_GLASS] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        g_actif = NO;
}

/* a_uset: 3-sprite door animation, sit + flush + refill.
   addr: a_uset() */

void
a_uset()
{
        short   result;
        short   saved_x;
        short   counter;

        house_get_position_xy(POS_MID_TOILET_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        /* Open the door if it isn't already. */
        if (lcp_toilet_door_open == NO) {
                lcp_facing_direction = FACING_LEFT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                od_draw(g_obidt, 187, 87);
                game_tick_and_animate(2);
                od_draw(g_obi09, 187, 87);
                sf_sele(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                od_draw(g_obi10, 187, 87);
                game_tick_and_animate(2);
                lcp_toilet_door_open = YES;
        }

        /* Walk into the toilet cubicle. */
        lcp_facing_direction = FACING_RIGHT;
        g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_3);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_3]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_3]] = 87;

        house_get_position_xy(POS_MID_TOILET_DOOR,
                              &g_wtx, &g_wty);
        g_wty = g_wty - 3;
        g_wtx = g_wtx - 10;
        g_actif = YES;
        lcp_walk_to_destination();
        saved_x = lcp_x;

        /* Close door behind the resident (3 sprite phases). */
        g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_2);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_2]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_2]] = 87;
        od_draw(g_obi09, 187, 87);
        game_tick_and_animate(1);

        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_1);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_1]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_1]] = 87;
        od_draw(g_obidt, 187, 87);
        hide_lcp_sprites();
        sf_sele(SFX_DOOR_CLOSE, 6L);
        game_tick_and_animate(1);

        /* Do the thing.  45..60 ticks, then flush + 16 tick refill. */
        counter = randomRange(45, 60);
        game_tick_and_animate(counter);
        sf_sele(SFX_TOILET_FLUSH, 6L);
        game_tick_and_animate(16);

        /* Reopen door + walk out. */
        g_selaf[SPRITE_DOOR_ANIM_1] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_2);
        show_lcp_sprites();
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_2]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_2]] = 87;
        od_draw(g_obi09, 187, 87);
        sf_sele(SFX_DOOR_OPEN, 6L);
        game_tick_and_animate(1);

        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_3);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_3]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_3]] = 87;
        od_draw(g_obi10, 187, 87);
        game_tick_and_animate(1);
        lcp_toilet_door_open = YES;

        lcp_x = saved_x;
        house_get_position_xy(POS_MID_TOILET_DOOR,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();

        if (lcp_toilet_door_open != NO) {
                g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
                sp_upds();
                game_tick_and_animate(0);
        }

        counter = randomRange(0, 100);
        if (lcp.initiative_threshold < counter ||
            intro_sequence_active != NO)
                a_clotd();

        lcp.bathroom_need  = NO;
        lcp.bathroom_timer = 9999;
        g_actif = NO;
}

/* a_wakum: scheduled morning routine.
   addr: a_wakum() */

void
a_wakum()
{
        short   counter;

        g_actif = YES;
        ctrl_a_alarm_pressed_flag = YES;
        counter = randomRange(40, 100);
        game_tick_and_animate(counter);
        if (lcp.is_sleeping == YES)
                a_gioob();

        g_actif = YES; a_wakfa();
        g_actif = YES; a_takes();
        g_actif = YES; a_brust();
        g_actif = YES; a_opcbc(0);
        g_actif = YES; a_eatm();
        g_actif = NO;
}

/* a_gotbn: scheduled bedtime routine.
   addr: a_gotbn() */

void
a_gotbn()
{
        g_actif = YES; a_takes();
        g_actif = YES; a_opcbc(1);
        g_actif = YES; a_kitcc();
        g_actif = YES; a_brust();
        g_actif = YES; a_gioob();
        g_actif = NO;
}

/* a_getd: pure head-anim routine.  Turns the head to face
   a canonical resting direction, then oscillates the vertical tilt bit
   four times (undressing / dressing motion communicated via head bob).
   No walking, no world state change.
   addr: a_getd() */

void
a_getd()
{
        short   entry_current;
        short   h;
        short   i;

        entry_current = g_hacur;
        h = g_hacur & 7;

        if (h == 0 || h == 1 || h == 7)
                g_hatas = 8;
        else if (h == 2)                        /* HEAD_ANIM_SHOWER value */
                g_hatas = 9;
        else if (h == 6)
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE |
                                         7 /* HEAD_MODE_H_AMPLITUDE mask */;
        else if (h == 3 || h == 4)
                g_hatas = 10;
        else if (h == 5)
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE |
                                         HEAD_ANIM_SHOWER;

        g_hamod = HEAD_ANIM_DISABLED;
        lcp_wait_head_reach_target();

        for (i = 0; i < 4; i = i + 1) {
                g_hatas = g_hacur & 7;
                lcp_wait_head_reach_target();
                g_hatas = g_hacur | 0x10;
                lcp_wait_head_reach_target();
        }

        g_hatas = entry_current;
        lcp_wait_head_reach_target();
}

/* li_lool / li_loor: the two 4-tick "stand-and-
   look" gestures used by the TV toggle, record player, and post-action
   idle transitions.  The 1985 code sets FACING_RIGHT in both -- the
   "left" / "right" naming refers to which head-frame direction the
   animation actually plays via g_hatas, not the body
   facing.  Preserved verbatim.
   addr: li_lool(), li_loor() */

void
li_lool()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        lcp_state = STATE_BEND_DOWN;
        game_tick_and_animate(4);
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

void
li_loor()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        lcp_state = STATE_BEND_DOWN;
        game_tick_and_animate(4);
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}
