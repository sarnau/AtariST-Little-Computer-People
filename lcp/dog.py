"""
Dog AI and animation for Little Computer People (Atari ST).
Translated from Ghidra decompilation of dog_move_and_animate() and
the dog logic embedded in screen_render_8hz().

addr: dog_move_and_animate(), screen_render_8hz() (dog section)

The dog:
  - Wanders to 9 HOUSE_POS destinations with idle countdowns
  - Approaches food bowl when near and bowl is not empty
  - Eats with countdown-based animation (depletes bowl at counts 60, 30, 4)
  - Renders in hardware slots 0 or 7 based on Y depth relative to LCP

Movement mirrors lcp_pathfind_one_step() with the same stair Y constants:
  161 (0xA1) — lower stair entry
  100 (0x64) — upper stair entry

Dog sprite selection:
  - Walking: dog_walk_anim_frames[dog_walk_anim_cycle] (8-frame cycle)
  - Eating:  dog_sprite_eating_anim_tab[dog_eating_countdown % 3] (3-frame)
  - Idle:    SPRITE_DOG_LAY_DOWN
"""

import random
from .enums import SPRITE_ID, DOG_BOWL_STATUS, HOUSE_POS
from .state import GameState
from .constants import (
    DOG_WALK_ANIM_FRAMES, DOG_EATING_ANIM_FRAMES,
    DOG_DESTINATION_POSITION_TABLE, DOG_DEST_X_OFFSET_TABLE,
    DOG_DEST_Y_OFFSET_TABLE, DOG_STAIR_TOP_Y_THRESHOLD,
    DOG_STAIR_BOTTOM_Y_THRESHOLD, STAIRCASE_WAYPOINT_COORDS,
    house_get_position_xy,
)
from .movement import get_floor_number_from_y, FLOOR_BOTTOM_Y_COORDS, FLOOR_CENTER_Y

# Y constants (from decompiled dog_move_and_animate)
DOG_STAIR_LOWER = 0xA1   # 161 — lower stair entry/exit
DOG_STAIR_UPPER = 0x64   # 100 — upper stair entry/exit

# Sentinel sprite value — SPRITE_DOG_WALK_RIGHT_9 never occurs during walk,
# used as "advance 2 pixels unless already 1 pixel from waypoint" check
SPRITE_DOG_WALK_RIGHT_9 = 42

# Player states used for depth comparison (from decompiled code)
PLAYER_STATE_READ_PAPER_HOLD = 50
PLAYER_STATE_READ_PAPER_TURN_PAGE = 51


def random_range(lo: int, hi: int) -> int:
    return random.randint(lo, hi)


def dog_calc_walk_path(gs: GameState) -> None:
    """
    Calculate the next waypoint for the dog.
    Routes through staircase waypoints when changing floors.
    addr: dog_calc_walk_path()
    """
    target_floor = get_floor_number_from_y(gs.dog_target_y)
    current_floor = get_floor_number_from_y(gs.dog_y)

    if current_floor == target_floor:
        gs.dog_on_stairs_flag = 0
        gs.dog_waypoint_x = gs.dog_target_x
        gs.dog_waypoint_y = gs.dog_target_y
    else:
        # Use current floor's staircase waypoint
        floor_from_y = get_floor_number_from_y(gs.dog_y)
        stair_index = (floor_from_y - 1) * 2
        gs.dog_waypoint_x = STAIRCASE_WAYPOINT_COORDS[stair_index]
        gs.dog_waypoint_y = STAIRCASE_WAYPOINT_COORDS[stair_index + 1]

        # Floor 2 going down: override with stair threshold coords
        if floor_from_y == 2:
            dest_floor = get_floor_number_from_y(gs.dog_target_y)
            if dest_floor < floor_from_y:
                gs.dog_waypoint_x = DOG_STAIR_TOP_Y_THRESHOLD - 3  # 121
                gs.dog_waypoint_y = DOG_STAIR_BOTTOM_Y_THRESHOLD   # 137

        gs.dog_on_stairs_flag = 0

        # If already at waypoint, transition to stairs and set next waypoint
        if gs.dog_x == gs.dog_waypoint_x and gs.dog_y == gs.dog_waypoint_y:
            floor_at = get_floor_number_from_y(gs.dog_y)
            if floor_at == 3:
                gs.dog_x -= 8

            gs.dog_on_stairs_flag = 1

            if gs.dog_target_y < gs.dog_y:
                # Going up: next floor's waypoint
                gs.dog_waypoint_x = STAIRCASE_WAYPOINT_COORDS[stair_index + 2]
                gs.dog_waypoint_y = STAIRCASE_WAYPOINT_COORDS[stair_index + 3]
            else:
                # Going down: previous floor's waypoint
                gs.dog_waypoint_x = STAIRCASE_WAYPOINT_COORDS[stair_index - 2]
                gs.dog_waypoint_y = STAIRCASE_WAYPOINT_COORDS[stair_index - 1]

            # Floor 1 entering stairs: override with stair thresholds
            floor_at = get_floor_number_from_y(gs.dog_y)
            if floor_at == 1:
                gs.dog_waypoint_x = DOG_STAIR_TOP_Y_THRESHOLD    # 124
                gs.dog_waypoint_y = DOG_STAIR_BOTTOM_Y_THRESHOLD  # 137


