/*
 * stx_u1.c -- unity translation unit for the first of the original's
 * game-code objects.
 *
 * The original's game code is ~7 large objects, where the port keeps
 * many small source files.  as68 shortens a call only when the callee
 * is in the SAME assembly unit, so reproducing the original's call
 * shapes requires reproducing its object partition -- the default
 * build therefore compiles these sources as one unit.
 *
 * The ORDER of the #include lines below is the object's function (and
 * data) order and must not change.
 *
 * alcyon_build.sh skips the constituents listed in
 * tools/stx_units.txt while building this file.
 */


/* Headers first: they emit no code, so the object layout is
   unaffected, but the parts/ bodies below need them in scope. */
#include "types.h"
#include "structs.h"
#include "enums.h"
#include "obdefs1.h"
#ifdef HOST
#include "hostgem.h"
#else
#include <vdibind.h>
#endif
#include <osbind.h>
#include "globals.h"
#include "protos.h"
#include "rnd.h"
#include "calendar.h"
#include "events.h"
#include "sprglobs.h"
#include "sprites.h"

#include "dat_u1.c"


#include "parts/countSongs.c"
#include "parts/makeMask.c"
/* moveDog is followed directly by walk.c's dogNextWaypt. */
#include "parts/moveDog.c"
#include "parts/dogNextWaypt.c"
/* walk.c straddles two objects: walkStep and playFootstep live here
   with floorOfY, while walkToTarget and friends are in stx_u2.c. */
#include "parts/walkStep.c"
#include "parts/playFootstep.c"
#include "parts/nextWaypoint.c"
#include "parts/floorOfY.c"
/* assets.c straddles: the two asset loaders are in this object,
   right after floorOfY.  They need the trap bindings. */
#include "parts/loadObjects.c"
#include "parts/loadSprites.c"
#include "parts/decodeScn.c"
#include "parts/unpackFile.c"
#include "sprload.h"
#include "tables.h"
#include "tick_tables.h"
#include "dat_u1d.c"
#include "parts/main.c"
#include "dog.c"
/* save.c straddles too: loadSavedGame and defineSprite sit between placeDog and
   gameLoop. */
#include "parts/loadSavedGame.c"
#include "parts/defineSprite.c"
/* main.c straddles: gameLoop is in this object, between defineSprite
   and runEvent. */
#include "parts/gameLoop.c"
#include "parts/chooseAction.c"
#include "ai.c"
#include "actions.c"
/* runEvent's and runAction's switch jump tables land in the data segment
   right here, so the globals that follow them come after this point,
   not with the rest at the top. */
#include "dat_u1b.c"
/* pickIdleAction sits between runAction and posToXY. */
#include "airandom.c"
#include "movement.c"
#include "parts/blitRect.c"
/* letload.c straddles: loadLetterText is in this object, just ahead of
   copyScreen.  gfx_prim.c straddles too: copyScreen, initHouseBuf and sprites.c's
   initMfdb are in this object. */
#include "parts/loadFrameFile.c"
#include "parts/loadLetterText.c"
#include "parts/copyScreen.c"
#include "parts/initHouseBuf.c"
#include "parts/initMfdb.c"
/* vdiInit is split in two: the opener, and the attribute/clear half
   it calls, which must follow it directly. */
#include "parts/vdiInit.c"
#include "parts/vdiClear.c"
#include "parts/initAes.c"
#include "parts/initMirror.c"
#include "parts/buildMirrorTable.c"
/* fillPanel is in this object, not stx_u2's where render.c's other
   functions live. */
#include "parts/fillPanel.c"
#include "parts/getKey.c"
/* getKey's jump table lands in the data segment here, so the last
   globals of this unit are declared behind it. */
#include "dat_u1c.c"
/* The bare Random() wrapper, just past getKey. */
#include "parts/rnd.c"
#include "parts/rollResident.c"
#include "parts/resetDailyFlags.c"
#include "renderx.c"
/* titleScreen is a real interactive title screen. */
#include "parts/titleScreen.c"
#include "parts/enterField.c"
#include "parts/eraseChar.c"
/* save.c's file helpers come near the end of this object. */
#include "parts/openFile.c"
#include "parts/readFile.c"
/* outOfMemory closes the object. */
#include "parts/outOfMemory.c"

