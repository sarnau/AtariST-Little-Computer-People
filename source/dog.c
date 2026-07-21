/*
 * dog.c -- dog movement, animation, and sprite update.
 *
 * The dog is an autonomous agent: it wanders, approaches the food bowl
 * when hungry, and comes when called.  Movement runs at 8 Hz driven by
 * dg_mvAni() from the frame loop; sprite state is pushed
 * out to hardware slots 0 or 7 (behind/in-front of LCP by Y depth) via
 * sp_spud().
 *
 * addr: dg_mvAni(), sp_spud()
 */

#include "types.h"
#include "enums.h"
#include "dog.h"
#include "globals.h"
#include "movement.h"
#include "sprglobs.h"
#include "sprites.h"
#include "walk.h"
/* g_sedim/g_sedms = PTR_ARRAY_0005a156/0x54016; populated by sp_reglp
   and used by sp_sprs/sp_ssco/sp_ss02 as well as the dog path. */

/* dog_init_position: Ghidra 0x??.  Place the dog at its startup spot
   (bottom floor near the food bowl) and NULL-out the two dog sprite
   slots via sp_spud(SPRITE_UNUSED_0=-1).  The dog becomes visible on
   the very next sc_ren8 tick once dg_mvAni picks a target
   and calls sp_spud again with a valid walk-cycle sprite id from
   g_dwanf. */
void
dg_ipos()
{
        dog_x = 100;
        dog_y = 195;
        sp_spud(-1, 1, NO);
}

/* dg_mvAni: 8 Hz movement + walk-cycle advance.  If the dog
   has no target the routine is a no-op.  Handles flat walking (X/Y
   equal steps to waypoint) and stair navigation (staircase_waypoint_
   coords[] gate for the two staircase entrances).  Layer depth is
   1 (in-front) when the dog is below the resident, -1 (behind) when
   above -- newspaper reading forces in-front so the dog doesn't disappear
   behind the paper.

   addr: dg_mvAni() */

void
dg_mvAni()
{
        short   floor_num;
        BOOL16  h_flip;
        short   depth_layer;
        short   x_distance;
        short   next_x;

        g_dwanc = g_dwanc + 1;
        if (g_dwanc > 7)
                g_dwanc = 0;

        if (g_dtx == 0 && g_dty == 0)
                return;

        if (lcp_y < (short) (dog_y + 5))
                depth_layer = 1;
        else
                depth_layer = -1;
        if (lcp_st == STATE_READ_PAPER_HOLD ||
            lcp_st == STATE_READ_PAPER_TURN_PAGE)
                depth_layer = 1;

        if (g_dyx == 0 && g_dyy == 0)
                dg_wkPth();

        /* Exit stair-mode when reaching a floor boundary. */
        if (dg_stair != NO) {
                floor_num = getFlrY(g_dyy);
                if (dog_y <= flr_by[floor_num - 1]) {
                        if (floor_num == 3)
                                dg_stair = NO;
                        else if (stair_wp[(floor_num - 1) * 2 + 1] <= dog_y)
                                dg_stair = NO;
                }
        }

        if (dog_x == g_dyx && dog_y == g_dyy) {
                if (dog_x == g_dtx && dog_y == g_dty) {
                        g_dtx = 0;
                        g_dty = 0;
                        g_dyx = 0;
                        g_dyy = 0;
                        g_dsid = SPRITE_DOG_LAY_DOWN;
                        sp_spud(SPRITE_DOG_LAY_DOWN,
                                              depth_layer, NO);
                        return;
                }
                dg_wkPth();
        }

        g_dsid = g_dwanf[g_dwanc];
        h_flip = NO;

        if (dg_stair == NO) {
                if (dog_x < g_dyx) {
                        h_flip = NO;
                        dog_x = dog_x + 1;
                } else if (g_dyx < dog_x) {
                        h_flip = YES;
                        dog_x = dog_x - 1;
                }
                x_distance = (dog_x < g_dyx)
                             ? (g_dyx - dog_x)
                             : (dog_x - g_dyx);
                if (x_distance < 8) {
                        if (dog_y < g_dyy)
                                dog_y = dog_y + 1;
                        else if (g_dyy < dog_y)
                                dog_y = dog_y - 1;
                } else {
                        floor_num = getFlrY(dog_y);
                        if (dog_y < flr_cy[floor_num - 1])
                                dog_y = dog_y + 1;
                        floor_num = getFlrY(dog_y);
                        if (flr_cy[floor_num - 1] < dog_y)
                                dog_y = dog_y - 1;
                }
        }

        next_x = dog_x;

        if (dg_stair != NO) {
                /* Stair traversal: fixed X/Y patches at 0x62, 0x64,
                   0x9f, 0xa1 anchor the sprite to the stair-edge tiles.
                   Between anchors we drift by one pixel per frame in
                   the direction g_dyy indicates. */
                if (g_dyy < dog_y) {
                        /* Going up */
                        if (dog_y == 0xa1) {
                                h_flip = YES;
                                dog_y = 0x9f;
                                next_x = dog_x - 0x11;
                        } else if (dog_y == 100) {
                                h_flip = NO;
                                dog_y = 0x62;
                                next_x = dog_x + 3;
                        } else if (dog_y < 162 &&
                                   (dog_y < 101 || 139 < dog_y)) {
                                if (dog_y < 100) {
                                        h_flip = NO;
                                        dog_y = dog_y - 1;
                                        if (g_dsid != SPRITE_DOG_WLK_R9) {
                                                next_x = dog_x + 1;
                                                if (next_x != g_dyx)
                                                        next_x = dog_x + 2;
                                        }
                                } else if (dog_y < 0xa1) {
                                        h_flip = YES;
                                        dog_y = dog_y - 1;
                                        if (g_dsid != SPRITE_DOG_WLK_R9) {
                                                next_x = dog_x - 1;
                                                if (next_x != g_dyx)
                                                        next_x = dog_x - 2;
                                        }
                                }
                        } else {
                                h_flip = NO;
                                dog_y = dog_y - 2;
                        }
                } else if (dog_y < g_dyy) {
                        /* Going down */
                        if (dog_y == 0xa1) {
                                h_flip = NO;
                                dog_y = 165;
                                next_x = dog_x + 1;
                        } else if (dog_y == 100) {
                                h_flip = NO;
                                dog_y = 102;
                                next_x = dog_x + 3;
                        } else if (dog_y < 162 &&
                                   (dog_y < 101 || 131 < dog_y)) {
                                if (dog_y < 100) {
                                        h_flip = YES;
                                        dog_y = dog_y + 1;
                                        if (g_dsid != SPRITE_DOG_WLK_R9) {
                                                next_x = dog_x - 1;
                                                if (next_x != g_dyx)
                                                        next_x = dog_x - 2;
                                        }
                                } else if (dog_y < 161) {
                                        h_flip = NO;
                                        dog_y = dog_y + 1;
                                        if (g_dsid != SPRITE_DOG_WLK_R9) {
                                                next_x = dog_x + 1;
                                                if (next_x != g_dyx)
                                                        next_x = dog_x + 2;
                                        }
                                }
                        } else {
                                h_flip = NO;
                                dog_y = dog_y + 1;
                        }
                }
        }

        dog_x = next_x;
        sp_spud(g_dsid, depth_layer, h_flip);
}

