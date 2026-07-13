/*
 * alerts.c -- GEM form_alert wrappers for fatal errors.
 *
 * On the ST, form_alert pops a modal dialog with a REBOOT / RETRY
 * button.  The 1985 code loops on it forever -- there's no OK path.
 * On the host, form_alert is a no-op returning 1, so we'd busy-loop
 * quickly; we terminate the process instead so tests don't hang.
 *
 * addr: error_not_enough_memory(), error_unable_to_write()
 */

#include "types.h"
#include <osbind.h>

#ifdef HOST
#include <stdlib.h>             /* exit */
#include <stdio.h>              /* fprintf */
#endif

/* error_not_enough_memory: infinite loop showing REBOOT alert on the
   ST.  On host, print to stderr and exit(1) so tests fail fast.
   addr: error_not_enough_memory() */

void
error_not_enough_memory()
{
#ifdef HOST
        fprintf(stderr,
                "FATAL: Not enough memory.\n");
        exit(1);
#else
        for (;;)
                form_alert(0, "[1][Not enough memory.|Requires ROMs.][REBOOT]");
#endif
}

/* error_unable_to_write: single-shot RETRY alert.  Note the 1985
   code doesn't loop on this one -- it fires once and returns to the
   caller, which is expected to retry the file operation itself
   (lcp_save, create_file).  On host, print and continue so the retry
   loop terminates.
   addr: error_unable_to_write() */

void
error_unable_to_write()
{
#ifdef HOST
        fprintf(stderr,
                "WARN: Unable to write to disk.\n");
#else
        form_alert(0, "[1][Unable to write.|Check disk.][RETRY]");
#endif
}
