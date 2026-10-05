/* Single-shot RETRY alert; caller is expected to retry the file op.

   Must sit right after ensureFile. */
void
writeErrorAlert()
{
#ifdef HOST
        fprintf(stderr, "WARN: Unable to write to disk.\n");
#else
        form_alert(ALERT_NO_DEFAULT, "[1][Unable to write.|Check disk.][RETRY]");
#endif
}
