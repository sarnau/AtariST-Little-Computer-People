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
/* lc_load -> parts/lc_load.c (STX: 0x5ac8, in the 0x400c object
   ahead of gameLoop -- stx_u1.c includes it there). */
#ifdef FAITHFUL
#include "parts/lc_load.c"
#endif

/* lcp_std -> parts/lcp_std.c (STX puts it immediately after
   a_opcuc in the 0xdece object). */
#ifdef FAITHFUL
#include "parts/lcp_std.c"
#endif
