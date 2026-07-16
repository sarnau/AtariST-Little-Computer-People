/*
 * aletter.c -- ACTION_WRITE_LETTER + its typewriter helpers.
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
 *   Greeting        -- one of 4 sign-offs from g_ltg[]
 *   Signature       -- lcp.character_name
 *
 * Text is fed one character at a time through letter_type_character_
 * animated(), which drives the desk-typing sprite pipeline (four
 * SPRITE_TYPING_1..4 width brackets) and picks a random typewriter
 * click SFX per character.  lt_tysa() word-wraps
 * at 40 columns (0x27 threshold) and inserts a carriage return when
 * a word would overflow.
 *
 * The letter template pointers (g_ltlp[], letter_greeting_
 * table[]) live in globals.c initialised to NULL.  file_load_letter_
 * template() -- still stubbed -- would populate them from letter.txt
 * on the ST disk.  Until that lands, lt_tysa()
 * detects the NULL pointer and returns immediately, so the outer
 * animation still runs but no text is typed.
 *
 * addr: a_writl(), lt_tysa(),
 *       lt_tyca()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    date_day;
extern short    date_month;
extern short    date_year;
extern PLAYER   lcp;                            /* the resident LCP */
extern short    lcp_x;
extern short    lcp_y;
extern short    g_hatas;
extern short    g_hamod;
extern BOOL16   g_actif;
extern short    g_wtx;
extern short    g_wty;
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern short    lcp_record_playing;
extern short    disable_key_input_flag;
extern short    text_scroll_timer;
extern short    g_srsdc;
extern short    g_cdibp;
extern char *   g_lttx;
extern char *   g_ltlp[];
extern char *   g_ltg[];
extern char *   month_name_table[];
extern short    g_ltcwt[];
extern char     g_ltscb[];
extern void     house_get_position_xy();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_selaf[];
extern short    g_seslm[];
extern short    randomRange();                  /* random.c */
#include <osbind.h>
#include <stdio.h>              /* sprintf */

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     sp_sprs();
extern void     sp_upds();
extern void     sf_sele();
extern void     hide_lcp_sprites();
extern void     show_lcp_sprites();
extern void     a_opcfc();
extern void     a_watat();
extern void     fill_top_rect_with_background();
extern void     fl_ltpl();
extern void     print_char();
extern void     lt_sets();
extern void     select_random_click_sound();
extern void     error_not_enough_memory();
extern char     input_string[];

/* Forward-declare the two helpers + a_playp so calls before
   the definitions still resolve under -Werror. */
extern void     lt_tyca();
extern short    lt_tysa();
extern void     a_playp();

/* lt_tyca: emit one character.  On CR (< space),
   scrolls the text pane down; otherwise plays a random click and blits
   the char via print_char, then swaps in the appropriate width sprite
   from g_ltcwt[] based on current buffer position.

   addr: lt_tyca() */

void
lt_tyca(ch)
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

                g_srsdc = 4;
                g_cdibp = 0;

                g_selaf[SPRITE_TYPING_1] = SPRITE_HIDDEN;
                g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
                g_selaf[SPRITE_TYPING_3] = SPRITE_HIDDEN;
                g_selaf[SPRITE_TYPING_4] = SPRITE_HIDDEN;
                sp_upds();

                /* Pick the width-bracket sprite for the (now-empty)
                   buffer position.  Buckets: 0..9, 10..19, 20..29, 30+.
                   Original code re-checks the just-cleared buffer_pos
                   here, so i always resolves to 0 (< 10); preserved
                   verbatim as it may be a placeholder for future logic. */
                i = 3;
                if (g_cdibp < 10)      i = 0;
                else if (g_cdibp < 20) i = 1;
                else if (g_cdibp < 30) i = 2;

                g_selaf[g_ltcwt[i]] = SPRITE_IN_FRONT;
                sp_sprs(g_ltcwt[i]);
                g_sepex[g_seslm[g_ltcwt[i]]] = 211;
                g_sepey[g_seslm[g_ltcwt[i]]] =  44;
                lt_sets();
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
        print_char(ch, g_cdibp << 3, 23, COLOR_black);
        g_cdibp = g_cdibp + 1;

        g_selaf[SPRITE_TYPING_1] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_3] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_4] = SPRITE_HIDDEN;
        sp_upds();

        i = 3;
        if (g_cdibp < 10)      i = 0;
        else if (g_cdibp < 20) i = 1;
        else if (g_cdibp < 30) i = 2;

        g_selaf[g_ltcwt[i]] = SPRITE_IN_FRONT;
        sp_sprs(g_ltcwt[i]);
        g_sepex[g_seslm[g_ltcwt[i]]] = 211;
        g_sepey[g_seslm[g_ltcwt[i]]] =  44;

        /* 1/21 chance of a short pause between keystrokes. */
        pick = randomRange(0, 20);
        if (pick == 0) {
                pick = randomRange(0, 3);
                game_tick_and_animate(pick);
        }
}

/* lt_tysa: word-wrapped string typer.
   `val` argument: leading-space indent count (negative means "always
   indent"; positive means "only indent if the previous line ended
   mid-line").  Returns the last character emitted so the caller can
   check if it was a space (used to decide whether to insert a break
   before the next chunk).

   addr: lt_tysa() */

