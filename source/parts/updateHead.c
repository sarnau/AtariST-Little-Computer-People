/*
 * parts/updateHead.c -- included by stx_u3.c; never compiled on its own.
 */
/* updateHead: pick head frame from PEx.LCP by happiness + headFrame,
   expand via expandFrame into slot 4.  Tracks body position; head lowers
   1 px while carrying on stair states 13..16. */

void
updateHead()
{
        short   headIndex;

        while (pendReady[HW_SLOT_LCP_HEAD] == YES)
                ;

        headIndex = (headFrame & 0x7f) +
                    moodHeadBase[resident.happiness];

        /* Same 168-src/84-dest stride as updateBody. */
        expandFrame((short *) pexFrames[headIndex],
                (short *) headShapes[headIndex],
                headImage, headMask,
                2, 21, headMirror, 0);

        if (headMirror == NO)
                drawnX[HW_SLOT_LCP_HEAD] = resX + headXOffset[animState] - 4;
        else
                drawnX[HW_SLOT_LCP_HEAD] = resX + headXOffset[animState] - 14;

        drawnY[HW_SLOT_LCP_HEAD] = (resY + bodyYOffset[animState]) -
                             (headYOffset[animState] + 21);
        if (debugHideLcp != NO)
                drawnY[HW_SLOT_LCP_HEAD] = 300;

        /* The stair range is spelled inclusively and the y stepped in
           place; both shapes are the original's. */
        if (isCarrying != NO &&
            animState >= STATE_STR_TOP_F0 && animState <= STATE_STR_TOP_F3S)
                drawnY[HW_SLOT_LCP_HEAD]++;

        pendHeight[HW_SLOT_LCP_HEAD] = 21;
        pendWidth[HW_SLOT_LCP_HEAD]  = 32;
        pendImage[HW_SLOT_LCP_HEAD]  = headImage;
        pendMask[HW_SLOT_LCP_HEAD]   = headMask;

        if (lcpHidden != NO)
                pendImage[HW_SLOT_LCP_HEAD] = NULL;

        pendReady[HW_SLOT_LCP_HEAD] = YES;
}
