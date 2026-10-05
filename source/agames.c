/*
 * agames.c -- ACTION_PLAY_COMPUTER and ACTION_PLAY_A_GAME.
 * useComputer: type at computer 0x80..0x1FF ticks, rare "clear screen".
 * playGame: filing cabinet -> game menu (1..5) -> game main -> cleanup.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "protos.h"
#include "events.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprites.h"


/* useComputer lives in parts/useComputer.c, in the object right before tvClearAnim. */

/* The menu waits on the textTimer timeout (300, reloaded with 250 once it
   runs low); while idle the resident yawns (dozeOff(1)) between polls. */


void
playGame()
{
        short   spare0;
        short   keycode;
        short   game_running;
        short   spare3, spare4, spare5, spare6;
        short   selected_game;

        dogNoTopFlr        = YES;
        dogIdleCount = 1;

        posToXY(POS_TOP_FILING_CABINET,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();

        if (filingCabOpen == NO) {
                animState = STATE_BEND_DOWN;
                gameTick(1);
                animState = STATE_REACH_FORWARD;
                drawObject(OBJ_FILING_CAB_OPEN_1, FILING_CAB_X, FILING_CAB_Y);
                gameTick(2);
                animState = STATE_PICK_UP_FROM_FLOOR;
                drawObject(OBJ_FILING_CAB_OPEN_2, FILING_CAB_X, FILING_CAB_Y);
                gameTick(2);
                filingCabOpen = YES;
                animState = STATE_BEND_DOWN;
                gameTick(1);
                animState = STATE_STAND_FACING_SCREEN;
                gameTick(1);
        }

        animState              = STATE_STAND_SIDE_VIEW;
        headTarget = 8;
        waitHeadTurn();
        gameTick(5);
        fillPanel(0x1b);
        textTimer      = 300;
        keysBlocked = YES;
        printString("What game do you want to play?", 5,  8, COLOR_black);
        printString("1. Anagrams   2. War  3. Poker",  5, 16, COLOR_red);
        printString("4. Blackjack  5. Word Puzzles",   5, 24, COLOR_red);

        keycode      = 0;
        game_running = NO;

        while (keycode < '1' || keycode > '5') {
        keycode = getKey();
        gameTick(0);

        if (textTimer < 0x32 && game_running == NO) {
                /* Menu timed out -- yawn and idle. */
                textTimer      = 250;
                selected_game    = 8;
                walkXTarget    = resX;
                walkYTarget    = floorWalkY[floorOfY(resY) - 1];
                noPreempt = YES;
                walkToTarget();
                noPreempt = NO;

                resFacing   = FACING_RIGHT;
                animState              = STATE_STAND_SIDE_VIEW;
                headTarget = 8;
                waitHeadTurn();

                while (selected_game-- != 0) {
                        dozeOff(1);
                        gameTick(rndRng(0, 2));
                        keycode = getKey();
                        if (keycode >= '1' && keycode <= '5')
                                break;
                }

                animState = STATE_STAND_SIDE_VIEW;
                gameTick(0);
                posToXY(POS_TOP_FILING_CABINET,
                                      &walkXTarget, &walkYTarget);
                noPreempt = YES;
                walkToTarget();
                noPreempt = NO;

                animState              = STATE_STAND_SIDE_VIEW;
                headTarget = 8;
                waitHeadTurn();
                game_running = YES;
        }

        else if (textTimer == 0 && game_running != NO) {
                idleShrug();
                keysBlocked = NO;
                dogNoTopFlr = NO;
                return;
        }

        }

        textTimer      = 0;
        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();

        rummageCabinet();
        animState = STATE_STAND_SIDE_VIEW;
        carryBehind(SPRITE_GAME_BOX);
        gameTick(0);

        posToXY(POS_BTM_KITCHEN_SINK,
                              &walkXTarget, &walkYTarget);
        walkYTarget += 6;
        walkXTarget += 2;
        noPreempt = YES;
        walkToTarget();

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_TABLE_SETTING);
        pendX[spriteSlot[SPRITE_TABLE_SETTING]] = 103;
        pendY[spriteSlot[SPRITE_TABLE_SETTING]] = 180;

        posToXY(POS_BTM_TABLE_RIGHT,
                              &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_TABLE_LEFT,
                              &walkXTarget, &walkYTarget);
        walkToTarget();

        animState            = STATE_STAND_SIDE_VIEW;
        resFacing = FACING_RIGHT;
        carryInFront(SPRITE_GAME_BOX);
        headTarget = 8;
        waitHeadTurn();

        animState = STATE_EAT_BITE;
        resY += 8;
        resX += 6;
        gameTick(0);
        isCarrying = NO;

        pendX[spriteSlot[SPRITE_GAME_BOX]] += 3;
        pendY[spriteSlot[SPRITE_GAME_BOX]] -= 4;
        gameTick(0);

        if (keycode == '1')
                playAnagrams();
        else if (keycode == '2')
                playWar();
        else if (keycode == '3')
                playPoker();
        else if (keycode == '4')
                playBlackjack();
        else if (keycode == '5')
                playWordPuzzle();

        isCarrying = YES;
        carryBehind(SPRITE_GAME_BOX);
        resY -= 8;
        resX -= 6;
        animState = STATE_STAND_SIDE_VIEW;
        gameTick(0);

        posToXY(POS_BTM_TABLE_RIGHT,
                              &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_KITCHEN_SINK,
                              &walkXTarget, &walkYTarget);
        walkYTarget += 5;
        walkToTarget();

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        layoutSlots();

        posToXY(POS_TOP_FILING_CABINET,
                              &walkXTarget, &walkYTarget);
        walkToTarget();

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_ANIM_HORIZONTAL_RANGE;
        spriteLayer[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;
        waitHeadTurn();
        closeFilingCab();
        dogNoTopFlr = NO;
        noPreempt = NO;
}
