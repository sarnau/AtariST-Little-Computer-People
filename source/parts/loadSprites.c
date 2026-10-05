/* Reads the 14000-byte SPRITES file into sprFileBuf[]. */

void
loadSprites()
{
        short   fhnd;

        fhnd = openFile("sprites", RMODE_RD);
        readFile(fhnd, 14000L, sprFileBuf);
        Fclose(fhnd);
}
