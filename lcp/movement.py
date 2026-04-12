"""
LCP character movement and pathfinding for Little Computer People (Atari ST).
Translated from Ghidra decompilation of lcp_pathfind_one_step() and
lcp_walk_to_destination().

addr: lcp_pathfind_one_step(), lcp_walk_to_destination()

The movement system handles:
  - Flat-floor walking with 8-frame cycle (PLAYER_STATE 0–7)
  - Stair climbing/descending (PLAYER_STATE 9–24) via fixed waypoints
  - Waypoint routing through staircase at X=160
  - Footstep sound triggering on frames 3, 7, 12
  - Carried-object sprite layer management
  - Sickness speed penalty (extra game_tick_and_animate call when sick)

Key Y constants (from decompiled code):
  161 (0xA1) — lower stair entry/exit (between bottom and middle floors)
  100 (0x64) — upper stair entry/exit (between middle and top floors)
  0xA5 = 165 — stair bottom landing Y adjust
  0x66 = 102 — stair top landing Y adjust

Floor Y boundaries (from get_floor_number_from_y()):
  Y < 78:  floor 3 (top)
  Y < 141: floor 2 (middle)
  Y ≥ 141: floor 1 (bottom)
"""

from .enums import PLAYER_STATE, FACING_DIR, SPRITE_LAYER
from .state import GameState
from .constants import STAIRCASE_WAYPOINT_COORDS


# ---------------------------------------------------------------------------
# Floor Y coordinate tables
# addr: floor_center_y_coords[3], floor_bottom_y_coords[3]
# These define per-floor Y centering and boundary used during movement.
# Floor indices: 1=bottom, 2=middle, 3=top (1-based in original C)
# ---------------------------------------------------------------------------
FLOOR_BOTTOM_Y_COORDS = [202, 140, 77]    # floor_bottom_y_coords[0..2] @ 0x1a07e
FLOOR_CENTER_Y_COORDS = [198, 135, 71]    # floor_center_y_coords[0..2] @ 0x1a076

# Staircase X centre and key Y waypoints
STAIR_X      = 160
STAIR_LOWER  = 161   # Y boundary between bottom and middle stair flights
STAIR_UPPER  = 100   # Y boundary between middle and top stair flights

# floor_center_y_coords for Y-alignment when far from waypoint (>= 8 px)
# Indexed as: 0=bottom, 1=middle, 2=top — extracted from DATA segment at 0x1a076
FLOOR_CENTER_Y = [198, 135, 71]


# ---------------------------------------------------------------------------
# Floor number helper
# addr: get_floor_number_from_y()
# Returns 1=bottom, 2=middle, 3=top
# ---------------------------------------------------------------------------
def get_floor_number_from_y(y: int) -> int:
    """
    Return floor number (1–3) from a Y pixel coordinate.
    addr: get_floor_number_from_y()
    """
    if y < 78:
        return 3   # top floor
    if y < 141:
        return 2   # middle floor
    return 1       # bottom floor


# ---------------------------------------------------------------------------
# Waypoint calculation
# addr: lcp_calc_floor_waypoint()
# Sets walk_waypoint_x/y to the next intermediate point (stair entry or dest)
# ---------------------------------------------------------------------------
def lcp_calc_floor_waypoint(gs: GameState) -> None:
    """
    Compute the next waypoint for the LCP.
    If the destination is on a different floor, route through the staircase.
    addr: lcp_calc_floor_waypoint()
    """
    current_floor = get_floor_number_from_y(gs.lcp_y)
    target_floor  = get_floor_number_from_y(gs.walk_target_y)

    if current_floor == target_floor:
        # Same floor — go directly to destination
        gs.walk_waypoint_x = gs.walk_target_x
        gs.walk_waypoint_y = gs.walk_target_y
    else:
        # Different floor — route through nearest stair entry on current floor
        gs.lcp_on_stairs_flag = 1
        if current_floor == 1:
            # Bottom → stair base at Y=161
            gs.walk_waypoint_x = STAIR_X
            gs.walk_waypoint_y = STAIR_LOWER
        elif current_floor == 2:
            if target_floor == 3:
                # Middle → stair top entry at Y=100
                gs.walk_waypoint_x = STAIR_X
                gs.walk_waypoint_y = STAIR_UPPER
            else:
                # Middle → stair bottom at Y=161
                gs.walk_waypoint_x = STAIR_X
                gs.walk_waypoint_y = STAIR_LOWER
        else:
            # Top → stair entry at Y=100
            gs.walk_waypoint_x = STAIR_X
            gs.walk_waypoint_y = STAIR_UPPER


