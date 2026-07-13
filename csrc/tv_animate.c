/*
 * tv_animate.c -- TV screen contents (bouncing line, pattern lines).
 *
 * When action_play_computer triggers the rare "clear the screen and
 * pretend to be watching TV" gesture, tv_show_screen_clear blanks the
 * 15x7-pixel TV rectangle at (293, 99)..(308, 106) with a filled
 * black bar, then picks (via 1 bit of XBIOS Random) between two
 * "programs":
 *
 *   tv_show_bouncing_line -- 64..319 tick loop, single-pixel line
 *     bouncing around inside the rectangle with a random colour each
 *     step.  Wall-collision reverses dx/dy.
 *   tv_show_pattern_lines -- 4 sets of 8 pre-computed lines from
 *     tv_pattern_0/1/2/3_x/y_coords[], each drawn in a specific
 *     colour from tv_pattern_color_indices[].
 *
 * addr: tv_show_screen_clear(), tv_show_bouncing_line(),
 *       tv_show_pattern_lines()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern void     screen_set_draw_to_backbuffer();
extern void     screen_set_draw_to_frontbuffer();
extern void     vsl_color();
extern void     v_pline();
extern void     v_bar();

/* tv_show_screen_clear: blank the TV area with a filled rectangle,
   then dispatch to one of the two programs via a coin flip on
   XBIOS Random.
   addr: tv_show_screen_clear() */

extern void     tv_show_bouncing_line();
extern void     tv_show_pattern_lines();

void
tv_show_screen_clear()
{
        RECT16          rect;
        unsigned short  rnd;

        rect.x1 = 293; rect.y1 =  99;
        rect.x2 = 308; rect.y2 = 106;

        screen_set_draw_to_backbuffer();
        v_bar(vdihandle, &rect.x1);
        screen_set_draw_to_frontbuffer();
        game_tick_and_animate(1);

        rnd = (unsigned short) Random();
        if ((rnd & 1) == 0)
                tv_show_bouncing_line();
        else
                tv_show_pattern_lines();
}

/* tv_show_bouncing_line: `(rnd & 0xff) | 0x40` iterations (64..319) of
   one-pixel step + reverse-on-wall, each step drawn in a random colour
   from `(rnd & 0xf) | 1` (never picks 0=black so the pixels are always
   visible).
   addr: tv_show_bouncing_line() */

void
tv_show_bouncing_line()
{
        unsigned short  rnd;
        unsigned short  rcolor;
        short           pos[2];
        short           dx, dy;
        short           frame;
        short           line_pos[2];

        rnd = (unsigned short) Random();
        line_pos[1] = (rnd & 7) + 293;
        rnd = (unsigned short) Random();
        line_pos[0] = (rnd & 3) + 99;
        dx = 1;
        dy = 1;

        rnd = (unsigned short) Random();
        for (frame = 0;
             frame < (short) ((rnd & 0xff) | 0x40);
             frame = frame + 1) {
                rcolor = (unsigned short) Random();
                vsl_color(vdihandle, (rcolor & 0xf) | 1);

                line_pos[1] = line_pos[1] + dy;
                line_pos[0] = line_pos[0] + dx;
                pos[0] = line_pos[1];
                pos[1] = line_pos[0];

                screen_set_draw_to_backbuffer();
                v_pline(vdihandle, 2, pos);
                screen_set_draw_to_frontbuffer();
                game_tick_and_animate(0);

                if (pos[0] == 308) dy = -1;
                if (pos[0] == 293) dy =  1;
                if (pos[1] == 106) dx = -1;
                if (pos[1] ==  99) dx =  1;
        }
}

/* tv_show_pattern_lines: 4 canned pattern sets, each drawing 1..8 line
   segments (count determined by `Random() & 7`) in a per-pattern
   colour.  Coordinate tables live in globals.c; the values are
   plausible stand-ins pending a Ghidra data-segment dump.
   addr: tv_show_pattern_lines() */

void
tv_show_pattern_lines()
{
        unsigned short  rnd;
        short           point[2];
        short           i;
        short           pattern;
        short *         xs;
        short *         ys;

        for (pattern = 0; pattern < 4; pattern = pattern + 1) {
                rnd = (unsigned short) Random();
                switch (pattern) {
                case 0:
                        xs = tv_pattern_0_x_coords;
                        ys = tv_pattern_0_y_coords;
                        break;
                case 1:
                        xs = tv_pattern_1_x_coords;
                        ys = tv_pattern_1_y_coords;
                        break;
                case 2:
                        xs = tv_pattern_2_x_coords;
                        ys = tv_pattern_2_y_coords;
                        break;
                default:
                        xs = tv_pattern_3_x_coords;
                        ys = tv_pattern_3_y_coords;
                        break;
                }

                for (i = 0; i <= (short) (rnd & 7); i = i + 1) {
                        point[0] = xs[i];
                        point[1] = ys[i];
                        screen_set_draw_to_backbuffer();
                        vsl_color(vdihandle,
                                  _vdi_color_table[
                                    tv_pattern_color_indices[pattern]]);
                        v_pline(vdihandle, 2, point);
                        screen_set_draw_to_frontbuffer();
                        game_tick_and_animate(1);
                }
        }
}
