/*
 * parts/cl_redrH.c -- shared body; LCP_ORG links it in init.c,
 * LCP_STX in the 0xdece object (0x137d4).  Files under parts/
 * are never compiled standalone.
 */
void
cl_redrH()
{
        if (g_cmmin == t_min)
                return;
        cl_drwH(g_cmmin, g_chhou, COLOR_white);
        g_cmmin = t_min;
        g_chhou   = t_hour;
        cl_drwH(t_min, t_hour, COLOR_grey);
}
