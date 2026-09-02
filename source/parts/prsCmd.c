/*
 * parts/prsCmd.c -- shared body; LCP_ORG links it in ai.c,
 * LCP_STX in the 0xdece object (0x1721c, in the 0x148fe object after prCh).  Files under parts/
 * are never compiled standalone.
 */
/* prsCmd: called from deal_kc on Enter.  Runs chk_encm() on g_cdinb;
   valid ACTION_ID with queue room is appended at g_aprio priority.
   addr: prsCmd() */


void
prsCmd()
{
#ifdef FAITHFUL
        short   entered;
#else
        /* STX's frame is 8 bytes: a short the function never uses is
           declared ahead of `entered`, which sits at -4 there. */
        short   unused;
        short   entered;
#endif

        cmd_inp = g_cdinb;
#ifdef FAITHFUL
        entered = chk_encm(g_cdinb);
#else
        entered = chk_encm(cmd_inp);    /* STX reloads the global */
#endif
        if (entered >= 0 && g_aliss < 10) {
                g_aqueu[g_aliss]           = entered;
                g_apriq[g_aliss]  = g_aprio;
#ifdef FAITHFUL
                g_aliss = g_aliss + 1;
#else
                g_aliss++;
#endif
        }
}
