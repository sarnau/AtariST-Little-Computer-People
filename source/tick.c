/*
 * tick.c -- main frame driver (gameTick).
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "protos.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick_tables.h"



/* Advance the game by counter + 1 animation ticks.  In carrying mode
   (isCarrying) the carried sprite is first repositioned relative to the
   resident; then each tick waits for the compositor, steps the house
   animations and the simulation, and polls the keyboard. */
void
gameTick(counter)
short   counter;
{
        short           key;
        short           index;
        unsigned short  count;
        short           psi;    /* petting sprite id / scratch */

        /* Deliberate original shapes, all of which must stay: the
           carrying-mode arm comes FIRST with the test inverted;
           spriteSlot[carriedSprite] is recomputed at every use instead of going
           through a local; the inner tests re-check isCarrying redundantly
           (both arms of the first pair assign the same thing); the clamp
           indexes pendX with carriedSprite rather than the slot and lives
           INSIDE the facing-left arm; and the per-object Y offset is
           written out as a switch. */
        if (isCarrying != NO) {
                if (resFacing == FACING_RIGHT) {
                        if (isCarrying == NO)
                                pendX[spriteSlot[carriedSprite]] = resX + 10;
                        else
                                pendX[spriteSlot[carriedSprite]] = resX + 10;
                } else {
                        if (isCarrying == NO)
                                pendX[spriteSlot[carriedSprite]] =
                                        resX - drawnWidth[spriteSlot[carriedSprite]] + 8;
                        else
                                pendX[spriteSlot[carriedSprite]] =
                                        resX - drawnWidth[spriteSlot[carriedSprite]] + 16;
                        if (pendX[carriedSprite] < 0)
                                pendX[carriedSprite] = 0;
                }

                switch (carriedSprite) {
                case SPRITE_SUITCASE:
                        pendY[spriteSlot[SPRITE_SUITCASE]] = resY - 20;
                        break;
                case SPRITE_GLASS:
                        pendY[spriteSlot[SPRITE_GLASS]] = resY - 20;
                        break;
                case SPRITE_GAME_BOX:
                        pendY[spriteSlot[SPRITE_GAME_BOX]] = resY - 20;
                        break;
                case SPRITE_BOOK:
                        pendY[spriteSlot[SPRITE_BOOK]] = resY - 20;
                        break;
                case SPRITE_FOOD_PACKAGE:
                        pendY[spriteSlot[SPRITE_FOOD_PACKAGE]] = resY - 20;
                        break;
                case SPRITE_FIREWOOD:
                        pendY[spriteSlot[SPRITE_FIREWOOD]] = resY - 20;
                        break;
                case SPRITE_COOKING_POT:
                        pendY[spriteSlot[SPRITE_COOKING_POT]] = resY - 20;
                        break;
                case SPRITE_VINYL_CARRY:
                        pendY[spriteSlot[SPRITE_VINYL_CARRY]] = resY - 20;
                        break;
                case SPRITE_COOKED_MEAL:
                        pendY[spriteSlot[SPRITE_COOKED_MEAL]] = resY - 20;
                        break;
                }
        }

        /* The tick loop is NOT an else arm: carrying mode falls
           straight into it. */
        {
                count = frameCount;
                for (index = 0; index < counter + 1; index++) {
                        while (count == frameCount)
                                renderFrame();
                        count = frameCount;

                        tickCount++;

                        /* Clock pendulum: 4-frame animation. */
                        psi = (tickCount >> 2) & 3;
                        drawObject(clockFrames[psi], 271, 92);
                        simStep();
                        redrawHands();

                        /* Ctrl-P petting-hand animation cycle. */
                        if (patActive != NO) {
                                /* Tested as `> 10` with the finish arm
                                   first, as in the original. */
                                if (patFrame > 10) {
                                        spriteLayer[patLastSprite] =
                                                SPRITE_HIDDEN;
                                        layoutSlots();
                                        patActive = NO;
                                } else {
                                        if (patFrame != 0) {
                                                spriteLayer[patSprites[patFrame - 1]] =
                                                        SPRITE_HIDDEN;
                                        }
                                        /* The lookup is repeated at every
                                           use rather than cached in a
                                           local, as in the original. */
                                        spriteLayer[patSprites[patFrame]] =
                                                SPRITE_BEHIND_LCP;
                                        activateSprite(patSprites[patFrame]);
                                        pendX[spriteSlot[patSprites[patFrame]]] =
                                                192;
                                        pendY[spriteSlot[patSprites[patFrame]]] =
                                                165;
                                        patFrame++;
                                }
                        }

                        /* Dog food bowl: current fill state + countdown. */
                        drawObject(bowlFrames[bowlLevel], DOG_BOWL_X, DOG_BOWL_Y);
                        if (bowlChange < 0) {
                                if (bowlLevel != BOWL_EMPTY)
                                        bowlLevel--;
                                if (bowlLevel < 0)
                                        bowlLevel = BOWL_EMPTY;
                        }
                        if (bowlChange > 0) {
                                bowlLevel++;
                                if (bowlLevel > BOWL_FULL)
                                        bowlLevel = BOWL_FULL;
                        }

                        /* Fireplace animation + auto-extinguish. */
                        if (fireBurning != NO) {
                                drawObject(fireFrames[tickCount & 3], FIREPLACE_X, FIREPLACE_Y);
                                if (--fireTimeLeft == 0)
                                        fireDouse = YES;
                        }
                        if (fireDouse != NO) {
                                fireDouse = NO;
                                fireBurning = NO;
                                drawObject(OBJ_FIRE_OFF, FIREPLACE_X, FIREPLACE_Y);
                        }

                        /* Alarm clock SFX + animation. */
                        if (alarmRinging != NO) {
                                if (alarmSounding == NO) {
                                        sfxSelect(SFX_ALARM_CLOCK, 100000L);
                                        alarmSounding = YES;
                                } else if (sfxPlaying == NO) {
                                        sfxSelect(SFX_ALARM_CLOCK, 100000L);
                                }
                                drawObject(alarmFrames[tickCount & 1],
                                        53, 102);
                        }
                        if (alarmRinging == NO) {
                                alarmSounding = NO;
                                if (sfxPlaying != NO && sfxCurId == SFX_ALARM_CLOCK)
                                        stopSfx();
                        }

                        /* Phone ring. */
                        if (phoneRinging != NO) {
                                if (ringCountdown == 0) {
                                        sfxSelect(SFX_PHONE_RING, 10000L);
                                        ringCountdown = 26;
                                }
                                ringCountdown--;
                                if (ringCountdown > 10) {
                                        drawObject(phoneFrames[tickCount & 3], PHONE_X, PHONE_Y);
                                } else {
                                        if (sfxPlaying != NO &&
                                            sfxCurId == SFX_PHONE_RING)
                                                stopSfx();
                                        drawObject(OBJ_PHONE_2, PHONE_X, PHONE_Y);
                                }
                        }
                        if (phoneHangUp != NO) {
                                drawObject(OBJ_PHONE_2, PHONE_X, PHONE_Y);
                                phoneHangUp = NO;
                                if (sfxPlaying != NO && sfxCurId == SFX_PHONE_RING)
                                        stopSfx();
                                ringCountdown = 0;
                        }

                        if (recordPlaying != NO) animRecPlayer();
                        if (tvRunning != NO)          tvNoise();

                        updateBody();
                        stepHead();
                        updateHead();

                        if (stripScroll > 0) {
                                scrollStrip();
                                stripScroll--;
                        } else {
                                if (keysBlocked == NO &&
                                    movingIn == NO) {
                                        key = getKey();
                                        /* getKey returns -1 for "no key",
                                           not 0. */
                                        if (key != -1) {
                                                if (key != KEY_CTRL_W_WATER &&
                                                    key != KEY_CTRL_B_BOOK &&
                                                    key != KEY_CTRL_R_RECORD &&
                                                    key != KEY_CTRL_F_FOOD &&
                                                    key != KEY_CTRL_C_CALL &&
                                                    key != KEY_CTRL_D_DOGFOOD &&
                                                    key != KEY_CTRL_A_ALARM &&
                                                    key != KEY_CTRL_P_PATTING) {
                                                        if (textTimer == 0) {
                                                                fillPanel(27);
                                                                typedCursor = 0;
                                                        }
                                                        textTimer = 160;
                                                }
                                                handleKey(key);
                                        }
                                } else if (typingOff != NO) {
                                        if ((key = getKey()) != -1)
                                                handleKey(key);
                                }
                        }

                        renderFrame();
                }
        }
}
