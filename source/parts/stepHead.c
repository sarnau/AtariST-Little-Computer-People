/* Moves the resident's head one step per tick.  headPose is the current
   head pose (direction in bits 0-2, tilt in bits 3-4) and headTarget the
   target.  Once the head has reached its target and the headDelay
   countdown runs out, a new random target is picked within the limits
   headMode allows, either turning the head or tilting it.  The pose then
   steps one notch toward the target, and is turned into the head sprite
   frame headFrame plus the mirror flag headMirror for directions past 4.
   Called from the game tick. */
void
stepHead()
{
        /* `unused` is never touched but must stay: removing it changes
           the compiled code. */
        short   animMode;
        short   curTilt;
        short   tgtTilt;
        short   currentPos;
        short   curDir;
        short   tgtDir;
        short   unused;
        short   targetFrame;
        short   randomSeed;
        short   movementMask;
        short   faceDir;

        if (headPose != headTarget ||
            headMode < 0)
                goto apply_current;

        headDelay--;
        if (headDelay > 0)
                goto apply_current;

        /* Pick a fresh target.  Coin-flip between a horizontal
           adjustment and a vertical one -- the test is for the bit being
           CLEAR, with the horizontal picker first, as in the original. */
        headDelay = rndRng(2, 9);

        if ((rnd() & 0x10) == 0) {
                /* Horizontal picker. */
                if ((headMode & HEAD_ANIM_HORIZONTAL_AMPLITUDE) == 0)
                        randomSeed = ((rnd() & HEAD_ANIM_HORIZONTAL_AMPLITUDE) | 1) - 1;
                else
                        randomSeed = (headMode & HEAD_ANIM_HORIZONTAL_AMPLITUDE) - 1;

                if (((headMode & HEAD_ANIM_HORIZONTAL_RANGE) == 0 &&
                     (rnd() & 8) != 0) ||
                    (headMode & HEAD_ANIM_HORIZONTAL_RANGE) >= 8)
                        randomSeed = -randomSeed;

                randomSeed = (headRestDir[animState] + randomSeed) & HEAD_DIR_MASK;
                if (resFacing == FACING_LEFT)
                        randomSeed = (HEAD_DIRS - randomSeed) & HEAD_DIR_MASK;
                headTarget = (headTarget & HEAD_TILT_MASK) | randomSeed;
        } else {
                /* Vertical picker. */
                /* Embedded: the store's own flags drive the test. */
                if ((movementMask = headMode & HEAD_ANIM_VERTICAL_RANGE) == 0) {
                        movementMask = rnd() & HEAD_ANIM_VERTICAL_RANGE;
                        if (movementMask == 0)
                                movementMask = 0x40;
                }
                movementMask = (movementMask >> 5) - 1;
                if ((headMode & (HEAD_ANIM_VERTICAL_OVERRIDE |
                                 HEAD_ANIM_VERTICAL_RANGE)) <= 0x80)
                        movementMask = ((rnd() & 4) >> 2) +
                                        (movementMask & 1);
                else
                        movementMask = 7 - (headMode >> 5);
                headTarget = (headTarget & HEAD_DIR_MASK) | (movementMask << HEAD_TILT_SHIFT);
        }

apply_current:
        if (headTarget >= 0) {
                curTilt = headPose & HEAD_TILT_MASK;
                tgtTilt = headTarget & HEAD_TILT_MASK;
                if ((currentPos = tgtTilt - curTilt) > 0)
                        headPose += 8;
                else if (currentPos < 0)
                        headPose -= 8;

                curDir = headPose & HEAD_DIR_MASK;
                tgtDir = headTarget & HEAD_DIR_MASK;
                targetFrame = headTurnStep[(tgtDir - curDir) + 7];
                if (targetFrame == HEAD_TURN_NONE) {
                        faceDir = (headRestDir[animState] + (resFacing << 2)) & HEAD_DIR_MASK;
                        targetFrame = headTurnStep[(faceDir - curDir) + 7];
                }
                if (targetFrame == HEAD_TURN_NONE)
                        targetFrame = -1;

                headPose = (headPose & HEAD_TILT_MASK) +
                          ((headPose + targetFrame) & HEAD_DIR_MASK);
        }

        if (headPose >= 0 && headPose < 0x80) {
                headFrame = headTiltFrame[(headPose & HEAD_TILT_MASK) >> HEAD_TILT_SHIFT];
                if ((animMode = headPose & HEAD_DIR_MASK) <= HEAD_DIR_BACK) {
                        headFrame += animMode;
                        headMirror = NO;
                } else {
                        headFrame += HEAD_DIRS - animMode;
                        headMirror = YES;
                }
        }
}
