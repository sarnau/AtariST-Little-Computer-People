/*
 * keyboard.c -- keyboard polling + Ctrl-key event dispatch.
 *
 * getKey() polls GEMDOS Cconis then Crawcin.  Crawcin
 * returns a 32-bit value packing scancode (bits 16..23) and ASCII
 * (bits 0..7); function keys and cursor keys have ASCII 0, so we
 * fall through to the scancode dispatch.  On the host, Cconis
 * always returns 0 (no keyboard) so this reduces to a noop -- the
 * ST-side game state is unchanged.
 *
 * deal_kc() is the router.  Ctrl+A..W trigger event
 * queue items or one-shot flags; Ctrl+M (Enter) submits the command
 * buffer; cursor-left is backspace; other printable ASCII appends
 * to the g_cdinb and prints via prCh.
 *
 * addr: getKey(), deal_kc()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "ai.h"
#include "events.h"
#include "globals.h"
#include "keyboard.h"
#include "render.h"
#include "renderx.h"
#include "sound.h"


/* getKey: poll BIOS for the next keycode.  Returns KEY_NONE
   (-1) when the buffer is empty.  When the ASCII byte is 0 we consult
   the scancode (bits 16..23) for function keys and cursor keys, and
   fold them into a compact 16-bit value (0xE0 | scancode) that
   deal_kc dispatches on.
   addr: getKey() */

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

/* deal_kc: dispatch one keycode to its handler.
   addr: deal_kc() */

void
deal_kc(keycode)
short   keycode;
{
        char    ascii_char;
        unsigned short  food_count;

        switch (keycode) {
        case KEY_CTRL_A_ALARM:
                alarm_p = YES;
                return;

        case KEY_CTRL_B_BOOK:
                p_dobls();
                putEv(ACTION_EVENT_BOOK_DELIVERY);
                return;

        case KEY_CTRL_C_CALL:
                if (ph_ans == NO) {
                        ph_call = YES;
                        putEv(ACTION_EVENT_PHONE_CALL);
                }
                return;

        case KEY_CTRL_D_DOGFOOD:
                p_dobls();
                putEv(ACTION_EVENT_DOG_FOOD);
                return;

        case KEY_CTRL_F_FOOD:
                food_count = (lcp.door_states_and_flags >> 9) & 7;
                if (food_dlv != NO && food_count < 4)
                        food_dlv = NO;
                if (food_count == 4) {
                        food_dlv = YES;
                } else {
                        p_dobls();
                        putEv(ACTION_EVENT_FOOD_DELIVERY);
                }
                return;

        case KEY_CTRL_R_RECORD:
                if (g_inpmd == NO) {
                        p_dobls();
                        putEv(ACTION_EVENT_RECORD_DELIVERY);
                }
                return;

        case KEY_CTRL_W_WATER:
                if (lcp_watr != 10) {
                        sf_sele(SFX_WATER_TAP, -1L);
                        updWtLv(1);
                }
                return;

        case KEY_CTRL_P_PATTING:
                if (dg_petok != NO && g_ptdoa == NO) {
                        g_ptanf          = 0;
                        g_ptdoa          = YES;
                        lcp.happiness               = MOOD_HAPPY;
                        lcp.happiness_direction     = DIR_WORSENING;
                        lcp.happiness_duration_active =
                                lcp.happiness_initial_countdown;
                }
                return;

        case KEY_CTRL_M:
                if (g_inpmd == NO) {
                        prsCmd();
                        g_srsdc = 4;
                        g_cdibp = 0;
                }
                return;

        case KEY_CURSOR_LEFT:
                if (g_inpmd == NO && g_cdibp > 0) {
                        g_cdibp = g_cdibp - 1;
                        ascii_char = g_cdinb[g_cdibp];
                        g_cdinb[g_cdibp] = '\0';
                        prCh((short) ascii_char,
                                   g_cdibp << 3, 23,
                                   COLOR_white);
                }
                return;
        }

        /* Default: printable character in text-input mode. */
        if (g_inpmd == NO &&
            g_cdibp < 38 &&
            keycode > 0x1f) {
                g_cdinb[g_cdibp] =
                        (char) keycode;
                g_cdibp = g_cdibp + 1;
                g_cdinb[g_cdibp] = '\0';
                prCh(keycode,
                           (g_cdibp - 1) * 8, 23,
                           COLOR_black);
        }
}
