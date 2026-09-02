/*
 * save.c -- HYBER save file I/O and the study-door save flow.
 * addr: crFile(), fr_read(), lcp_save(), lc_load(), lcp_std()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include <stdio.h>
#include "alerts.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "save.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* fOpen -> parts/fOpen.c (STX: 0x730e). */
#ifdef FAITHFUL
#include "parts/fOpen.c"
#endif

/* crFile -> parts/crFile.c (STX: 0x1488e, right after lcp_save). */
#ifdef FAITHFUL
#include "parts/crFile.c"
#endif

/* addr: fr_read() */
#ifdef FAITHFUL
void
#else
short                   /* STX returns the Fread result */
#endif
/* fr_read -> parts/fr_read.c (STX: 0x736c). */
#ifdef FAITHFUL
#include "parts/fr_read.c"
#endif

/* 4-byte header: discarded temp short, then payload size short.
   addr: fLoad() */
void
fLoad(filename, buffer)
char *  filename;
void *  buffer;
{
        short   fhnd;
        short   size;
        short   temp;

        fhnd = fOpen(filename, 0);
        fr_read(fhnd, 2L, &temp);
        fr_read(fhnd, 2L, &size);
        fr_read(fhnd, (long) size, buffer);
        Fclose(fhnd);
}

/* lcp_save -> parts/lcp_save.c (STX: 0xdece object, 0x1481c). */
#ifdef FAITHFUL
#include "parts/lcp_save.c"
#endif

/* addr: lc_load() */
#ifdef FAITHFUL
short
lc_load()
{
#ifdef FAITHFUL
        short   fhnd;

        fhnd = Fopen("hyber", 0L);      /* ROM passes the mode as 0L */
        if (fhnd < 0)
                return 0;
#else
        /* STX: link #-8 -- the result goes through a second local and
           the open mode is a word. */
        short   fhnd;
        short   ok;

        ok = 0;
        fhnd = Fopen("hyber", 0);
        if (fhnd < 0)
                return ok;
        ok = 1;
#endif

        fr_read(fhnd, 0x80L, &lcp);
        Fclose(fhnd);

        lcp_watr         = lcp.water_level;
        lcp_frdO     = lcp.door_states_and_flags & DSF_FRONT_DOOR;
        lcp_drsO        = (lcp.door_states_and_flags & DSF_DRESSER)          >> 4;
        lcp_cabO        = (lcp.door_states_and_flags & DSF_KITCHEN_CABINET)  >> 3;
        lcp_clsO    = (lcp.door_states_and_flags & DSF_CLOSET_DOOR)      >> 2;
        studyDrO     = (lcp.door_states_and_flags & DSF_STUDY_DOOR)       >> 1;
        lcp_toiO    = (lcp.door_states_and_flags & DSF_TOILET_DOOR)      >> 5;
        lcp_flcO = (lcp.door_states_and_flags & DSF_FILING_CABINET)   >> 6;
        lcp_bwlS     = (lcp.door_states_and_flags & DSF_DOG_BOWL_MASK)    >> 7;
        lcp_food          = lcp.food_supply;
        lcp_recP      = lcp.record_playing;
        lcp_tv               = lcp.tv_on;

        lcp_upal();
#ifdef FAITHFUL
        return 1;
#else
        return ok;
#endif
}
#else   /* STX: link #-8 -- the result goes through a second local,
           the open mode is a word, and the whole body hangs off the
           open test. */

short
lc_load()
{
        short   fhnd;
        short   ok;

        ok = 0;
        if ((fhnd = Fopen("hyber", 0)) >= 0) {
                ok = 1;
                fr_read(fhnd, 0x80L, &lcp);
                Fclose(fhnd);

                lcp_watr         = lcp.water_level;
                lcp_frdO     = lcp.door_states_and_flags & DSF_FRONT_DOOR;
                lcp_drsO        = (lcp.door_states_and_flags & DSF_DRESSER)          >> 4;
                lcp_cabO        = (lcp.door_states_and_flags & DSF_KITCHEN_CABINET)  >> 3;
                lcp_clsO    = (lcp.door_states_and_flags & DSF_CLOSET_DOOR)      >> 2;
                studyDrO     = (lcp.door_states_and_flags & DSF_STUDY_DOOR)       >> 1;
                lcp_toiO    = (lcp.door_states_and_flags & DSF_TOILET_DOOR)      >> 5;
                lcp_flcO = (lcp.door_states_and_flags & DSF_FILING_CABINET)   >> 6;
                lcp_bwlS     = (lcp.door_states_and_flags & DSF_DOG_BOWL_MASK)    >> 7;
                lcp_food          = lcp.food_supply;
                lcp_recP      = lcp.record_playing;
                lcp_tv               = lcp.tv_on;

                lcp_upal();
        }
        return ok;
}
#endif

/* lcp_std -> parts/lcp_std.c (STX puts it immediately after
   a_opcuc in the 0xdece object). */
#ifdef FAITHFUL
#include "parts/lcp_std.c"
#endif
