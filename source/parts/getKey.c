/* Returns KEY_NONE (-1) when the buffer is empty.  When the ASCII byte
   is 0, the scancode (bits 16..23) selects cursor-left or F1..F10.
   The dead `break` after each `return` is part of the original code
   and must stay; the last arm has none. */
short
getKey()
{
        short   retKey;
        short   scancode;
        long    keycode;

        if (Cconis() == 0)
                return KEY_NONE;

        keycode  = Crawcin();
        retKey  = keycode;
        scancode = keycode >> 16;
        if (retKey != 0)
                return retKey;
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
