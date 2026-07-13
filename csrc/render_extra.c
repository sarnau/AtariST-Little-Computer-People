/*
 * render_extra.c -- palette, TV, screen-scroll, and print_char.
 *
 * Split from render.c to keep the file digest manageable.  Everything
 * here is a real port of a Ghidra-verified function; the underlying
 * VDI/XBIOS traps (Setpalette, Setscreen, Logbase, vst_color, vswr_mode,
 * v_gtext) fall through to host stubs in osbind.h / stubs.c when
 * building without the ST hardware.
 *
 * addr: palette_apply_clothing_colors(), palette_apply_skin_colors(),
 *       lcp_update_palette_colors(), tv_draw_static_line(),
 *       tv_draw_static_noise(), screen_scroll_text_down(),
 *       print_char()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern short    randomRange();
extern void     draw_line();
extern void     blkcopy32();
extern void     screen_fill_row_white();
extern void     vst_color();
extern void     vswr_mode();
extern void     v_gtext();
extern void     _draw_pixel();
extern void     print_char();

/* palette_apply_clothing_colors: pick a random or player-configured
   CLOTHING_COLOR_ID (0..15) and load its primary/secondary colours
   into palette slots 1 and 2, then Setpalette.  If the random pick
   overshoots the 16-entry table, falls back to lcp.clothing_color.
   addr: palette_apply_clothing_colors() */

void
palette_apply_clothing_colors()
{
        short   index;

        index = randomRange(0, 0x1f);
        if (index > 0xf)
                index = lcp.clothing_color;

        main_colorpalette[1] = clothing_color_primary[index];
        main_colorpalette[2] = clothing_color_secondary[index];
        _xbios(XBIOS_Setpalette, (long) main_colorpalette, 0L, 0L);
}

/* palette_apply_skin_colors: same shape, 8-entry skin table.
   addr: palette_apply_skin_colors() */

void
palette_apply_skin_colors()
{
        short   index;

        index = randomRange(0, 0xf);
        if (index > 7)
                index = lcp.skin_color;

        main_colorpalette[1] = skin_color_palette[index];
        main_colorpalette[2] = skin_color_palette[index];
        _xbios(XBIOS_Setpalette, (long) main_colorpalette, 0L, 0L);
}

/* lcp_update_palette_colors: refresh the sickness tint at palette
   slot 6.  ST_PEACH (0x743) when healthy, ST_SICK_GREEN (0x363) when
   sick.  Called from sim.c on recovery, from health.c on onset, and
   from lcp_load after HYBER restore.
   addr: lcp_update_palette_colors() */

void
lcp_update_palette_colors()
{
        if (lcp.sickness_level == SICKNESS_HEALTHY)
                main_colorpalette[6] = ST_PEACH;
        else
                main_colorpalette[6] = ST_SICK_GREEN;
        _xbios(XBIOS_Setpalette, (long) main_colorpalette, 0L, 0L);
}

/* tv_draw_static_line: draw the 5-line rabbit-ear antenna on top of
   the TV.  Lines diagonal-up-right from (44..48, 51..49) to (44..48,
   57..55).  Color is passed by the caller (COLOR_white when off,
   random color when on for static effect).
   addr: tv_draw_static_line() */

void
tv_draw_static_line(color)
short   color;
{
        short   i;

        for (i = 0; i < 5; i = i + 1)
                draw_line(i + 44, 51 - (i >> 1),
                          i + 44, 57 - (i >> 1),
                          color);
}

/* tv_draw_static_noise: random-colour antenna each frame while the TV
   is on.  Called from the frame loop; the mask (& COLOR_dk_brown = 0xf)
   keeps the picked colour within the 16-entry palette.
   addr: tv_draw_static_noise() */

void
tv_draw_static_noise()
{
        long    rnd;

        rnd = Random();
        tv_draw_static_line((short) rnd & COLOR_dk_brown);
}

