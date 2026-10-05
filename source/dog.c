/*
 * dog.c -- dog movement, animation, and sprite update.
 *
 * The dog is an autonomous agent: it wanders, approaches the food bowl
 * when hungry, and comes when called.  Movement runs at 8 Hz driven by
 * moveDog() from the frame loop; sprite state is pushed
 * out to hardware slots 0 or 7 (behind/in-front of LCP by Y depth) via
 * setDogSprite().
 */

#include "types.h"
#include "enums.h"
#include "protos.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprites.h"
/* spriteBitmap/spriteMask are filled by defineSprite and used by activateSprite/carryBehind/
   carryInFront as well as the dog path. */

/* Place the dog at its startup spot (bottom floor near the food bowl)
   and clear the dog sprite slots with sprite id 0.  The dog becomes
   visible on the next renderFrame tick once moveDog picks a target and
   calls setDogSprite again with a walk-cycle sprite id from dogWalkSprites. */
void
placeDog()
{
        dogX = 100;
        dogY = 195;
        setDogSprite(0, 1, NO);
}

/* setDogSprite (with flipSprite) lives in alerts.c, which shares its object:
   it pushes the dog frame into hardware slots 0 (behind) or 7
   (in-front) depending on layerPosition, mirroring horizontally via
   flipSprite if needed.  dogHidden suppresses the push while the dog
   hasn't been placed in the world yet. */

