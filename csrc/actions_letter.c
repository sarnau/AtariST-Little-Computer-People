/*
 * actions_letter.c -- ACTION_WRITE_LETTER + its typewriter helpers.
 *
 * The resident walks to the filing cabinet, grabs paper, moves to the
 * desk, and types a procedurally-assembled letter:
 *
 *   Date line       -- month_name_table[date_month] date_day, 1900+date_year
 *   Salutation      -- "Dear <owner_name>,"
 *   2..4 paragraphs -- picked from 4 topic sections in shuffled order.
 *                      Each section has 3 lines (opening / middle /
 *                      ending) chosen from 4 alternates each.  Sickness
 *                      or happiness modifies which alternate row we
 *                      pull from.
 *   Greeting        -- one of 4 sign-offs from letter_greeting_table[]
 *   Signature       -- lcp.character_name
 *
 * Text is fed one character at a time through letter_type_character_
 * animated(), which drives the desk-typing sprite pipeline (four
 * SPRITE_TYPING_1..4 width brackets) and picks a random typewriter
 * click SFX per character.  letter_type_string_animated() word-wraps
 * at 40 columns (0x27 threshold) and inserts a carriage return when
 * a word would overflow.
 *
 * The letter template pointers (letter_line_ptr[], letter_greeting_
 * table[]) live in globals.c initialised to NULL.  file_load_letter_
 * template() -- still stubbed -- would populate them from letter.txt
 * on the ST disk.  Until that lands, letter_type_string_animated()
 * detects the NULL pointer and returns immediately, so the outer
 * animation still runs but no text is typed.
 *
 * addr: action_write_letter(), letter_type_string_animated(),
 *       letter_type_character_animated()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>
#include <stdio.h>              /* sprintf */

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     spritedata_select();
extern void     sprite_update_slots();
extern void     soundeffect_select();
extern void     hide_lcp_sprites();
extern void     show_lcp_sprites();
extern void     action_open_close_filing_cabinet();
extern void     action_walk_to_and_turn();
extern void     fill_top_rect_with_background();
extern void     file_load_letter_template();
extern void     print_char();
extern void     letter_select_typewriter_sound();
extern void     select_random_click_sound();
extern void     error_not_enough_memory();
extern char     input_string[];

/* Forward-declare the two helpers + action_play_piano so calls before
   the definitions still resolve under -Werror. */
extern void     letter_type_character_animated();
extern short    letter_type_string_animated();
extern void     action_play_piano();

/* letter_type_character_animated: emit one character.  On CR (< space),
   scrolls the text pane down; otherwise plays a random click and blits
   the char via print_char, then swaps in the appropriate width sprite
   from letter_char_width_table[] based on current buffer position.

   addr: letter_type_character_animated() */

