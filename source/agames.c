/*
 * agames.c -- ACTION_PLAY_COMPUTER and ACTION_PLAY_A_GAME.
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
 *                            Word Puzzles) via strPr + text
 *                            scroll pane, then hands off to the
 *                            picked game's main() and cleans up.
 *
 * addr: a_playc(), a_plaag()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "adoors.h"
#include "agames.h"
#include "aidle.h"
#include "events.h"
#include "games.h"
#include "globals.h"
#include "keyboard.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "tvanim.h"
#include "walk.h"


/* Mini-game entry points.  Each lives in its own games/*.c when we
   port them for real; for now they're stubs in stubs.c that return
   immediately. */

/* a_playc: sit and type.  The 3 state constants in
   pst_arr are the two typing poses plus the resting
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

        pst_arr[0] = STATE_HANDS_DOWN;
        pst_arr[1] = STATE_HANDS_UP;
        pst_arr[2] = STATE_SITTING_AT_DESK;

        hs_posXY(POS_MID_COMPUTER_DESK,
                              &g_wtx, &g_wty);
        walk_result = lcp_wkD();
        if (walk_result != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        g_hamod = HEAD_ANIM_COMPUTER;

        /* First XBIOS Random call is discarded, matching the 1985
           binary; the second seeds the loop length. */
        (void) Random();
        random_seed = (unsigned short) Random();

        lcp_st = pst_arr[2];
        gameTick(25);

        type_counter = 0;
        while ((short) type_counter <
               (short) ((random_seed & 0x1ff) | 0x80) &&
               introSeq == NO &&
               g_trel[0] == ACTION_NONE) {
                random_duration = (unsigned short) Random();
                is_even_frame   = ((type_counter & 1) == 0);

                if (is_even_frame) {
                        lcp_face = FACING_RIGHT;
                        lcp_st = pst_arr[1];
                } else {
                        random_anim = (unsigned short) Random();
                        lcp_face = (random_anim & 2) >> 1;
                        lcp_st = pst_arr[0];
                        sfClick();
                }
                /* The 1985 code flips `is_even_frame` here, so the
                   even/odd branches actually swap for the tick call. */
                is_even_frame = !is_even_frame;
                if (is_even_frame)
                        gameTick(0);
                else
                        gameTick(random_duration & 3);

                /* Rare "clear the screen" gesture: pause typing, look
                   up, blank the display, look back down. */
                random_duration = (unsigned short) Random();
                if ((random_duration & 0x7f) < 3 && is_even_frame) {
                        g_hamod         = HEAD_ANIM_DISABLED;
                        lcp_st              = pst_arr[2];
                        g_hatas = 10;
                        lcp_face   = FACING_RIGHT;
                        lcp_hwt();
                        tv_scrc();
                        gameTick(5);
                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                        lcp_hwt();
                        g_hamod = HEAD_ANIM_COMPUTER;
                }

                type_counter = type_counter + 1;
        }

        lcp_st            = STATE_STAND_FACING_SCREEN;
        lcp_face = FACING_RIGHT;
        g_hamod       = HEAD_ANIM_DISABLED;
        gameTick(5);
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
      clear dg_vis.

   addr: a_plaag() */

