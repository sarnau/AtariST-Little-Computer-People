/*
 * stx_u2.c -- unity unit for the largest game object: rendering,
 * sprites, the action bodies, the TV, health and the letter writer.
 * See stx_u1.c for the rationale.
 *
 * The include list below IS the object's function order and must not
 * change.  The original did not group this object by source file --
 * the leisure actions alone are spread across most of it -- so the
 * port has no action .c files left at all: every body lives in parts/
 * and this list is the order.
 *
 * alcyon_build.sh skips the constituents listed in
 * tools/stx_units.txt while building this file.
 */


/* Headers first: they emit no code, so the object layout is
   unaffected, but the parts/ bodies below need them in scope. */
#include "types.h"
#include <osbind.h>       /* the sc_sdt* parts use Setscreen/Logbase */
#include <stdio.h>        /* sprintf, for the letter writer */
#ifdef HOST
#include "hostgem.h"
#else
#include <vdibind.h>
#endif
#include "structs.h"
#include "enums.h"
#include "obdefs1.h"
#include "globals.h"
#include "protos.h"
#include "rnd.h"
#include "calendar.h"
#include "events.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tables.h"
#include "vdiown.h"

#include "dat_u2.c"


#include "parts/moffmon.c"     /* hideMouse, showMouse */
#include "parts/leaveGameTable.c"
#include "parts/rejoinTable.c"
#include "parts/activateSprite.c"
#include "render.c"            /* drawObject, drawFoodCab */
#include "parts/beginDraw.c"
#include "parts/endDraw.c"
#include "parts/checkFrontDoor.c"
#include "parts/hideResident.c"
#include "parts/showResident.c"
#include "parts/moveInScene.c"
#include "parts/wakeFromAlarm.c"
#include "parts/cleanUp.c"
#include "parts/closeBedCloset.c"
#include "parts/goToFridge.c"
#include "parts/putInFridge.c"
#include "parts/openDresser.c"
#include "parts/openKitchenCab.c"
#include "parts/changeClothes.c"
#include "parts/enterStudy.c"
#include "parts/studyVisit.c"
#include "parts/sayHello.c"
/* Order here is tvc, spe, hnd, grt -- the sound ids in the wrappers
   settle it. */
#include "parts/sfxTvClick.c"
#include "parts/sfxSpeech.c"
#include "parts/sfxHeadNod.c"
#include "parts/sfxGreeting.c"
#include "parts/playOrgan.c"
#include "parts/feedDog.c"
#include "parts/walkToFrontDoor.c"
#include "parts/cookMeal.c"
#include "parts/openFrontDoor.c"
#include "parts/useToilet.c"
#include "parts/closeToiletDoor.c"
#include "parts/takeShower.c"
#include "parts/petDog.c"
#include "parts/callDog.c"
#include "parts/rummageCabinet.c"
#include "parts/tidyHouse.c"
#include "parts/answerPhone.c"
#include "parts/sitWithDog.c"
#include "parts/dogFoodDelivery.c"
#include "parts/recordDelivery.c"
#include "parts/lightFire.c"
#include "parts/foodDelivery.c"
#include "parts/bookDelivery.c"
#include "parts/eatFromCabinet.c"
#include "parts/brushTeeth.c"
#include "agames.c"            /* playGame */
#include "parts/closeFilingCab.c"
#include "parts/peekAround.c"
#include "parts/nodOk.c"
#include "parts/nodHead.c"
#include "parts/carryBehind.c"
#include "parts/carryInFront.c"
#include "parts/drinkWater.c"
#include "parts/updateWaterTank.c"
#include "parts/washAtSink.c"
#include "parts/paceNervously.c"
#include "parts/idleShrug.c"
#include "parts/dozeOff.c"
#include "parts/danceToMusic.c"
#include "parts/yawnAndStretch.c"
#include "parts/washHands.c"
#include "parts/getInOutOfBed.c"
#include "parts/recordStoop.c"
#include "parts/tvStoop.c"
#include "parts/exercise.c"
#include "parts/readNewspaper.c"
#include "parts/useComputer.c"
#include "parts/tvClearAnim.c"
#include "tvanim.c"            /* tvBounce */
#include "parts/tvPattern.c"
#include "parts/calcWeekday.c"
#include "parts/drawClock.c"
#include "sim.c"               /* simStep */
#include "health.c"            /* fallSick, startRecovery, setSkinColor */
#include "parts/morningRoutine.c"
#include "parts/nightRoutine.c"
#include "parts/daysInMonth.c"
#include "parts/redrawHands.c"
#include "parts/drawHands.c"
#include "parts/drawLine.c"
#include "parts/drawPixel.c"
#include "parts/playRecord.c"
#include "parts/stopRecord.c"
#include "parts/animRecPlayer.c"
#include "parts/toggleTv.c"
#include "parts/tvOn.c"
#include "parts/tvOff.c"
#include "parts/tvNoise.c"
#include "parts/drawTvPicture.c"
#include "dat_u2b.c"
#include "parts/writeLetter.c"
#include "parts/typeString.c"
#include "parts/typeChar.c"
#include "parts/typeKeySound.c"
#include "parts/sfxClick.c"
#include "walk.c"              /* walkToTarget */
#include "parts/saveFile.c"
#include "parts/ensureFile.c"
#include "parts/writeErrorAlert.c"

