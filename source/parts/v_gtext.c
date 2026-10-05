/* VDI graphic text: draws str at (x,y) on workstation handle.  The
   characters are copied into intin one per word and contrl[3] is set
   to the string length. */
void
v_gtext(handle, x, y, str)
short   handle;
short   x;
short   y;
char *  str;
{
        short   i;

        /* The point is set first and the string copied with the
           while (dst[i++] = *src++) idiom, masking to a byte; this
           exact shape is the original's. */
        ptsin[0]  = x;
        ptsin[1]  = y;
        i = 0;
        while (intin[i++] = *str++ & 0xff)
                ;
        contrl[0] = VDI_V_GTEXT;
        contrl[1] = 1;
        contrl[3] = --i;
        contrl[6] = handle;
        vdi_go();
}
