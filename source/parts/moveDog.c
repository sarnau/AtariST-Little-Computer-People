/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* moveDog: 8 Hz movement + walk-cycle advance.  If the dog
   has no target the routine is a no-op.  Handles flat walking (X/Y
   equal steps to waypoint) and stair navigation (staircase_waypoint_
   coords[] gate for the two staircase entrances).  Layer depth is
   1 (in-front) when the dog is below the resident, -1 (behind) when
   above -- newspaper reading forces in-front so the dog doesn't disappear
   behind the paper. */

void
moveDog()
{
        /* Six shorts, declared in this order; unused1 and next_x are
           never touched but must stay, as in the original. */
        short   x_distance;
        short   floor_num;
        short   unused1;
        short   next_x;
        short   depth_layer;
        BOOL16  h_flip;

        dogStepIdx++;
        if (dogStepIdx > 7)
                dogStepIdx = 0;

        if (dogXTarget == 0 && dogYTarget == 0)
                return;

        if ((short) (dogY + 5) <= resY)
                depth_layer = -1;
        else
                depth_layer = 1;
        if (animState == STATE_READ_PAPER_HOLD ||
            animState == STATE_READ_PAPER_TURN_PAGE)
                depth_layer = 1;

        if (dogXWaypt == 0 && dogYWaypt == 0)
                dogNextWaypt();

        /* Exit stair-mode when reaching a floor boundary. */
        if (dogOnStairs != NO) {
                /* The assignment is embedded on purpose, so the index
                   reuses floorOfY's result without a reload. */
                if (dogY <= floorBottomY[(floor_num = floorOfY(dogYWaypt)) - 1]) {
                        if (floor_num == FLOOR_TOP)
                                dogOnStairs = NO;
                        else if (stairWaypts[(floor_num - 1) * 2 + 1] <= dogY)
                                dogOnStairs = NO;
                }
        }

        if (dogX == dogXWaypt && dogY == dogYWaypt) {
                if (dogX == dogXTarget && dogY == dogYTarget) {
                        dogXTarget = 0;
                        dogYTarget = 0;
                        dogXWaypt = 0;
                        dogYWaypt = 0;
                        dogSpriteId = SPRITE_DOG_LAY_DOWN;
                        setDogSprite(dogSpriteId, depth_layer, NO);
                        return;
                } else
                        dogNextWaypt();
        }

        dogSpriteId = dogWalkSprites[dogStepIdx];

        if (dogOnStairs == NO) {
                if (dogX < dogXWaypt) {
                        h_flip = NO;
                        dogX++;
                } else if (dogX > dogXWaypt) {
                        h_flip = YES;
                        dogX--;
                }
                /* An if/else with the store duplicated in both arms,
                   not a ternary, as in the original. */
                if (dogX >= dogXWaypt)
                        x_distance = dogX - dogXWaypt;
                else
                        x_distance = dogXWaypt - dogX;
                if (x_distance < 8) {
                        if (dogY < dogYWaypt)
                                dogY++;
                        else if (dogY > dogYWaypt)
                                dogY--;
                } else {
                        if (dogY < floorWalkY[floorOfY(dogY) - 1])
                                dogY++;
                        if (floorWalkY[floorOfY(dogY) - 1] < dogY)
                                dogY--;
                }
        }

        /* The stair patches step dogX and dogY in place, and every
           anchor is written as a relative step rather than an absolute
           coordinate. */
        if (dogOnStairs != NO) {
                if (dogY > dogYWaypt) {
                        /* Going up */
                        if (dogY == 0xa1) {
                                h_flip = YES;
                                dogX -= 17;
                                dogY -= 2;
                        } else if (dogY == 100) {
                                h_flip = NO;
                                dogX += 3;
                                dogY -= 2;
                        } else if (dogY > 161 ||
                                   (dogY > 100 && dogY < 140)) {
                                h_flip = NO;
                                dogY -= 2;
                        } else if (dogY < 100) {
                                h_flip = NO;
                                dogY--;
                                if (dogSpriteId != SPRITE_DOG_WLK_R9) {
                                        dogX++;
                                        if (dogX != dogXWaypt)
                                                dogX++;
                                }
                        } else if (dogY < 0xa1) {
                                h_flip = YES;
                                dogY--;
                                if (dogSpriteId != SPRITE_DOG_WLK_R9) {
                                        dogX--;
                                        if (dogX != dogXWaypt)
                                                dogX--;
                                }
                        }
                } else if (dogY < dogYWaypt) {
                        /* Going down */
                        if (dogY == 0xa1) {
                                h_flip = NO;
                                dogY += 4;
                                dogX++;
                        } else if (dogY == 100) {
                                h_flip = NO;
                                dogY += 2;
                                dogX += 3;
                        } else if (dogY > 161 ||
                                   (dogY > 100 && dogY < 132)) {
                                h_flip = NO;
                                dogY++;
                        } else if (dogY < 100) {
                                h_flip = YES;
                                dogY++;
                                if (dogSpriteId != SPRITE_DOG_WLK_R9) {
                                        dogX--;
                                        if (dogX != dogXWaypt)
                                                dogX--;
                                }
                        } else if (dogY < 0xa1) {
                                h_flip = NO;
                                dogY++;
                                if (dogSpriteId != SPRITE_DOG_WLK_R9) {
                                        dogX++;
                                        if (dogX != dogXWaypt)
                                                dogX++;
                                }
                        }
                }
        }

        setDogSprite(dogSpriteId, depth_layer, h_flip);
}