# ---------------------------------------------------------------------------
# Sprite helpers (stubs — actual rendering is in sprites.py)
# addr: spritedata_select_carried_object_left/right(), sprite_update_slots()
# ---------------------------------------------------------------------------
def spritedata_select_carried_object_left(gs: GameState, obj_id: int) -> None:
    """
    Activate a sprite as a carried object in the behind-LCP layer.
    Sets sprite_layer_flags to SPRITE_BEHIND_LCP (-1), assigns to a
    hardware slot, copies sprite definition data, and sets carry flag.
    addr: spritedata_select_carried_object_left()
    """
    from .sprites import sprite_update_slots as _sprite_update_slots
    from .enums import SPRITE_LAYER

    if obj_id < 0 or obj_id >= 60:
        return
    gs.sprite_layer_flags[obj_id] = SPRITE_LAYER.SPRITE_BEHIND_LCP
    _sprite_update_slots(gs)
    slot = gs.sprite_slot_map[obj_id]
    if 0 <= slot < 8:
        gs.sprite_active_image[slot]  = gs.sprite_def_image[obj_id]
        gs.sprite_active_mask[slot]   = gs.sprite_def_mask[obj_id]
        gs.sprite_active_height[slot] = gs.sprite_def_height[obj_id]
        gs.sprite_active_width[slot]  = gs.sprite_def_width[obj_id]
    gs.lcp_carrying_object_flag = 1
    gs.lcp_carried_object = obj_id


def spritedata_select_carried_object_right(gs: GameState, obj_id: int) -> None:
    """
    Activate a sprite as a carried object in the in-front-of-LCP layer.
    Same as left variant but uses SPRITE_IN_FRONT (+1).
    addr: spritedata_select_carried_object_right()
    """
    from .sprites import sprite_update_slots as _sprite_update_slots
    from .enums import SPRITE_LAYER

    if obj_id < 0 or obj_id >= 60:
        return
    gs.sprite_layer_flags[obj_id] = SPRITE_LAYER.SPRITE_IN_FRONT
    _sprite_update_slots(gs)
    slot = gs.sprite_slot_map[obj_id]
    if 0 <= slot < 8:
        gs.sprite_active_image[slot]  = gs.sprite_def_image[obj_id]
        gs.sprite_active_mask[slot]   = gs.sprite_def_mask[obj_id]
        gs.sprite_active_height[slot] = gs.sprite_def_height[obj_id]
        gs.sprite_active_width[slot]  = gs.sprite_def_width[obj_id]
    gs.lcp_carrying_object_flag = 1
    gs.lcp_carried_object = obj_id


def sprite_update_slots(gs: GameState) -> None:
    """
    Recompute hardware sprite slot assignments.
    Delegates to the full implementation in sprites.py.
    addr: sprite_update_slots()
    """
    from .sprites import sprite_update_slots as _sprite_update_slots
    _sprite_update_slots(gs)


# ---------------------------------------------------------------------------
# Footstep sound stub
# addr: lcp_play_footstep_sound()
# ---------------------------------------------------------------------------
def lcp_play_footstep_sound(gs: GameState) -> None:
    """
    Play the appropriate footstep sound for the current surface.
    addr: lcp_play_footstep_sound()
    Only triggers if footstep_trigger_flag is set.
    """
    if not gs.footstep_trigger_flag:
        return
    gs.footstep_trigger_flag = 0
    # sound.py handles actual playback — just set a pending SFX
    from .enums import SOUND_EFFECT_ID
    floor = get_floor_number_from_y(gs.lcp_y)
    if gs.lcp_on_stairs_flag:
        gs.soundeffect_pending = SOUND_EFFECT_ID.SFX_FOOTSTEP_STAIRS
    elif floor == 1:
        gs.soundeffect_pending = SOUND_EFFECT_ID.SFX_FOOTSTEP_CARPET
    else:
        gs.soundeffect_pending = SOUND_EFFECT_ID.SFX_FOOTSTEP_WOOD


