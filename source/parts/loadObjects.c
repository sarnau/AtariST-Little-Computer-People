/*
 * parts/loadObjects.c -- included by stx_u1.c; never compiled on its own.
 */
/* Read the 14000-byte OBJECTS file into objFileBuf[]. */

void
loadObjects()
{
        short   fhnd;

        fhnd = openFile("objects", RMODE_RD);
        readFile(fhnd, 14000L, objFileBuf);
        Fclose(fhnd);
}
