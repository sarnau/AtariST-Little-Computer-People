/*
 * tvanim.c -- TV screen contents (bouncing line, pattern lines).
 * TV rect: (293,99)..(308,106).
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>

#ifdef HOST

#include "hostgem.h"

#else

#include <vdibind.h>

#endif
#include "protos.h"
#include "globals.h"

/* tvClearAnim lives in parts/tvClearAnim.c, included by stx_u2.c right after
   useComputer. */

/* A dot bouncing inside the TV rectangle in random colours: v_pline
   draws a two-point line whose ends coincide.  The order of the local
   declarations is part of the original code; keep it. */

void
tvBounce()
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
                vsl_color(vdiHandle, (int) ((Random() & 0xf) | 1));

                pts[0] = xpos + dx;
                pts[1] = ypos + dy;
                pts[2] = pts[0];
                pts[3] = pts[1];
                xpos   = pts[0];
                ypos   = pts[1];

                beginDraw();
                v_pline(vdiHandle, 2, pts);
                endDraw();
                gameTick(0);

                if (pts[0] == 308) dx = -1;
                if (pts[0] == 293) dx =  1;
                if (pts[1] == 106) dy = -1;
                if (pts[1] ==  99) dy =  1;
        }
}

/* tvPattern lives in parts/tvPattern.c, included by stx_u2.c. */
