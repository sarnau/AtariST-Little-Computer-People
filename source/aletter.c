/*
 * aletter.c -- ACTION_WRITE_LETTER + typewriter helpers.
 * Procedurally-assembled letter: date, salutation, 2..4 paragraphs
 * from 4 topic sections (3 lines x 4 alternates, biased by mood),
 * sign-off (g_ltg[4]), signature.  Word-wraps at 40 cols (0x27).
 * addr: a_writl(), lt_tysa(), lt_tyca()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include <stdio.h>              /* sprintf */
#include "adoors.h"
#include "aleisure.h"
#include "alerts.h"
#include "aletter.h"
#include "globals.h"
#include "letload.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"


/* a_writl: walk, malloc letter buffer, assemble body from shuffled
   template sections (2..4 paragraphs, each 3 lines picked from 4
   alternates via section_id * 96 + mood offset), free, walk out.
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

        if (lcp_recP != NO)
                a_playp();

        hs_posXY(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        a_watat();
        walk_result = rndRng(0, 100);
        if (lcp.initiative_threshold < walk_result)
                a_opcfc();

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        g_wty = g_wty +  3;
        result = lcp_wkD();
        if (result != 0)
                return;

        g_actif = YES;

        g_selaf[SPRITE_TYPEWRITER] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPEWRITER);
        g_sepex[g_seslm[SPRITE_TYPEWRITER]] = 201;
        g_sepey[g_seslm[SPRITE_TYPEWRITER]] =  51;
        g_selaf[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPING_2);
        g_sepex[g_seslm[SPRITE_TYPING_2]] = 211;
        g_sepey[g_seslm[SPRITE_TYPING_2]] =  44;

        hs_posXY(POS_TOP_DESK_CHAIR,
                              &g_wtx, &g_wty);
        g_wty = g_wty -  4;
        g_wtx = g_wtx - 14;
        lcp_wkD();

        lcp_st              = STATE_STAND_SIDE_VIEW;
        lcp_face   = FACING_RIGHT;
        g_hatas = 8;
        lcp_hwt();

        lcp_x = lcp_x + 5;
        lcp_y = lcp_y + 6;
        lcp_st = STATE_WRITE_AT_DESK;
        gameTick(1);

        g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_TYPING_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPING_1);
        g_sepex[g_seslm[SPRITE_TYPING_1]] = 211;
        g_sepey[g_seslm[SPRITE_TYPING_1]] =  44;

        g_hamod         = HEAD_ANIM_READING;
        no_keyin = YES;
        fillTopR(0x1b);

        g_lttx = (char *) Malloc(0x2900L);
        if (g_lttx == (char *) 0)
                er_nomem();
        fl_ltpl();

        tx_sctm = 9999;
        gameTick(2);

        full_year = dt_year + 1900;
        sprintf(in_str, "%s %d, %4d",
                mo_names[dt_mon],
                date_day + 1, full_year);
        g_cdibp = 0;
        lt_tysa(in_str, -12);
        lt_tyca('\r');

        sprintf(in_str, "Dear %s,", lcp.owner_name);
        lt_tysa(in_str, 0);
        lt_tyca('\r');

        /* Shuffle the 4 section indices via 16 random swaps. */
        for (i = 0; i < 4; i = i + 1)
                section_order[i] = i;
        for (i = 0; i < 16; i = i + 1) {
                swap_a = rndRng(0, 3);
                swap_b = rndRng(0, 3);
                swap_temp = section_order[swap_a];
                section_order[swap_a] = section_order[swap_b];
                section_order[swap_b] = swap_temp;
        }

        /* Body: 2..4 paragraphs from the shuffled sections. */
        paragraph_count = rndRng(2, 4);
        for (i = 0; i < paragraph_count; i = i + 1) {
                section_id     = section_order[i];
                template_index = section_id * 0x60;
                if (section_id == 3) {
                        walk_result = rndRng(0, 5);
                        template_index = walk_result * 0xc + template_index;
                } else if (lcp.sickness_level < 1) {
                        walk_result = rndRng(0, 1);
                        template_index = lcp.happiness * 0xc +
                                         walk_result * 0x30 +
                                         template_index;
                } else {
                        walk_result = rndRng(0, 1);
                        template_index = walk_result * 0x30 + 0x24 +
                                         template_index;
                }

                /* Opening line -- indent 5 spaces on the first paragraph
                   only. */
                if (i == 0)
                        line_spacing = -5;
                else
                        line_spacing =  2;
                walk_result = rndRng(0, 3);
                cursor_y = lt_tysa(
                        g_ltlp[template_index + walk_result],
                        line_spacing);
                char_test = (cursor_y != '-');

                /* Middle line */
                walk_result = rndRng(0, 3);
                cursor_y = lt_tysa(
                        g_ltlp[template_index + walk_result + 4],
                        char_test);
                char_test = (cursor_y != '-');

                /* Ending line */
                walk_result = rndRng(0, 3);
                lt_tysa(
                        g_ltlp[template_index + walk_result + 8],
                        char_test);
        }

        /* Sign-off. */
        lt_tyca('\r');
        line_spacing = -8;
        walk_result = rndRng(0, 3);
        lt_tysa(
                g_ltg[walk_result], line_spacing);
        lt_tyca('\r');

        sprintf(in_str, "%s", lcp.character_name);
        lt_tysa(in_str, -10);
        gameTick(60);

        /* Cleanup: free buffer, hide typing sprites, walk out. */
        tx_sctm        = 0;
        g_cdibp = 0;
        no_keyin   = NO;
        Mfree(g_lttx);

        g_selaf[SPRITE_TYPING_1] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_3] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_4] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPING_2);
        g_sepex[g_seslm[SPRITE_TYPING_2]] = 211;
        g_sepey[g_seslm[SPRITE_TYPING_2]] =  44;
        gameTick(4);

        lcp_st      = STATE_STAND_SIDE_VIEW;
        g_hamod = HEAD_ANIM_DISABLED;
        lcp_y = lcp_y - 6;
        gameTick(0);
        g_actif = YES;

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        g_wty = g_wty +  3;
        lcp_wkD();
        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_TYPEWRITER] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_1]   = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_2]   = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_3]   = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_4]   = SPRITE_HIDDEN;
        sp_upds();
        g_actif = NO;
}

