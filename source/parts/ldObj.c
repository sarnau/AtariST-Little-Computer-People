/*
 * parts/ldObj.c -- shared body; LCP_ORG links it in assets.c,
 * LCP_STX in the 0xdece object (0x524a).  Files under parts/
 * are never compiled standalone.
 */
/* ldObj: read the 14000-byte OBJECTS file into obj_file[].
   addr: ldObj() */

void
ldObj()
{
        short   fhnd;

        fhnd = fOpen("objects", 0);
        fr_read(fhnd, 14000L, obj_file);
        Fclose(fhnd);
}
