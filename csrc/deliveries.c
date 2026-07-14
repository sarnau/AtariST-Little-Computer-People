/*
 * deliveries.c -- Ctrl+F/B/R/D/C doorbell events.
 *
 * All five deferred events share the same open-door-pick-up pattern:
 *   1. walk_to_front_door
 *   2. face right, stand facing screen, look forward
 *   3. open front door (unless already open)
 *   4. bend down, reach forward, bend down again (the "pickup")
 *   5. maybe close the door (initiative_threshold roll)
 *   6. attach a carried sprite and walk to the destination shelf
 *   7. bend down / reach forward again to put it down
 *
 * The Ctrl+D dog-food variant reuses er_food with
 * g_dvdog=YES so the food goes to the dog bowl instead of
 * the kitchen cabinet.
 *
 * event_answer_phone is grouped here because it's the same event-queue
 * consumer even though it's a phone call rather than a delivery.
 *
 * addr: er_food(), er_bood(),
 *       er_recd(), er_dogf(),
 *       event_answer_phone(), walk_to_front_door(),
 *       a_opcfd(), a_opecc()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern BOOL16   phone_answered_flag;
extern BOOL16   phone_call_active_flag;
extern short    lcp_y;
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;
extern short    g_hsfra;
extern long     g_sfret;
extern BOOL16   g_actif;
extern BOOL16   dog_pettable_flag;
extern short    g_wtx;
extern short    g_wty;
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern short    lcp_front_door_open;
extern short    lcp_cabinet_open;
extern short    lcp_dog_bowl_status;
extern short    lcp_food_count;
extern short    g_obidf;
extern short    g_obi05;
extern short    g_obi06;
extern short    g_obicc;
extern short    g_obico;
extern short    g_obi02;
extern short    g_obipc;
extern BOOL16   g_dvdog;
extern BOOL16   phone_hangup_flag;
extern BOOL16   g_ptdoa;
extern void     house_get_position_xy();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_lcyof;
extern short    g_selaf[];
extern short    randomRange();                  /* random.c */
extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     sp_ssco();
extern void     sp_ss02();
extern void     sp_upds();
extern void     sf_sele();
extern void     od_draw();
extern void     a_feedd();
extern void     a_gesff();
extern void     a_calld();
extern void     p_sftvc();
extern void     p_sfgrt();
extern void     p_sfspe();
extern void     p_sfhnd();
extern void     sc_drfc();

extern void     a_opcfd();
extern void     a_opecc();

/* walk_to_front_door: tiny helper used by all four delivery events.
   addr: walk_to_front_door() */

void
walk_to_front_door()
{
        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();
}

/* a_opcfd: toggle the front door with SFX+draw.
   Called from every delivery event and from event handlers via the
   initiative-threshold roll.  door_status=0 opens, 1 closes.
   addr: a_opcfd() */