# ---------------------------------------------------------------------------
# game_tick_and_animate stub (to avoid circular import)
# addr: game_tick_and_animate()
# ---------------------------------------------------------------------------
def _game_tick(gs: GameState) -> None:
    """Call game_tick_and_animate(0). Imported lazily to avoid circular deps."""
    from .main import game_tick_and_animate
    game_tick_and_animate(gs, 0)


# ---------------------------------------------------------------------------
# lcp_pathfind_one_step — main movement function
# addr: lcp_pathfind_one_step()
# ---------------------------------------------------------------------------
def lcp_pathfind_one_step(gs: GameState) -> None:
    """
    Advance the LCP one pixel-step toward its current waypoint.
    Handles flat walking, stair states, carried-object layers, and footsteps.
    Called in a loop by lcp_walk_to_destination().
    addr: lcp_pathfind_one_step()
    """
    gs.footstep_trigger_flag = 0   # footstep_trigger_flag = NO

    if gs.walk_target_x == 0 and gs.walk_target_y == 0:
        return

    # Compute waypoint if not set
    if gs.walk_waypoint_x == 0 and gs.walk_waypoint_y == 0:
        lcp_calc_floor_waypoint(gs)

    # Check if stairs are still needed (stair landing reached)
    # addr: staircase_waypoint_coords[(floor_num-1) + floor_num - 1 + 1] = [2*floor_num - 1]
    if gs.lcp_on_stairs_flag:
        floor_num = get_floor_number_from_y(gs.walk_waypoint_y)
        floor_idx = floor_num - 1   # 0-based
        if gs.lcp_y <= FLOOR_BOTTOM_Y_COORDS[floor_idx]:
            if floor_num == 3:
                gs.lcp_on_stairs_flag = 0
            elif STAIRCASE_WAYPOINT_COORDS[2 * floor_num - 1] <= gs.lcp_y:
                gs.lcp_on_stairs_flag = 0

    # Waypoint reached
    if gs.lcp_x == gs.walk_waypoint_x and gs.lcp_y == gs.walk_waypoint_y:
        if gs.lcp_x == gs.walk_target_x and gs.lcp_y == gs.walk_target_y:
            # Destination reached
            gs.walk_target_x = 0
            gs.walk_target_y = 0
            gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
            _game_tick(gs)
            return
        lcp_calc_floor_waypoint(gs)

    # -----------------------------------------------------------------------
    # Flat-floor walking
    # -----------------------------------------------------------------------
    if not gs.lcp_on_stairs_flag:
        if gs.lcp_carrying_object_flag:
            spritedata_select_carried_object_left(gs, gs.lcp_carried_object)

        if gs.lcp_x < gs.walk_waypoint_x:
            # Walk right
            gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
            if gs.lcp_state < 8:
                gs.lcp_state += 1   # STATE_WALK_FRAME_1 = 1
                if gs.lcp_state > 7:
                    gs.lcp_state = PLAYER_STATE.STATE_WALK_FRAME_0
            else:
                gs.lcp_state = PLAYER_STATE.STATE_WALK_FRAME_0
            gs.lcp_x += 1
            if gs.head_anim_mode != 10:   # last_walk_sound_id != 10
                gs.head_anim_target = 10
                gs.head_anim_mode = 10

        elif gs.lcp_x > gs.walk_waypoint_x:
            # Walk left
            gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
            if gs.lcp_state < 8:
                gs.lcp_state += 1
                if gs.lcp_state > 7:
                    gs.lcp_state = PLAYER_STATE.STATE_WALK_FRAME_0
            else:
                gs.lcp_state = PLAYER_STATE.STATE_WALK_FRAME_0
            gs.lcp_x -= 1
            if gs.head_anim_mode != 0xe:   # last_walk_sound_id != 0xE
                gs.head_anim_target = 14
                gs.head_anim_mode = 0xe

        else:
            # X aligned — advance walk frame for Y movement
            gs.lcp_state += 1
            if gs.lcp_state > 7:
                gs.lcp_state = PLAYER_STATE.STATE_WALK_FRAME_0

        # Y adjustment
        x_distance = abs(gs.walk_waypoint_x - gs.lcp_x)
        if x_distance < 8:
            # Near waypoint — track Y directly
            if gs.lcp_y < gs.walk_waypoint_y:
                gs.lcp_y += 1
            elif gs.lcp_y > gs.walk_waypoint_y:
                gs.lcp_y -= 1
        else:
            # Far from waypoint — snap Y toward floor centre line
            floor_num = get_floor_number_from_y(gs.lcp_y)
            floor_idx = floor_num - 1
            centre_y = FLOOR_CENTER_Y[floor_idx]
            if gs.lcp_y < centre_y:
                gs.lcp_y += 1
            floor_num = get_floor_number_from_y(gs.lcp_y)
            floor_idx = floor_num - 1
            centre_y = FLOOR_CENTER_Y[floor_idx]
            if gs.lcp_y > centre_y:
                gs.lcp_y -= 1

        # Footstep on frames 3 and 7
        if gs.lcp_state in (PLAYER_STATE.STATE_WALK_FRAME_3,
                             PLAYER_STATE.STATE_WALK_FRAME_7):
            gs.footstep_trigger_flag = 1   # YES

    # -----------------------------------------------------------------------
    # Stair movement
    # -----------------------------------------------------------------------
    if gs.lcp_on_stairs_flag:
        if gs.walk_waypoint_y < gs.lcp_y:
            # --- Climbing upward ---
            if gs.lcp_y == STAIR_LOWER:   # 161 — lower stair entry going up
                if gs.lcp_carrying_object_flag:
                    spritedata_select_carried_object_left(gs, gs.lcp_carried_object)
                gs.lcp_state = PLAYER_STATE.STATE_STAIR_UP_0
                gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
                gs.lcp_x -= 6
                gs.lcp_y -= 2
                if gs.head_anim_mode != 0xe:
                    gs.head_anim_target = 14
                    gs.head_anim_mode = 0xe

            elif gs.lcp_y == STAIR_UPPER:   # 100 — upper stair entry going up
                if gs.lcp_carrying_object_flag:
                    spritedata_select_carried_object_left(gs, gs.lcp_carried_object)
                gs.lcp_state = PLAYER_STATE.STATE_STAIR_UP_0
                gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
                gs.lcp_x += 3
                gs.lcp_y -= 2
                if gs.head_anim_mode != 10:
                    gs.head_anim_target = 10
                    gs.head_anim_mode = 10

            elif gs.lcp_y < (STAIR_LOWER + 1) and (gs.lcp_y < (STAIR_UPPER + 1) or gs.lcp_y > 139):
                # In stair flight (not landing zone)
                if gs.lcp_y < STAIR_UPPER:
                    # Upper stair flight (Y < 100) — facing right, moving right+up
                    if gs.lcp_carrying_object_flag:
                        spritedata_select_carried_object_left(gs, gs.lcp_carried_object)
                    gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
                    gs.lcp_y -= 1
                    new_x = gs.lcp_x
                    if (gs.lcp_state != PLAYER_STATE.STATE_STAIR_UP_3
                            and gs.lcp_x + 1 != gs.walk_waypoint_x):
                        new_x = gs.lcp_x + 2
                    else:
                        new_x = gs.lcp_x + 1
                    gs.lcp_x = new_x
                    gs.lcp_state += 1
                    if gs.lcp_state > 0xC:   # > STATE_STAIR_UP_3 (12)
                        gs.lcp_state = PLAYER_STATE.STATE_STAIR_UP_0
                    if gs.lcp_state == PLAYER_STATE.STATE_STAIR_UP_3:
                        gs.footstep_trigger_flag = 1
                    if gs.head_anim_mode != 10:
                        gs.head_anim_target = 10
                        gs.head_anim_mode = 10

                elif gs.lcp_y < STAIR_LOWER:   # < 0xA1 = 161
                    # Lower stair flight — facing left, moving left+up
                    if gs.lcp_carrying_object_flag:
                        spritedata_select_carried_object_left(gs, gs.lcp_carried_object)
                    gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
                    gs.lcp_y -= 1
                    new_x = gs.lcp_x
                    if (gs.lcp_state != PLAYER_STATE.STATE_STAIR_UP_3
                            and gs.lcp_x - 1 != gs.walk_waypoint_x):
                        new_x = gs.lcp_x - 2
                    else:
                        new_x = gs.lcp_x - 1
                    gs.lcp_x = new_x
                    gs.lcp_state += 1
                    if gs.lcp_state > 0xC:
                        gs.lcp_state = PLAYER_STATE.STATE_STAIR_UP_0
                    if gs.lcp_state == PLAYER_STATE.STATE_STAIR_UP_3:
                        gs.footstep_trigger_flag = 1
                    if gs.head_anim_mode != 0xe:
                        gs.head_anim_target = 14
                        gs.head_anim_mode = 0xe

            else:
                # Stair top landing transition
                if gs.lcp_carrying_object_flag:
                    gs.sprite_layer_flags[gs.lcp_carried_object] = SPRITE_LAYER.SPRITE_BEHIND_LCP
                    sprite_update_slots(gs)

                if gs.lcp_state < 0xD or gs.lcp_state > 0x10:
                    gs.lcp_state = PLAYER_STATE.STATE_STAIR_TOP_0
                else:
                    gs.lcp_state += 1
                    if gs.lcp_state > 0x10:   # > STATE_STAIR_TOP_3 (16)
                        gs.lcp_state = PLAYER_STATE.STATE_STAIR_TOP_0
                        gs.lcp_facing_direction ^= FACING_DIR.FACING_LEFT
                    if gs.lcp_state in (PLAYER_STATE.STATE_STAIR_TOP_3,
                                        PLAYER_STATE.STATE_STAIR_TOP_0):
                        gs.lcp_y -= 2
                    if gs.lcp_state == PLAYER_STATE.STATE_STAIR_TOP_3:
                        gs.footstep_trigger_flag = 1

                if gs.head_anim_mode != 0xC:
                    gs.head_anim_target = 12
                    gs.head_anim_mode = 0xC

        elif gs.lcp_y < gs.walk_waypoint_y:
            # --- Descending downward ---
            if gs.lcp_carrying_object_flag:
                spritedata_select_carried_object_left(gs, gs.lcp_carried_object)

            if gs.lcp_y == 0xA1:   # 161 — lower landing start descend
                gs.lcp_state = PLAYER_STATE.STATE_STAIR_BTM_0
                gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
                gs.lcp_y = 0xA5   # 165
                gs.lcp_x += 6
                if gs.head_anim_mode != 8:
                    gs.head_anim_target = 8
                    gs.head_anim_mode = 8
                if gs.lcp_carrying_object_flag:
                    spritedata_select_carried_object_right(gs, gs.lcp_carried_object)

            elif gs.lcp_y == STAIR_UPPER:   # 100 — upper landing start descend
                gs.lcp_state = PLAYER_STATE.STATE_STAIR_BTM_0
                gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
                gs.lcp_y = 0x66   # 102
                gs.lcp_x -= 2
                if gs.head_anim_mode != 8:
                    gs.head_anim_target = 8
                    gs.head_anim_mode = 8
                if gs.lcp_carrying_object_flag:
                    spritedata_select_carried_object_right(gs, gs.lcp_carried_object)

            elif gs.lcp_y < (0xA2) and (gs.lcp_y < 0x65 or gs.lcp_y > 0x83):
                if gs.lcp_y < STAIR_UPPER:
                    # Upper stair descend — facing left, moving left+down
                    if gs.lcp_carrying_object_flag:
                        spritedata_select_carried_object_right(gs, gs.lcp_carried_object)
                    gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
                    gs.lcp_y += 1
                    new_x = gs.lcp_x
                    if (gs.lcp_state != PLAYER_STATE.STATE_STAIR_DOWN_3
                            and gs.lcp_x - 1 != gs.walk_waypoint_x):
                        new_x = gs.lcp_x - 2
                    else:
                        new_x = gs.lcp_x - 1
                    gs.lcp_x = new_x
                    if 0x10 < gs.lcp_state < 0x15:
                        gs.lcp_state += 1
                        if gs.lcp_state > 0x14:
                            gs.lcp_state = PLAYER_STATE.STATE_STAIR_DOWN_0
                    else:
                        gs.lcp_state = PLAYER_STATE.STATE_STAIR_DOWN_0
                    if gs.head_anim_mode != 0xe:
                        gs.head_anim_target = 14
                        gs.head_anim_mode = 0xe
                    if gs.lcp_state == PLAYER_STATE.STATE_STAIR_DOWN_1:
                        gs.footstep_trigger_flag = 1

                elif gs.lcp_y < STAIR_LOWER:   # < 0xA1
                    # Lower stair descend — facing right, moving right+down
                    if gs.lcp_carrying_object_flag:
                        spritedata_select_carried_object_right(gs, gs.lcp_carried_object)
                    gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
                    gs.lcp_y += 1
                    new_x = gs.lcp_x
                    if (gs.lcp_state != PLAYER_STATE.STATE_STAIR_DOWN_3
                            and gs.lcp_x + 1 != gs.walk_waypoint_x):
                        new_x = gs.lcp_x + 2
                    else:
                        new_x = gs.lcp_x + 1
                    gs.lcp_x = new_x
                    if 0x10 < gs.lcp_state < 0x15:
                        gs.lcp_state += 1
                        if gs.lcp_state > 0x14:
                            gs.lcp_state = PLAYER_STATE.STATE_STAIR_DOWN_0
                    else:
                        gs.lcp_state = PLAYER_STATE.STATE_STAIR_DOWN_0
                    if gs.head_anim_mode != 10:
                        gs.head_anim_target = 10
                        gs.head_anim_mode = 10
                    if gs.lcp_state == PLAYER_STATE.STATE_STAIR_DOWN_1:
                        gs.footstep_trigger_flag = 1

            else:
                # Stair bottom landing
                if gs.lcp_carrying_object_flag:
                    spritedata_select_carried_object_right(gs, gs.lcp_carried_object)
                if gs.lcp_state < 0x15 or gs.lcp_state > 0x18:
                    gs.lcp_state = PLAYER_STATE.STATE_STAIR_BTM_0
                    gs.lcp_x += 2
                else:
                    gs.lcp_state += 1
                    if gs.lcp_state > 0x18:
                        gs.lcp_state = PLAYER_STATE.STATE_STAIR_BTM_0
                        gs.lcp_facing_direction ^= FACING_DIR.FACING_LEFT
                    if gs.lcp_state in (PLAYER_STATE.STATE_STAIR_BTM_1,
                                        PLAYER_STATE.STATE_STAIR_BTM_2):
                        gs.lcp_y += 2
                    if gs.lcp_state == PLAYER_STATE.STATE_STAIR_BTM_3:
                        gs.footstep_trigger_flag = 1
                if gs.head_anim_mode != 8:
                    gs.head_anim_target = 8
                    gs.head_anim_mode = 8

    # Sickness speed penalty: extra tick when not healthy
    if gs.lcp.sickness_level != 0:   # != SICKNESS_HEALTHY
        _game_tick(gs)
        lcp_play_footstep_sound(gs)

    _game_tick(gs)
    if gs.lcp.sickness_level == 0:   # == SICKNESS_HEALTHY
        lcp_play_footstep_sound(gs)


