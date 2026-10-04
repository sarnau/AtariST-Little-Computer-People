/*
 * parts/ldObj.c -- included by stx_u1.c; never compiled on its own.
 */
/* Read the 14000-byte OBJECTS file into obj_file[]. */

void
ldObj()
{
        short   fhnd;

        fhnd = fOpen("objects", RMODE_RD);
        fr_read(fhnd, 14000L, obj_file);
        Fclose(fhnd);
}
