/*
 * actions_games.c -- ACTION_PLAY_COMPUTER and ACTION_PLAY_A_GAME.
 *
 * Two long routines that share the "walk to a specific room, sit down,
 * and interact with a device" shape but branch out completely different
 * subsystems:
 *
 *   action_play_computer  -- Atari ST computer at the desk.  Types
 *                            randomly on the keyboard for ~ random
 *                            duration (0x80..0x1FF ticks), with a rare
 *                            "clear screen" gesture when the RNG rolls
 *                            small (< 3 out of 128).
 *   action_play_a_game    -- Filing cabinet -> game box -> table.
 *                            Prompts the user with a 5-game menu
 *                            (Anagrams / War / Poker / Blackjack /
 *                            Word Puzzles) via string_print + text
 *                            scroll pane, then hands off to the
 *                            picked game's main() and cleans up.
 *
 * addr: action_play_computer(), action_play_a_game()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern short    get_pressed_key();
extern void     select_random_click_sound();
extern void     tv_show_screen_clear();
extern void     string_print();
extern void     spritedata_select_carried_object_left();
extern void     spritedata_select_carried_object_right();
extern void     spritedata_select();
extern void     sprite_update_slots();
extern void     object_draw();
extern void     fill_top_rect_with_background();
extern void     action_walk_to_and_turn();
extern void     action_open_close_filing_cabinet();
extern void     action_sleep();
extern void     action_wander_idly();

/* Mini-game entry points.  Each lives in its own games/*.c when we
   port them for real; for now they're stubs in stubs.c that return
   immediately. */
extern void     anagram_main();
extern void     poker_war_main();
extern void     poker_main();
extern void     poker_blackjack_main();
extern void     word_puzzle_main();

/* action_play_computer: sit and type.  The 3 state constants in
   PLAYER_STATE_ARRAY are the two typing poses plus the resting
   sit-at-desk pose used between keystrokes and during the "clear
   screen" mini-animation.
   addr: action_play_computer() */