def spritedata_update_dog(gs: GameState, sprite_id: int,
                          depth_layer: int, flip: int) -> None:
    """
    Update dog sprite in hardware slots 0 and 7.
    depth_layer: +1 = slot 7 (behind LCP), -1 = slot 0 (in front).
    Y is offset by -17 for sprite positioning.
    addr: spritedata_update_dog()
    """
    gs.dog_sprite_id = sprite_id
    gs._dog_depth_layer = depth_layer
    gs._dog_flip = flip
    gs._dog_render_x = gs.dog_x
    gs._dog_render_y = gs.dog_y - 17


def dog_move_and_animate(gs: GameState) -> None:
    """
    Advance the dog one pixel-step and update its animation frame.
    Called once per frame from dog_frame_update() (~8 Hz).
    Returns early if no target set (idle).
    addr: dog_move_and_animate()
    """
    # Advance walk animation cycle
    gs.dog_walk_anim_cycle += 1
    if gs.dog_walk_anim_cycle > 7:
        gs.dog_walk_anim_cycle = 0

    # Nothing to do if no target — caller handles idle/wander/eating
    if gs.dog_target_x == 0 and gs.dog_target_y == 0:
        return

    # Determine depth relative to LCP (for rendering layer)
    if gs.lcp_y < gs.dog_y + 5:
        depth_layer = 1     # dog behind LCP → slot 7
    else:
        depth_layer = -1    # dog in front of LCP → slot 0

    # Override depth for specific LCP states
    if gs.lcp_state in (PLAYER_STATE_READ_PAPER_HOLD, PLAYER_STATE_READ_PAPER_TURN_PAGE):
        depth_layer = 1

    # Compute waypoint if not set
    if gs.dog_waypoint_x == 0 and gs.dog_waypoint_y == 0:
        dog_calc_walk_path(gs)

    # Clear stair flag if landing reached
    if gs.dog_on_stairs_flag != 0:
        floor_num = get_floor_number_from_y(gs.dog_waypoint_y)
        floor_idx = floor_num - 1
        if gs.dog_y <= FLOOR_BOTTOM_Y_COORDS[floor_idx]:
            if floor_num == 3:
                gs.dog_on_stairs_flag = 0
            elif STAIRCASE_WAYPOINT_COORDS[(floor_num - 1) * 2 + 1] <= gs.dog_y:
                gs.dog_on_stairs_flag = 0

    # Waypoint reached
    if gs.dog_x == gs.dog_waypoint_x and gs.dog_y == gs.dog_waypoint_y:
        if gs.dog_x == gs.dog_target_x and gs.dog_y == gs.dog_target_y:
            # Destination reached — lay down
            gs.dog_target_x = 0
            gs.dog_target_y = 0
            gs.dog_waypoint_x = 0
            gs.dog_waypoint_y = 0
            gs.dog_sprite_id = SPRITE_ID.SPRITE_DOG_LAY_DOWN
            spritedata_update_dog(gs, SPRITE_ID.SPRITE_DOG_LAY_DOWN, depth_layer, 0)
            return
        dog_calc_walk_path(gs)

    # Current walk frame sprite
    dog_sprite = DOG_WALK_ANIM_FRAMES[gs.dog_walk_anim_cycle]
    gs.dog_sprite_id = dog_sprite
    horizontal_flip = 0

    # -------------------------------------------------------------------
    # Flat-floor movement
    # -------------------------------------------------------------------
    if gs.dog_on_stairs_flag == 0:
        if gs.dog_x < gs.dog_waypoint_x:
            horizontal_flip = 0    # facing right
            gs.dog_x += 1
        elif gs.dog_waypoint_x < gs.dog_x:
            horizontal_flip = 1    # facing left
            gs.dog_x -= 1

        x_distance = abs(gs.dog_waypoint_x - gs.dog_x)
        if x_distance < 8:
            if gs.dog_y < gs.dog_waypoint_y:
                gs.dog_y += 1
            elif gs.dog_waypoint_y < gs.dog_y:
                gs.dog_y -= 1
        else:
            floor_num = get_floor_number_from_y(gs.dog_y)
            centre = FLOOR_CENTER_Y[floor_num - 1]
            if gs.dog_y < centre:
                gs.dog_y += 1
            floor_num = get_floor_number_from_y(gs.dog_y)
            centre = FLOOR_CENTER_Y[floor_num - 1]
            if centre < gs.dog_y:
                gs.dog_y -= 1

    new_x = gs.dog_x

    # -------------------------------------------------------------------
    # Stair movement
    # -------------------------------------------------------------------
    if gs.dog_on_stairs_flag != 0:
        if gs.dog_waypoint_y < gs.dog_y:
            # Climbing upward
            if gs.dog_y == 0xA1:   # 161
                horizontal_flip = 1
                gs.dog_y = 0x9F    # 159
                new_x = gs.dog_x - 0x11  # -17
            elif gs.dog_y == 100:
                horizontal_flip = 0
                gs.dog_y = 0x62    # 98
                new_x = gs.dog_x + 3
            elif gs.dog_y < 162 and (gs.dog_y < 101 or gs.dog_y > 139):
                if gs.dog_y < 100:
                    horizontal_flip = 0
                    gs.dog_y -= 1
                    new_x = gs.dog_x + 1
                    if (dog_sprite != SPRITE_DOG_WALK_RIGHT_9
                            and gs.dog_x + 1 != gs.dog_waypoint_x):
                        new_x = gs.dog_x + 2
                elif gs.dog_y < 0xA1:
                    horizontal_flip = 1
                    gs.dog_y -= 1
                    new_x = gs.dog_x - 1
                    if (dog_sprite != SPRITE_DOG_WALK_RIGHT_9
                            and gs.dog_x - 1 != gs.dog_waypoint_x):
                        new_x = gs.dog_x - 2
            else:
                horizontal_flip = 0
                gs.dog_y -= 2

        elif gs.dog_y < gs.dog_waypoint_y:
            # Descending downward
            if gs.dog_y == 0xA1:   # 161
                horizontal_flip = 0
                gs.dog_y = 165
                new_x = gs.dog_x + 1
            elif gs.dog_y == 100:
                horizontal_flip = 0
                gs.dog_y = 102
                new_x = gs.dog_x + 3
            elif gs.dog_y < 162 and (gs.dog_y < 101 or gs.dog_y > 131):
                if gs.dog_y < 100:
                    horizontal_flip = 1
                    gs.dog_y += 1
                    new_x = gs.dog_x - 1
                    if (dog_sprite != SPRITE_DOG_WALK_RIGHT_9
                            and gs.dog_x - 1 != gs.dog_waypoint_x):
                        new_x = gs.dog_x - 2
                elif gs.dog_y < 161:
                    horizontal_flip = 0
                    gs.dog_y += 1
                    new_x = gs.dog_x + 1
                    if (dog_sprite != SPRITE_DOG_WALK_RIGHT_9
                            and gs.dog_x + 1 != gs.dog_waypoint_x):
                        new_x = gs.dog_x + 2
            else:
                horizontal_flip = 0
                gs.dog_y += 1

    gs.dog_x = new_x
    spritedata_update_dog(gs, gs.dog_sprite_id, depth_layer, horizontal_flip)