/* screen_scroll_text_down: 1-row block scroll on the top text strip
   (used by the letter typewriter when a line wraps).  Copies 13 rows
   of 40 words each downward using the blitter, then blanks the top
   two rows (24 and 25) to white.
   addr: screen_scroll_text_down() */

void
screen_scroll_text_down()
{
        short   row;
        char *  dest_ptr;
        char *  src_ptr;

        dest_ptr = (char *) dest_screenbase_ptr;
        src_ptr  = (char *) dest_screenbase_ptr;
        for (row = 0; row < 13; row = row + 1) {
                src_ptr = src_ptr + 320;
                blkcopy32(src_ptr, dest_ptr, 10);
                dest_ptr = dest_ptr + 320;
        }
        screen_fill_row_white(dest_screenbase_ptr, 24);
        screen_fill_row_white(dest_screenbase_ptr, 25);
}

/* print_char: render one character via VDI.  Sets the log-base to the
   back-buffer, calls vst_color to set the current text ink, switches
   to MD_TRANS (transparent overlay), calls v_gtext to blit, then
   restores MD_REPLACE and the original log-base.  The Setscreen calls
   pass (void*)-1 for phys and rez which the trap treats as "leave
   unchanged".
   addr: print_char() */

void
print_char(ch, x, y, color)
short   ch;
short   x;
short   y;
short   color;
{
        char    str[2];
        void *  saved_log;

        str[0] = (char) ch;
        str[1] = 0;

        saved_log = (void *) _xbios(XBIOS_Logbase, 0L, 0L, 0L);
        _xbios(XBIOS_Setscreen, (long) dest_screenbase_ptr,
               -1L, -1L);
        vst_color(vdihandle, _vdi_color_table[color]);
        vswr_mode(vdihandle, MD_TRANS);
        v_gtext(vdihandle, x, y, str);
        vswr_mode(vdihandle, MD_REPLACE);
        _xbios(XBIOS_Setscreen, (long) saved_log, -1L, -1L);
}

/* record_player_animate_needle: sweep the needle back and forth from
   x=70..83 at y=42, one pixel per frame, wrapping at 0.  If music is
   playing and we're not currently browsing records, roll a random VU
   meter LED (0..6) at y=47 and toggle its lit/unlit state (red if the
   new mask overlaps the accumulated `letter_paragraph_count`, else
   black).  The `letter_line_count` / `letter_paragraph_count` variable
   names are 1985 shared-storage reuse -- they double as record-player
   state when no letter is being written.
   addr: record_player_animate_needle() */

void
record_player_animate_needle()
{
        unsigned short  rnd;
        short           col;

        if (letter_line_count >= 0)
                _draw_pixel(letter_line_count + 70, 42, COLOR_white);
        letter_line_count = letter_line_count - 2;
        if (letter_line_count < 0)
                letter_line_count = 13;
        _draw_pixel(letter_line_count + 70, 42, COLOR_black);

        if (midi_is_playing == NO || record_browsing_active != NO)
                return;

        rnd = (unsigned short) Random();
        rnd = rnd & 7;
        if (rnd < 7) {
                letter_paragraph_count = _record_led_mask_table[rnd] ^
                                         letter_paragraph_count;
                if ((_record_led_mask_table[rnd] &
                     letter_paragraph_count) == 0)
                        col = COLOR_black;
                else
                        col = COLOR_red;
                _draw_pixel(rnd * 2 + 66, 47, col);
        }
}

/* string_print: paint a NUL-terminated string starting at (x, y).
   Loops print_char one char at a time, bumping x by 8 pixels between
   chars (the font advance width for the 8x8 system font used by the
   status strip / game menu).
   addr: string_print() */

void
string_print(str, x, y, color)
char *  str;
short   x;
short   y;
short   color;
{
        char    ch;

        for (;;) {
                ch = *str;
                str = str + 1;
                if (ch == 0)
                        break;
                print_char((short) ch, x, y, color);
                x = x + 8;
        }
}
