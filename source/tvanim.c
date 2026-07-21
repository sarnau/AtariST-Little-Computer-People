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

/* addr: tv_boul() */
void
tv_boul()
{
        unsigned short  rnd;
        unsigned short  rcolor;
        short           pos[4];
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
                pos[2] = line_pos[1];
                pos[3] = line_pos[0];

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

/* addr: tv_patl() */
void
tv_patl()
{
        unsigned short  rnd;
        short           pxy[4];
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
                        pxy[0] = xs[i];
                        pxy[1] = ys[i];
                        pxy[2] = pxy[0] + 3;
                        pxy[3] = pxy[1];
                        sc_sdtb();
                        vsl_color(vdihnd,
                                  vdi_colt[
                                    g_tpcoi[pattern]]);
                        v_pline(vdihnd, 2, pxy);
                        sc_sdtf();
                        gameTick(1);
                }
        }
}
