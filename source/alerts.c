/*
 * alerts.c -- GEM form_alert wrappers for fatal errors.
 * On host, form_alert is a no-op returning 1, so we exit instead of
 * busy-looping.
 * addr: er_nomem(), er_write()
 */

#include "types.h"
#include <osbind.h>
#include "alerts.h"

#ifdef HOST
#include <stdlib.h>             /* exit */
#include <stdio.h>              /* fprintf */
#endif

/* addr: er_nomem() */
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

/* Single-shot RETRY alert; caller is expected to retry the file op.
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
