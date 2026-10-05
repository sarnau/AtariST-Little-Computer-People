/* loadFrameFile: load BODY.LCP / PE2..6.LCP into caller buffer.
   Header: {count:BE16, total_bytes:BE16, payload}.  Returns frame count. */

short
loadFrameFile(filename, destBuf)
char *          filename;
unsigned char * destBuf;
{
        /* unused1/unused2 must stay ahead of the two
           header words and the handle: removing them changes the
           compiled code.  There is no size cap and no return value
           (despite the declared short) -- the header's second word IS
           the length. */
        short   unused1;
        short   unused2;
        short   count;
        short   total;
        short   fhnd;

        fhnd = openFile(filename, RMODE_RD);
        readFile(fhnd, 2L, &count);
        readFile(fhnd, 2L, &total);
        readFile(fhnd, (long) total, destBuf);
        Fclose(fhnd);
}
