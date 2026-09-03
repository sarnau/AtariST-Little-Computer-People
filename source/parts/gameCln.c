/*
 * parts/gameCln.c -- shared body; LCP_STX puts this static helper at
 * 0x75c8, between mg_stp and vst_h20.  Files under parts/ are never
 * compiled standalone.
 */

/* Shared cleanup at exit from any game.  LCP_STX's version takes no
   argument and does not free -- the minigame mains free their own
   buffer inline -- and nothing in that build actually calls it. */

#ifdef FAITHFUL
static void
gameCln(buffer)
void *  buffer;
{
        tx_sctm      = 0;
        no_keyin = NO;
        if (buffer != (void *) 0)
                Mfree(buffer);
}
#else
static void
gameCln()
{
        tx_sctm  = 0;
        no_keyin = NO;
}
#endif
