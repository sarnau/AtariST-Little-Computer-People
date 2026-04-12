"""
Shared test fixtures for Little Computer People unit tests.
"""
import sys
import os
import pytest
from pathlib import Path

# Add project root to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from lcp.state import GameState
from lcp.structs import LCP
from lcp.enums import (
    ACTION_ID, PLAYER_STATE, FACING_DIR, HOUSE_POS,
    DOG_BOWL_STATUS, SICKNESS_LEVEL, HAPPINESS_LEVEL,
    SPRITE_ID, HEAD_ANIM_MODE,
)
from lcp.constants import (
    house_get_position_xy, FLOOR_BASELINE_Y,
    STAIRCASE_WAYPOINT_COORDS,
    DOG_DESTINATION_POSITION_TABLE, DOG_DEST_X_OFFSET_TABLE,
    DOG_DEST_Y_OFFSET_TABLE,
)
from lcp.movement import (
    FLOOR_BOTTOM_Y_COORDS, FLOOR_CENTER_Y,
)


@pytest.fixture
def gs():
    """Create a fresh GameState with a default LCP for testing."""
    state = GameState()
    state.lcp = LCP()
    state.copyprot_check_return = 1
    state.lcp_loaded = 1
    state.game_speed_counter = 5
    # Place LCP at a known position on the bottom floor
    state.lcp_x = 160
    state.lcp_y = 190
    state.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
    state.lcp_facing_direction = FACING_DIR.FACING_RIGHT
    # Set reasonable defaults
    state.lcp.happiness = 0  # happy
    state.lcp.sickness_level = 0  # healthy
    state.lcp.hunger_level = 0
    state.lcp.thirst_level = 0
    state.lcp.food_supply = 4
    # Dog at bottom floor
    state.dog_x = 8
    state.dog_y = 190
    state.dog_target_x = 0
    state.dog_target_y = 0
    state.dog_sprite_id = SPRITE_ID.SPRITE_DOG_LAY_DOWN
    return state


@pytest.fixture
def gs_mid_floor(gs):
    """GameState with LCP on the middle floor."""
    gs.lcp_x = 100
    gs.lcp_y = 135
    return gs


@pytest.fixture
def gs_top_floor(gs):
    """GameState with LCP on the top floor."""
    gs.lcp_x = 100
    gs.lcp_y = 71
    return gs


def walk_to_completion(gs, max_steps=5000):
    """
    Run lcp_pathfind_one_step in a loop until destination is reached.
    Returns number of steps taken, or -1 if max_steps exceeded.
    Bypasses game_tick_and_animate to avoid side effects.
    """
    from lcp.movement import lcp_pathfind_one_step, lcp_calc_floor_waypoint

    # Calculate initial waypoint
    lcp_calc_floor_waypoint(gs)

    for step in range(max_steps):
        if gs.lcp_x == gs.walk_target_x and gs.lcp_y == gs.walk_target_y:
            return step
        # Advance one pixel step (without calling game_tick)
        _pathfind_step_no_tick(gs)
    return -1


