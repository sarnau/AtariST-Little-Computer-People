/*
 * Included by stx_u2.c; never compiled on its own.
 */
/* Paint the clock-face centre, then the hands. */

void
cl_drini()
{
        drwLine(278, 83, 281, 83, COLOR_white);
        cl_redrH();
}
