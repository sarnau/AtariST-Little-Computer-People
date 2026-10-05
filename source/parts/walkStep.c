/* walkStep: one 8Hz step along current waypoint.
   Waypoint reached -> done or pick next.  Not on stairs -> flat walk
   toward waypoint (X first, then Y).  On stairs -> stair-phase by Y
   bucket.  Sets footstepDue on the two foot-plant frames. */

void
walkStep()
{
        /* Four locals; the stair branch reuses x_distance for the
           next-X pick, and the last two are written once and never
           read -- leftovers, kept as the original has them. */
        short   x_distance;
        short   floor_num;
        short   ani_snap;
        short   spd_snap;

        footstepDue = NO;

        if (walkXTarget == 0 && walkYTarget == 0)
                return;

        /* Dead stores, kept on purpose.  This is the only place that
           reads walkAdjust. */
        ani_snap = frameCount;
        spd_snap = walkAdjust + walkSpeed;

        if (xWaypoint == 0 && yWaypoint == 0)
                nextWaypoint();

        /* Exit stair mode when we've reached the target floor. */
        if (onStairs != NO) {
                if (resY <= floorBottomY[(floor_num = floorOfY(yWaypoint)) - 1]) {
                        if (floor_num == FLOOR_TOP)
                                onStairs = NO;
                        else if (stairWaypts[(floor_num - 1) * 2 + 1] <= resY)
                                onStairs = NO;
                }
        }

        /* Waypoint reached? */
        if (resX == xWaypoint && resY == yWaypoint) {
                if (resX == walkXTarget && resY == walkYTarget) {
                        walkXTarget = 0;
                        walkYTarget = 0;
                        animState = STATE_STAND_IDLE;
                        gameTick(0);
                        return;
                } else
                        nextWaypoint();
        }

        /* ---- Flat walking (not on stairs) --------------------------- */
        if (onStairs == NO) {
                if (isCarrying != NO)
                        carryBehind(carriedSprite);

                if (resX < xWaypoint) {
                        resFacing = FACING_RIGHT;
                        if (animState > STATE_WALK_FRAME_7_STEP)
                                animState = STATE_WALK_FRAME_0;
                        else if (++animState > STATE_WALK_FRAME_7_STEP)
                                animState = STATE_WALK_FRAME_0;
                        resX++;
                        if (headLastWalk != 10) {
                                headTarget = HEAD_POSE(HEAD_DIR_RIGHT, HEAD_TILT_LOWER);
                                headLastWalk = headTarget;
                        }
                } else if (resX > xWaypoint) {
                        resFacing = FACING_LEFT;
                        if (animState > STATE_WALK_FRAME_7_STEP)
                                animState = STATE_WALK_FRAME_0;
                        else if (++animState > STATE_WALK_FRAME_7_STEP)
                                animState = STATE_WALK_FRAME_0;
                        resX--;
                        if (headLastWalk != HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER)) {
                                headTarget = HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER);
                                headLastWalk = headTarget;
                        }
                } else {
                        if (++animState > STATE_WALK_FRAME_7_STEP)
                                animState = STATE_WALK_FRAME_0;
                }

                if (resX >= xWaypoint)
                        x_distance = resX - xWaypoint;
                else
                        x_distance = xWaypoint - resX;
                if (x_distance < 8) {
                        if (resY < yWaypoint)
                                resY++;
                        else if (resY > yWaypoint)
                                resY--;
                } else {
                        if (floorWalkY[floorOfY(resY) - 1] > resY)
                                resY++;
                        if (floorWalkY[floorOfY(resY) - 1] < resY)
                                resY--;
                }

                if (animState == STATE_WALK_FRAME_3_STEP ||
                    animState == STATE_WALK_FRAME_7_STEP)
                        footstepDue = YES;
        }

        /* ---- Stair traversal --------------------------------------- */
        if (onStairs != NO) {
                if (resY > yWaypoint) {
                        /* Ascending */
                        if (resY == 161) {
                                if (isCarrying != NO)
                                        carryBehind(carriedSprite);
                                animState = STATE_STR_CLIMB_F0;
                                resFacing = FACING_LEFT;
                                resX -= 6;
                                resY -= 2;
                                if (headLastWalk != HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER)) {
                                        headTarget = HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER);
                                        headLastWalk = headTarget;
                                }
                        } else if (resY == 100) {
                                if (isCarrying != NO)
                                        carryBehind(carriedSprite);
                                animState = STATE_STR_CLIMB_F0;
                                resFacing = FACING_RIGHT;
                                resX += 3;
                                resY -= 2;
                                if (headLastWalk != 10) {
                                        headTarget = HEAD_POSE(HEAD_DIR_RIGHT, HEAD_TILT_LOWER);
                                        headLastWalk = headTarget;
                                }
                        } else if (resY > 161 ||
                                   (resY > 100 && resY < 140)) {
                                /* Top-of-stair frame (state 13..16). */
                                if (isCarrying != NO) {
                                        spriteLayer[carriedSprite] = SPRITE_BEHIND_LCP;
                                        layoutSlots();
                                }
                                if (animState < STATE_STR_TOP_F0 || animState > STATE_STR_TOP_F3S) {
                                        animState = STATE_STR_TOP_F0;
                                } else {
                                        /* Wraps to F0, and flips the
                                           facing on the wrap. */
                                        if (++animState > STATE_STR_TOP_F3S) {
                                                animState = STATE_STR_TOP_F0;
                                                resFacing ^= FACING_LEFT;
                                        }
                                        if (animState == STATE_STR_TOP_F3S ||
                                            animState == STATE_STR_TOP_F0)
                                                resY -= 2;
                                        if (animState == STATE_STR_TOP_F3S)
                                                footstepDue = YES;
                                }
                                if (headLastWalk != HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER)) {
                                        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
                                        headLastWalk = headTarget;
                                }
                        } else {
                                if (resY < 100) {
                                        /* Upper flight of stairs, going up-right */
                                        if (isCarrying != NO)
                                                carryBehind(carriedSprite);
                                        resFacing = FACING_RIGHT;
                                        resY--;
                                        if (animState != STATE_STR_CLIMB_F3S) {
                                                resX++;
                                                if (resX != xWaypoint)
                                                        resX++;
                                        }
                                        if (++animState > STATE_STR_CLIMB_F3S)
                                                animState = STATE_STR_CLIMB_F0;
                                        if (animState == STATE_STR_CLIMB_F3S)
                                                footstepDue = YES;
                                        if (headLastWalk != 10) {
                                                headTarget = HEAD_POSE(HEAD_DIR_RIGHT, HEAD_TILT_LOWER);
                                                headLastWalk = headTarget;
                                        }
                                } else if (resY < 161) {
                                        /* Lower flight, going up-left */
                                        if (isCarrying != NO)
                                                carryBehind(carriedSprite);
                                        resFacing = FACING_LEFT;
                                        resY--;
                                        if (animState != STATE_STR_CLIMB_F3S) {
                                                resX--;
                                                if (resX != xWaypoint)
                                                        resX--;
                                        }
                                        if (++animState > STATE_STR_CLIMB_F3S)
                                                animState = STATE_STR_CLIMB_F0;
                                        if (animState == STATE_STR_CLIMB_F3S)
                                                footstepDue = YES;
                                        if (headLastWalk != HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER)) {
                                                headTarget = HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER);
                                                headLastWalk = headTarget;
                                        }
                                }
                        }
                } else if (resY < yWaypoint) {
                        /* Descending */
                        if (isCarrying != NO)
                                carryBehind(carriedSprite);

                        if (resY == 161) {
                                animState = STATE_STR_BTM_F0;
                                resFacing = FACING_RIGHT;
                                resY += 4;
                                resX += 6;
                                if (headLastWalk != 8) {
                                        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
                                        headLastWalk = headTarget;
                                }
                                if (isCarrying != NO)
                                        carryInFront(carriedSprite);
                        } else if (resY == 100) {
                                animState = STATE_STR_BTM_F0;
                                resFacing = FACING_RIGHT;
                                resY += 2;
                                resX -= 2;
                                if (headLastWalk != 8) {
                                        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
                                        headLastWalk = headTarget;
                                }
                                if (isCarrying != NO)
                                        carryInFront(carriedSprite);
                        } else if (resY > 161 ||
                                   (resY > 100 && resY < 132)) {
                                /* Bottom-of-stair frame (state 21..24). */
                                if (isCarrying != NO)
                                        carryInFront(carriedSprite);
                                if (animState < STATE_STR_BTM_F0 || animState > STATE_STR_BTM_F3) {
                                        animState = STATE_STR_BTM_F0;
                                        resX += 2;
                                } else {
                                        if (++animState > STATE_STR_BTM_F3) {
                                                animState = STATE_STR_BTM_F0;
                                                resFacing ^= FACING_LEFT;
                                        }
                                        if (animState == STATE_STR_BTM_F1 ||
                                            animState == STATE_STR_BTM_F2)
                                                resY += 2;
                                        if (animState == STATE_STR_BTM_F3)
                                                footstepDue = YES;
                                }
                                if (headLastWalk != 8) {
                                        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
                                        headLastWalk = headTarget;
                                }
                        } else {
                                if (resY < 100) {
                                        /* Upper flight, going down-left */
                                        if (isCarrying != NO)
                                                carryInFront(carriedSprite);
                                        resFacing = FACING_LEFT;
                                        resY++;
                                        if (animState != STATE_STR_DESC_F3S) {
                                                resX--;
                                                if (resX != xWaypoint)
                                                        resX--;
                                        }
                                        /* Wraps to F0, it does not
                                           clamp at F3S. */
                                        if (animState > STATE_STR_DESC_F3S ||
                                            animState < STATE_STR_DESC_F0)
                                                animState = STATE_STR_DESC_F0;
                                        else if (++animState > STATE_STR_DESC_F3S)
                                                animState = STATE_STR_DESC_F0;
                                        if (headLastWalk != HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER)) {
                                                headTarget = HEAD_POSE(HEAD_DIR_LEFT, HEAD_TILT_LOWER);
                                                headLastWalk = headTarget;
                                        }
                                        if (animState == STATE_STR_DESC_F1)
                                                footstepDue = YES;
                                } else if (resY < 161) {
                                        /* Lower flight, going down-right */
                                        if (isCarrying != NO)
                                                carryInFront(carriedSprite);
                                        resFacing = FACING_RIGHT;
                                        resY++;
                                        if (animState != STATE_STR_DESC_F3S) {
                                                resX++;
                                                if (resX != xWaypoint)
                                                        resX++;
                                        }
                                        /* Wraps to F0, it does not
                                           clamp at F3S. */
                                        if (animState > STATE_STR_DESC_F3S ||
                                            animState < STATE_STR_DESC_F0)
                                                animState = STATE_STR_DESC_F0;
                                        else if (++animState > STATE_STR_DESC_F3S)
                                                animState = STATE_STR_DESC_F0;
                                        if (headLastWalk != 10) {
                                                headTarget = HEAD_POSE(HEAD_DIR_RIGHT, HEAD_TILT_LOWER);
                                                headLastWalk = headTarget;
                                        }
                                        if (animState == STATE_STR_DESC_F1)
                                                footstepDue = YES;
                                }
                        }
                }
        }

        /* Sickness slows the walk: two ticks per step and delayed
           footstep sound.  Healthy: one tick with immediate sound. */
        if (resident.sickness_level != SICKNESS_HEALTHY) {
                gameTick(0);
                playFootstep();
        }
        gameTick(0);
        if (resident.sickness_level == SICKNESS_HEALTHY)
                playFootstep();
}
