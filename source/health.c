/* health.c -- sickness onset and recovery. */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "globals.h"
#include "health.h"
#include "renderx.h"

/* addr: lcp_sick() */
void
lcp_sick()
{
        lcp.sickness_level      = SICKNESS_MILD;
        lcp.sickness_countdown  = 60;
        lcp.sickness_direction  = DIR_WORSENING;
        lcp.happiness_direction = DIR_WORSENING;
        if (lcp.happiness < 2)
                /* LCP_ORG compiles the register-form add; the STX
                   revision's addq shape comes from +=. */
#ifdef FAITHFUL
                lcp.happiness = lcp.happiness + MOOD_CONTENT;
#else
                lcp.happiness += MOOD_CONTENT;
#endif
        lcp_upal();
}

/* addr: lcp_rcov() */
void
lcp_rcov()
{
        if (lcp.hunger_level == NEED_SATISFIED &&
            lcp.thirst_level == NEED_SATISFIED) {
                lcp.sickness_direction = DIR_IMPROVING;
                lcp.sickness_countdown = 5;
        }
}

/* In the STX revision lcp_upal lives HERE (after lcp_rcov, same
   object -- lcp_sick reaches it with a bsr).  LCP_ORG links it in
   renderx.c instead; see the #ifdef FAITHFUL twin there. */
#ifndef FAITHFUL
void
lcp_upal()
{
        if (lcp.sickness_level == SICKNESS_HEALTHY)
                main_pal[6] = ST_PEACH;
        else
                main_pal[6] = ST_SICK_GREEN;
        Setpalette(main_pal);
}
#endif
