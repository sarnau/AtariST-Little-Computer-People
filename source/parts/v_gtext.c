/*
 * parts/v_gtext.c -- shared body.  vdistx.c includes it at its LCP_STX
 * position in the binding module.
 * Files under parts/ are never compiled standalone.
 */

void
v_gtext(handle, x, y, str)
short   handle;
short   x;
short   y;
char *  str;
{
        short   i;

        /* STX sets the point first and copies with the classic
           while (dst[i++] = *src++) idiom, masking to a byte. */
        ptsin[0]  = x;
        ptsin[1]  = y;
        i = 0;
        while (intin[i++] = *str++ & 0xff)
                ;
        contrl[0] = 8;
        contrl[1] = 1;
        contrl[3] = --i;
        contrl[6] = handle;
        vdi_go();
}
