/*
 * tvanim.c -- TV screen contents (bouncing line, pattern lines).
 *
 * When a_playc triggers the rare "clear the screen and
 * pretend to be watching TV" gesture, tv_scrc blanks the
 * 15x7-pixel TV rectangle at (293, 99)..(308, 106) with a filled
 * black bar, then picks (via 1 bit of XBIOS Random) between two
 * "programs":
 *
 *   tv_boul -- 64..319 tick loop, single-pixel line
 *     bouncing around inside the rectangle with a random colour each
 *     step.  Wall-collision reverses dx/dy.
 *   tv_patl -- 4 sets of 8 pre-computed lines from
 *     tv_pattern_0/1/2/3_x/y_coords[], each drawn in a specific
 *     colour from g_tpcoi[].
 *
 * addr: tv_scrc(), tv_boul(),
 *       tv_patl()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern void     gameTick();
extern short    vdihnd;
extern short    vdi_colt[];
extern short    g_tp0xc[];
extern short    g_tp0yc[];
extern short    g_tp1xc[];
extern short    g_tp1yc[];
extern short    g_tp2xc[];
extern short    g_tp2yc[];
extern short    g_tp3xc[];
extern short    g_tp3yc[];
extern short    g_tpcoi[];
#include <osbind.h>

extern void     sc_sdtb();
extern void     sc_sdtf();
extern void     vsl_color();
extern void     v_pline();
extern void     v_bar();

/* tv_scrc: blank the TV area with a filled rectangle,
   then dispatch to one of the two programs via a coin flip on
   XBIOS Random.
   addr: tv_scrc() */

extern void     tv_boul();
extern void     tv_patl();

void
tv_scrc()
{
        RECT16          rect;
        unsigned short  rnd;

        rect.x1 = 293; rect.y1 =  99;
        rect.x2 = 308; rect.y2 = 106;

        sc_sdtb();
        v_bar(vdihnd, &rect.x1);
        sc_sdtf();
        gameTick(1);

        rnd = (unsigned short) Random();
        if ((rnd & 1) == 0)
                tv_boul();
        else
                tv_patl();
}

/* tv_boul: `(rnd & 0xff) | 0x40` iterations (64..319) of
   one-pixel step + reverse-on-wall, each step drawn in a random colour
   from `(rnd & 0xf) | 1` (never picks 0=black so the pixels are always
   visible).
   addr: tv_boul() */

void
tv_boul()
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
                vsl_color(vdihnd, (rcolor & 0xf) | 1);

                line_pos[1] = line_pos[1] + dy;
                line_pos[0] = line_pos[0] + dx;
                pos[0] = line_pos[1];
                pos[1] = line_pos[0];

                sc_sdtb();
                v_pline(vdihnd, 2, pos);
                sc_sdtf();
                gameTick(0);

                if (pos[0] == 308) dy = -1;
                if (pos[0] == 293) dy =  1;
                if (pos[1] == 106) dx = -1;
                if (pos[1] ==  99) dx =  1;
        }
}

/* tv_patl: 4 canned pattern sets, each drawing 1..8 line
   segments (count determined by `Random() & 7`) in a per-pattern
   colour.  Coordinate tables live in globals.c; the values are
   plausible stand-ins pending a Ghidra data-segment dump.
   addr: tv_patl() */

void
tv_patl()
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
                        xs = g_tp0xc;
                        ys = g_tp0yc;
                        break;
                case 1:
                        xs = g_tp1xc;
                        ys = g_tp1yc;
                        break;
                case 2:
                        xs = g_tp2xc;
                        ys = g_tp2yc;
                        break;
                default:
                        xs = g_tp3xc;
                        ys = g_tp3yc;
                        break;
                }

                for (i = 0; i <= (short) (rnd & 7); i = i + 1) {
                        point[0] = xs[i];
                        point[1] = ys[i];
                        sc_sdtb();
                        vsl_color(vdihnd,
                                  vdi_colt[
                                    g_tpcoi[pattern]]);
                        v_pline(vdihnd, 2, point);
                        sc_sdtf();
                        gameTick(1);
                }
        }
}
