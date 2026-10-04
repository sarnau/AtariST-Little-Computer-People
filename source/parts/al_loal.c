/*
 * parts/al_loal.c -- included by stx_u1.c; never compiled on its own.
 */
/* al_loal: load BODY.LCP / PE2..6.LCP into caller buffer.
   Header: {count:BE16, total_bytes:BE16, payload}.  Returns frame count. */

short
al_loal(filename, dest_buf)
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

        fhnd = fOpen(filename, RMODE_RD);
        fr_read(fhnd, 2L, &count);
        fr_read(fhnd, 2L, &total);
        fr_read(fhnd, (long) total, dest_buf);
        Fclose(fhnd);
}