void
a_plaag()
{
        short   walk_result;
        short   counter;
        short   selected_game;
        short   keycode;
        short   game_running;

        dg_vis        = YES;
        dg_idlcd = 1;

        hs_posXY(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        walk_result = lcp_wkD();
        if (walk_result != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        /* Open the filing cabinet if it isn't already. */
        if (lcp_flcO == NO) {
                lcp_st = STATE_BEND_DOWN;
                gameTick(1);
                lcp_st = STATE_REACH_FORWARD;
                od_draw(OBJ_FILING_CAB_OPEN_1, 258, 47);
                gameTick(2);
                lcp_st = STATE_PICK_UP_FROM_FLOOR;
                od_draw(OBJ_FILING_CAB_OPEN_2, 258, 47);
                gameTick(2);
                lcp_flcO = YES;
                lcp_st = STATE_BEND_DOWN;
                gameTick(1);
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(1);
        }

        /* Draw the menu. */
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();
        gameTick(5);
        fillTopR(0x1b);
        tx_sctm      = 300;
        no_keyin = YES;
        strPr("What game do you want to play?", 5,  8, COLOR_black);
        strPr("1. Anagrams   2. War  3. Poker",  5, 16, COLOR_red);
        strPr("4. Blackjack  5. Word Puzzles",   5, 24, COLOR_red);

        keycode      = 0;
        game_running = NO;

        /* Menu poll loop -- keep at it until either a valid digit is
           pressed or the menu times out to a yawn animation. */
        while (tx_sctm != 0 || game_running == NO) {
                if (keycode > '0' && keycode < '6') {
                        tx_sctm      = 0;
                        lcp_face   = FACING_RIGHT;
                        lcp_st              = STATE_STAND_FACING_SCREEN;
                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                        lcp_hwt();

                        a_watat();
                        lcp_st = STATE_STAND_SIDE_VIEW;
                        sp_ssco(SPRITE_GAME_BOX);
                        gameTick(0);

                        hs_posXY(POS_BTM_KITCHEN_SINK,
                                              &g_wtx,
                                              &g_wty);
                        g_wty = g_wty + 6;
                        g_wtx = g_wtx + 2;
                        g_actif = YES;
                        lcp_wkD();

                        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
                        sp_sprs(SPRITE_TABLE_SETTING);
                        g_sepex[g_seslm[SPRITE_TABLE_SETTING]] = 103;
                        g_sepey[g_seslm[SPRITE_TABLE_SETTING]] = 180;

                        hs_posXY(POS_BTM_TABLE_RIGHT,
                                              &g_wtx,
                                              &g_wty);
                        lcp_wkD();
                        hs_posXY(POS_BTM_TABLE_LEFT,
                                              &g_wtx,
                                              &g_wty);
                        lcp_wkD();

                        lcp_st            = STATE_STAND_SIDE_VIEW;
                        lcp_face = FACING_RIGHT;
                        sp_ss02(SPRITE_GAME_BOX);
                        g_hatas = 8;
                        lcp_hwt();

                        lcp_st = STATE_EAT_BITE;
                        lcp_y = lcp_y + 8;
                        lcp_x = lcp_x + 6;
                        gameTick(0);
                        g_lcyof = NO;

                        /* Nudge the head sprite over so the game
                           overlay doesn't clip the resident. */
                        g_sepex[g_seslm[4]] =
                                g_sepex[g_seslm[4]] + 3;
                        g_sepey[g_seslm[4]] =
                                g_sepey[g_seslm[4]] - 4;
                        gameTick(0);

                        /* Dispatch. */
                        switch (keycode) {
                        case '1': ag_main();         break;
                        case '2': pk_wrMn();       break;
                        case '3': pk_main();           break;
                        case '4': pk_bjMn(); break;
                        case '5': wp_main();     break;
                        }

                        /* Pack up. */
                        g_lcyof = YES;
                        sp_ssco(SPRITE_GAME_BOX);
                        lcp_y = lcp_y - 8;
                        lcp_x = lcp_x - 6;
                        lcp_st = STATE_STAND_SIDE_VIEW;
                        gameTick(0);

                        hs_posXY(POS_BTM_TABLE_RIGHT,
                                              &g_wtx,
                                              &g_wty);
                        lcp_wkD();
                        hs_posXY(POS_BTM_KITCHEN_SINK,
                                              &g_wtx,
                                              &g_wty);
                        g_wty = g_wty + 5;
                        lcp_wkD();

                        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
                        sp_upds();

                        hs_posXY(POS_TOP_FILING_CABINET,
                                              &g_wtx,
                                              &g_wty);
                        lcp_wkD();

                        lcp_face   = FACING_RIGHT;
                        lcp_st              = STATE_STAND_FACING_SCREEN;
                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                        g_selaf[4] = SPRITE_HIDDEN;
                        sp_upds();
                        g_lcyof = NO;
                        lcp_hwt();
                        a_opcfc();
                        g_actif = NO;
                        dg_vis = NO;
                        return;
                }

                keycode = getKey();
                gameTick(0);
                if (tx_sctm > 0x31 || game_running != NO)
                        continue;

                /* Menu timed out -- yawn and idle for a bit. */
                tx_sctm = 250;
                selected_game    = 8;
                g_wtx    = lcp_x;
                walk_result      = getFlrY(lcp_y);
                g_wty    = flr_cy[walk_result - 1];
                g_actif = YES;
                lcp_wkD();
                g_actif = NO;

                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                lcp_hwt();

                do {
                        walk_result = selected_game - 1;
                        if (selected_game == 0)
                                break;
                        a_sleep(1);
                        counter = rndRng(0, 2);
                        gameTick(counter);
                        keycode = getKey();
                        selected_game = walk_result;
                } while (keycode < '1' || keycode > '5');

                lcp_st = STATE_STAND_SIDE_VIEW;
                gameTick(0);
                hs_posXY(POS_TOP_FILING_CABINET,
                                      &g_wtx, &g_wty);
                g_actif = YES;
                lcp_wkD();
                g_actif = NO;

                lcp_st              = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                lcp_hwt();
                game_running = YES;
        }

        a_wandi();
        no_keyin = NO;
        dg_vis = NO;
}
