/*
 * Fatal out-of-memory alert; never returns.
 *
 * Included by stx_u1.c; never compiled on its own.
 */

void
er_nomem()
{
#ifdef HOST
        fprintf(stderr,
                "FATAL: Not enough memory.\n");
        exit(1);
#else
        for (;;)
                form_alert(ALERT_NO_DEFAULT, "[1][Not enough memory.|Requires ROMs.][REBOOT]");
#endif
}
