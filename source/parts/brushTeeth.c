/*
 * the resident brushes his teeth at the bathroom sink.
 */

void
brushTeeth()
{
        short           brush_cycles;   /* signed: unsigned compiles differently */
        /* walkToTarget()'s result is tested in place, not kept in a local. */
        short           x_left;
        short           x_right;

        brush_cycles = (unsigned short) rndRng(24, 35);
        posToXY(POS_MID_BATHROOM_SINK, &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        headMode = HEAD_ANIM_DISABLED;
        resFacing = FACING_RIGHT;
        animState = STATE_BRUSH_TEETH;
        headTarget = HEAD_POSE(HEAD_DIR_RIGHT, HEAD_TILT_LOWER);
        resY -= 2;
        waitHeadTurn();

        spriteLayer[SPRITE_STUDY_DOOR_FRAME] = SPRITE_BEHIND_LCP;
        activateSprite(SPRITE_STUDY_DOOR_FRAME);
        x_left = resX + 8;
        x_right = resX + 12;
        pendX[spriteSlot[SPRITE_STUDY_DOOR_FRAME]] = x_left;
        pendY[spriteSlot[SPRITE_STUDY_DOOR_FRAME]] = resY - 24;

        /* The loop is driven by a post-decrement, so the body sees the
           already-decremented value.  Keep this shape: it is the
           original's. */
        while (brush_cycles--) {
                if (brush_cycles & 1)
                        pendX[spriteSlot[SPRITE_STUDY_DOOR_FRAME]] = x_left;
                else
                        pendX[spriteSlot[SPRITE_STUDY_DOOR_FRAME]] = x_right;
                gameTick(0);
        }

        spriteLayer[SPRITE_STUDY_DOOR_FRAME] = SPRITE_HIDDEN;
        layoutSlots();
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        resY += 2;
        gameTick(0);
}
