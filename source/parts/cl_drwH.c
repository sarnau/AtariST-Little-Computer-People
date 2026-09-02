/*
 * parts/cl_drwH.c -- shared body; LCP_ORG links it in init.c,
 * LCP_STX in the 0xdece object (between cl_redrH and drwLine).  Files under parts/
 * are never compiled standalone.
 */

void
cl_drwH(minute, hour, color)
short   minute;
short   hour;
short   color;
{
        short   m;
        short   h;

        m = minute / 5;
        h = hour % 12;

        drwLine(278, 85,
                  278 + g_cmmip[m],
                   85 - g_cmmip[m + 3],
                  color);
        drwLine(278, 85,
                  278 + g_chhop[h],
                   85 - g_chhop[h + 3],
                  color);
}
