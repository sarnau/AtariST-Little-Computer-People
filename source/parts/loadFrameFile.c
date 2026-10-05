/*
 * parts/loadFrameFile.c -- included by stx_u1.c; never compiled on its own.
 */
/* loadFrameFile: load BODY.LCP / PE2..6.LCP into caller buffer.
   Header: {count:BE16, total_bytes:BE16, payload}.  Returns frame count. */

short
loadFrameFile(filename, dest_buf)
char *          filename;
unsigned char * dest_buf;
{
        /* pad1/pad2 are unused, but they must stay ahead of the two
           header words and the handle: removing them changes the
           compiled code.  There is no size cap and no return value
           (despite the declared short) -- the header's second word IS
           the length. */
        short   pad1;
        short   pad2;
        short   count;
        short   total;
        short   fhnd;

        fhnd = openFile(filename, RMODE_RD);
        readFile(fhnd, 2L, &count);
        readFile(fhnd, 2L, &total);
        readFile(fhnd, (long) total, dest_buf);
        Fclose(fhnd);
}