void
action_play_computer()
{
        short           walk_result;
        unsigned short  random_seed;
        unsigned short  random_duration;
        unsigned short  random_anim;
        unsigned short  type_counter;
        short           is_even_frame;

        PLAYER_STATE_ARRAY[0] = STATE_TYPING_HANDS_DOWN;
        PLAYER_STATE_ARRAY[1] = STATE_TYPING_HANDS_UP;
        PLAYER_STATE_ARRAY[2] = STATE_SITTING_AT_DESK;

        house_get_position_xy(POS_MID_COMPUTER_DESK,
                              &walk_target_x, &walk_target_y);
        walk_result = lcp_walk_to_destination();
        if (walk_result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        head_anim_mode = HEAD_ANIM_COMPUTER;

        /* First XBIOS Random call is discarded, matching the 1985
           binary; the second seeds the loop length. */
        (void) Random();
        random_seed = (unsigned short) Random();

        lcp_state = PLAYER_STATE_ARRAY[2];
        game_tick_and_animate(25);

        type_counter = 0;
        while ((short) type_counter <
               (short) ((random_seed & 0x1ff) | 0x80) &&
               intro_sequence_active == NO &&
               triggered_event_list[0] == ACTION_NONE) {
                random_duration = (unsigned short) Random();
                is_even_frame   = ((type_counter & 1) == 0);

                if (is_even_frame) {
                        lcp_facing_direction = FACING_RIGHT;
                        lcp_state = PLAYER_STATE_ARRAY[1];
                } else {
                        random_anim = (unsigned short) Random();
                        lcp_facing_direction = (random_anim & 2) >> 1;
                        lcp_state = PLAYER_STATE_ARRAY[0];
                        select_random_click_sound();
                }
                /* The 1985 code flips `is_even_frame` here, so the
                   even/odd branches actually swap for the tick call. */
                is_even_frame = !is_even_frame;
                if (is_even_frame)
                        game_tick_and_animate(0);
                else
                        game_tick_and_animate(random_duration & 3);

                /* Rare "clear the screen" gesture: pause typing, look
                   up, blank the display, look back down. */
                random_duration = (unsigned short) Random();
                if ((random_duration & 0x7f) < 3 && is_even_frame) {
                        head_anim_mode         = HEAD_ANIM_DISABLED;
                        lcp_state              = PLAYER_STATE_ARRAY[2];
                        head_anim_target_state = 10;
                        lcp_facing_direction   = FACING_RIGHT;
                        lcp_wait_head_reach_target();
                        tv_show_screen_clear();
                        game_tick_and_animate(5);
                        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                        lcp_wait_head_reach_target();
                        head_anim_mode = HEAD_ANIM_COMPUTER;
                }

                type_counter = type_counter + 1;
        }

        lcp_state            = STATE_STAND_FACING_SCREEN;
        lcp_facing_direction = FACING_RIGHT;
        head_anim_mode       = HEAD_ANIM_DISABLED;
        game_tick_and_animate(5);
}

/* action_play_a_game: main menu -> pick -> game -> cleanup.

   Structure (heavily nested in the 1985 code; slightly flattened here
   with an early-return-on-menu-timeout for readability):

   1. Walk to filing cabinet, open it if closed.
   2. Draw the 5-line game-selection prompt.
   3. Loop polling keys, sleeping between polls with occasional
      action_sleep(1) yawn animations when the menu times out (300 ->
      250 tick reload cycles).  Any digit '1'..'5' picked bumps out of
      the menu loop.
   4. Face the desk, grab SPRITE_GAME_BOX, walk to the kitchen table,
      set the game up (SPRITE_TABLE_SETTING), sit down (STATE_EAT_BITE
      pose), and hand off to the picked game's main().
   5. On return, walk everything back to the filing cabinet, close it,
      clear dog_visible.

   addr: action_play_a_game() */

void
action_play_a_game()
{
        short   walk_result;
        short   counter;
        short   selected_game;
        short   keycode;
        short   game_running;

        dog_visible        = YES;
        dog_idle_countdown = 1;

        house_get_position_xy(POS_TOP_FILING_CABINET,
                              &walk_target_x, &walk_target_y);
        walk_result = lcp_walk_to_destination();
        if (walk_result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        /* Open the filing cabinet if it isn't already. */
        if (lcp_filing_cabinet_open == NO) {
                lcp_state = STATE_BEND_DOWN;
                game_tick_and_animate(1);
                lcp_state = STATE_REACH_FORWARD;
                object_draw(object_id_filing_cabinet_open_1, 258, 47);
                game_tick_and_animate(2);
                lcp_state = STATE_PICK_UP_FROM_FLOOR;
                object_draw(object_id_filing_cabinet_open_2, 258, 47);
                game_tick_and_animate(2);
                lcp_filing_cabinet_open = YES;
                lcp_state = STATE_BEND_DOWN;
                game_tick_and_animate(1);
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(1);
        }

        /* Draw the menu. */
        lcp_state              = STATE_STAND_SIDE_VIEW;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();
        game_tick_and_animate(5);
        fill_top_rect_with_background(0x1b);
        text_scroll_timer      = 300;
        disable_key_input_flag = YES;
        string_print("What game do you want to play?", 5,  8, COLOR_black);
        string_print("1. Anagrams   2. War  3. Poker",  5, 16, COLOR_red);
        string_print("4. Blackjack  5. Word Puzzles",   5, 24, COLOR_red);

        keycode      = 0;
        game_running = NO;

        /* Menu poll loop -- keep at it until either a valid digit is
           pressed or the menu times out to a yawn animation. */
        while (text_scroll_timer != 0 || game_running == NO) {
                if (keycode > '0' && keycode < '6') {
                        text_scroll_timer      = 0;
                        lcp_facing_direction   = FACING_RIGHT;
                        lcp_state              = STATE_STAND_FACING_SCREEN;
                        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                        lcp_wait_head_reach_target();

                        action_walk_to_and_turn();
                        lcp_state = STATE_STAND_SIDE_VIEW;
                        spritedata_select_carried_object_left(SPRITE_GAME_BOX);
                        game_tick_and_animate(0);

                        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                                              &walk_target_x,
                                              &walk_target_y);
                        walk_target_y = walk_target_y + 6;
                        walk_target_x = walk_target_x + 2;
                        action_interruptible_flag = YES;
                        lcp_walk_to_destination();

                        sprite_layer_flags[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
                        spritedata_select(SPRITE_TABLE_SETTING);
                        sprite_pending_x[sprite_slot_map[SPRITE_TABLE_SETTING]] = 103;
                        sprite_pending_y[sprite_slot_map[SPRITE_TABLE_SETTING]] = 180;

                        house_get_position_xy(POS_BTM_TABLE_RIGHT,
                                              &walk_target_x,
                                              &walk_target_y);
                        lcp_walk_to_destination();
                        house_get_position_xy(POS_BTM_TABLE_LEFT,
                                              &walk_target_x,
                                              &walk_target_y);
                        lcp_walk_to_destination();

                        lcp_state            = STATE_STAND_SIDE_VIEW;
                        lcp_facing_direction = FACING_RIGHT;
                        spritedata_select_carried_object_right(SPRITE_GAME_BOX);
                        head_anim_target_state = 8;
                        lcp_wait_head_reach_target();

                        lcp_state = STATE_EAT_BITE;
                        lcp_y = lcp_y + 8;
                        lcp_x = lcp_x + 6;
                        game_tick_and_animate(0);
                        lcp_carrying_object_flag = NO;

                        /* Nudge the head sprite over so the game
                           overlay doesn't clip the resident. */
                        sprite_pending_x[sprite_slot_map[4]] =
                                sprite_pending_x[sprite_slot_map[4]] + 3;
                        sprite_pending_y[sprite_slot_map[4]] =
                                sprite_pending_y[sprite_slot_map[4]] - 4;
                        game_tick_and_animate(0);

                        /* Dispatch. */
                        switch (keycode) {
                        case '1': anagram_main();         break;
                        case '2': poker_war_main();       break;
                        case '3': poker_main();           break;
                        case '4': poker_blackjack_main(); break;
                        case '5': word_puzzle_main();     break;
                        }

                        /* Pack up. */
                        lcp_carrying_object_flag = YES;
                        spritedata_select_carried_object_left(SPRITE_GAME_BOX);
                        lcp_y = lcp_y - 8;
                        lcp_x = lcp_x - 6;
                        lcp_state = STATE_STAND_SIDE_VIEW;
                        game_tick_and_animate(0);

                        house_get_position_xy(POS_BTM_TABLE_RIGHT,
                                              &walk_target_x,
                                              &walk_target_y);
                        lcp_walk_to_destination();
                        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                                              &walk_target_x,
                                              &walk_target_y);
                        walk_target_y = walk_target_y + 5;
                        lcp_walk_to_destination();

                        sprite_layer_flags[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
                        sprite_update_slots();

                        house_get_position_xy(POS_TOP_FILING_CABINET,
                                              &walk_target_x,
                                              &walk_target_y);
                        lcp_walk_to_destination();

                        lcp_facing_direction   = FACING_RIGHT;
                        lcp_state              = STATE_STAND_FACING_SCREEN;
                        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                        sprite_layer_flags[4] = SPRITE_HIDDEN;
                        sprite_update_slots();
                        lcp_carrying_object_flag = NO;
                        lcp_wait_head_reach_target();
                        action_open_close_filing_cabinet();
                        action_interruptible_flag = NO;
                        dog_visible = NO;
                        return;
                }

                keycode = get_pressed_key();
                game_tick_and_animate(0);
                if (text_scroll_timer > 0x31 || game_running != NO)
                        continue;

                /* Menu timed out -- yawn and idle for a bit. */
                text_scroll_timer = 250;
                selected_game    = 8;
                walk_target_x    = lcp_x;
                walk_result      = get_floor_number_from_y(lcp_y);
                walk_target_y    = floor_center_y_coords[walk_result - 1];
                action_interruptible_flag = YES;
                lcp_walk_to_destination();
                action_interruptible_flag = NO;

                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_SIDE_VIEW;
                head_anim_target_state = 8;
                lcp_wait_head_reach_target();

                do {
                        walk_result = selected_game - 1;
                        if (selected_game == 0)
                                break;
                        action_sleep(1);
                        counter = randomRange(0, 2);
                        game_tick_and_animate(counter);
                        keycode = get_pressed_key();
                        selected_game = walk_result;
                } while (keycode < '1' || keycode > '5');

                lcp_state = STATE_STAND_SIDE_VIEW;
                game_tick_and_animate(0);
                house_get_position_xy(POS_TOP_FILING_CABINET,
                                      &walk_target_x, &walk_target_y);
                action_interruptible_flag = YES;
                lcp_walk_to_destination();
                action_interruptible_flag = NO;

                lcp_state              = STATE_STAND_SIDE_VIEW;
                head_anim_target_state = 8;
                lcp_wait_head_reach_target();
                game_running = YES;
        }

        action_wander_idly();
        disable_key_input_flag = NO;
        dog_visible = NO;
}
