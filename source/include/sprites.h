/* sprites.h -- extern declarations for sprites.c. */

#ifndef SPRITES_H
#define SPRITES_H

/* Expanded-sprite buffer size in SHORTS (image or mask, one plane
   set) that expandFrame writes into bodyImage / bodyMask / headImage /
   headMask.  Both call sites pass width=2, height=21 and expandFrame
   writes 4 shorts per (x,y), so only 21*2*4 = 168 are ever touched --
   but the original declares a round 256 for all four buffers, and
   the memory layout depends on that size. */
#define LCP_BODY_DEST_WORDS     256

extern void updateBody();
extern void carryBehind();
extern void activateSprite();
extern void waitHeadTurn();
extern void hideResident();
extern void showResident();
extern void carryInFront();
extern void expandFrame();
extern void flipSprite();
extern void layoutSlots();
extern void updateHead();
extern void initSlots();
extern void deadHook();
extern void maskBody();
extern void maskHead();
extern void buildMasks();

#endif /* SPRITES_H */