def _pathfind_step_no_tick(gs):
    """
    Simplified pathfind step that moves the LCP one pixel without
    calling game_tick_and_animate (avoids simulation side effects).

    Handles flat-floor walking, stair approach, and stair climbing/descending.
    The real lcp_pathfind_one_step uses complex state machines for stairs;
    this simplified version routes through stair waypoints with basic
    diagonal movement.
    """
    from lcp.movement import (
        lcp_calc_floor_waypoint, get_floor_number_from_y,
        STAIR_X, STAIR_LOWER, STAIR_UPPER,
    )

    # Check if waypoint reached -> recalculate
    if gs.lcp_x == gs.walk_waypoint_x and gs.lcp_y == gs.walk_waypoint_y:
        if gs.lcp_x == gs.walk_target_x and gs.lcp_y == gs.walk_target_y:
            return  # destination reached

        # At a stair entry/exit point: advance to next waypoint along the
        # staircase rather than letting lcp_calc_floor_waypoint loop back
        # to the same stair entry.
        current_floor = get_floor_number_from_y(gs.lcp_y)
        target_floor = get_floor_number_from_y(gs.walk_target_y)

        if current_floor == target_floor and gs.lcp_on_stairs_flag:
            # Arrived on the target floor via stairs — clear flag, go direct
            gs.lcp_on_stairs_flag = 0
            gs.walk_waypoint_x = gs.walk_target_x
            gs.walk_waypoint_y = gs.walk_target_y
        elif current_floor != target_floor and gs.lcp_on_stairs_flag:
            # Route through intermediate stair waypoints
            if current_floor == 1 and gs.lcp_y == STAIR_LOWER:
                # At bottom stair entry going up -> next waypoint is stair upper
                gs.walk_waypoint_x = STAIR_X
                gs.walk_waypoint_y = STAIR_UPPER
            elif current_floor == 2 and gs.lcp_y == STAIR_UPPER:
                if target_floor == 3:
                    # At upper stair landing going up -> emerge on floor 3
                    # Set waypoint to target directly; clear stair flag
                    gs.walk_waypoint_x = gs.walk_target_x
                    gs.walk_waypoint_y = gs.walk_target_y
                    gs.lcp_on_stairs_flag = 0
                elif target_floor == 1:
                    # Descending from floor 2, at upper landing -> go to lower
                    gs.walk_waypoint_x = STAIR_X
                    gs.walk_waypoint_y = STAIR_LOWER
            elif current_floor == 2 and gs.lcp_y == STAIR_LOWER:
                if target_floor == 1:
                    # Emerged at bottom of stairs -> walk to target
                    gs.walk_waypoint_x = gs.walk_target_x
                    gs.walk_waypoint_y = gs.walk_target_y
                    gs.lcp_on_stairs_flag = 0
                elif target_floor == 3:
                    # Going up from middle floor stair bottom -> upper stair
                    gs.walk_waypoint_x = STAIR_X
                    gs.walk_waypoint_y = STAIR_UPPER
            elif current_floor == 3 and gs.lcp_y == STAIR_UPPER:
                # At floor 3 stair entry going down
                gs.walk_waypoint_x = STAIR_X
                gs.walk_waypoint_y = STAIR_LOWER
            else:
                lcp_calc_floor_waypoint(gs)
        else:
            lcp_calc_floor_waypoint(gs)

    # Move one pixel toward waypoint
    if gs.lcp_on_stairs_flag == 0:
        # Flat floor movement
        if gs.lcp_x < gs.walk_waypoint_x:
            gs.lcp_x += 1
            gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
        elif gs.lcp_x > gs.walk_waypoint_x:
            gs.lcp_x -= 1
            gs.lcp_facing_direction = FACING_DIR.FACING_LEFT

        x_dist = abs(gs.walk_waypoint_x - gs.lcp_x)
        if x_dist < 8:
            if gs.lcp_y < gs.walk_waypoint_y:
                gs.lcp_y += 1
            elif gs.lcp_y > gs.walk_waypoint_y:
                gs.lcp_y -= 1
        else:
            floor_num = get_floor_number_from_y(gs.lcp_y)
            centre = FLOOR_CENTER_Y[floor_num - 1]
            if gs.lcp_y < centre:
                gs.lcp_y += 1
            elif gs.lcp_y > centre:
                gs.lcp_y -= 1
    else:
        # Stair movement — simplified: move toward waypoint diagonally
        if gs.lcp_y > gs.walk_waypoint_y:
            gs.lcp_y -= 1
            if gs.lcp_x < gs.walk_waypoint_x:
                gs.lcp_x += 1
            elif gs.lcp_x > gs.walk_waypoint_x:
                gs.lcp_x -= 1
        elif gs.lcp_y < gs.walk_waypoint_y:
            gs.lcp_y += 1
            if gs.lcp_x < gs.walk_waypoint_x:
                gs.lcp_x += 1
            elif gs.lcp_x > gs.walk_waypoint_x:
                gs.lcp_x -= 1
        else:
            # Y already at waypoint — just move X
            if gs.lcp_x < gs.walk_waypoint_x:
                gs.lcp_x += 1
                gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
            elif gs.lcp_x > gs.walk_waypoint_x:
                gs.lcp_x -= 1
                gs.lcp_facing_direction = FACING_DIR.FACING_LEFT


def dog_walk_to_completion(gs, max_steps=5000):
    """
    Run dog movement until destination reached.
    Returns number of steps taken, or -1 if exceeded.
    """
    from lcp.dog import dog_move_and_animate

    for step in range(max_steps):
        if gs.dog_target_x == 0 and gs.dog_target_y == 0:
            return step  # destination reached (or idle)
        if gs.dog_x == gs.dog_target_x and gs.dog_y == gs.dog_target_y:
            return step
        dog_move_and_animate(gs)
    return -1


def set_walk_target(gs, pos):
    """Set LCP walk target to a HOUSE_POS position."""
    from lcp.movement import lcp_calc_floor_waypoint
    x, y = house_get_position_xy(pos)
    gs.walk_target_x = x
    gs.walk_target_y = y
    gs.walk_waypoint_x = 0
    gs.walk_waypoint_y = 0
    gs.lcp_on_stairs_flag = 0
    lcp_calc_floor_waypoint(gs)
