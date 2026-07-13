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
 * to the command_input_buffer and prints via print_char.
 *
 * addr: get_pressed_key(), deal_with_keycode()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern void     put_event_to_list();
extern void     play_doorbell_sound();
extern void     parse_command_to_action();
extern void     soundeffect_select();
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
                play_doorbell_sound();
                put_event_to_list(ACTION_EVENT_BOOK_DELIVERY);
                return;

        case KEY_CTRL_C_CALL:
                if (phone_answered_flag == NO) {
                        phone_call_active_flag = YES;
                        put_event_to_list(ACTION_EVENT_PHONE_CALL);
                }
                return;

        case KEY_CTRL_D_DOGFOOD:
                play_doorbell_sound();
                put_event_to_list(ACTION_EVENT_DOG_FOOD);
                return;

        case KEY_CTRL_F_FOOD:
                food_count = (lcp.door_states_and_flags >> 9) & 7;
                if (food_delivery_available != NO && food_count < 4)
                        food_delivery_available = NO;
                if (food_count == 4) {
                        food_delivery_available = YES;
                } else {
                        play_doorbell_sound();
                        put_event_to_list(ACTION_EVENT_FOOD_DELIVERY);
                }
                return;

        case KEY_CTRL_R_RECORD:
                if (game_input_mode_flag == NO) {
                        play_doorbell_sound();
                        put_event_to_list(ACTION_EVENT_RECORD_DELIVERY);
                }
                return;

        case KEY_CTRL_W_WATER:
                if (lcp_water_level != 10) {
                        soundeffect_select(SFX_WATER_TAP, -1L);
                        update_water_level_bar(1);
                }
                return;

        case KEY_CTRL_P_PATTING:
                if (dog_pettable_flag != NO && petting_dog_active == NO) {
                        petting_anim_frame          = 0;
                        petting_dog_active          = YES;
                        lcp.happiness               = MOOD_HAPPY;
                        lcp.happiness_direction     = DIR_WORSENING;
                        lcp.happiness_duration_active =
                                lcp.happiness_initial_countdown;
                }
                return;

        case KEY_CTRL_M:
                if (game_input_mode_flag == NO) {
                        parse_command_to_action();
                        screen_scroll_down_count = 4;
                        command_input_buffer_pos = 0;
                }
                return;

        case KEY_CURSOR_LEFT:
                if (game_input_mode_flag == NO && command_input_buffer_pos > 0) {
                        command_input_buffer_pos = command_input_buffer_pos - 1;
                        ascii_char = command_input_buffer[command_input_buffer_pos];
                        command_input_buffer[command_input_buffer_pos] = '\0';
                        print_char((short) ascii_char,
                                   command_input_buffer_pos << 3, 23,
                                   COLOR_white);
                }
                return;
        }

        /* Default: printable character in text-input mode. */
        if (game_input_mode_flag == NO &&
            command_input_buffer_pos < 38 &&
            keycode > 0x1f) {
                command_input_buffer[command_input_buffer_pos] =
                        (char) keycode;
                command_input_buffer_pos = command_input_buffer_pos + 1;
                command_input_buffer[command_input_buffer_pos] = '\0';
                print_char(keycode,
                           (command_input_buffer_pos - 1) * 8, 23,
                           COLOR_black);
        }
}