# ---------------------------------------------------------------------------
# lcp_walk_to_destination — outer walk loop
# addr: lcp_walk_to_destination()
# ---------------------------------------------------------------------------
def lcp_walk_to_destination(gs: GameState) -> int:
    """
    Walk the LCP to (walk_target_x, walk_target_y).
    Loops calling lcp_pathfind_one_step() until the destination is reached
    or a higher-priority event interrupts the walk.

    Returns 0 on success, -1 if interrupted.
    addr: lcp_walk_to_destination()
    """
    from .enums import HEAD_ANIM_MODE
    gs.head_anim_mode = HEAD_ANIM_MODE.HEAD_ANIM_WALKING
    gs.head_anim_mode = 0   # last_walk_sound_id = 0

    while True:
        if gs.walk_target_x == 0 and gs.walk_target_y == 0:
            return 0
        lcp_pathfind_one_step(gs)

        # Check for interruption (event arrived while walking)
        interrupted = (
            gs.in_execute_event_routine_flag == 0
            and gs.triggered_event_list[0] != 0xFFFF   # ACTION_NONE
            and gs.lcp_carrying_object_flag == 0
            and gs.intro_sequence_active == 0
            and gs.lcp_on_stairs_flag == 0
            and gs.action_interruptible_flag == 0
        )
        if interrupted:
            gs.walk_target_y = 0
            gs.walk_target_x = 0
            return -1
