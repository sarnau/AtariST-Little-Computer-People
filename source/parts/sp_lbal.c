/*
 * parts/sp_lbal.c -- shared body; LCP_STX places it in the sprite object
 * (see stx_u3.c for the address).  Files under parts/ are never
 * compiled standalone.
 */
/* STX order: sp_lbal (0x167b0) is followed directly by sp_lbbd
   (0x1682e, a bsr.s target) and then sp_lbhd (0x169b4). */
void
sp_lbal()
{
        /* One local: STX subscripts the four arrays directly instead
           of walking char* accumulators. */
        short   index;

        for (index = 0; index < 98; index++)
                sp_lbbd((short *) body_ptr[index],
                        (short *) body_shp[index], 21);
        for (index = 0; index < 66; index++)
                sp_lbhd((short *) pex_ptr[index],
                        (short *) hd_shp[index], 21);
}
