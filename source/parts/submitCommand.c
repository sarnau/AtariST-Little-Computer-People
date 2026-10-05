/*
 * Included by stx_u3.c; never compiled on its own.
 */
/* submitCommand: called from handleKey on Enter.  Runs matchCommand() on typedLine;
   valid ACTION_ID with queue room is appended at cmdPriority priority. */


void
submitCommand()
{
        /* Unused, but it must stay ahead of `entered`: removing it
           changes the compiled code. */
        short   unused;
        short   entered;

        parsedLine = typedLine;
        entered = matchCommand(parsedLine);    /* reloads the global on purpose */
        if (entered >= 0 && queueCount < 10) {
                queueActions[queueCount]           = entered;
                queuePriority[queueCount]  = cmdPriority;
                queueCount++;
        }
}
