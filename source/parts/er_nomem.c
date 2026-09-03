/*
 * parts/er_nomem.c -- er_nomem's body, shared between configurations.
 *
 * LCP_STX has it at the end of the 0x400c object, after the
 * dog/ai/actions/movement/calendar/renderx code (stx_u1.c).  Files
 * under parts/ are never compiled standalone -- alcyon_build.sh only
 * globs source/*.c -- they are #included by whichever translation
 * unit the active configuration needs them in.
 *
 * addr: er_nomem()
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
                form_alert(0, "[1][Not enough memory.|Requires ROMs.][REBOOT]");
#endif
}
