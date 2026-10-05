/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* peekAround: a quick glance.  The head is settled at position 8, head
   animation is suspended so frame 2 can be forced through headFrame
   for 6 ticks, then the saved frame and position 8 are restored.
   Used as an action and by the War card game after the resident's
   remark on a round. */
void
peekAround()
{
        short   saved_frame;

        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headMode         = HEAD_ANIM_DISABLED;
        waitHeadTurn();

        saved_frame            = headFrame;
        headTarget = HEAD_ANIM_DISABLED;
        headPose      = HEAD_ANIM_DISABLED;
        headFrame      = 2;
        gameTick(6);

        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headPose      = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headFrame      = saved_frame;
        gameTick(0);
}
