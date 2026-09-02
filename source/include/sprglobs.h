/* sprglobs.h -- extern declarations for sprglobs.c. */

#ifndef SPRGLOBS_H
#define SPRGLOBS_H

#include "types.h"

/* Number of logical sprite-definition slots.  The port keeps one
   shared 60-entry table (matching the ROM); slot IDs are
   SPRITE_* enum values in include/enums.h.  The sprite-slot map
   `g_seslm[SPRITE_SLOTS]` picks which of the SPRITE_HW_SLOTS
   hardware slots each logical sprite lands on. */
#define SPRITE_SLOTS    60

/* Number of hardware sprite slots the compositor maintains.  Matches
   ROM.  Slot roles are named individually below. */
#define SPRITE_HW_SLOTS         8

/* Hardware-slot IDs (indices into g_sepim/g_sepms/g_seaim/g_seams/
   g_seacx/g_seacy/etc.).  Z-order runs low-to-high back-to-front:
   slot 0 draws first, slot 7 draws last on top of everything.

     0  DOG_BACK        dog when depth puts it behind the LCP
     1  BEHIND_OVERFLOW behind-LCP overlay, secondary
     2  BEHIND_PRIMARY  behind-LCP overlay, primary (compositor
                        assigns here first for SPRITE_BEHIND_LCP;
                        overflow falls back to slot 1)
     3  LCP_BODY        the character body sprite
     4  LCP_HEAD        the character head sprite (drawn over body)
     5  FRONT_OVERFLOW  in-front overlay, secondary
     6  FRONT_PRIMARY   in-front overlay, primary (compositor
                        assigns here first for SPRITE_IN_FRONT;
                        overflow falls back to slot 5)
     7  DOG_FRONT       dog when depth puts it in front of the LCP

   `HW_SLOT_NONE` is a sentinel value stored in g_seslm[] meaning
   "this logical sprite is not currently mapped to any hardware
   slot" (i.e. not drawn). */
#define HW_SLOT_DOG_BACK        0
#define HW_SLOT_BEHIND_OVERFLOW 1
#define HW_SLOT_BEHIND_PRIMARY  2
#define HW_SLOT_LCP_BODY        3
#define HW_SLOT_LCP_HEAD        4
#define HW_SLOT_FRONT_OVERFLOW  5
#define HW_SLOT_FRONT_PRIMARY   6
#define HW_SLOT_DOG_FRONT       7
#define HW_SLOT_NONE            9

/* Allocation size for the hardware-slot pending/active arrays below.
   Logical render slots are 0..7 (SPRITE_HW_SLOTS); HW_SLOT_NONE (9) is
   the "disabled" slot that sp_upds parks HIDDEN sprites in.  gameTick's
   carrying path writes g_sepex/g_sepey[g_seslm[g_lcieo]] every frame,
   and sp_ssco/sp_ss02 write g_seaim/g_seams/g_seach/g_seacw the same
   way -- so any of these arrays can be indexed at HW_SLOT_NONE when a
   carried sprite is momentarily hidden.  The ROM's 8-entry arrays
   tolerate the [9] write only because it overflows into the adjacent
   array (g_sepey[9] == g_seacw[1], a harmless short).  The port's
   linker instead places g_obtmt[0].fd_addr right after g_sepey, so the
   same stray write corrupts an MFDB bitmap pointer -> odd address ->
   TOS VDI bus error (~30 min in).  Allocating through HW_SLOT_NONE
   keeps every slot-9 write in-bounds and inert regardless of link
   order.  Real-slot loops/bounds checks still use SPRITE_HW_SLOTS (8),
   matching the ROM's `i < 8`. */
#ifdef FAITHFUL
#define SPRITE_HW_SLOTS_ALLOC   SPRITE_HW_SLOTS
#else
#define SPRITE_HW_SLOTS_ALLOC   (HW_SLOT_NONE + 1)
#endif

extern short lcp_st;
extern short lcp_face;
extern short g_lcyof;
extern short g_lcieo;
extern short g_lssh;
extern short dbg_hide;
extern short dog_x;
extern short dog_y;
extern short g_dtx;
extern short g_dty;
extern short g_dyx;
extern short g_dyy;
extern short g_dwanc;
extern short g_dsid;
extern short dg_stair;
extern short dg_init;
extern short g_sepef[];
extern short* g_sepim[];
extern short* g_sepms[];
extern short g_sepex[];
extern short g_sepey[];
extern short g_sepeh[];
extern short g_sepew[];
extern short* g_seaim[];
extern short* g_seams[];
extern short g_seacx[];
extern short g_seacy[];
extern short g_seach[];
extern short g_seacw[];
extern short* g_sedim[];
extern short* g_sedms[];
extern short g_sedeh[];
extern short g_sedew[];
extern short g_selaf[];
extern short g_seslm[];
extern short body_frT[];
extern short cy_frT[];
extern short body_yof[];
#ifdef FAITHFUL
extern short* body_ptr;
#else
extern unsigned char body_ptr[][168];   /* LCP_BODY_FRAME_SIZE */
#endif
#ifdef FAITHFUL
extern short* body_shp;
extern short bshdbuf[];
#else
extern unsigned char body_shp[][84];    /* LCP_BODY_SHAPE_SIZE */
#endif
extern short g_lsimg[];
extern short g_lsmas[];
extern short g_dwanf[];
extern short g_dfimb[];
extern short g_dfmab[];
extern short flr_by[];
extern short flr_cy[];
extern short stair_wp[];
extern short subAniC;
extern short g_hsbuf[];
extern short g_hsmas[];
extern short g_hsmif;
extern unsigned char pex_ptr[][168];    /* LCP_BODY_FRAME_SIZE */
extern unsigned char hd_shp[][84];      /* LCP_BODY_SHAPE_SIZE */
extern short g_hadec;
extern short mood_hfo[];
extern short hd_xoff[];
extern short hd_hgt[];
extern short hd_dang[];
extern short hd_mvd[];
extern short hd_tilt[];
extern short g_wyx;
extern short g_wyy;
extern short lcp_stR;
extern BOOL16 fs_trg;
extern short g_hastl;
extern short stair_ty;
extern short stair_by;

#endif /* SPRGLOBS_H */
