/*
 * Must sit right after crFile.
 *
 * Included by stx_u2.c; never compiled on its own.
 */
/* Single-shot RETRY alert; caller is expected to retry the file op. */
void
er_write()
{
#ifdef HOST
        fprintf(stderr,
                "WARN: Unable to write to disk.\n");
#else
        form_alert(ALERT_NO_DEFAULT, "[1][Unable to write.|Check disk.][RETRY]");
#endif
}
