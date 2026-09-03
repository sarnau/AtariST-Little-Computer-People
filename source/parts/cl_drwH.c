/*
 * parts/cl_drwH.c -- shared body; LCP_STX links it in the 0xdece
 * object (between cl_redrH and drwLine). Files under parts/ are never
 * compiled standalone.
 */


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
