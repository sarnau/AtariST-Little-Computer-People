/*
 * dog.c -- dog movement, animation, and sprite update.
 *
 * The dog is an autonomous agent: it wanders, approaches the food bowl
 * when hungry, and comes when called.  Movement runs at 8 Hz driven by
 * dog_move_and_animate() from the frame loop; sprite state is pushed
 * out to hardware slots 0 or 7 (behind/in-front of LCP by Y depth) via
 * spritedata_update_dog().
 *
 * addr: dog_move_and_animate(), spritedata_update_dog()
 */

#include "types.h"
#include "enums.h"
#include "globals.h"

extern void     dog_calc_walk_path();
extern void     sprite_flip_horizontal();
extern void     spritedata_update_dog();

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

        dog_walk_anim_cycle = dog_walk_anim_cycle + 1;
        if (dog_walk_anim_cycle > 7)
                dog_walk_anim_cycle = 0;

        if (dog_target_x == 0 && dog_target_y == 0)
                return;

        if (lcp_y < (short) (dog_y + 5))
                depth_layer = 1;
        else
                depth_layer = -1;
        if (lcp_state == STATE_READ_PAPER_HOLD ||
            lcp_state == STATE_READ_PAPER_TURN_PAGE)
                depth_layer = 1;

        if (dog_waypoint_x == 0 && dog_waypoint_y == 0)
                dog_calc_walk_path();

        /* Exit stair-mode when reaching a floor boundary. */
        if (dog_on_stairs_flag != NO) {
                floor_num = get_floor_number_from_y(dog_waypoint_y);
                if (dog_y <= floor_bottom_y_coords[floor_num - 1]) {
                        if (floor_num == 3)
                                dog_on_stairs_flag = NO;
                        else if (staircase_waypoint_coords[(floor_num - 1) * 2 + 1] <= dog_y)
                                dog_on_stairs_flag = NO;
                }
        }

        /* Waypoint reached? */
        if (dog_x == dog_waypoint_x && dog_y == dog_waypoint_y) {
                if (dog_x == dog_target_x && dog_y == dog_target_y) {
                        dog_target_x = 0;
                        dog_target_y = 0;
                        dog_waypoint_x = 0;
                        dog_waypoint_y = 0;
                        dog_sprite_id = SPRITE_DOG_LAY_DOWN;
                        spritedata_update_dog(SPRITE_DOG_LAY_DOWN,
                                              depth_layer, NO);
                        return;
                }
                dog_calc_walk_path();
        }

        dog_sprite_id = dog_walk_anim_frames[dog_walk_anim_cycle];
        h_flip = NO;

        if (dog_on_stairs_flag == NO) {
                if (dog_x < dog_waypoint_x) {
                        h_flip = NO;
                        dog_x = dog_x + 1;
                } else if (dog_waypoint_x < dog_x) {
                        h_flip = YES;
                        dog_x = dog_x - 1;
                }
                x_distance = (dog_x < dog_waypoint_x)
                             ? (dog_waypoint_x - dog_x)
                             : (dog_x - dog_waypoint_x);
                if (x_distance < 8) {
                        if (dog_y < dog_waypoint_y)
                                dog_y = dog_y + 1;
                        else if (dog_waypoint_y < dog_y)
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
                   the direction dog_waypoint_y indicates. */
                if (dog_waypoint_y < dog_y) {
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
                                        if (dog_sprite_id != SPRITE_DOG_WALK_RIGHT_9) {
                                                next_x = dog_x + 1;
                                                if (next_x != dog_waypoint_x)
                                                        next_x = dog_x + 2;
                                        }
                                } else if (dog_y < 0xa1) {
                                        h_flip = YES;
                                        dog_y = dog_y - 1;
                                        if (dog_sprite_id != SPRITE_DOG_WALK_RIGHT_9) {
                                                next_x = dog_x - 1;
                                                if (next_x != dog_waypoint_x)
                                                        next_x = dog_x - 2;
                                        }
                                }
                        } else {
                                h_flip = NO;
                                dog_y = dog_y - 2;
                        }
                } else if (dog_y < dog_waypoint_y) {
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
                                        if (dog_sprite_id != SPRITE_DOG_WALK_RIGHT_9) {
                                                next_x = dog_x - 1;
                                                if (next_x != dog_waypoint_x)
                                                        next_x = dog_x - 2;
                                        }
                                } else if (dog_y < 161) {
                                        h_flip = NO;
                                        dog_y = dog_y + 1;
                                        if (dog_sprite_id != SPRITE_DOG_WALK_RIGHT_9) {
                                                next_x = dog_x + 1;
                                                if (next_x != dog_waypoint_x)
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
        spritedata_update_dog(dog_sprite_id, depth_layer, h_flip);
}

/* spritedata_update_dog: push the dog frame into hardware slots 0
   (behind) or 7 (in-front) depending on layerPosition; mirror horizontally
   via sprite_flip_horizontal if needed.  dog_initialized suppresses the
   push while the dog hasn't been placed in the world yet.

   addr: spritedata_update_dog() */

void
spritedata_update_dog(sprite_id, layer_position, flip_horizontal)
short   sprite_id;
short   layer_position;
BOOL16  flip_horizontal;
{
        sprite_active_image[0] = NULL;
        sprite_active_image[7] = NULL;

        if (sprite_id < 0 || dog_initialized != NO)
                return;

        if (flip_horizontal != NO) {
                sprite_flip_horizontal(dog_sprite_pointers[sprite_id],
                                       (unsigned short *) dog_flip_image_buffer,
                                       15, 2);
                sprite_flip_horizontal(dog_mask_pointers[sprite_id],
                                       (unsigned short *) dog_flip_mask_buffer,
                                       15, 2);
        }

        sprite_active_height[0] = sprite_def_height[0x21];
        sprite_active_height[7] = sprite_def_height[0x21];
        sprite_active_width[0]  = sprite_def_width[0x21];
        sprite_active_width[7]  = sprite_def_width[0x21];
        sprite_pending_x[0] = dog_x;
        sprite_pending_x[7] = dog_x;
        sprite_pending_y[0] = dog_y - 17;
        sprite_pending_y[7] = dog_y - 17;

        if (flip_horizontal == NO) {
                sprite_active_mask[0] = dog_mask_pointers[sprite_id];
                sprite_active_mask[7] = dog_mask_pointers[sprite_id];
                if (layer_position == 1)
                        sprite_active_image[7] = dog_sprite_pointers[sprite_id];
                else
                        sprite_active_image[0] = dog_sprite_pointers[sprite_id];
        } else {
                sprite_active_mask[0] = dog_flip_mask_buffer;
                sprite_active_mask[7] = dog_flip_mask_buffer;
                if (layer_position == 1)
                        sprite_active_image[7] = dog_flip_image_buffer;
                else
                        sprite_active_image[0] = dog_flip_image_buffer;
        }
}
