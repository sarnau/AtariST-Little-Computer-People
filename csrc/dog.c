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
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    lcp_y;
extern short    getFlrY();
extern short    lcp_st;
extern short    dog_x;
extern short    dog_y;
extern short    g_dtx;
extern short    g_dty;
extern short    g_dyx;
extern short    g_dyy;
extern short    g_dwanc;
extern short    g_dsid;
extern short    dg_stair;
extern short    dg_init;
extern short    g_sepex[];
extern short    g_sepey[];
extern short *  g_seaim[];
extern short *  g_seams[];
extern short    g_seach[];
extern short    g_seacw[];
extern short    g_sedeh[];
extern short    g_sedew[];
extern short    g_dwanf[];
/* g_sedim/g_sedms = PTR_ARRAY_0005a156/0x54016; populated by sp_reglp
   and used by sp_sprs/sp_ssco/sp_ss02 as well as the dog path. */
extern short *  g_sedim[];
extern short *  g_sedms[];
extern short    g_dfimb[];
extern short    g_dfmab[];
extern short    flr_by[];
extern short    flr_cy[];
extern short    stair_wp[];
extern void     dg_wkPth();
extern void     sp_flih();
extern void     sp_spud();

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

        /* Waypoint reached? */
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
                                        if (g_dsid != SPRITE_DOG_WALK_RIGHT_9) {
                                                next_x = dog_x + 1;
                                                if (next_x != g_dyx)
                                                        next_x = dog_x + 2;
                                        }
                                } else if (dog_y < 0xa1) {
                                        h_flip = YES;
                                        dog_y = dog_y - 1;
                                        if (g_dsid != SPRITE_DOG_WALK_RIGHT_9) {
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
                                        if (g_dsid != SPRITE_DOG_WALK_RIGHT_9) {
                                                next_x = dog_x - 1;
                                                if (next_x != g_dyx)
                                                        next_x = dog_x - 2;
                                        }
                                } else if (dog_y < 161) {
                                        h_flip = NO;
                                        dog_y = dog_y + 1;
                                        if (g_dsid != SPRITE_DOG_WALK_RIGHT_9) {
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
        g_seaim[0] = NULL;
        g_seaim[7] = NULL;

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

        g_seach[0] = g_sedeh[0x21];
        g_seach[7] = g_sedeh[0x21];
        g_seacw[0]  = g_sedew[0x21];
        g_seacw[7]  = g_sedew[0x21];
        g_sepex[0] = dog_x;
        g_sepex[7] = dog_x;
        g_sepey[0] = dog_y - 17;
        g_sepey[7] = dog_y - 17;

        if (flipH2 == NO) {
                g_seams[0] = g_sedms[g_seid];
                g_seams[7] = g_sedms[g_seid];
                if (layer_p == 1)
                        g_seaim[7] = g_sedim[g_seid];
                else
                        g_seaim[0] = g_sedim[g_seid];
        } else {
                g_seams[0] = g_dfmab;
                g_seams[7] = g_dfmab;
                if (layer_p == 1)
                        g_seaim[7] = g_dfimb;
                else
                        g_seaim[0] = g_dfimb;
        }
}
