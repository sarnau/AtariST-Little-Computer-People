/*
 * parts/cl_drwH.c -- shared body; LCP_ORG links it in init.c,
 * LCP_STX in the 0xdece object (between cl_redrH and drwLine).  Files under parts/
 * are never compiled standalone.
 */

#ifdef FAITHFUL
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
#else   /* STX: link #-12 -- dx, dy, m, h; each endpoint offset is
           latched before the call. */

void
cl_drwH(minute, hour, color)
short   minute;
short   hour;
short   color;
{
        short   dx;
        short   dy;
        short   m;
        short   h;

        m  = minute / 5;
        dx = g_cmmip[m];
        dy = g_cmmip[m + 3];
        drwLine(278, 85, 278 + dx, 85 - dy, color);

        h  = hour % 12;
        dx = g_chhop[h];
        dy = g_chhop[h + 3];
        drwLine(278, 85, 278 + dx, 85 - dy, color);
}
#endif
