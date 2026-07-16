/*
 * dog.c -- dog movement, animation, and sprite update.
 *
 * The dog is an autonomous agent: it wanders, approaches the food bowl
 * when hungry, and comes when called.  Movement runs at 8 Hz driven by
 * dog_move_and_animate() from the frame loop; sprite state is pushed
 * out to hardware slots 0 or 7 (behind/in-front of LCP by Y depth) via
 * sp_spud().
 *
 * addr: dog_move_and_animate(), sp_spud()
 */

#include "types.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    lcp_y;
extern short    get_floor_number_from_y();
extern short    lcp_state;
extern short    dog_x;
extern short    dog_y;
extern short    g_dtx;
extern short    g_dty;
extern short    g_dyx;
extern short    g_dyy;
extern short    g_dwanc;
extern short    g_dsid;
extern short    dog_on_stairs_flag;
extern short    dog_initialized;
extern short    g_sepex[];
extern short    g_sepey[];
extern short *  g_seaim[];
extern short *  g_seams[];
extern short    g_seach[];
extern short    g_seacw[];
extern short    g_sedeh[];
extern short    g_sedew[];
extern short    g_dwanf[];
extern short *  dog_sprite_pointers[];
extern short *  dog_mask_pointers[];
extern short    g_dfimb[];
extern short    g_dfmab[];
extern short    floor_bottom_y_coords[];
extern short    floor_center_y_coords[];
extern short    staircase_waypoint_coords[];
extern void     dog_calc_walk_path();
extern void     sp_flih();
extern void     sp_spud();

/* dog_init_position: Ghidra 0x??.  Place the dog at its startup spot
   (bottom floor near the food bowl) and NULL-out the two dog sprite
   slots via sp_spud(SPRITE_UNUSED_0=-1).  The dog becomes visible on
   the very next sc_ren8 tick once dog_move_and_animate picks a target
   and calls sp_spud again with a valid walk-cycle sprite id from
   g_dwanf. */
void
dg_ipos()
{
        dog_x = 100;
        dog_y = 195;
        sp_spud(-1, 1, NO);
}

/* dog_move_and_animate: 8 Hz movement + walk-cycle advance.  If the dog
   has no target the routine is a no-op.  Handles flat walking (X/Y
   equal steps to waypoint) and stair navigation (staircase_waypoint_
   coords[] gate for the two staircase entrances).  Layer depth is
   1 (in-front) when the dog is below the resident, -1 (behind) when
   above -- newspaper reading forces in-front so the dog doesn't disappear
   behind the paper.

   addr: dog_move_and_animate() */

void
dog_move_and_animate()
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
        if (lcp_state == STATE_READ_PAPER_HOLD ||
            lcp_state == STATE_READ_PAPER_TURN_PAGE)
                depth_layer = 1;

        if (g_dyx == 0 && g_dyy == 0)
                dog_calc_walk_path();

        /* Exit stair-mode when reaching a floor boundary. */
        if (dog_on_stairs_flag != NO) {
                floor_num = get_floor_number_from_y(g_dyy);
                if (dog_y <= floor_bottom_y_coords[floor_num - 1]) {
                        if (floor_num == 3)
                                dog_on_stairs_flag = NO;
                        else if (staircase_waypoint_coords[(floor_num - 1) * 2 + 1] <= dog_y)
                                dog_on_stairs_flag = NO;
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
                dog_calc_walk_path();
        }

        g_dsid = g_dwanf[g_dwanc];
        h_flip = NO;

        if (dog_on_stairs_flag == NO) {
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
                        floor_num = get_floor_number_from_y(dog_y);
                        if (dog_y < floor_center_y_coords[floor_num - 1])
                                dog_y = dog_y + 1;
                        floor_num = get_floor_number_from_y(dog_y);
                        if (floor_center_y_coords[floor_num - 1] < dog_y)
                                dog_y = dog_y - 1;
                }
        }

        next_x = dog_x;

        if (dog_on_stairs_flag != NO) {
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
   via sp_flih if needed.  dog_initialized suppresses the
   push while the dog hasn't been placed in the world yet.

   addr: sp_spud() */

void
sp_spud(g_seid, layer_position, flip_horizontal)
short   g_seid;
short   layer_position;
BOOL16  flip_horizontal;
{
        g_seaim[0] = NULL;
        g_seaim[7] = NULL;

        if (g_seid < 0 || dog_initialized != NO)
                return;

        if (flip_horizontal != NO) {
                sp_flih(dog_sprite_pointers[g_seid],
                                       (unsigned short *) g_dfimb,
                                       15, 2);
                sp_flih(dog_mask_pointers[g_seid],
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

        if (flip_horizontal == NO) {
                g_seams[0] = dog_mask_pointers[g_seid];
                g_seams[7] = dog_mask_pointers[g_seid];
                if (layer_position == 1)
                        g_seaim[7] = dog_sprite_pointers[g_seid];
                else
                        g_seaim[0] = dog_sprite_pointers[g_seid];
        } else {
                g_seams[0] = g_dfmab;
                g_seams[7] = g_dfmab;
                if (layer_position == 1)
                        g_seaim[7] = g_dfimb;
                else
                        g_seaim[0] = g_dfimb;
        }
}
