/*
 * afood.c -- meal, kitchen, feed-dog, snack handlers.
 *
 * All four share the kitchen-cabinet / fridge / stove workflow and
 * update food-supply / hunger / dog-bowl state at their tail.
 *
 * addr: a_eatm(), a_kitcc(),
 *       a_feedd(), a_gesff()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "adoors.h"
#include "afood.h"
#include "delivery.h"
#include "events.h"
#include "globals.h"
#include "health.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* a_eatm: pot from cabinet -> stove (cooking anim) -> chains into
   a_kitcc() to eat.  addr: a_eatm() */

/* a_eatm -> parts/a_eatm.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_eatm.c"
#endif

/* a_kitcc -> parts/a_kitcc.c (STX: 0xdece object, 0x11354). */
#ifdef FAITHFUL
#include "parts/a_kitcc.c"
#endif

/* a_feedd: fridge -> dog bowl -> fridge.
   value==0: standalone, open fridge first.
   value==1: Ctrl+D delivery path, package already in hand.
   addr: a_feedd() */

/* a_feedd -> parts/a_feedd.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_feedd.c"
#endif

/* a_gesff -> parts/a_gesff.c (STX places it between a_clocd
   and a_opecf in the 0xdece object). */
#ifdef FAITHFUL
#include "parts/a_gesff.c"
#endif