/* lt_tysa: word-wrapped string typer.
   val: leading-space indent (< 0 always; > 0 only if prev line mid).
   Returns last char emitted.
   addr: lt_tysa() */

short
lt_tysa(str, val)
char *  str;
short   val;
{
        /* STX declares the g_ltscb index first and has no NULL guard;
           both scan loops are `while ((ch = *str++) <op> ' ')`, which
           Alcyon compiles by saving the flags across the pointer
           increment. */
        short   i;
        short   word_length;
        short   ch;
        BOOL16  word_wrap_needed;

        if (val < 0 || g_cdibp > 0) {
                if (val < 0)
                        val = -val;
                for (i = 0; i < val; i++)
                        lt_tyca(' ');
        }

        word_wrap_needed = NO;
        while (word_wrap_needed == NO) {
                /* Skip inter-word spaces (emit if line already started),
                   then step back onto the first non-space. */
                while ((ch = *str++) == ' ')
                        if (g_cdibp > 0)
                                lt_tyca(ch);
                str--;

                i = 0;
                while ((ch = *str++) > ' ') {
                        /* Index first: Alcyon folds the base into the
                           address register (add.l #g_ltscb,a1). */
                        *(i + g_ltscb) = ch;
                        i++;
                }
                if (ch != ' ')
                        word_wrap_needed = YES;
                else
                        str--;

                /* Word-wrap at 40 columns. */
                if (g_cdibp + i > 39)
                        lt_tyca(13);

                for (word_length = 0; word_length < i; word_length++) {
                        ch = g_ltscb[word_length];
                        lt_tyca(ch);
                }
        }
        return ch;
}


/* lt_tyca: emit one char.  CR (< space) scrolls the pane; else plays a
   random click, blits via prCh, swaps in the g_ltcwt width sprite
   per buffer position.
   addr: lt_tyca() */

void
lt_tyca(ch)
short   ch;
{
        /* One local: STX consumes every rndRng result in place. */
        short   i;

        if (ch < ' ') {                 /* CR */
                lcp_st = STATE_DESK_TYPE_L;
                gameTick(0);
                if (rndRng(0, 5) == 0) {
                        lcp_st = STATE_DESK_TYPE_R;
                        gameTick(0);
                        lcp_st = STATE_DESK_TYPE_L;
                        gameTick(0);
                }
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_WRITE_AT_DESK;
                gameTick(0);

                g_srsdc = 4;
                g_cdibp = 0;

                g_selaf[SPRITE_TYPING_1] = SPRITE_HIDDEN;
                g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
                g_selaf[SPRITE_TYPING_3] = SPRITE_HIDDEN;
                g_selaf[SPRITE_TYPING_4] = SPRITE_HIDDEN;
                sp_upds();

                /* Width-bracket sprite for buffer_pos (0..9/10..19/20..29/30+).
                   i resolves to 0 here (buffer_pos just cleared);
                   preserved verbatim. */
                i = 3;
                if (g_cdibp < 10)      i = 0;
                else if (g_cdibp < 20) i = 1;
                else if (g_cdibp < 30) i = 2;

                g_selaf[g_ltcwt[i]] = SPRITE_IN_FRONT;
                sp_sprs(g_ltcwt[i]);
                g_sepex[g_seslm[g_ltcwt[i]]] = 211;
                g_sepey[g_seslm[g_ltcwt[i]]] =  44;
                lt_sets();
                gameTick(6);
                return;
        }

        /* Printable char. */
        if (ch == ' ') {
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_DESK_TYPE_L;
                gameTick(0);
        } else {
                lcp_face = rndRng(0, 1);
                lcp_st = STATE_DESK_TYPE_L;
                gameTick(0);
                if (rndRng(0, 5) == 0) {
                        lcp_st = STATE_DESK_TYPE_R;
                        gameTick(0);
                        lcp_st = STATE_DESK_TYPE_L;
                        gameTick(0);
                }
        }
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_WRITE_AT_DESK;
        sfClick();
        gameTick(0);
        prCh(ch, g_cdibp << 3, 23, COLOR_black);
        g_cdibp++;

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
        if (rndRng(0, 20) == 0)
                gameTick(rndRng(0, 3));
}
