/*
 * parts/prsCmd.c -- shared body; LCP_STX links it in the 0xdece object
 * (0x1721c, in the 0x148fe object after prCh). Files under parts/ are
 * never compiled standalone.
 */
/* prsCmd: called from deal_kc on Enter.  Runs chk_encm() on g_cdinb;
   valid ACTION_ID with queue room is appended at g_aprio priority.
   addr: prsCmd() */


void
prsCmd()
{
        /* STX's frame is 8 bytes: a short the function never uses is
           declared ahead of `entered`, which sits at -4 there. */
        short   unused;
        short   entered;

        cmd_inp = g_cdinb;
        entered = chk_encm(cmd_inp);    /* STX reloads the global */
        if (entered >= 0 && g_aliss < 10) {
                g_aqueu[g_aliss]           = entered;
                g_apriq[g_aliss]  = g_aprio;
                g_aliss++;
        }
}
