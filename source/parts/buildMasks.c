/*
 * buildMasks is followed directly by maskBody and then maskHead.
 */
void
buildMasks()
{
        /* One local: the four arrays are subscripted directly
           instead of walked with char* accumulators. */
        short   index;

        for (index = 0; index < BODY_FRAMES; index++)
                maskBody((short *) bodyFrames[index],
                        (short *) bodyShapes[index], 21);
        for (index = 0; index < HEAD_FRAMES; index++)
                maskHead((short *) pexFrames[index],
                        (short *) headShapes[index], 21);
}
