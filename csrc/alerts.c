/*
 * alerts.c -- GEM form_alert wrappers for fatal errors.
 *
 * On the ST, form_alert pops a modal dialog with a REBOOT / RETRY
 * button.  The 1985 code loops on it forever -- there's no OK path.
 * On the host, form_alert is a no-op returning 1, so we'd busy-loop
 * quickly; we terminate the process instead so tests don't hang.
 *
 * addr: er_nomem(), er_write()
 */

#include "types.h"
#include <osbind.h>

#ifdef HOST
#include <stdlib.h>             /* exit */
#include <stdio.h>              /* fprintf */
#endif

/* er_nomem: infinite loop showing REBOOT alert on the
   ST.  On host, print to stderr and exit(1) so tests fail fast.
   addr: er_nomem() */

void
er_nomem()
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

/* er_write: single-shot RETRY alert.  Note the 1985
   code doesn't loop on this one -- it fires once and returns to the
   caller, which is expected to retry the file operation itself
   (lcp_save, crFile).  On host, print and continue so the retry
   loop terminates.
   addr: er_write() */

void
er_write()
{
#ifdef HOST
        fprintf(stderr,
                "WARN: Unable to write to disk.\n");
#else
        form_alert(0, "[1][Unable to write.|Check disk.][RETRY]");
#endif
}
