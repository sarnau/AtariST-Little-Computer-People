/*
 * sprite_render.c -- masked-blit sprite renderer.
 *
 * sprite_draw is called from screen_render_8hz for each of the 8 hardware
 * sprite slots.  It uses the standard Atari ST two-pass masked blit:
 *
 *   Pass 1  vdi_copy_rect(NOTS_AND_D, mask, screen)
 *     Punches a transparent hole in the background where the sprite
 *     will go.  The mask has 1-bits where the sprite is opaque, so
 *     inverting it and AND'ing clears the destination pixels only
 *     under the opaque part of the sprite.
 *
 *   Pass 2  vdi_copy_rect(S_XOR_D, image, screen)
 *     XOR the sprite image onto the cleared area.  Since we just wrote
 *     zeros there, XOR effectively becomes a copy.
 *
 * sprite_init_MFDB is a tiny helper that fills in the sprite's MFDB
 * descriptor from raw address + width/height so the VDI wrappers
 * can consume it.
 *
 * addr: sprite_draw(), sprite_init_MFDB()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

extern void     vdi_copy_rect();

/* sprite_init_MFDB: populate an MFDB with the ST low-res format
   defaults (device-specific, 4 bitplanes).  The first parameter is
   ignored -- the 1985 code had it as `nplanes` but hardcoded to 4
   inside; preserved for signature fidelity.
   addr: sprite_init_MFDB() */

void
sprite_init_MFDB(unused, mfdb, addr, width, height)
long    unused;
MFDB *  mfdb;
void *  addr;
short   width;
short   height;
{
        (void) unused;
        mfdb->fd_addr    = addr;
        mfdb->fd_w       = width;
        mfdb->fd_h       = height;
        mfdb->fd_wdwidth = width / 16;
        mfdb->fd_stand   = 0;
        mfdb->fd_nplanes = 4;
}

/* sprite_draw: composite a single sprite slot onto screen_mfdb.
   addr: sprite_draw() */

void
sprite_draw(index)
short   index;
{
        short   x1;
        short   y1;
        short   w;
        short   h;

        x1 = sprite_pending_x[index];
        y1 = sprite_pending_y[index];
        w  = sprite_active_width[index];
        h  = sprite_active_height[index];

        sprite_init_MFDB(0L, &sprite_mfdb_image[index],
                         sprite_active_image[index], w, h);
        sprite_init_MFDB(0L, &sprite_mfdb_mask[index],
                         sprite_active_mask[index],  w, h);

        vdi_copy_rect(vdihandle, NOTS_AND_D,
                      &sprite_mfdb_mask[index], &screen_mfdb,
                      0, 0, w - 1, h - 1,
                      x1, y1, x1 + w - 1, y1 + h - 1);

        vdi_copy_rect(vdihandle, S_XOR_D,
                      &sprite_mfdb_image[index], &screen_mfdb,
                      0, 0, w - 1, h - 1,
                      x1, y1, x1 + w - 1, y1 + h - 1);
}
