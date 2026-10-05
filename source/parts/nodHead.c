/* nodHead: nod the head.  The resident turns side-on, his head is
   settled at position 8, then head animation is suspended (headPose =
   headTarget = -1) so three head frames can be forced through headFrame in
   turn.  The saved frame and head position 8 are restored afterwards. */
void
nodHead()
{
        short   savedFrame;

        scratchArr[0] = STATE_WALK_FRAME_3_STEP;
        scratchArr[1] = STATE_WALK_FRAME_4;
        scratchArr[2] = STATE_WALK_FRAME_5;
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_SIDE_VIEW;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headMode = HEAD_ANIM_DISABLED;
        waitHeadTurn();

        savedFrame = headFrame;
        headTarget = HEAD_ANIM_DISABLED;
        headPose = HEAD_ANIM_DISABLED;

        headFrame = scratchArr[0];
        gameTick(1);
        headFrame = scratchArr[1];
        gameTick(1);
        headFrame = scratchArr[2];
        gameTick(2);

        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headPose = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headFrame = savedFrame;
        gameTick(0);
}
