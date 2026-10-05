/* sprload.h -- extern declarations for the sprite-loading data and
   functions (genMaskBuf lives in sprglobs.c). */

#ifndef SPRLOAD_H
#define SPRLOAD_H

extern short spriteFileId[];
extern unsigned char genMaskBuf[];


extern void makeMask();
extern void defineSprite();

#endif /* SPRLOAD_H */
