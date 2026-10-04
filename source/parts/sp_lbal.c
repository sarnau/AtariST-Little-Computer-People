/*
 * sp_lbal is followed directly by sp_lbbd and then sp_lbhd.
 *
 * Included by stx_u3.c; never compiled on its own.
 */
void
sp_lbal()
{
        /* One local: the four arrays are subscripted directly
           instead of walked with char* accumulators. */
        short   index;

        for (index = 0; index < 98; index++)
                sp_lbbd((short *) body_ptr[index],
                        (short *) body_shp[index], 21);
        for (index = 0; index < 66; index++)
                sp_lbhd((short *) pex_ptr[index],
                        (short *) hd_shp[index], 21);
}
