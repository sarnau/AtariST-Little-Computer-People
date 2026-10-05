/*
 * Included by stx_u3.c; never compiled on its own.
 */

/* Head-animation mode bits (sprhead.c's own defines, repeated here so
   the unity unit can compile this body). */
#undef  HEAD_MODE_H_AMPLITUDE
#undef  HEAD_MODE_H_RANGE
#define HEAD_MODE_H_AMPLITUDE           0x03
#define HEAD_MODE_H_RANGE               0x0c
#ifndef HEAD_MODE_V_RANGE
#define HEAD_MODE_V_RANGE               0x60
#define HEAD_MODE_V_OVERRIDE            0x80
#endif

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
        short   anim_mode;
        short   curTilt;
        short   tgtTilt;
        short   current_pos;
        short   curDir;
        short   tgtDir;
        short   unused;
        short   target_frame;
        short   random_seed;
        short   movement_mask;
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
                if ((headMode & HEAD_MODE_H_AMPLITUDE) == 0)
                        random_seed = ((rnd() & HEAD_MODE_H_AMPLITUDE) | 1) - 1;
                else
                        random_seed = (headMode & HEAD_MODE_H_AMPLITUDE) - 1;

                if (((headMode & HEAD_MODE_H_RANGE) == 0 &&
                     (rnd() & 8) != 0) ||
                    (headMode & HEAD_MODE_H_RANGE) >= 8)
                        random_seed = -random_seed;

                random_seed = (headRestDir[animState] + random_seed) & HEAD_DIR_MASK;
                if (resFacing == FACING_LEFT)
                        random_seed = (HEAD_DIRS - random_seed) & HEAD_DIR_MASK;
                headTarget = (headTarget & HEAD_TILT_MASK) | random_seed;
        } else {
                /* Vertical picker. */
                /* Embedded: the store's own flags drive the test. */
                if ((movement_mask = headMode & HEAD_MODE_V_RANGE) == 0) {
                        movement_mask = rnd() & HEAD_MODE_V_RANGE;
                        if (movement_mask == 0)
                                movement_mask = 0x40;
                }
                movement_mask = (movement_mask >> 5) - 1;
                if ((headMode & (HEAD_MODE_V_OVERRIDE |
                                HEAD_MODE_V_RANGE)) <= 0x80)
                        movement_mask = ((rnd() & 4) >> 2) +
                                        (movement_mask & 1);
                else
                        movement_mask = 7 - (headMode >> 5);
                headTarget = (headTarget & HEAD_DIR_MASK) | (movement_mask << HEAD_TILT_SHIFT);
        }

apply_current:
        if (headTarget >= 0) {
                curTilt = headPose & HEAD_TILT_MASK;
                tgtTilt = headTarget & HEAD_TILT_MASK;
                if ((current_pos = tgtTilt - curTilt) > 0)
                        headPose += 8;
                else if (current_pos < 0)
                        headPose -= 8;

                curDir = headPose & HEAD_DIR_MASK;
                tgtDir = headTarget & HEAD_DIR_MASK;
                target_frame = headTurnStep[(tgtDir - curDir) + 7];
                if (target_frame == HEAD_TURN_NONE) {
                        faceDir = (headRestDir[animState] + (resFacing << 2)) & HEAD_DIR_MASK;
                        target_frame = headTurnStep[(faceDir - curDir) + 7];
                }
                if (target_frame == HEAD_TURN_NONE)
                        target_frame = -1;

                headPose = (headPose & HEAD_TILT_MASK) +
                          ((headPose + target_frame) & HEAD_DIR_MASK);
        }

        if (headPose >= 0 && headPose < 0x80) {
                headFrame = headTiltFrame[(headPose & HEAD_TILT_MASK) >> HEAD_TILT_SHIFT];
                if ((anim_mode = headPose & HEAD_DIR_MASK) <= HEAD_DIR_BACK) {
                        headFrame += anim_mode;
                        headMirror = NO;
                } else {
                        headFrame += HEAD_DIRS - anim_mode;
                        headMirror = YES;
                }
        }
}
