/*
 * actions_games.c -- ACTION_PLAY_COMPUTER and ACTION_PLAY_A_GAME.
 *
 * Two long routines that share the "walk to a specific room, sit down,
 * and interact with a device" shape but branch out completely different
 * subsystems:
 *
 *   a_playc  -- Atari ST computer at the desk.  Types
 *                            randomly on the keyboard for ~ random
 *                            duration (0x80..0x1FF ticks), with a rare
 *                            "clear screen" gesture when the RNG rolls
 *                            small (< 3 out of 128).
 *   a_plaag    -- Filing cabinet -> game box -> table.
 *                            Prompts the user with a 5-game menu
 *                            (Anagrams / War / Poker / Blackjack /
 *                            Word Puzzles) via string_print + text
 *                            scroll pane, then hands off to the
 *                            picked game's main() and cleans up.
 *
 * addr: a_playc(), a_plaag()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern BOOL16   intro_sequence_active;
extern short    triggered_event_list[];
extern short    lcp_x;
extern short    lcp_y;
extern short    g_hatas;
extern short    g_hamod;
extern BOOL16   action_interruptible_flag;
extern short    g_wtx;
extern short    g_wty;
extern short    PLAYER_STATE_ARRAY[];
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern short    lcp_filing_cabinet_open;
extern short    disable_key_input_flag;
extern short    text_scroll_timer;
extern short    g_obi13;
extern short    g_obi14;
extern BOOL16   dog_visible;
extern short    dog_idle_countdown;
extern void     house_get_position_xy();
extern short    get_floor_number_from_y();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_lcyof;
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_selaf[];
extern short    g_seslm[];
extern short    floor_center_y_coords[];
extern short    randomRange();                  /* random.c */
#include <osbind.h>

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern short    get_pressed_key();
extern void     select_random_click_sound();
extern void     tv_scrc();
extern void     string_print();
extern void     sp_ssco();
extern void     sp_ss02();
extern void     sp_sprs();
extern void     sp_upds();
extern void     object_draw();
extern void     fill_top_rect_with_background();
extern void     a_watat();
extern void     a_opcfc();
extern void     a_sleep();
extern void     a_wandi();

/* Mini-game entry points.  Each lives in its own games/*.c when we
   port them for real; for now they're stubs in stubs.c that return
   immediately. */
extern void     anagram_main();
extern void     poker_war_main();
extern void     poker_main();
extern void     poker_blackjack_main();
extern void     word_puzzle_main();

/* a_playc: sit and type.  The 3 state constants in
   PLAYER_STATE_ARRAY are the two typing poses plus the resting
   sit-at-desk pose used between keystrokes and during the "clear
   screen" mini-animation.
   addr: a_playc() */

void
a_playc()
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
                              &g_wtx, &g_wty);
        walk_result = lcp_walk_to_destination();
        if (walk_result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        g_hamod = HEAD_ANIM_COMPUTER;

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
                        g_hamod         = HEAD_ANIM_DISABLED;
                        lcp_state              = PLAYER_STATE_ARRAY[2];
                        g_hatas = 10;
                        lcp_facing_direction   = FACING_RIGHT;
                        lcp_wait_head_reach_target();
                        tv_scrc();
                        game_tick_and_animate(5);
                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                        lcp_wait_head_reach_target();
                        g_hamod = HEAD_ANIM_COMPUTER;
                }

                type_counter = type_counter + 1;
        }

        lcp_state            = STATE_STAND_FACING_SCREEN;
        lcp_facing_direction = FACING_RIGHT;
        g_hamod       = HEAD_ANIM_DISABLED;
        game_tick_and_animate(5);
}

/* a_plaag: main menu -> pick -> game -> cleanup.

   Structure (heavily nested in the 1985 code; slightly flattened here
   with an early-return-on-menu-timeout for readability):

   1. Walk to filing cabinet, open it if closed.
   2. Draw the 5-line game-selection prompt.
   3. Loop polling keys, sleeping between polls with occasional
      a_sleep(1) yawn animations when the menu times out (300 ->
      250 tick reload cycles).  Any digit '1'..'5' picked bumps out of
      the menu loop.
   4. Face the desk, grab SPRITE_GAME_BOX, walk to the kitchen table,
      set the game up (SPRITE_TABLE_SETTING), sit down (STATE_EAT_BITE
      pose), and hand off to the picked game's main().
   5. On return, walk everything back to the filing cabinet, close it,
      clear dog_visible.

   addr: a_plaag() */