void
letter_type_character_animated(ch)
short   ch;
{
        short   pick;
        short   i;

        if (ch < ' ') {                 /* control char, treated as CR */
                lcp_state = STATE_TYPE_AT_DESK_LEFT_HAND;
                game_tick_and_animate(0);
                pick = randomRange(0, 5);
                if (pick == 0) {
                        lcp_state = STATE_TYPE_AT_DESK_RIGHT_HAND;
                        game_tick_and_animate(0);
                        lcp_state = STATE_TYPE_AT_DESK_LEFT_HAND;
                        game_tick_and_animate(0);
                }
                lcp_facing_direction = FACING_RIGHT;
                lcp_state = STATE_WRITE_AT_DESK;
                game_tick_and_animate(0);

                screen_scroll_down_count = 4;
                command_input_buffer_pos = 0;

                sprite_layer_flags[SPRITE_TYPING_1] = SPRITE_HIDDEN;
                sprite_layer_flags[SPRITE_TYPING_2] = SPRITE_HIDDEN;
                sprite_layer_flags[SPRITE_TYPING_3] = SPRITE_HIDDEN;
                sprite_layer_flags[SPRITE_TYPING_4] = SPRITE_HIDDEN;
                sprite_update_slots();

                /* Pick the width-bracket sprite for the (now-empty)
                   buffer position.  Buckets: 0..9, 10..19, 20..29, 30+.
                   Original code re-checks the just-cleared buffer_pos
                   here, so i always resolves to 0 (< 10); preserved
                   verbatim as it may be a placeholder for future logic. */
                i = 3;
                if (command_input_buffer_pos < 10)      i = 0;
                else if (command_input_buffer_pos < 20) i = 1;
                else if (command_input_buffer_pos < 30) i = 2;

                sprite_layer_flags[letter_char_width_table[i]] = SPRITE_IN_FRONT;
                spritedata_select(letter_char_width_table[i]);
                sprite_pending_x[sprite_slot_map[letter_char_width_table[i]]] = 211;
                sprite_pending_y[sprite_slot_map[letter_char_width_table[i]]] =  44;
                letter_select_typewriter_sound();
                game_tick_and_animate(6);
                return;
        }

        /* Printable char. */
        if (ch == ' ') {
                lcp_facing_direction = FACING_RIGHT;
                lcp_state = STATE_TYPE_AT_DESK_LEFT_HAND;
                game_tick_and_animate(0);
        } else {
                lcp_facing_direction = randomRange(0, 1);
                lcp_state = STATE_TYPE_AT_DESK_LEFT_HAND;
                game_tick_and_animate(0);
                pick = randomRange(0, 5);
                if (pick == 0) {
                        lcp_state = STATE_TYPE_AT_DESK_RIGHT_HAND;
                        game_tick_and_animate(0);
                        lcp_state = STATE_TYPE_AT_DESK_LEFT_HAND;
                        game_tick_and_animate(0);
                }
        }
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_WRITE_AT_DESK;
        select_random_click_sound();
        game_tick_and_animate(0);
        print_char(ch, command_input_buffer_pos << 3, 23, COLOR_black);
        command_input_buffer_pos = command_input_buffer_pos + 1;

        sprite_layer_flags[SPRITE_TYPING_1] = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_3] = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_4] = SPRITE_HIDDEN;
        sprite_update_slots();

        i = 3;
        if (command_input_buffer_pos < 10)      i = 0;
        else if (command_input_buffer_pos < 20) i = 1;
        else if (command_input_buffer_pos < 30) i = 2;

        sprite_layer_flags[letter_char_width_table[i]] = SPRITE_IN_FRONT;
        spritedata_select(letter_char_width_table[i]);
        sprite_pending_x[sprite_slot_map[letter_char_width_table[i]]] = 211;
        sprite_pending_y[sprite_slot_map[letter_char_width_table[i]]] =  44;

        /* 1/21 chance of a short pause between keystrokes. */
        pick = randomRange(0, 20);
        if (pick == 0) {
                pick = randomRange(0, 3);
                game_tick_and_animate(pick);
        }
}

/* letter_type_string_animated: word-wrapped string typer.
   `val` argument: leading-space indent count (negative means "always
   indent"; positive means "only indent if the previous line ended
   mid-line").  Returns the last character emitted so the caller can
   check if it was a space (used to decide whether to insert a break
   before the next chunk).

   addr: letter_type_string_animated() */

short
letter_type_string_animated(str, val)
char *  str;
short   val;
{
        short   line_remaining;
        short   word_length;
        short   i;
        BOOL16  word_wrap_needed;

        /* Guard against unloaded template slots.  Without letter.txt
           the pointers are NULL; treat as "print nothing, return 0". */
        if (str == (char *) 0)
                return 0;

        /* Emit leading spaces. */
        if (val < 0 || command_input_buffer_pos > 0) {
                if (val < 0)
                        val = -val;
                for (i = 0; i < val; i = i + 1)
                        letter_type_character_animated(' ');
        }

        word_wrap_needed = NO;
        line_remaining   = 0;
        while (word_wrap_needed == NO) {
                /* Skip inter-word spaces (emit them if we've already
                   started a line). */
                while (*str == ' ') {
                        str = str + 1;
                        if (command_input_buffer_pos > 0)
                                letter_type_character_animated(' ');
                }

                /* Collect one word into letter_scratch_buffer. */
                i = 0;
                for (;;) {
                        line_remaining = (short) *str;
                        if (line_remaining < 0x21)      /* < '!' */
                                break;
                        letter_scratch_buffer[i] = *str;
                        i = i + 1;
                        str = str + 1;
                }

                if (line_remaining != ' ') {
                        word_wrap_needed = YES;
                        str = str + 1;
                }

                /* Word-wrap: if this word would overflow the 40-col
                   line, insert a CR first. */
                if ((short) (i + command_input_buffer_pos) > 0x27)
                        letter_type_character_animated('\r');

                for (word_length = 0; word_length < i; word_length = word_length + 1) {
                        line_remaining = (short) letter_scratch_buffer[word_length];
                        letter_type_character_animated(line_remaining);
                }
        }
        return line_remaining;
}

