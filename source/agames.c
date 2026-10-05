/*
 * agames.c -- playGame, the ACTION_PLAY_A_GAME handler.
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

/* Play a card or word game with the player.  The resident walks to the
   filing cabinet on the top floor, opens its drawer if it is shut, and
   shows the game menu (1..5) in the text panel, with keysBlocked set so
   gameTick leaves the keyboard to this loop.  The menu waits on
   textTimer (300 ticks).  When fewer than 50 remain the first time, the
   resident walks to the middle of the floor and naps in short spells
   (up to eight dozeOff(1) rounds, still polling for a digit), then
   returns to the cabinet with a fresh 250 ticks; if that runs out too,
   he shrugs and gives up.  Once a game is picked he takes the game box
   out of the cabinet, carries it down to the kitchen table, sits and
   runs the chosen game's main, then carries the box back up and closes
   the cabinet.  dogNoTopFlr keeps the dog off the top floor meanwhile. */

void
playGame()
{
        /* The unused locals must stay, in this order: they set the
           stack frame. */
        short   unused1;
        short   keycode;
        short   waitedOnce;
        short   unused2, unused3, unused4, unused5;
        short   napsLeft;

        dogNoTopFlr = YES;
        dogIdleCount = 1;

        posToXY(POS_TOP_FILING_CABINET, &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
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

        animState = STATE_STAND_SIDE_VIEW;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();
        gameTick(5);
        fillPanel(0x1b);
        textTimer = 300;
        keysBlocked = YES;
        printString("What game do you want to play?", 5,  8, COLOR_black);
        printString("1. Anagrams   2. War  3. Poker",  5, 16, COLOR_red);
        printString("4. Blackjack  5. Word Puzzles",   5, 24, COLOR_red);

        keycode = 0;
        waitedOnce = NO;

        while (keycode < '1' || keycode > '5') {
        keycode = getKey();
        gameTick(0);

        if (textTimer < 50 && waitedOnce == NO) {
                /* First timeout: nap in the middle of the floor, then retry. */
                textTimer = 250;
                napsLeft = 8;
                walkXTarget = resX;
                walkYTarget = floorWalkY[floorOfY(resY) - 1];
                noPreempt = YES;
                walkToTarget();
                noPreempt = NO;

                resFacing = FACING_RIGHT;
                animState = STATE_STAND_SIDE_VIEW;
                headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
                waitHeadTurn();

                while (napsLeft-- != 0) {
                        dozeOff(1);
                        gameTick(rndRng(0, 2));
                        keycode = getKey();
                        if (keycode >= '1' && keycode <= '5')
                                break;
                }

                animState = STATE_STAND_SIDE_VIEW;
                gameTick(0);
                posToXY(POS_TOP_FILING_CABINET, &walkXTarget, &walkYTarget);
                noPreempt = YES;
                walkToTarget();
                noPreempt = NO;

                animState = STATE_STAND_SIDE_VIEW;
                headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
                waitHeadTurn();
                waitedOnce = YES;
        }

        else if (textTimer == 0 && waitedOnce != NO) {
                idleShrug();
                keysBlocked = NO;
                dogNoTopFlr = NO;
                return;
        }

        }

        textTimer = 0;
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        rummageCabinet();
        animState = STATE_STAND_SIDE_VIEW;
        carryBehind(SPRITE_GAME_BOX);
        gameTick(0);

        posToXY(POS_BTM_KITCHEN_SINK, &walkXTarget, &walkYTarget);
        walkYTarget += 6;
        walkXTarget += 2;
        noPreempt = YES;
        walkToTarget();

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_TABLE_SETTING);
        pendX[spriteSlot[SPRITE_TABLE_SETTING]] = 103;
        pendY[spriteSlot[SPRITE_TABLE_SETTING]] = 180;

        posToXY(POS_BTM_TABLE_RIGHT, &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_TABLE_LEFT, &walkXTarget, &walkYTarget);
        walkToTarget();

        animState = STATE_STAND_SIDE_VIEW;
        resFacing = FACING_RIGHT;
        carryInFront(SPRITE_GAME_BOX);
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
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

        posToXY(POS_BTM_TABLE_RIGHT, &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_KITCHEN_SINK, &walkXTarget, &walkYTarget);
        walkYTarget += 5;
        walkToTarget();

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        layoutSlots();

        posToXY(POS_TOP_FILING_CABINET, &walkXTarget, &walkYTarget);
        walkToTarget();

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        spriteLayer[SPRITE_GAME_BOX] = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;
        waitHeadTurn();
        closeFilingCab();
        dogNoTopFlr = NO;
        noPreempt = NO;
}
