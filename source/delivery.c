/*
 * delivery.c -- Ctrl+F/B/R/D/C doorbell events.
 * Shared shape: wkFrDr, open door, bend/reach/bend, maybe close,
 * attach carried sprite, walk to shelf, put down.
 * Ctrl+D dog-food reuses er_food with g_dvdog=YES.
 * ev_ansPh is here because it's the same event-queue consumer.
 * addr: er_food(), er_bood(), er_recd(), er_dogf(), ev_ansPh(),
 *       wkFrDr(), a_opcfd(), a_opecc()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "afood.h"
#include "asimple.h"
#include "delivery.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"


/* addr: wkFrDr() */

/* wkFrDr -> parts/wkFrDr.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/wkFrDr.c"
#endif

/* a_opcfd: toggle the front door.  door_st=0 opens, 1 closes.
   addr: a_opcfd() */

/* a_opcfd -> parts/a_opcfd.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_opcfd.c"
#endif

/* a_opecc: kitchen cabinet toggle.  addr: a_opecc() */

/* a_opecc -> parts/a_opecc.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_opecc.c"
#endif

/* Common "at the door, pick up" sequence: open door, bend/reach/bend,
   then optionally close via the initiative-threshold roll. */

#ifdef FAITHFUL
static void
dv_pick()
{
        short   roll;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_opcfd(0);

        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_REACH_FORWARD;
        gameTick(2);
        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        roll = rndRng(0, 100);
        if (lcp.initiative_threshold < roll)
                a_opcfd(1);
}
#endif

/* er_food: Ctrl+F grocery event.  Reused by er_dogf with g_dvdog set.
   addr: er_food() */

/* er_food -> parts/er_food.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/er_food.c"
#endif

/* er_bood: Ctrl+B.  Book -> bookshelf.  addr: er_bood() */

/* er_bood -> parts/er_bood.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/er_bood.c"
#endif

/* er_recd: Ctrl+R.  Record -> dance floor shelf.
   Note the original also increments lcp_food at the end -- this
   looks like an off-by-one bug (should have been counting records), but
   preserved for faithfulness.
   addr: er_recd() */

/* er_recd -> parts/er_recd.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/er_recd.c"
#endif

/* er_dogf: Ctrl+D.  Trampoline into er_food with g_dvdog set.
   addr: er_dogf() */

/* er_dogf -> parts/er_dogf.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/er_dogf.c"
#endif

/* ev_ansPh: Ctrl+C or random daytime call.  a_calld() puts the resident
   at the phone (position 43); talks 40..50 ticks with random head
   positions/SFX; ph_ans guards re-entry.  addr: ev_ansPh() */

/* ev_ansPh -> parts/ev_ansPh.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/ev_ansPh.c"
#endif
