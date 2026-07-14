/*
 * keyboard.c -- keyboard polling + Ctrl-key event dispatch.
 *
 * get_pressed_key() polls GEMDOS Cconis then Crawcin.  Crawcin
 * returns a 32-bit value packing scancode (bits 16..23) and ASCII
 * (bits 0..7); function keys and cursor keys have ASCII 0, so we
 * fall through to the scancode dispatch.  On the host, Cconis
 * always returns 0 (no keyboard) so this reduces to a noop -- the
 * ST-side game state is unchanged.
 *
 * deal_with_keycode() is the router.  Ctrl+A..W trigger event
 * queue items or one-shot flags; Ctrl+M (Enter) submits the command
 * buffer; cursor-left is backspace; other printable ASCII appends
 * to the g_cdinb and prints via print_char.
 *
 * addr: get_pressed_key(), deal_with_keycode()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern BOOL16   phone_answered_flag;
extern BOOL16   phone_call_active_flag;
extern BOOL16   ctrl_a_alarm_pressed_flag;
extern short    lcp_water_level;
extern BOOL16   dog_pettable_flag;
extern short    g_srsdc;
extern short    g_cdibp;
extern BOOL16   game_input_mode_flag;
extern char     g_cdinb[];
extern BOOL16   food_delivery_available;
extern short    g_ptanf;
extern BOOL16   g_ptdoa;
extern void     put_event_to_list();            /* ai.c      */
#include <osbind.h>

extern void     put_event_to_list();
extern void     p_dobls();
extern void     parse_command_to_action();
extern void     sf_sele();
extern void     update_water_level_bar();
extern void     print_char();

/* get_pressed_key: poll BIOS for the next keycode.  Returns KEY_NONE
   (-1) when the buffer is empty.  When the ASCII byte is 0 we consult
   the scancode (bits 16..23) for function keys and cursor keys, and
   fold them into a compact 16-bit value (0xE0 | scancode) that
   deal_with_keycode dispatches on.
   addr: get_pressed_key() */

short
get_pressed_key()
{
        long    keycode;
        short   ret_key;
        short   scancode;

        keycode = _gemdos(GEMDOS_Cconis, 0L, 0L, 0L);
        if (keycode == 0)
                return KEY_NONE;

        keycode = _gemdos(GEMDOS_Crawcin, 0L, 0L, 0L);
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

/* deal_with_keycode: dispatch one keycode to its handler.
   addr: deal_with_keycode() */

void
deal_with_keycode(keycode)
short   keycode;
{
        char    ascii_char;
        unsigned short  food_count;

        switch (keycode) {
        case KEY_CTRL_A_ALARM:
                ctrl_a_alarm_pressed_flag = YES;
                return;

        case KEY_CTRL_B_BOOK:
                p_dobls();
                put_event_to_list(ACTION_EVENT_BOOK_DELIVERY);
                return;

        case KEY_CTRL_C_CALL:
                if (phone_answered_flag == NO) {
                        phone_call_active_flag = YES;
                        put_event_to_list(ACTION_EVENT_PHONE_CALL);
                }
                return;

        case KEY_CTRL_D_DOGFOOD:
                p_dobls();
                put_event_to_list(ACTION_EVENT_DOG_FOOD);
                return;

        case KEY_CTRL_F_FOOD:
                food_count = (lcp.door_states_and_flags >> 9) & 7;
                if (food_delivery_available != NO && food_count < 4)
                        food_delivery_available = NO;
                if (food_count == 4) {
                        food_delivery_available = YES;
                } else {
                        p_dobls();
                        put_event_to_list(ACTION_EVENT_FOOD_DELIVERY);
                }
                return;

        case KEY_CTRL_R_RECORD:
                if (game_input_mode_flag == NO) {
                        p_dobls();
                        put_event_to_list(ACTION_EVENT_RECORD_DELIVERY);
                }
                return;

        case KEY_CTRL_W_WATER:
                if (lcp_water_level != 10) {
                        sf_sele(SFX_WATER_TAP, -1L);
                        update_water_level_bar(1);
                }
                return;

        case KEY_CTRL_P_PATTING:
                if (dog_pettable_flag != NO && g_ptdoa == NO) {
                        g_ptanf          = 0;
                        g_ptdoa          = YES;
                        lcp.happiness               = MOOD_HAPPY;
                        lcp.happiness_direction     = DIR_WORSENING;
                        lcp.happiness_duration_active =
                                lcp.happiness_initial_countdown;
                }
                return;

        case KEY_CTRL_M:
                if (game_input_mode_flag == NO) {
                        parse_command_to_action();
                        g_srsdc = 4;
                        g_cdibp = 0;
                }
                return;

        case KEY_CURSOR_LEFT:
                if (game_input_mode_flag == NO && g_cdibp > 0) {
                        g_cdibp = g_cdibp - 1;
                        ascii_char = g_cdinb[g_cdibp];
                        g_cdinb[g_cdibp] = '\0';
                        print_char((short) ascii_char,
                                   g_cdibp << 3, 23,
                                   COLOR_white);
                }
                return;
        }

        /* Default: printable character in text-input mode. */
        if (game_input_mode_flag == NO &&
            g_cdibp < 38 &&
            keycode > 0x1f) {
                g_cdinb[g_cdibp] =
                        (char) keycode;
                g_cdibp = g_cdibp + 1;
                g_cdinb[g_cdibp] = '\0';
                print_char(keycode,
                           (g_cdibp - 1) * 8, 23,
                           COLOR_black);
        }
}
