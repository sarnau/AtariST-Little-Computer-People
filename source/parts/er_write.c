/*
 * parts/er_write.c -- shared body; LCP_ORG links it in alerts.c,
 * LCP_STX in the 0xdece object (0x148e6, right after crFile).  Files under parts/
 * are never compiled standalone.
 */
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
