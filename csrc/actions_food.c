/*
 * actions_food.c -- meal, kitchen, feed-dog, snack handlers.
 *
 * All four share the kitchen-cabinet / fridge / stove workflow and
 * update food-supply / hunger / dog-bowl state at their tail.
 *
 * addr: a_eatm(), a_kitcc(),
 *       a_feedd(), a_gesff()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern short    triggered_event_list[];
extern short    lcp_x;
extern short    lcp_y;
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;
extern short    g_hsfra;
extern BOOL16   action_interruptible_flag;
extern short    g_wtx;
extern short    g_wty;
extern short    PLAYER_STATE_ARRAY[];
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern short    lcp_dog_bowl_status;
extern short    g_obiso;
extern short    g_obisa[];
extern short    g_obi15;
extern short    g_obi16;
extern short    g_obi17;
extern short    dog_food_bowl_change;
extern void     house_get_position_xy();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_lcyof;
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_selaf[];
extern short    g_seslm[];
extern short    randomRange();                  /* random.c */
extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     sp_ssco();
extern void     sp_ss02();
extern void     sp_sprs();
extern void     sp_upds();
extern void     sf_sele();
extern void     object_draw();
extern void     a_opecc();
extern void     a_opecf();
extern void     a_kitcc();
extern void     sc_drfc();
extern void     lcp_check_recovery();

/* a_eatm: pot from cabinet -> stove (with cooking animation)
   -> table setting; ends with a kitchen_cabinet call to actually eat.
   addr: a_eatm() */

