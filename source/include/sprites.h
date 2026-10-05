/* sprites.h -- extern declarations for sprites.c. */

#ifndef SPRITES_H
#define SPRITES_H

/* Source-packed byte size of one 16x21 LCP body/head sprite frame:
   21 rows * 4 bytes per row * 2 bit-planes.  Applies to every frame in
   BODY.LCP and PEn.LCP, and to the source stride used by expandFrame and
   maskBody / maskHead when walking the raw sprite table. */
#define LCP_BODY_FRAME_SIZE     (21 * 4 * 2)

/* Dilated body/head shape stride: 21 rows * 4 bytes per row.
   Half of LCP_BODY_FRAME_SIZE because the shape buffers (bodyShapes,
   headShapes) collapse the 2-plane source into a single-plane
   silhouette used by maskBody / maskHead. */
#define LCP_BODY_SHAPE_SIZE     (21 * 4)

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
