/*
 * parts/getKey.c -- shared body; LCP_ORG links it in keyboard.c,
 * LCP_STX in the 0x400c object at 0x68ee, just before rnd.  Files
 * under parts/ are never compiled standalone.
 */
/* Returns KEY_NONE (-1) when the buffer is empty.  When ASCII byte is 0
   the scancode (bits 16..23) is folded into 0x100 | scan.
   addr: getKey() */
#ifdef FAITHFUL
short
getKey()
{
        long    keycode;
        short   ret_key;
        short   scancode;

        keycode = Cconis();
        if (keycode == 0)
                return KEY_NONE;

        keycode = Crawcin();
        ret_key = (short) (keycode & 0xff);
        if (ret_key != 0)
                return ret_key;

        /* ASCII 0 -> function or cursor key.  Extract the scan byte
           from bits 16..23 and remap to our 0x100 | scan encoding. */
        scancode = (short) ((unsigned long) keycode >> 16);
        switch (scancode) {
        case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44:
        case 0x4b:
                return 0x100 | scancode;
        }
        return KEY_NONE;
}
#else   /* STX: no 0xff mask, a signed shift, both values taken
           before the ASCII test, and a switch with a default arm and
           its own small key codes. */
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
#endif