void
a_plaag()
{
        short   walk_result;
        short   counter;
        short   selected_game;
        short   keycode;
        short   game_running;

        dog_visible        = YES;
        dog_idle_countdown = 1;

        house_get_position_xy(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        walk_result = lcp_walk_to_destination();
        if (walk_result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        /* Open the filing cabinet if it isn't already. */
        if (lcp_filing_cabinet_open == NO) {
                lcp_state = STATE_BEND_DOWN;
                game_tick_and_animate(1);
                lcp_state = STATE_REACH_FORWARD;
                object_draw(g_obi13, 258, 47);
                game_tick_and_animate(2);
                lcp_state = STATE_PICK_UP_FROM_FLOOR;
                object_draw(g_obi14, 258, 47);
                game_tick_and_animate(2);
                lcp_filing_cabinet_open = YES;
                lcp_state = STATE_BEND_DOWN;
                game_tick_and_animate(1);
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(1);
        }

        /* Draw the menu. */
        lcp_state              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
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
                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                        lcp_wait_head_reach_target();

                        a_watat();
                        lcp_state = STATE_STAND_SIDE_VIEW;
                        sp_ssco(SPRITE_GAME_BOX);
                        game_tick_and_animate(0);

                        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                                              &g_wtx,
                                              &g_wty);
                        g_wty = g_wty + 6;
                        g_wtx = g_wtx + 2;
                        action_interruptible_flag = YES;
                        lcp_walk_to_destination();

                        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
                        sp_sprs(SPRITE_TABLE_SETTING);
                        g_sepex[g_seslm[SPRITE_TABLE_SETTING]] = 103;
                        g_sepey[g_seslm[SPRITE_TABLE_SETTING]] = 180;

                        house_get_position_xy(POS_BTM_TABLE_RIGHT,
                                              &g_wtx,
                                              &g_wty);
                        lcp_walk_to_destination();
                        house_get_position_xy(POS_BTM_TABLE_LEFT,
                                              &g_wtx,
                                              &g_wty);
                        lcp_walk_to_destination();

                        lcp_state            = STATE_STAND_SIDE_VIEW;
                        lcp_facing_direction = FACING_RIGHT;
                        sp_ss02(SPRITE_GAME_BOX);
                        g_hatas = 8;
                        lcp_wait_head_reach_target();

                        lcp_state = STATE_EAT_BITE;
                        lcp_y = lcp_y + 8;
                        lcp_x = lcp_x + 6;
                        game_tick_and_animate(0);
                        g_lcyof = NO;

                        /* Nudge the head sprite over so the game
                           overlay doesn't clip the resident. */
                        g_sepex[g_seslm[4]] =
                                g_sepex[g_seslm[4]] + 3;
                        g_sepey[g_seslm[4]] =
                                g_sepey[g_seslm[4]] - 4;
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
                        g_lcyof = YES;
                        sp_ssco(SPRITE_GAME_BOX);
                        lcp_y = lcp_y - 8;
                        lcp_x = lcp_x - 6;
                        lcp_state = STATE_STAND_SIDE_VIEW;
                        game_tick_and_animate(0);

                        house_get_position_xy(POS_BTM_TABLE_RIGHT,
                                              &g_wtx,
                                              &g_wty);
                        lcp_walk_to_destination();
                        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                                              &g_wtx,
                                              &g_wty);
                        g_wty = g_wty + 5;
                        lcp_walk_to_destination();

                        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
                        sp_upds();

                        house_get_position_xy(POS_TOP_FILING_CABINET,
                                              &g_wtx,
                                              &g_wty);
                        lcp_walk_to_destination();

                        lcp_facing_direction   = FACING_RIGHT;
                        lcp_state              = STATE_STAND_FACING_SCREEN;
                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                        g_selaf[4] = SPRITE_HIDDEN;
                        sp_upds();
                        g_lcyof = NO;
                        lcp_wait_head_reach_target();
                        a_opcfc();
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
                g_wtx    = lcp_x;
                walk_result      = get_floor_number_from_y(lcp_y);
                g_wty    = floor_center_y_coords[walk_result - 1];
                action_interruptible_flag = YES;
                lcp_walk_to_destination();
                action_interruptible_flag = NO;

                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                lcp_wait_head_reach_target();

                do {
                        walk_result = selected_game - 1;
                        if (selected_game == 0)
                                break;
                        a_sleep(1);
                        counter = randomRange(0, 2);
                        game_tick_and_animate(counter);
                        keycode = get_pressed_key();
                        selected_game = walk_result;
                } while (keycode < '1' || keycode > '5');

                lcp_state = STATE_STAND_SIDE_VIEW;
                game_tick_and_animate(0);
                house_get_position_xy(POS_TOP_FILING_CABINET,
                                      &g_wtx, &g_wty);
                action_interruptible_flag = YES;
                lcp_walk_to_destination();
                action_interruptible_flag = NO;

                lcp_state              = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                lcp_wait_head_reach_target();
                game_running = YES;
        }

        a_wandi();
        disable_key_input_flag = NO;
        dog_visible = NO;
}
