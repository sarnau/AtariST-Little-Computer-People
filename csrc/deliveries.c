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
 * The Ctrl+D dog-food variant reuses event_receive_food_delivery with
 * delivery_is_for_dog=YES so the food goes to the dog bowl instead of
 * the kitchen cabinet.
 *
 * event_answer_phone is grouped here because it's the same event-queue
 * consumer even though it's a phone call rather than a delivery.
 *
 * addr: event_receive_food_delivery(), event_receive_book_delivery(),
 *       event_receive_record_delivery(), event_receive_dog_food(),
 *       event_answer_phone(), walk_to_front_door(),
 *       action_open_close_front_door(), action_open_close_cabinet()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     spritedata_select_carried_object_left();
extern void     spritedata_select_carried_object_right();
extern void     sprite_update_slots();
extern void     soundeffect_select();
extern void     object_draw();
extern void     action_feed_dog();
extern void     action_get_snack_from_fridge();
extern void     action_call_dog();
extern void     play_soundeffect_tv_click();
extern void     play_soundeffect_greeting();
extern void     play_soundeffect_speech();
extern void     play_soundeffect_head_nod();
extern void     screen_draw_food_cabinet();

extern void     action_open_close_front_door();
extern void     action_open_close_cabinet();

/* walk_to_front_door: tiny helper used by all four delivery events.
   addr: walk_to_front_door() */

void
walk_to_front_door()
{
        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();
}

/* action_open_close_front_door: toggle the front door with SFX+draw.
   Called from every delivery event and from event handlers via the
   initiative-threshold roll.  door_status=0 opens, 1 closes.
   addr: action_open_close_front_door() */

