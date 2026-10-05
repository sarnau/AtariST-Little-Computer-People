/*
 * Must follow gameTick.
 */
/* Select the body pose for animState -> slot 3.  When carrying an
   object during walk states (< 25), uses arms-up frames from carryFrames.
   X = resX - 4 (right) or resX - 14 (left); Y = resY + bodyYOffset[st] - 21. */

void
updateBody()
{
        short   frame;

        while (pendReady[HW_SLOT_LCP_BODY] == YES)
                ;

        frame = bodyIndex[animState];
        /* The bound is spelled inclusively on the previous state,
           not `< 25`: the two compile differently. */
        if (isCarrying != NO && animState <= STATE_STR_BTM_F3)
                frame = carryFrames[animState];

        /* Row strides are 168 (bodyFrames) and 84 (bodyShapes).  bodyFrames
           and bodyShapes are real arrays and the index is not cast to
           long, so the multiply stays 16-bit (no long-multiply call). */
        expandFrame((short *) bodyFrames[frame],
                (short *) bodyShapes[frame],
                (short *) bodyImage,
                (short *) bodyMask,
                2, 21, resFacing, 1);

        if (resFacing == FACING_RIGHT)
                drawnX[HW_SLOT_LCP_BODY] = resX - 4;
        else
                drawnX[HW_SLOT_LCP_BODY] = resX - 14;

        drawnY[HW_SLOT_LCP_BODY] = resY + bodyYOffset[animState] - 21;
        if (debugHideLcp != NO)
                drawnY[HW_SLOT_LCP_BODY] = 300;

        pendHeight[HW_SLOT_LCP_BODY] = 21;
        pendWidth[HW_SLOT_LCP_BODY]  = 32;
        pendImage[HW_SLOT_LCP_BODY]  = bodyImage;
        pendMask[HW_SLOT_LCP_BODY]   = bodyMask;

        if (lcpHidden != NO)
                pendImage[HW_SLOT_LCP_BODY] = NULL;

        pendReady[HW_SLOT_LCP_BODY] = YES;
}