/* action_write_letter: outer flow -- walk, allocate letter buffer,
   assemble letter body from shuffled template sections, free buffer,
   walk out.

   Letter structure (each template section spans 96 short entries):
     paragraph_count   = randomRange(2, 4)
     for i in 0..paragraph_count-1:
       section_id      = section_order[i]           (0..3, shuffled)
       template_index  = section_id * 96
       if section_id == 3:      last section, 6 alternates
           template_index += randomRange(0, 5) * 12
       elif sickness == 0:      healthy, use happiness row
           template_index += randomRange(0, 1) * 48 + happiness * 12
       else:                    sick, use dedicated sick row
           template_index += randomRange(0, 1) * 48 + 36
       type 3 lines from that subsection at offsets +0..3, +4..7, +8..11

   addr: action_write_letter() */

void
action_write_letter()
{
        short   result;
        short   section_order[4];
        short   i;
        short   swap_a;
        short   swap_b;
        short   swap_temp;
        short   paragraph_count;
        short   section_id;
        short   template_index;
        short   line_spacing;
        short   cursor_y;
        short   char_test;
        short   walk_result;
        short   full_year;

        if (lcp_record_playing != NO)
                action_play_piano();

        house_get_position_xy(POS_TOP_FILING_CABINET,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        action_walk_to_and_turn();
        walk_result = randomRange(0, 100);
        if (lcp.initiative_threshold < walk_result)
                action_open_close_filing_cabinet();

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &walk_target_x, &walk_target_y);
        walk_target_x = walk_target_x - 10;
        walk_target_y = walk_target_y +  3;
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        action_interruptible_flag = YES;

        /* Drop the typewriter + typing sprites. */
        sprite_layer_flags[SPRITE_TYPEWRITER] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_TYPEWRITER);
        sprite_pending_x[sprite_slot_map[SPRITE_TYPEWRITER]] = 201;
        sprite_pending_y[sprite_slot_map[SPRITE_TYPEWRITER]] =  51;
        sprite_layer_flags[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_TYPING_2);
        sprite_pending_x[sprite_slot_map[SPRITE_TYPING_2]] = 211;
        sprite_pending_y[sprite_slot_map[SPRITE_TYPING_2]] =  44;

        house_get_position_xy(POS_TOP_DESK_CHAIR,
                              &walk_target_x, &walk_target_y);
        walk_target_y = walk_target_y -  4;
        walk_target_x = walk_target_x - 14;
        lcp_walk_to_destination();

        lcp_state              = STATE_STAND_SIDE_VIEW;
        lcp_facing_direction   = FACING_RIGHT;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();

        lcp_x = lcp_x + 5;
        lcp_y = lcp_y + 6;
        lcp_state = STATE_WRITE_AT_DESK;
        game_tick_and_animate(1);

        sprite_layer_flags[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_TYPING_1] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_TYPING_1);
        sprite_pending_x[sprite_slot_map[SPRITE_TYPING_1]] = 211;
        sprite_pending_y[sprite_slot_map[SPRITE_TYPING_1]] =  44;

        head_anim_mode         = HEAD_ANIM_READING;
        disable_key_input_flag = YES;
        fill_top_rect_with_background(0x1b);

        /* Allocate the letter template buffer -- 0x2900 bytes. */
        letter_txt_content = (char *) _gemdos(GEMDOS_Malloc, 0x2900L, 0L, 0L);
        if (letter_txt_content == (char *) 0)
                error_not_enough_memory();
        file_load_letter_template();

        text_scroll_timer = 9999;
        game_tick_and_animate(2);

        /* --- Date + salutation --- */
        full_year = date_year + 1900;
        sprintf(input_string, "%s %d, %4d",
                month_name_table[date_month],
                date_day + 1, full_year);
        command_input_buffer_pos = 0;
        letter_type_string_animated(input_string, -12);
        letter_type_character_animated('\r');

        sprintf(input_string, "Dear %s,", lcp.owner_name);
        letter_type_string_animated(input_string, 0);
        letter_type_character_animated('\r');

        /* --- Shuffle the 4 section indices via 16 random swaps. --- */
        for (i = 0; i < 4; i = i + 1)
                section_order[i] = i;
        for (i = 0; i < 16; i = i + 1) {
                swap_a = randomRange(0, 3);
                swap_b = randomRange(0, 3);
                swap_temp = section_order[swap_a];
                section_order[swap_a] = section_order[swap_b];
                section_order[swap_b] = swap_temp;
        }

        /* --- Body: 2..4 paragraphs from the shuffled sections. --- */
        paragraph_count = randomRange(2, 4);
        for (i = 0; i < paragraph_count; i = i + 1) {
                section_id     = section_order[i];
                template_index = section_id * 0x60;
                if (section_id == 3) {
                        walk_result = randomRange(0, 5);
                        template_index = walk_result * 0xc + template_index;
                } else if (lcp.sickness_level < 1) {
                        walk_result = randomRange(0, 1);
                        template_index = lcp.happiness * 0xc +
                                         walk_result * 0x30 +
                                         template_index;
                } else {
                        walk_result = randomRange(0, 1);
                        template_index = walk_result * 0x30 + 0x24 +
                                         template_index;
                }

                /* Opening line -- indent 5 spaces on the first paragraph
                   only. */
                if (i == 0)
                        line_spacing = -5;
                else
                        line_spacing =  2;
                walk_result = randomRange(0, 3);
                cursor_y = letter_type_string_animated(
                        letter_line_ptr[template_index + walk_result],
                        line_spacing);
                char_test = (cursor_y != '-');

                /* Middle line */
                walk_result = randomRange(0, 3);
                cursor_y = letter_type_string_animated(
                        letter_line_ptr[template_index + walk_result + 4],
                        char_test);
                char_test = (cursor_y != '-');

                /* Ending line */
                walk_result = randomRange(0, 3);
                letter_type_string_animated(
                        letter_line_ptr[template_index + walk_result + 8],
                        char_test);
        }

        /* --- Sign-off. --- */
        letter_type_character_animated('\r');
        line_spacing = -8;
        walk_result = randomRange(0, 3);
        letter_type_string_animated(
                letter_greeting_table[walk_result], line_spacing);
        letter_type_character_animated('\r');

        sprintf(input_string, "%s", lcp.character_name);
        letter_type_string_animated(input_string, -10);
        game_tick_and_animate(60);

        /* --- Cleanup: free buffer, hide typing sprites, walk out. --- */
        text_scroll_timer        = 0;
        command_input_buffer_pos = 0;
        disable_key_input_flag   = NO;
        _gemdos(GEMDOS_Mfree, (long) letter_txt_content, 0L, 0L);

        sprite_layer_flags[SPRITE_TYPING_1] = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_3] = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_4] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_TYPING_2);
        sprite_pending_x[sprite_slot_map[SPRITE_TYPING_2]] = 211;
        sprite_pending_y[sprite_slot_map[SPRITE_TYPING_2]] =  44;
        game_tick_and_animate(4);

        lcp_state      = STATE_STAND_SIDE_VIEW;
        head_anim_mode = HEAD_ANIM_DISABLED;
        lcp_y = lcp_y - 6;
        game_tick_and_animate(0);
        action_interruptible_flag = YES;

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &walk_target_x, &walk_target_y);
        walk_target_x = walk_target_x - 10;
        walk_target_y = walk_target_y +  3;
        lcp_walk_to_destination();
        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();

        sprite_layer_flags[SPRITE_TYPEWRITER] = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_1]   = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_2]   = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_3]   = SPRITE_HIDDEN;
        sprite_layer_flags[SPRITE_TYPING_4]   = SPRITE_HIDDEN;
        sprite_update_slots();
        action_interruptible_flag = NO;
}
