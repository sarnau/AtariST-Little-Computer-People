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
/* tv_scrc -> parts/tv_scrc.c (STX: 0xdece object, 0x13074, right after a_playc). */
#ifdef FAITHFUL
#include "parts/tv_scrc.c"
#endif

/* addr: tv_boul() (ROM 0xd404).  v_pline is called with count=2 but
   only pos[0..1] initialised: the second point deliberately overlaps
   the rcolor/rnd locals in the ROM frame layout, so the TV draws a
   line to a pseudo-random point.  Local order below reproduces the
   exact ROM frame (-2 rnd, -4 rcolor, -8 pos, -10 dy, -12 dx,
   -14 frame, -16 xpos, -18 ypos). */
#ifdef FAITHFUL
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
#else   /* STX: link #-36 -- xpos, ypos, frame, limit, dx, dy and a
           10-short point buffer at -32; every random draw is consumed
           in place and the step goes through the buffer. */

void
tv_boul()
{
        short   xpos;
        short   ypos;
        short   frame;
        short   limit;
        short   dx;
        short   dy;
        short   pts[10];

        xpos = (int) (Random() & 7) + 293;
        ypos = (int) (Random() & 3) + 99;
        dx = 1;
        dy = 1;

        limit  = Random() & 0xff;
        limit |= 0x40;
        for (frame = 0; frame < limit; frame++) {
                vsl_color(vdihnd, (int) ((Random() & 0xf) | 1));

                pts[0] = xpos + dx;
                pts[1] = ypos + dy;
                pts[2] = pts[0];
                pts[3] = pts[1];
                xpos   = pts[0];
                ypos   = pts[1];

                sc_sdtb();
                v_pline(vdihnd, 2, pts);
                sc_sdtf();
                gameTick(0);

                if (pts[0] == 308) dx = -1;
                if (pts[0] == 293) dx =  1;
                if (pts[1] == 106) dy = -1;
                if (pts[1] ==  99) dy =  1;
        }
}
#endif

/* tv_patl -> parts/tv_patl.c (STX: 0xdece object, 0x13204). */
#ifdef FAITHFUL
#include "parts/tv_patl.c"
#endif
