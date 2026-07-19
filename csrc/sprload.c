/*
 * sprload.c -- sprite pointer registration pass for the dog pipeline.
 *
 * Ghidra's main() walks the SPRITES file twice: first via
 * asset_load_sprites_table (al_lost -> per-record MFDBs in g_setmt)
 * and again via a second loop that calls spritedata_create_with_mask
 * indexed by sprite_file_index_table[i] so the resulting pointers land
 * at the correct g_sedim[sprite_id] slot.  This file
 * hosts that second pass (sp_reglp) plus the mask buffer it writes to.
 *
 * addr: spritedata_create_with_mask() @ Ghidra 0x15BDC,
 *       spritedata_generate_mask_from_color() @ Ghidra (inlined near
 *       0x15BDC), plus the driver loop from main() at
 *       0x1579c..0x15828.
 */

#include "types.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprload.h"


/* sp_fidx (Ghidra sprite_file_index_table @ 0x2A084, 50 shorts):
   file-record index -> sprite_id slot to store its pointers in. */
short   sp_fidx[50] = {
        12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27,
        28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43,
        44,  9, 45, 46, 47, 48, 49,  3,
         4, 50,  7,  6, 51, 52, 53, 54,
         8, 55
};

/* sp_mbuf: 14 KB shared mask buffer.  sp_regs writes a generated
   transparency mask here parallel to the sprite's image bytes in
   spr_file.  g_sedms[id] then references a slice
   here. */
unsigned char   sp_mbuf[14000];

/* sp_genma: Ghidra spritedata_generate_mask_from_color.  For each
   16-pixel word group (4 interleaved bitplane words), OR the planes
   -- any non-colour-0 pixel becomes an opaque mask bit -- then
   broadcast the result to all 4 mask planes so the mask has the same
   MFDB stride as the image. */
static void
sp_genma(imgPtr, maskPtr, width, height)
unsigned short *        imgPtr;
unsigned short *        maskPtr;
unsigned short          width;
unsigned short          height;
{
        unsigned short  index;
        unsigned short  m;
        unsigned short  n;

        n = ((unsigned short) ((width >> 2) * height)) >> 2;
        for (index = 0; index < n; index = index + 1) {
                m = imgPtr[3] | imgPtr[2] | imgPtr[1] | imgPtr[0];
                imgPtr    = imgPtr + 4;
                maskPtr[0] = m;
                maskPtr[1] = m;
                maskPtr[2] = m;
                maskPtr[3] = m;
                maskPtr   = maskPtr + 4;
        }
}

/* sp_regs: Ghidra spritedata_create_with_mask.  Store per-sprite
   pointers and dimensions at slot spriteID, then auto-generate the
   1-bit mask into maskPtr. */
static void
sp_regs(spriteID, imgPtr, maskPtr, height, width)
short                   spriteID;
unsigned short *        imgPtr;
unsigned short *        maskPtr;
short                   height;
short                   width;
{
        g_sedim[spriteID] = (short *) imgPtr;
        g_sedms[spriteID]   = (short *) maskPtr;
        g_sedeh[spriteID]             = height;
        g_sedew[spriteID]             = width;
        sp_genma(imgPtr, maskPtr,
                 (unsigned short) width,
                 (unsigned short) height);
}

/* sp_reglp: second-pass registration loop over spr_file.
   Mirrors Ghidra main() at 0x1579c..0x15828 exactly.  Called once
   from main() right after al_lost. */

void
sp_reglp()
{
        long    offset;
        long    mask_offset;
        short   count;
        short   width;
        short   height;
        long    words_per_row;
        long    record_bytes;
        short   id;

        offset      = 0;
        mask_offset = 0;
        count       = 0;
        while (offset < 14000L && count < 50) {
                height = ((short) spr_file[offset]     << 8) |
                                  spr_file[offset + 1];
                width  = ((short) spr_file[offset + 2] << 8) |
                                  spr_file[offset + 3];
                if (height == 0 || width == 0)
                        break;
                offset = offset + 4;

                words_per_row = (width + 15) / 16;
                record_bytes  = (long) height * words_per_row * 4 * 2;

                id = sp_fidx[count];
                sp_regs(id,
                        (unsigned short *) (spr_file + offset),
                        (unsigned short *) (sp_mbuf       + mask_offset),
                        height,
                        (short) (words_per_row * 16));

                offset      = offset      + record_bytes;
                mask_offset = mask_offset + record_bytes;
                count       = count + 1;
        }
}
