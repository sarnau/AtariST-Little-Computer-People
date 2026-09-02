/*
 * parts/ldSpr.c -- shared body; LCP_ORG links it in assets.c,
 * LCP_STX in the 0xdece object (0x528a).  Files under parts/
 * are never compiled standalone.
 */
/* ldSpr: read the 14000-byte SPRITES file into spr_file[].
   addr: ldSpr() */

void
ldSpr()
{
        short   fhnd;

        fhnd = fOpen("sprites", 0);
        fr_read(fhnd, 14000L, spr_file);
        Fclose(fhnd);
}
