/*
 * parts/scn_dec.c -- LCP_STX 0x52ca.  The .SCN nibble decoder; the
 * file handling around it is inlined in main (there is no unScn).
 * Files under parts/ are never compiled standalone.
 */
void
scn_dec(src, out, count)
char *  src;
short * out;
short   count;
{
        short   flag;
        short   val;
        short   i;
        short   j;

        flag = 1;
        for (i = 0; i < count; i++) {
                if (flag != 0) {
                        val = (*src >> 4) & 0x0f;
                } else {
                        val = *src & 0x0f;
                        src++;
                }
                flag = (flag != 0) ? 0 : 1;

                if (val != 0xf) {
                        *out = scn_dic[val];
                        out++;
                } else {
                        val = 0;
                        for (j = 0; j < 4; j++) {
                                val = val << 4;
                                if (flag != 0) {
                                        val |= (*src >> 4) & 0x0f;
                                } else {
                                        val |= *src & 0x0f;
                                        src++;
                                }
                                flag = (flag != 0) ? 0 : 1;
                        }
                        *out = val;
                        out++;
                }
        }
}
