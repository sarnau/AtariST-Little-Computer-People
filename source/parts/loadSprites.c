/*
 * parts/loadSprites.c -- included by stx_u1.c; never compiled on its own.
 */
/* Reads the 14000-byte SPRITES file into spr_file[]. */

void
loadSprites()
{
        short   fhnd;

        fhnd = openFile("sprites", RMODE_RD);
        readFile(fhnd, 14000L, spr_file);
        Fclose(fhnd);
}