void
a_eatm()
{
        short   result;
        short   counter;
        short   pick;

        house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD;game_tick_and_animate(2);
        lcp_state = STATE_STAND_FACING_SCREEN; game_tick_and_animate(0);

        /* Pot from cabinet to stove */
        sp_ssco(SPRITE_COOKING_POT);
        house_get_position_xy(POS_BTM_STOVE,
                              &g_wtx, &g_wty);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();

        g_selaf[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        sp_upds();
        sp_ssco(SPRITE_COOKING_POT);
        g_lcyof = NO;
        g_sepex[g_seslm[SPRITE_COOKING_POT]] = 11;
        g_sepey[g_seslm[SPRITE_COOKING_POT]] = 172;

        lcp_facing_direction = FACING_LEFT;
        lcp_state            = STATE_BEND_AND_REACH;

        /* 30..50 tick cooking animation, rotating stove frames. */
        counter = randomRange(30, 50);
        while (counter != 0) {
                pick = randomRange(0, 2);
                object_draw(g_obisa[pick], 6, 172);
                game_tick_and_animate(1);
                counter = counter - 1;
        }
        object_draw(g_obiso, 6, 172);

        g_selaf[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        sp_upds();
        sp_ssco(SPRITE_55);

        /* Back to cabinet, then chain into kitchen_cabinet to eat. */
        house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();
        g_selaf[SPRITE_55] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        game_tick_and_animate(0);
        a_kitcc();
        action_interruptible_flag = NO;
}

/* a_kitcc: the eat routine.  Open cabinet, decrement
   food count, carry package to table, eat 10..20 bite/chew cycles,
   return the package.  This is where hunger actually gets reset.
   addr: a_kitcc() */

void
a_kitcc()
{
        short           saved_head_frame;
        short           chew_delay;
        short           eat_cycles;
        short           inner;
        unsigned short  food_count;
        short           roll;

        PLAYER_STATE_ARRAY[0] = STATE_EAT_BITE;
        PLAYER_STATE_ARRAY[1] = STATE_EAT_CHEW;
        action_interruptible_flag = YES;

        house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        a_opecc(0);

        food_count = (lcp.door_states_and_flags >> 9) & 7;
        if (food_count == 0) {
                game_tick_and_animate(2);
                action_interruptible_flag = NO;
                return;
        }

        /* Take one package: decrement the 3-bit food-count nibble. */
        lcp_state = STATE_REACH_INTO_CABINET;
        game_tick_and_animate(3);
        lcp.door_states_and_flags =
                (lcp.door_states_and_flags & ~DSF_FOOD_MASK) |
                ((food_count - 1) * 0x200);
        sc_drfc();
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(2);

        roll = randomRange(0, 100);
        if (lcp.initiative_threshold < roll)
                a_opecc(1);

        sp_ssco(SPRITE_FOOD_PACKAGE);
        house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();
        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();

        /* Drop a table setting sprite in the foreground. */
        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TABLE_SETTING);
        g_sepex[g_seslm[SPRITE_TABLE_SETTING]] = 103;
        g_sepey[g_seslm[SPRITE_TABLE_SETTING]] = 180;

        house_get_position_xy(POS_BTM_TABLE_RIGHT,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();
        house_get_position_xy(POS_BTM_TABLE_LEFT,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();

        g_hamod       = HEAD_ANIM_DISABLED;
        lcp_state            = STATE_STAND_SIDE_VIEW;
        lcp_facing_direction = FACING_RIGHT;
        sp_ss02(SPRITE_FOOD_PACKAGE);
        g_hatas = 8;
        lcp_wait_head_reach_target();

        saved_head_frame = g_hsfra;
        lcp_state        = PLAYER_STATE_ARRAY[0];
        lcp_y = lcp_y + 8;
        lcp_x = lcp_x + 6;
        eat_cycles       = randomRange(10, 20);
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;
        game_tick_and_animate(0);
        g_lcyof = NO;
        g_sepex[g_seslm[SPRITE_FOOD_PACKAGE]] =
                g_sepex[g_seslm[SPRITE_FOOD_PACKAGE]] + 3;
        g_sepey[g_seslm[SPRITE_FOOD_PACKAGE]] =
                g_sepey[g_seslm[SPRITE_FOOD_PACKAGE]] - 4;
        game_tick_and_animate(0);

        while (eat_cycles > 0) {
                lcp_state = PLAYER_STATE_ARRAY[1];
                game_tick_and_animate(2);
                g_hsfra = 0;
                chew_delay = randomRange(1, 2);
                game_tick_and_animate(chew_delay);
                lcp_state = PLAYER_STATE_ARRAY[0];
                g_hsfra = saved_head_frame;
                game_tick_and_animate(0);

                inner = randomRange(4, 8);
                while (inner > 0 &&
                       triggered_event_list[0] == ACTION_NONE) {
                        chew_delay = randomRange(1, 2);
                        game_tick_and_animate(chew_delay);
                        g_hsfra = 1;
                        game_tick_and_animate(0);
                        g_hsfra = 2;
                        game_tick_and_animate(0);
                        inner = inner - 1;
                }
                g_hsfra = saved_head_frame;
                eat_cycles = eat_cycles - 1;
        }

        g_lcyof = YES;
        g_hatas   = 8;
        g_hacur        = 8;
        sp_ssco(SPRITE_FOOD_PACKAGE);
        lcp_y = lcp_y - 8;
        lcp_x = lcp_x - 6;
        lcp_state = STATE_STAND_SIDE_VIEW;
        lcp_wait_head_reach_target();
        game_tick_and_animate(0);

        house_get_position_xy(POS_BTM_TABLE_RIGHT,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();
        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_FOOD_PACKAGE]  = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        game_tick_and_animate(4);

        lcp.hunger_level   = NEED_SATISFIED;
        lcp.bathroom_timer = lcp.bathroom_timer_max;
        lcp_check_recovery();
        action_interruptible_flag = NO;
}

/* a_feedd: fridge -> dog bowl -> fridge.  Called both
   standalone (value == 0, open fridge first) and from the Ctrl+D
   delivery path (value == 1, already have the package in hand).
   addr: a_feedd() */

void
a_feedd(value)
short   value;
{
        short   result;

        if (value == 0) {
                house_get_position_xy(POS_BTM_FRIDGE,
                                      &g_wtx, &g_wty);
                result = lcp_walk_to_destination();
                if (result != 0)
                        return;

                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();

                lcp_facing_direction = FACING_LEFT;
                lcp_state            = STATE_REACH_INTO_CABINET;
                object_draw(g_obi15, 24, 153);
                game_tick_and_animate(1);
                object_draw(g_obi16, 24, 153);
                sf_sele(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(1);
                object_draw(g_obi17, 24, 153);
                game_tick_and_animate(1);

                lcp_facing_direction = FACING_RIGHT;
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(2);

                lcp_facing_direction = FACING_LEFT;
                lcp_state = STATE_REACH_INTO_CABINET;
                game_tick_and_animate(3);

                lcp_facing_direction = FACING_RIGHT;
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(2);

                object_draw(g_obi16, 24, 153);
                game_tick_and_animate(1);
                object_draw(g_obi15, 24, 153);
                sf_sele(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(1);

                sp_ssco(SPRITE_FOOD_PACKAGE);
        }

        /* Package -> dog bowl (fill it). */
        house_get_position_xy(POS_BTM_DOG_BOWL,
                              &g_wtx, &g_wty);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_wait_head_reach_target();

        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD;game_tick_and_animate(2);
        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);

        dog_food_bowl_change = 1;
        lcp_dog_bowl_status  = BOWL_FULL;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);

        /* Package back to fridge. */
        sp_ssco(SPRITE_FOOD_PACKAGE);
        house_get_position_xy(POS_BTM_FRIDGE,
                              &g_wtx, &g_wty);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();

        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        a_opecf();
        action_interruptible_flag = NO;
}

/* a_gesff: trampoline into a_opecf
   after walking to the fridge.
   addr: a_gesff() */

void
a_gesff()
{
        short   result;

        house_get_position_xy(POS_BTM_FRIDGE,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result == 0)
                a_opecf();
}
