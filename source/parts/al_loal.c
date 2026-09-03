/*
 * parts/al_loal.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x6428, in the 0x400c object ahead of fl_ltpl). Files under
 * parts/ are never compiled standalone.
 */
/* al_loal: load BODY.LCP / PE2..6.LCP into caller buffer.
   Header: {count:BE16, total_bytes:BE16, payload}.  Returns frame count. */

short
al_loal(filename, dest_buf)
char *          filename;
unsigned char * dest_buf;
{
        /* STX's frame is -14: two unused shorts ahead of the two
           header words and the handle.  There is no size cap and no
           return value -- the header's second word IS the length. */
        short   pad1;
        short   pad2;
        short   count;
        short   total;
        short   fhnd;

        fhnd = fOpen(filename, 0);
        fr_read(fhnd, 2L, &count);
        fr_read(fhnd, 2L, &total);
        fr_read(fhnd, (long) total, dest_buf);
        Fclose(fhnd);
}