void
action_open_close_front_door(door_status)
short   door_status;
{
        if (door_status == 0) {
                if (lcp_front_door_open != NO)
                        return;
                lcp_facing_direction = FACING_RIGHT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                object_draw(object_id_door_front_open_1, 294, 151);
                soundeffect_select(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                object_draw(object_id_door_front_open_2, 294, 151);
                game_tick_and_animate(2);
                lcp_front_door_open = YES;
        } else {
                if (lcp_front_door_open == NO)
                        return;
                object_draw(object_id_door_front_open_1, 294, 151);
                game_tick_and_animate(2);
                object_draw(object_id_door_front_closed, 294, 151);
                soundeffect_select(SFX_DOOR_CLOSE, 6L);
                game_tick_and_animate(2);
                lcp_front_door_open = NO;
        }
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* action_open_close_cabinet: kitchen cabinet toggle used by the food
   delivery to reveal / hide the stocked cabinet interior.
   addr: action_open_close_cabinet() */

void
action_open_close_cabinet(open_close_status)
short   open_close_status;
{
        if (open_close_status == 0) {
                if (lcp_cabinet_open != NO)
                        return;
                lcp_cabinet_open = YES;
                lcp_state = STATE_REACH_INTO_CABINET;
                game_tick_and_animate(3);
                object_draw(object_id_cabinet_open_1, 46, 140);
                soundeffect_select(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                object_draw(object_id_cabinet_open_2, 46, 140);
                screen_draw_food_cabinet();
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(2);
        } else if (lcp_cabinet_open != NO) {
                lcp_cabinet_open = NO;
                lcp_state = STATE_REACH_INTO_CABINET;
                game_tick_and_animate(3);
                object_draw(object_id_cabinet_open_1, 46, 140);
                game_tick_and_animate(2);
                object_draw(object_id_cabinet_closed, 46, 140);
                soundeffect_select(SFX_DOOR_CLOSE, 6L);
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(2);
        }
}

/* Small helper: the "at the door, pick up" sequence common to all
   deliveries: face right + look forward, open door, bend / reach /
   bend / stand, then optionally close the door based on the
   initiative-threshold roll. */

static void
delivery_pickup_at_door()
{
        short   roll;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        action_open_close_front_door(0);

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
                action_open_close_front_door(1);
}

/* event_receive_food_delivery: Ctrl+F grocery event.  Also reused by
   event_receive_dog_food with delivery_is_for_dog set.
   addr: event_receive_food_delivery() */

void
event_receive_food_delivery()
{
        unsigned short  food_count;
        short           roll;

        action_interruptible_flag = YES;
        walk_to_front_door();
        delivery_pickup_at_door();

        if (delivery_is_for_dog == NO) {
                spritedata_select_carried_object_left(SPRITE_FOOD_PACKAGE);
                house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                                      &walk_target_x, &walk_target_y);
                lcp_walk_to_destination();

                sprite_layer_flags[9] = SPRITE_HIDDEN;
                sprite_update_slots();
                lcp_carrying_object_flag = NO;
                lcp_facing_direction     = FACING_RIGHT;
                lcp_state                = STATE_STAND_FACING_SCREEN;
                head_anim_target_state   = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();

                action_open_close_cabinet(0);

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
                        screen_draw_food_cabinet();
                        lcp_state = STATE_STAND_FACING_SCREEN;
                        game_tick_and_animate(1);
                }

                roll = randomRange(0, 100);
                if (lcp.initiative_threshold < roll)
                        action_open_close_cabinet(1);
                action_interruptible_flag = NO;
        } else {
                spritedata_select_carried_object_left(SPRITE_FOOD_PACKAGE);
                if (lcp_dog_bowl_status == BOWL_EMPTY) {
                        action_feed_dog(1);
                } else {
                        action_get_snack_from_fridge();
                        sprite_layer_flags[9] = SPRITE_HIDDEN;
                        sprite_update_slots();
                        lcp_carrying_object_flag = NO;
                }
        }
}

/* event_receive_book_delivery: Ctrl+B.  Book -> bookshelf.
   addr: event_receive_book_delivery() */

void
event_receive_book_delivery()
{
        action_interruptible_flag = YES;
        walk_to_front_door();
        delivery_pickup_at_door();

        spritedata_select_carried_object_left(SPRITE_BOOK);
        house_get_position_xy(POS_MID_BATHROOM_ENTRANCE,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();

        sprite_layer_flags[SPRITE_BOOK] = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_carrying_object_flag = NO;
        lcp_facing_direction     = FACING_RIGHT;
        lcp_state                = STATE_STAND_FACING_SCREEN;
        head_anim_target_state   = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_state = STATE_REACH_INTO_CABINET;
        game_tick_and_animate(3);
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(2);
        action_interruptible_flag = NO;
}

/* event_receive_record_delivery: Ctrl+R.  Record -> dance floor shelf.
   Note the original also increments lcp_food_count at the end -- this
   looks like an off-by-one bug (should have been counting records), but
   preserved for faithfulness.
   addr: event_receive_record_delivery() */

void
event_receive_record_delivery()
{
        action_interruptible_flag = YES;
        walk_to_front_door();
        delivery_pickup_at_door();

        spritedata_select_carried_object_left(SPRITE_VINYL_CARRY);
        house_get_position_xy(POS_TOP_DANCE_FLOOR,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        sprite_layer_flags[SPRITE_VINYL_CARRY] = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_carrying_object_flag = NO;
        lcp_wait_head_reach_target();

        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD; game_tick_and_animate(2);
        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);

        lcp_food_count = lcp_food_count + 1;    /* 1985 typo, preserved */
        action_interruptible_flag = NO;
}

/* event_receive_dog_food: Ctrl+D.  Trivial trampoline into food
   delivery with delivery_is_for_dog set.
   addr: event_receive_dog_food() */

void
event_receive_dog_food()
{
        delivery_is_for_dog = YES;
        event_receive_food_delivery();
        delivery_is_for_dog = NO;
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

        action_interruptible_flag = YES;
        action_call_dog();
        action_interruptible_flag = NO;

        head_anim_mode         = HEAD_ANIM_DISABLED;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();

        lcp_y = lcp_y + 6;
        lcp_state = STATE_PHONE_PICKUP;
        game_tick_and_animate(1);

        phone_answered_flag    = YES;
        phone_call_active_flag = NO;
        phone_hangup_flag      = YES;
        game_tick_and_animate(0);
        object_draw(object_id_phone_call, 190, 168);

        lcp_state = STATE_PHONE_TALKING;
        game_tick_and_animate(1);

        saved_frame            = head_sprite_frame;
        head_anim_target_state = HEAD_ANIM_DISABLED;
        head_anim_current      = HEAD_ANIM_DISABLED;

        ticks = randomRange(0x28, 0x32);
        while (ticks != 0) {
                pick = randomRange(0, 2);
                if (pick == 0) {
                        head_sprite_frame = 5;
                        play_soundeffect_tv_click();
                } else if (pick == 1) {
                        head_sprite_frame = 6;
                        subpick = randomRange(0, 1);
                        if (subpick == 0)
                                play_soundeffect_greeting();
                        else
                                play_soundeffect_speech();
                } else {
                        head_sprite_frame = saved_frame;
                        play_soundeffect_head_nod();
                }
                subpick = randomRange(1, 2);
                game_tick_and_animate(subpick);
                soundeffect_remaining_ticks = (long) subpick;
                ticks = ticks - 1;
        }

        phone_hangup_flag = YES;
        lcp_state         = STATE_PHONE_PICKUP;
        head_sprite_frame = saved_frame;
        game_tick_and_animate(1);

        lcp_y = lcp_y - 6;
        lcp_state = STATE_CROUCH_DOWN;
        game_tick_and_animate(1);

        while (petting_dog_active != NO)
                game_tick_and_animate(0);

        dog_pettable_flag = NO;
        lcp_y = lcp_y - 2;
        head_anim_target_state = 8;
        head_anim_current      = 8;
        lcp_state = STATE_STAND_SIDE_VIEW;
        lcp_wait_head_reach_target();
        game_tick_and_animate(0);
        phone_answered_flag = NO;
}
