/*
 * agames.c -- ACTION_PLAY_COMPUTER and ACTION_PLAY_A_GAME.
 * a_playc: type at computer 0x80..0x1FF ticks, rare "clear screen".
 * a_plaag: filing cabinet -> game menu (1..5) -> game main -> cleanup.
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


/* a_playc -> parts/a_playc.c (STX: 0xdece object, 0x12e86,
   immediately before tv_scrc). */

/* a_plaag: cabinet -> menu -> game -> cleanup.
   tx_sctm timeout (300 -> 250 reload), a_sleep(1) yawn between polls.
   addr: a_plaag() */


void
a_plaag()
{
        short   spare0;
        short   keycode;
        short   game_running;
        short   spare3, spare4, spare5, spare6;
        short   selected_game;

        dg_vis        = YES;
        dg_idlcd = 1;

        hs_posXY(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        if (lcp_flcO == NO) {
                lcp_st = STATE_BEND_DOWN;
                gameTick(1);
                lcp_st = STATE_REACH_FORWARD;
                od_draw(od_fio1, 258, 47);
                gameTick(2);
                lcp_st = STATE_PICK_UP_FROM_FLOOR;
                od_draw(od_fio2, 258, 47);
                gameTick(2);
                lcp_flcO = YES;
                lcp_st = STATE_BEND_DOWN;
                gameTick(1);
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(1);
        }

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

        while (keycode < '1' || keycode > '5') {
        keycode = getKey();
        gameTick(0);

        if (tx_sctm < 0x32 && game_running == NO) {
                /* Menu timed out -- yawn and idle. */
                tx_sctm      = 250;
                selected_game    = 8;
                g_wtx    = lcp_x;
                g_wty    = flr_cy[getFlrY(lcp_y) - 1];
                g_actif = YES;
                lcp_wkD();
                g_actif = NO;

                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                lcp_hwt();

                while (selected_game-- != 0) {
                        a_sleep(1);
                        gameTick(rndRng(0, 2));
                        keycode = getKey();
                        if (keycode >= '1' && keycode <= '5')
                                break;
                }

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

        else if (tx_sctm == 0 && game_running != NO) {
                a_wandi();
                no_keyin = NO;
                dg_vis = NO;
                return;
        }

        }

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
                              &g_wtx, &g_wty);
        g_wty += 6;
        g_wtx += 2;
        g_actif = YES;
        lcp_wkD();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TABLE_SETTING);
        g_sepex[g_seslm[SPRITE_TABLE_SETTING]] = 103;
        g_sepey[g_seslm[SPRITE_TABLE_SETTING]] = 180;

        hs_posXY(POS_BTM_TABLE_RIGHT,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_TABLE_LEFT,
                              &g_wtx, &g_wty);
        lcp_wkD();

        lcp_st            = STATE_STAND_SIDE_VIEW;
        lcp_face = FACING_RIGHT;
        sp_ss02(SPRITE_GAME_BOX);
        g_hatas = 8;
        lcp_hwt();

        lcp_st = STATE_EAT_BITE;
        lcp_y += 8;
        lcp_x += 6;
        gameTick(0);
        g_lcyof = NO;

        g_sepex[g_seslm[SPRITE_GAME_BOX]] += 3;
        g_sepey[g_seslm[SPRITE_GAME_BOX]] -= 4;
        gameTick(0);

        if (keycode == '1')
                ag_main();
        else if (keycode == '2')
                pk_wrMn();
        else if (keycode == '3')
                pk_main();
        else if (keycode == '4')
                pk_bjMn();
        else if (keycode == '5')
                wp_main();

        g_lcyof = YES;
        sp_ssco(SPRITE_GAME_BOX);
        lcp_y -= 8;
        lcp_x -= 6;
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);

        hs_posXY(POS_BTM_TABLE_RIGHT,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        g_wty += 5;
        lcp_wkD();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        sp_upds();

        hs_posXY(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        lcp_wkD();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_hwt();
        a_opcfc();
        dg_vis = NO;
        g_actif = NO;
}