short
lt_tysa(str, val)
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
        if (val < 0 || g_cdibp > 0) {
                if (val < 0)
                        val = -val;
                for (i = 0; i < val; i = i + 1)
                        lt_tyca(' ');
        }

        word_wrap_needed = NO;
        line_remaining   = 0;
        while (word_wrap_needed == NO) {
                /* Skip inter-word spaces (emit them if we've already
                   started a line). */
                while (*str == ' ') {
                        str = str + 1;
                        if (g_cdibp > 0)
                                lt_tyca(' ');
                }

                /* Collect one word into g_ltscb. */
                i = 0;
                for (;;) {
                        line_remaining = (short) *str;
                        if (line_remaining < 0x21)      /* < '!' */
                                break;
                        g_ltscb[i] = *str;
                        i = i + 1;
                        str = str + 1;
                }

                if (line_remaining != ' ') {
                        word_wrap_needed = YES;
                        str = str + 1;
                }

                /* Word-wrap: if this word would overflow the 40-col
                   line, insert a CR first. */
                if ((short) (i + g_cdibp) > 0x27)
                        lt_tyca('\r');

                for (word_length = 0; word_length < i; word_length = word_length + 1) {
                        line_remaining = (short) g_ltscb[word_length];
                        lt_tyca(line_remaining);
                }
        }
        return line_remaining;
}

/* a_writl: outer flow -- walk, allocate letter buffer,
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

   addr: a_writl() */

void
a_writl()
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
                a_playp();

        house_get_position_xy(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        a_watat();
        walk_result = randomRange(0, 100);
        if (lcp.initiative_threshold < walk_result)
                a_opcfc();

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        g_wty = g_wty +  3;
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        g_actif = YES;

        /* Drop the typewriter + typing sprites. */
        g_selaf[SPRITE_TYPEWRITER] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPEWRITER);
        g_sepex[g_seslm[SPRITE_TYPEWRITER]] = 201;
        g_sepey[g_seslm[SPRITE_TYPEWRITER]] =  51;
        g_selaf[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPING_2);
        g_sepex[g_seslm[SPRITE_TYPING_2]] = 211;
        g_sepey[g_seslm[SPRITE_TYPING_2]] =  44;

        house_get_position_xy(POS_TOP_DESK_CHAIR,
                              &g_wtx, &g_wty);
        g_wty = g_wty -  4;
        g_wtx = g_wtx - 14;
        lcp_walk_to_destination();

        lcp_state              = STATE_STAND_SIDE_VIEW;
        lcp_facing_direction   = FACING_RIGHT;
        g_hatas = 8;
        lcp_wait_head_reach_target();

        lcp_x = lcp_x + 5;
        lcp_y = lcp_y + 6;
        lcp_state = STATE_WRITE_AT_DESK;
        game_tick_and_animate(1);

        g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_TYPING_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPING_1);
        g_sepex[g_seslm[SPRITE_TYPING_1]] = 211;
        g_sepey[g_seslm[SPRITE_TYPING_1]] =  44;

        g_hamod         = HEAD_ANIM_READING;
        disable_key_input_flag = YES;
        fill_top_rect_with_background(0x1b);

        /* Allocate the letter template buffer -- 0x2900 bytes. */
        g_lttx = (char *) _gemdos(GEMDOS_Malloc, 0x2900L, 0L, 0L);
        if (g_lttx == (char *) 0)
                error_not_enough_memory();
        fl_ltpl();

        text_scroll_timer = 9999;
        game_tick_and_animate(2);

        /* --- Date + salutation --- */
        full_year = date_year + 1900;
        sprintf(input_string, "%s %d, %4d",
                month_name_table[date_month],
                date_day + 1, full_year);
        g_cdibp = 0;
        lt_tysa(input_string, -12);
        lt_tyca('\r');

        sprintf(input_string, "Dear %s,", lcp.owner_name);
        lt_tysa(input_string, 0);
        lt_tyca('\r');

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
                cursor_y = lt_tysa(
                        g_ltlp[template_index + walk_result],
                        line_spacing);
                char_test = (cursor_y != '-');

                /* Middle line */
                walk_result = randomRange(0, 3);
                cursor_y = lt_tysa(
                        g_ltlp[template_index + walk_result + 4],
                        char_test);
                char_test = (cursor_y != '-');

                /* Ending line */
                walk_result = randomRange(0, 3);
                lt_tysa(
                        g_ltlp[template_index + walk_result + 8],
                        char_test);
        }

        /* --- Sign-off. --- */
        lt_tyca('\r');
        line_spacing = -8;
        walk_result = randomRange(0, 3);
        lt_tysa(
                g_ltg[walk_result], line_spacing);
        lt_tyca('\r');

        sprintf(input_string, "%s", lcp.character_name);
        lt_tysa(input_string, -10);
        game_tick_and_animate(60);

        /* --- Cleanup: free buffer, hide typing sprites, walk out. --- */
        text_scroll_timer        = 0;
        g_cdibp = 0;
        disable_key_input_flag   = NO;
        _gemdos(GEMDOS_Mfree, (long) g_lttx, 0L, 0L);

        g_selaf[SPRITE_TYPING_1] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_3] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_4] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPING_2);
        g_sepex[g_seslm[SPRITE_TYPING_2]] = 211;
        g_sepey[g_seslm[SPRITE_TYPING_2]] =  44;
        game_tick_and_animate(4);

        lcp_state      = STATE_STAND_SIDE_VIEW;
        g_hamod = HEAD_ANIM_DISABLED;
        lcp_y = lcp_y - 6;
        game_tick_and_animate(0);
        g_actif = YES;

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        g_wty = g_wty +  3;
        lcp_walk_to_destination();
        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();

        g_selaf[SPRITE_TYPEWRITER] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_1]   = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_2]   = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_3]   = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_4]   = SPRITE_HIDDEN;
        sp_upds();
        g_actif = NO;
}
