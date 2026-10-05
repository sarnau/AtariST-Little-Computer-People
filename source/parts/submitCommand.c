/*
 * Included by stx_u3.c; never compiled on its own.
 */
/* submitCommand: called from handleKey on Enter.  Runs matchCommand() on g_cdinb;
   valid ACTION_ID with queue room is appended at g_aprio priority. */


void
submitCommand()
{
        /* Unused, but it must stay ahead of `entered`: removing it
           changes the compiled code. */
        short   unused;
        short   entered;

        cmd_inp = g_cdinb;
        entered = matchCommand(cmd_inp);    /* reloads the global on purpose */
        if (entered >= 0 && g_aliss < 10) {
                g_aqueu[g_aliss]           = entered;
                g_apriq[g_aliss]  = g_aprio;
                g_aliss++;
        }
}
