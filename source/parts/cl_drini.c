/*
 * parts/cl_drini.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x133b4). Files under parts/ are never compiled standalone.
 */
/* cl_drini (Ghidra 0x233B4): paint clock-face center, cl_redrH. */

void
cl_drini()
{
        drwLine(278, 83, 281, 83, COLOR_white);
        cl_redrH();
}
