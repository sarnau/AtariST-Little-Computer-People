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
                case SCAN_CURSOR_LEFT: return KEY_CURSOR_LEFT; break;
                case SCAN_F1: return KEY_F1; break;
                case SCAN_F2: return KEY_F2; break;
                case SCAN_F3: return KEY_F3; break;
                case SCAN_F4: return KEY_F4; break;
                case SCAN_F5: return KEY_F5; break;
                case SCAN_F6: return KEY_F6; break;
                case SCAN_F7: return KEY_F7; break;
                case SCAN_F8: return KEY_F8; break;
                case SCAN_F9: return KEY_F9; break;
                case SCAN_F10: return KEY_F10; break;
                default:   return KEY_NONE;
                }
}