void
a_opcfd(door_status)
short   door_status;
{
        if (door_status == 0) {
                if (lcp_front_door_open != NO)
                        return;
                lcp_facing_direction = FACING_RIGHT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                od_draw(g_obi05, 294, 151);
                sf_sele(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                od_draw(g_obi06, 294, 151);
                game_tick_and_animate(2);
                lcp_front_door_open = YES;
        } else {
                if (lcp_front_door_open == NO)
                        return;
                od_draw(g_obi05, 294, 151);
                game_tick_and_animate(2);
                od_draw(g_obidf, 294, 151);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                game_tick_and_animate(2);
                lcp_front_door_open = NO;
        }
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* a_opecc: kitchen cabinet toggle used by the food
   delivery to reveal / hide the stocked cabinet interior.
   addr: a_opecc() */

void
a_opecc(open_close_status)
short   open_close_status;
{
        if (open_close_status == 0) {
                if (lcp_cabinet_open != NO)
                        return;
                lcp_cabinet_open = YES;
                lcp_state = STATE_REACH_INTO_CABINET;
                game_tick_and_animate(3);
                od_draw(g_obico, 46, 140);
                sf_sele(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                od_draw(g_obi02, 46, 140);
                sc_drfc();
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(2);
        } else if (lcp_cabinet_open != NO) {
                lcp_cabinet_open = NO;
                lcp_state = STATE_REACH_INTO_CABINET;
                game_tick_and_animate(3);
                od_draw(g_obico, 46, 140);
                game_tick_and_animate(2);
                od_draw(g_obicc, 46, 140);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(2);
        }
}

/* Small helper: the "at the door, pick up" sequence common to all
   deliveries: face right + look forward, open door, bend / reach /
   bend / stand, then optionally close the door based on the
   initiative-threshold roll. */

static void
dv_pick()
{
        short   roll;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        a_opcfd(0);

        lcp_state = STATE_BEND_DOWN;
        game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD;
        game_tick_and_animate(2);
        lcp_state = STATE_BEND_DOWN;
        game_tick_and_animate(1);
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);

        roll = randomRange(0, 100);
        if (lcp.initiative_threshold < roll)
                a_opcfd(1);
}

/* er_food: Ctrl+F grocery event.  Also reused by
   er_dogf with g_dvdog set.
   addr: er_food() */

void
er_food()
{
        unsigned short  food_count;
        short           roll;

        g_actif = YES;
        walk_to_front_door();
        dv_pick();

        if (g_dvdog == NO) {
                sp_ssco(SPRITE_FOOD_PACKAGE);
                house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                lcp_walk_to_destination();

                g_selaf[9] = SPRITE_HIDDEN;
                sp_upds();
                g_lcyof = NO;
                lcp_facing_direction     = FACING_RIGHT;
                lcp_state                = STATE_STAND_FACING_SCREEN;
                g_hatas   = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();

                a_opecc(0);

                /* Stock the cabinet: the 3-bit food count lives at
                   bits 9..11 of door_states_and_flags.  Bump it up to
                   4 packs, one visible reach-in per pack. */
                for (;;) {
                        food_count = ((lcp.door_states_and_flags >> 9) & 7)
                                     + 1;
                        if (food_count >= 5)
                                break;
                        lcp.door_states_and_flags =
                                (food_count * 0x200) |
                                (lcp.door_states_and_flags & ~DSF_FOOD_MASK);
                        lcp_state = STATE_REACH_INTO_CABINET;
                        game_tick_and_animate(3);
                        sc_drfc();
                        lcp_state = STATE_STAND_FACING_SCREEN;
                        game_tick_and_animate(1);
                }

                roll = randomRange(0, 100);
                if (lcp.initiative_threshold < roll)
                        a_opecc(1);
                g_actif = NO;
        } else {
                sp_ssco(SPRITE_FOOD_PACKAGE);
                if (lcp_dog_bowl_status == BOWL_EMPTY) {
                        a_feedd(1);
                } else {
                        a_gesff();
                        g_selaf[9] = SPRITE_HIDDEN;
                        sp_upds();
                        g_lcyof = NO;
                }
        }
}

/* er_bood: Ctrl+B.  Book -> bookshelf.
   addr: er_bood() */

void
er_bood()
{
        g_actif = YES;
        walk_to_front_door();
        dv_pick();

        sp_ssco(SPRITE_BOOK);
        house_get_position_xy(POS_MID_BATHROOM_ENTRANCE,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();

        g_selaf[SPRITE_BOOK] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_facing_direction     = FACING_RIGHT;
        lcp_state                = STATE_STAND_FACING_SCREEN;
        g_hatas   = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_state = STATE_REACH_INTO_CABINET;
        game_tick_and_animate(3);
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(2);
        g_actif = NO;
}

/* er_recd: Ctrl+R.  Record -> dance floor shelf.
   Note the original also increments lcp_food_count at the end -- this
   looks like an off-by-one bug (should have been counting records), but
   preserved for faithfulness.
   addr: er_recd() */

void
er_recd()
{
        g_actif = YES;
        walk_to_front_door();
        dv_pick();

        sp_ssco(SPRITE_VINYL_CARRY);
        house_get_position_xy(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_VINYL_CARRY] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_wait_head_reach_target();

        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD; game_tick_and_animate(2);
        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);

        lcp_food_count = lcp_food_count + 1;    /* 1985 typo, preserved */
        g_actif = NO;
}

/* er_dogf: Ctrl+D.  Trivial trampoline into food
   delivery with g_dvdog set.
   addr: er_dogf() */

void
er_dogf()
{
        g_dvdog = YES;
        er_food();
        g_dvdog = NO;
}

/* event_answer_phone: Ctrl+C or random daytime call.  Calls the dog
   over first (which puts the resident at position 43 next to the
   phone), then picks up the handset, talks for 40..50 ticks with
   random head positions and speech SFX, hangs up, crouches, waits for
   any petting to finish, then stands.  phone_answered_flag prevents
   re-entry while the animation is running.

   addr: event_answer_phone() */

void
event_answer_phone()
{
        short   pick;
        short   saved_frame;
        short   ticks;
        short   subpick;

        g_actif = YES;
        a_calld();
        g_actif = NO;

        g_hamod         = HEAD_ANIM_DISABLED;
        g_hatas = 8;
        lcp_wait_head_reach_target();

        lcp_y = lcp_y + 6;
        lcp_state = STATE_PHONE_PICKUP;
        game_tick_and_animate(1);

        phone_answered_flag    = YES;
        phone_call_active_flag = NO;
        phone_hangup_flag      = YES;
        game_tick_and_animate(0);
        od_draw(g_obipc, 190, 168);

        lcp_state = STATE_PHONE_TALKING;
        game_tick_and_animate(1);

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        ticks = randomRange(0x28, 0x32);
        while (ticks != 0) {
                pick = randomRange(0, 2);
                if (pick == 0) {
                        g_hsfra = 5;
                        p_sftvc();
                } else if (pick == 1) {
                        g_hsfra = 6;
                        subpick = randomRange(0, 1);
                        if (subpick == 0)
                                p_sfgrt();
                        else
                                p_sfspe();
                } else {
                        g_hsfra = saved_frame;
                        p_sfhnd();
                }
                subpick = randomRange(1, 2);
                game_tick_and_animate(subpick);
                g_sfret = (long) subpick;
                ticks = ticks - 1;
        }

        phone_hangup_flag = YES;
        lcp_state         = STATE_PHONE_PICKUP;
        g_hsfra = saved_frame;
        game_tick_and_animate(1);

        lcp_y = lcp_y - 6;
        lcp_state = STATE_CROUCH_DOWN;
        game_tick_and_animate(1);

        while (g_ptdoa != NO)
                game_tick_and_animate(0);

        dog_pettable_flag = NO;
        lcp_y = lcp_y - 2;
        g_hatas = 8;
        g_hacur      = 8;
        lcp_state = STATE_STAND_SIDE_VIEW;
        lcp_wait_head_reach_target();
        game_tick_and_animate(0);
        phone_answered_flag = NO;
}
