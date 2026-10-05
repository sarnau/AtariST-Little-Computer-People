/* The .SCN nibble decoder. Each nibble indexes the 15-entry scnDict
   dictionary; nibble 0xf escapes to a literal 16-bit value in the next
   four nibbles. The file handling around it is written out in main. */
void
decodeScn(src, out, count)
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
                        *out = scnDict[val];
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
