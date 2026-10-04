/*
 * parts/ldSpr.c -- included by stx_u1.c; never compiled on its own.
 */
/* Reads the 14000-byte SPRITES file into spr_file[]. */

void
ldSpr()
{
        short   fhnd;

        fhnd = fOpen("sprites", RMODE_RD);
        fr_read(fhnd, 14000L, spr_file);
        Fclose(fhnd);
}
