/*
 * parts/cl_redrH.c -- included by stx_u2.c; never compiled on its own.
 * Redraws the clock hands when the minute changes: erase in white, draw
 * in grey.
 */
void
cl_redrH()
{
        if (g_cmmin == t_min)
                return;
        cl_drwH(g_cmmin, g_chhou, COLOR_white);
        g_cmmin = t_min;
        g_chhou   = t_hour;
        /* The cached copies, not t_min/t_hour: the original reads the
           two globals back for this call. */
        cl_drwH(g_cmmin, g_chhou, COLOR_grey);
}
