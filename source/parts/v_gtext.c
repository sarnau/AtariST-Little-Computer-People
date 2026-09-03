/*
 * parts/v_gtext.c -- shared body.  the VDI binding module is ordered
 * differently in the two revisions, so vdistx.c includes these in
 * LCP_STX order and vdiown.c in LCP_ORG's.
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

#ifdef FAITHFUL
        for (i = 0; str[i] != 0; i = i + 1)
                intin[i] = str[i];
        contrl[0] = 8;
        contrl[1] = 1;
        contrl[3] = i;
        contrl[6] = handle;
        ptsin[0]  = x;
        ptsin[1]  = y;
#else
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
#endif
        vdi_go();
}
