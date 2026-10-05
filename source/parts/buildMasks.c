/*
 * buildMasks is followed directly by maskBody and then maskHead.
 *
 * Included by stx_u3.c; never compiled on its own.
 */
void
buildMasks()
{
        /* One local: the four arrays are subscripted directly
           instead of walked with char* accumulators. */
        short   index;

        for (index = 0; index < 98; index++)
                maskBody((short *) body_ptr[index],
                        (short *) body_shp[index], 21);
        for (index = 0; index < 66; index++)
                maskHead((short *) pex_ptr[index],
                        (short *) hd_shp[index], 21);
}
