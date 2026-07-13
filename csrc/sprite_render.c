/*
 * sprite_render.c -- masked-blit sprite renderer.
 *
 * sp_draw is called from sc_ren8 for each of the 8 hardware
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
 * sp_iniM is a tiny helper that fills in the sprite's MFDB
 * descriptor from raw address + width/height so the VDI wrappers
 * can consume it.
 *
 * addr: sp_draw(), sp_iniM()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    vdihandle;
extern MFDB     g_srmfd;
extern MFDB     g_semfi[];
extern MFDB     g_semfm[];
extern short    g_sepex[];
extern short    g_sepey[];
extern short *  sprite_active_image[];
extern short *  sprite_active_mask[];
extern short    g_seach[];
extern short    g_seacw[];
extern void     vdi_copy_rect();

/* sp_iniM: populate an MFDB with the ST low-res format
   defaults (device-specific, 4 bitplanes).  The first parameter is
   ignored -- the 1985 code had it as `nplanes` but hardcoded to 4
   inside; preserved for signature fidelity.
   addr: sp_iniM() */

void
sp_iniM(unused, mfdb, addr, width, height)
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

/* sp_draw: composite a single sprite slot onto g_srmfd.
   addr: sp_draw() */

void
sp_draw(index)
short   index;
{
        short   x1;
        short   y1;
        short   w;
        short   h;

        x1 = g_sepex[index];
        y1 = g_sepey[index];
        w  = g_seacw[index];
        h  = g_seach[index];

        sp_iniM(0L, &g_semfi[index],
                         sprite_active_image[index], w, h);
        sp_iniM(0L, &g_semfm[index],
                         sprite_active_mask[index],  w, h);

        vdi_copy_rect(vdihandle, NOTS_AND_D,
                      &g_semfm[index], &g_srmfd,
                      0, 0, w - 1, h - 1,
                      x1, y1, x1 + w - 1, y1 + h - 1);

        vdi_copy_rect(vdihandle, S_XOR_D,
                      &g_semfi[index], &g_srmfd,
                      0, 0, w - 1, h - 1,
                      x1, y1, x1 + w - 1, y1 + h - 1);
}
