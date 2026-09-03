/*
 * parts/gameCln.c -- shared body; LCP_STX puts this static helper at
 * 0x75c8, between mg_stp and vst_h20.  Files under parts/ are never
 * compiled standalone.
 */


/* Shared cleanup at exit from any game. */

static void
gameCln(buffer)
void *  buffer;
{
        tx_sctm      = 0;
        no_keyin = NO;
        if (buffer != (void *) 0)
                Mfree(buffer);
}
