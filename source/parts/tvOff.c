/* Turns the TV off: if it is on, the resident walks to the TV in the
   top-floor living room, does the look gesture (tvStoop), clears tvRunning
   and blanks the picture.  Returns -1 if the walk was interrupted, 0
   when done; returns no value when the TV was already off. */
short
tvOff()
{
        if (tvRunning == NO)
                return;

        posToXY(POS_TOP_LIVING_ROOM, &walkXTarget, &walkYTarget);
        walkXTarget += 0;
        if (walkToTarget() != 0)
                return -1;

        gameTick(2);
        tvStoop();
        tvRunning = NO;
        drawTvPicture(COLOR_white);
        return 0;
}