# ---------------------------------------------------------------------------
# Dog frame update — called once per frame from render.py
# Contains wander AI, eating logic, and calls dog_move_and_animate()
# addr: screen_render_8hz() — dog section
# ---------------------------------------------------------------------------

def dog_frame_update(gs: GameState) -> None:
    """
    Per-frame dog logic from screen_render_8hz().
    Handles movement, idle wander, eating, and bowl depletion.
    addr: screen_render_8hz() (dog section)
    """
    dog_move_and_animate(gs)

    # Clamp idle countdown to valid range
    if gs.dog_idle_countdown < 0 or gs.dog_idle_countdown > 200:
        gs.dog_idle_countdown = 5

    # --- Eating start check ---
    # Start eating when: idle (no target), bowl not empty, near food bowl,
    # not already eating, dog_x < 20, dog_y > 160
    if (gs.dog_target_x == 0 and gs.dog_target_y == 0
            and gs.dog_bowl_status != DOG_BOWL_STATUS.BOWL_EMPTY
            and gs.dog_near_food_bowl != 0
            and gs.dog_eating_active == 0
            and gs.dog_x < 0x14       # 20
            and gs.dog_y > 0xA0):     # 160
        gs.dog_eating_active = 1
        gs.dog_eating_countdown = random_range(0x52, 100)  # 82–100

    # --- Idle countdown ---
    if (gs.dog_target_x == 0 and gs.dog_target_y == 0
            and gs.dog_idle_countdown != 0
            and gs.dog_eating_active == 0):
        gs.dog_idle_countdown -= 1

    # --- Wander destination selection ---
    if (gs.dog_target_x == 0 and gs.dog_target_y == 0
            and gs.dog_idle_countdown == 0
            and gs.dog_eating_active == 0):

        # Pick range based on visibility
        min_idx = 0 if gs.dog_visible == 0 else 3

        # Pick random destination, avoiding last target
        while True:
            idx = random_range(min_idx, 8)
            if idx != gs.dog_last_target_index:
                break

        dest_pos = DOG_DESTINATION_POSITION_TABLE[idx]
        tx, ty = house_get_position_xy(dest_pos)
        gs.dog_target_y = DOG_DEST_Y_OFFSET_TABLE[idx] + ty
        gs.dog_target_x = DOG_DEST_X_OFFSET_TABLE[idx] + tx

        if dest_pos == HOUSE_POS.POS_BTM_0:  # POS_BTM_STAIR_LANDING
            gs.dog_near_food_bowl = 1

        gs.dog_last_target_index = idx
        gs.dog_idle_countdown = random_range(20, 200)

    # --- Eating animation ---
    if gs.dog_eating_active != 0:
        gs.dog_eating_countdown -= 1

        if gs.dog_eating_countdown == 0:
            # Eating finished
            gs.dog_eating_active = 0
            gs.dog_near_food_bowl = 0
            gs.dog_food_bowl_change = -1
        else:
            # Bowl depletion at specific countdown values
            if gs.dog_eating_countdown in (60, 30, 4):
                gs.dog_food_bowl_change = -1
            else:
                gs.dog_food_bowl_change = 0

            # Eating animation frame
            gs.dog_sprite_id = DOG_EATING_ANIM_FRAMES[gs.dog_eating_countdown % 3]
            spritedata_update_dog(gs, gs.dog_sprite_id, 1, 0)
