/*
 * stx_u3.c -- unity unit for the sprite object: writeErrorAlert, the sprite
 * engine, the compositor tick, keyboard dispatch and the parser.  See
 * stx_u1.c for the mechanism.
 *
 * The include list below IS the object's function order and must not
 * change.
 */

/* printChar needs obdefs.h (MD_TRANS/MD_REPLACE). */
#include "obdefs1.h"
#include "sprglobs.h"

/* handleKey/queueEvent need the globals and prototypes their own files
   pull in. */
#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "protos.h"
#include "events.h"
#include "vocab.h"

#include "dat_anim.c"

#include "alerts.c"
#include "sprites.c"
#include "parts/renderFrame.c"
/* waitHeadTurn immediately precedes gameTick. */
#include "parts/waitHeadTurn.c"
#include "tick.c"
/* Everything below follows gameTick, in this order. */
#include "parts/handleKey.c"
#include "parts/playDoorbell.c"
#include "parts/queueEvent.c"
#include "parts/nextEvent.c"
#include "parts/drawSlot.c"
#include "parts/deadHook.c"
#include "parts/updateBody.c"
#include "parts/stepHead.c"
#include "parts/updateHead.c"
/* The mask builders sit here, past updateHead, not with
   layoutSlots/initSlots in sprites.c at the front. */
#include "parts/buildMasks.c"
#include "parts/maskBody.c"
#include "parts/maskHead.c"
#include "parts/expandFrame.c"
/* paperRow must directly follow scrollStrip so the call between them
   stays a short branch. */
#include "parts/scrollStrip.c"
#include "parts/paperRow.c"
#include "parts/stripeRow.c" /* stripeRow, blackRow */
#include "parts/printString.c"
#include "parts/printChar.c"
#include "parts/matchCommand.c"
#include "parts/nextWord.c"
#include "parts/lookupWord.c"
#include "parts/submitCommand.c"
#include "parts/parseNumber.c"
#include "parts/toUpper.c"

#include "dat_parser.c"
