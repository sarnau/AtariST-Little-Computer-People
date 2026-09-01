/*
 * tvanim.c -- TV screen contents (bouncing line, pattern lines).
 * TV rect: (293,99)..(308,106).
 * addr: tv_scrc(), tv_boul(), tv_patl()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>

#include <vdibind.h>
#include "gfx_prim.h"
#include "globals.h"
#include "tick.h"
#include "tvanim.h"

/* addr: tv_scrc() */
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

/* addr: tv_boul() (ROM 0xd404).  v_pline is called with count=2 but
   only pos[0..1] initialised: the second point deliberately overlaps
   the rcolor/rnd locals in the ROM frame layout, so the TV draws a
   line to a pseudo-random point.  Local order below reproduces the
   exact ROM frame (-2 rnd, -4 rcolor, -8 pos, -10 dy, -12 dx,
   -14 frame, -16 xpos, -18 ypos). */
void
tv_boul()
{
        unsigned short  rnd;
        unsigned short  rcolor;
        short           pos[2];
        short           dy;
        short           dx;
        short           frame;
        short           xpos;
        short           ypos;

        rnd = (unsigned short) Random();
        xpos = (rnd & 7) + 293;
        rnd = (unsigned short) Random();
        ypos = (rnd & 3) + 99;
        dy = 1;
        dx = 1;

        rnd = (unsigned short) Random();
        for (frame = 0;
             frame < (short) ((rnd & 0xff) | 0x40);
             frame = frame + 1) {
                rcolor = (unsigned short) Random();
                vsl_color(vdihnd, (rcolor & 0xf) | 1);

                xpos = xpos + dx;
                ypos = ypos + dy;
                pos[0] = xpos;
                pos[1] = ypos;

                sc_sdtb();
                v_pline(vdihnd, 2, pos);
                sc_sdtf();
                gameTick(0);

                if (pos[0] == 308) dx = -1;
                if (pos[0] == 293) dx =  1;
                if (pos[1] == 106) dy = -1;
                if (pos[1] ==  99) dy =  1;
        }
}

/* addr: tv_patl() (ROM 0xd53c).  As in tv_boul, v_pline count=2 with
   one initialised point at &x: point 2 reads the rnd local and the
   saved frame pointer -- the ROM's exact layout, reproduced here
   (-2 rnd, -4 y, -6 x, -8 i, -10 pattern, -14 xs, -18 ys). */
void
tv_patl()
{
        unsigned short  rnd;
        short           y;
        short           x;
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
                        x = xs[i];
                        y = ys[i];
                        sc_sdtb();
                        vsl_color(vdihnd,
                                  vdi_colt[
                                    g_tpcoi[pattern]]);
                        v_pline(vdihnd, 2, &x);
                        sc_sdtf();
                        gameTick(1);
                }
        }
}