/* sp_spud: push the dog frame into hardware slots 0
   (behind) or 7 (in-front) depending on layerPosition; mirror horizontally
   via sp_flih if needed.  dg_init suppresses the
   push while the dog hasn't been placed in the world yet.

   addr: sp_spud() */

void
sp_spud(g_seid, layer_p, flipH2)
short   g_seid;
short   layer_p;
BOOL16  flipH2;
{
        g_seaim[HW_SLOT_DOG_BACK] = NULL;
        g_seaim[HW_SLOT_DOG_FRONT] = NULL;

        if (g_seid < 0 || dg_init != NO)
                return;

        if (flipH2 != NO) {
                sp_flih(g_sedim[g_seid],
                                       (unsigned short *) g_dfimb,
                                       15, 2);
                sp_flih(g_sedms[g_seid],
                                       (unsigned short *) g_dfmab,
                                       15, 2);
        }

        g_seach[HW_SLOT_DOG_BACK] = g_sedeh[SPRITE_DOG_LAY_DOWN];
        g_seach[HW_SLOT_DOG_FRONT] = g_sedeh[SPRITE_DOG_LAY_DOWN];
        g_seacw[HW_SLOT_DOG_BACK]  = g_sedew[SPRITE_DOG_LAY_DOWN];
        g_seacw[HW_SLOT_DOG_FRONT]  = g_sedew[SPRITE_DOG_LAY_DOWN];
        g_sepex[HW_SLOT_DOG_BACK] = dog_x;
        g_sepex[HW_SLOT_DOG_FRONT] = dog_x;
        g_sepey[HW_SLOT_DOG_BACK] = dog_y - 17;
        g_sepey[HW_SLOT_DOG_FRONT] = dog_y - 17;

        if (flipH2 == NO) {
                g_seams[HW_SLOT_DOG_BACK] = g_sedms[g_seid];
                g_seams[HW_SLOT_DOG_FRONT] = g_sedms[g_seid];
                if (layer_p == 1)
                        g_seaim[HW_SLOT_DOG_FRONT] = g_sedim[g_seid];
                else
                        g_seaim[HW_SLOT_DOG_BACK] = g_sedim[g_seid];
        } else {
                g_seams[HW_SLOT_DOG_BACK] = g_dfmab;
                g_seams[HW_SLOT_DOG_FRONT] = g_dfmab;
                if (layer_p == 1)
                        g_seaim[HW_SLOT_DOG_FRONT] = g_dfimb;
                else
                        g_seaim[HW_SLOT_DOG_BACK] = g_dfimb;
        }
}
