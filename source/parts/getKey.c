/*
 * parts/getKey.c -- shared body; LCP_STX links it in the 0x400c object
 * at 0x68ee, just before rnd. Files under parts/ are never compiled
 * standalone.
 */
/* Returns KEY_NONE (-1) when the buffer is empty.  When ASCII byte is 0
   the scancode (bits 16..23) is folded into 0x100 | scan.
   addr: getKey() */
short
getKey()
{
        short   ret_key;
        short   scancode;
        long    keycode;

        if (Cconis() == 0)
                return KEY_NONE;

        keycode  = Crawcin();
        ret_key  = keycode;
        scancode = keycode >> 16;
        if (ret_key != 0)
                return ret_key;
        else
                switch (scancode) {
                case 0x4b: return 8; break;
                case 0x3b: return 241; break;
                case 0x3c: return 242; break;
                case 0x3d: return 243; break;
                case 0x3e: return 244; break;
                case 0x3f: return 245; break;
                case 0x40: return 246; break;
                case 0x41: return 247; break;
                case 0x42: return 248; break;
                case 0x43: return 249; break;
                case 0x44: return 250; break;
                default:   return KEY_NONE;
                }
}
