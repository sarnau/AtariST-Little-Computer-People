/*
 * parts/cl_redrH.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x137d4). Files under parts/ are never compiled standalone.
 */
void
cl_redrH()
{
        if (g_cmmin == t_min)
                return;
        cl_drwH(g_cmmin, g_chhou, COLOR_white);
        g_cmmin = t_min;
        g_chhou   = t_hour;
        /* The cached copies, not t_min/t_hour: the reference reads the
           two globals back for this call. */
        cl_drwH(g_cmmin, g_chhou, COLOR_grey);
}
