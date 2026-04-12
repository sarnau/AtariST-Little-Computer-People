"""
Comprehensive tests for the LCP (player) movement and pathfinding system.

Tests cover:
  - Floor detection from Y coordinates
  - Same-floor and cross-floor waypoint calculation
  - Exhaustive same-floor movement between all HOUSE_POS pairs
  - Representative cross-floor movement paths
  - Facing direction during movement
  - Single pathfind step behaviour
  - Stair flag management during floor transitions

Floor coordinate mapping (from house_get_position_xy):
  Positions 0-15  (POS_TOP_*): baseline Y=202, actual Y ~188-199 -> floor 1 (bottom of screen)
  Positions 16-31 (POS_MID_*): baseline Y=140, actual Y ~125-137 -> floor 2 (middle)
  Positions 32-46 (POS_BTM_*): baseline Y=77,  actual Y ~63-75  -> floor 3 (top of screen)
  Position  47    (POS_BTM_47): Y=79                             -> floor 2 (edge case)

get_floor_number_from_y returns: 1 (Y>=141), 2 (78<=Y<141), 3 (Y<78)
"""
import pytest

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))

from lcp.state import GameState
from lcp.structs import LCP
from lcp.enums import ACTION_ID, PLAYER_STATE, FACING_DIR, HOUSE_POS
from lcp.constants import (
    house_get_position_xy, FLOOR_BASELINE_Y,
    STAIRCASE_WAYPOINT_COORDS,
)
from lcp.movement import (
    lcp_calc_floor_waypoint, lcp_pathfind_one_step, get_floor_number_from_y,
    STAIR_X, STAIR_LOWER, STAIR_UPPER, FLOOR_CENTER_Y,
)

# Import helpers from conftest (module-level functions, not fixtures)
from conftest import set_walk_target, walk_to_completion


# ---------------------------------------------------------------------------
# Floor groups by actual computed floor number (from get_floor_number_from_y)
# ---------------------------------------------------------------------------
# Floor 1 (Y >= 141): positions 0-15
FLOOR_1_POSITIONS = list(range(0, 16))
# Floor 2 (78 <= Y < 141): positions 16-31 plus pos 47
FLOOR_2_POSITIONS = list(range(16, 32))
# Floor 3 (Y < 78): positions 32-46 (pos 47 is floor 2 due to Y=79)
FLOOR_3_POSITIONS = list(range(32, 47))


def _get_valid_pos_xy(pos):
    """Return (x, y) for a HOUSE_POS, or None if the position is (0, 0)."""
    x, y = house_get_position_xy(pos)
    if x == 0 and y == 0:
        return None
    return x, y


# ===================================================================
# 1. Floor detection -- get_floor_number_from_y()
# ===================================================================
class TestFloorDetection:
    """Verify get_floor_number_from_y returns correct floor for Y values."""

    def test_bottom_floor_high_y(self):
        """Y well within bottom floor returns floor 1."""
        assert get_floor_number_from_y(190) == 1

    def test_middle_floor(self):
        """Y in mid-range returns floor 2."""
        assert get_floor_number_from_y(120) == 2

    def test_top_floor(self):
        """Y near top of screen returns floor 3."""
        assert get_floor_number_from_y(50) == 3

    def test_boundary_141_is_bottom(self):
        """Y=141 is at the boundary -- should be floor 1 (bottom)."""
        assert get_floor_number_from_y(141) == 1

    def test_boundary_140_is_middle(self):
        """Y=140 is just above the boundary -- should be floor 2 (middle)."""
        assert get_floor_number_from_y(140) == 2

    def test_boundary_78_is_middle(self):
        """Y=78 is at the upper boundary -- should be floor 2 (middle)."""
        assert get_floor_number_from_y(78) == 2

    def test_boundary_77_is_top(self):
        """Y=77 is just above the boundary -- should be floor 3 (top)."""
        assert get_floor_number_from_y(77) == 3

    def test_extreme_bottom(self):
        """Y=250 (off screen low) still returns floor 1."""
        assert get_floor_number_from_y(250) == 1

    def test_extreme_top(self):
        """Y=0 (top of screen) returns floor 3."""
        assert get_floor_number_from_y(0) == 3

    @pytest.mark.parametrize("y,expected", [
        (200, 1), (141, 1), (140, 2), (130, 2),
        (100, 2), (78, 2), (77, 3), (50, 3), (10, 3),
    ])
    def test_parametrized_floor_values(self, y, expected):
        """Parametrized floor detection across the full Y range."""
        assert get_floor_number_from_y(y) == expected


# ===================================================================
# 2. Same-floor waypoint calculation
# ===================================================================
class TestSameFloorWaypoint:
    """When target is on the same floor, waypoint should equal the target."""

    def test_same_floor_1(self, gs):
        """Floor 1 (positions 0-15): waypoint equals target, no stair flag."""
        # gs starts at (160, 190) which is floor 1
        tx, ty = house_get_position_xy(HOUSE_POS.POS_TOP_FIREPLACE)  # pos 4, floor 1
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.walk_waypoint_x == tx
        assert gs.walk_waypoint_y == ty
        assert gs.lcp_on_stairs_flag == 0

    def test_same_floor_2(self, gs):
        """Floor 2 (positions 16-31): waypoint equals target, no stair flag."""
        gs.lcp_x = 100
        gs.lcp_y = 130  # floor 2
        tx, ty = house_get_position_xy(HOUSE_POS.POS_MID_BED)  # pos 17, floor 2
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.walk_waypoint_x == tx
        assert gs.walk_waypoint_y == ty
        assert gs.lcp_on_stairs_flag == 0

    def test_same_floor_3(self, gs):
        """Floor 3 (positions 32-46): waypoint equals target, no stair flag."""
        gs.lcp_x = 50
        gs.lcp_y = 65  # floor 3
        tx, ty = house_get_position_xy(38)  # pos 38, floor 3 (dog bowl area)
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.walk_waypoint_x == tx
        assert gs.walk_waypoint_y == ty
        assert gs.lcp_on_stairs_flag == 0


# ===================================================================
# 3. Cross-floor waypoint calculation
# ===================================================================
class TestCrossFloorWaypoint:
    """Cross-floor routing should direct through the staircase."""

    def test_floor1_to_floor2_routes_to_stair_lower(self, gs):
        """Floor 1 -> Floor 2: waypoint is staircase at STAIR_LOWER (Y=161)."""
        # gs at (160, 190) = floor 1; target on floor 2
        tx, ty = house_get_position_xy(HOUSE_POS.POS_MID_BED)  # floor 2
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.walk_waypoint_x == STAIR_X
        assert gs.walk_waypoint_y == STAIR_LOWER
        assert gs.lcp_on_stairs_flag == 1

    def test_floor1_to_floor3_routes_to_stair_lower(self, gs):
        """Floor 1 -> Floor 3: first waypoint is staircase at STAIR_LOWER."""
        tx, ty = house_get_position_xy(35)  # pos 35, floor 3
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.walk_waypoint_x == STAIR_X
        assert gs.walk_waypoint_y == STAIR_LOWER
        assert gs.lcp_on_stairs_flag == 1

    def test_floor2_to_floor3_routes_to_stair_upper(self, gs):
        """Floor 2 -> Floor 3: waypoint is staircase at STAIR_UPPER (Y=100)."""
        gs.lcp_x = 100
        gs.lcp_y = 130  # floor 2
        tx, ty = house_get_position_xy(33)  # pos 33, floor 3
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.walk_waypoint_x == STAIR_X
        assert gs.walk_waypoint_y == STAIR_UPPER
        assert gs.lcp_on_stairs_flag == 1

    def test_floor2_to_floor1_routes_to_stair_lower(self, gs):
        """Floor 2 -> Floor 1: waypoint is staircase at STAIR_LOWER (Y=161)."""
        gs.lcp_x = 100
        gs.lcp_y = 130  # floor 2
        tx, ty = house_get_position_xy(HOUSE_POS.POS_TOP_STUDY_DOOR)  # pos 7, floor 1
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.walk_waypoint_x == STAIR_X
        assert gs.walk_waypoint_y == STAIR_LOWER
        assert gs.lcp_on_stairs_flag == 1

    def test_floor3_to_floor1_routes_to_stair_upper(self, gs):
        """Floor 3 -> Floor 1: first waypoint is staircase at STAIR_UPPER."""
        gs.lcp_x = 50
        gs.lcp_y = 65  # floor 3
        tx, ty = house_get_position_xy(HOUSE_POS.POS_TOP_FIREPLACE)  # pos 4, floor 1
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.walk_waypoint_x == STAIR_X
        assert gs.walk_waypoint_y == STAIR_UPPER
        assert gs.lcp_on_stairs_flag == 1

    def test_floor3_to_floor2_routes_to_stair_upper(self, gs):
        """Floor 3 -> Floor 2: waypoint is staircase at STAIR_UPPER."""
        gs.lcp_x = 50
        gs.lcp_y = 65  # floor 3
        tx, ty = house_get_position_xy(HOUSE_POS.POS_MID_COUCH)  # pos 20, floor 2
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.walk_waypoint_x == STAIR_X
        assert gs.walk_waypoint_y == STAIR_UPPER
        assert gs.lcp_on_stairs_flag == 1

    def test_staircase_waypoint_coords_values(self):
        """Verify the STAIRCASE_WAYPOINT_COORDS constant values."""
        assert STAIRCASE_WAYPOINT_COORDS == [170, 185, 133, 124, 182, 72]


# ===================================================================
# 4. Same-floor movement -- exhaustive parametrized tests
# ===================================================================
def _build_same_floor_pairs(positions):
    """Generate all (start, end) pairs for positions on the same floor."""
    pairs = []
    for src in positions:
        src_xy = _get_valid_pos_xy(src)
        if src_xy is None:
            continue
        for dst in positions:
            if src == dst:
                continue
            dst_xy = _get_valid_pos_xy(dst)
            if dst_xy is None:
                continue
            pairs.append((src, dst))
    return pairs


_floor1_pairs = _build_same_floor_pairs(FLOOR_1_POSITIONS)
_floor2_pairs = _build_same_floor_pairs(FLOOR_2_POSITIONS)
_floor3_pairs = _build_same_floor_pairs(FLOOR_3_POSITIONS)


def _place_lcp_at_position(gs, pos):
    """Place the LCP at the given HOUSE_POS coordinates."""
    x, y = house_get_position_xy(pos)
    gs.lcp_x = x
    gs.lcp_y = y
    gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
    gs.lcp_on_stairs_flag = 0


class TestSameFloorMovementFloor1:
    """Exhaustive same-floor movement tests for floor 1 (positions 0-15)."""

    @pytest.mark.parametrize("src,dst", _floor1_pairs,
                             ids=[f"{s}->{d}" for s, d in _floor1_pairs])
    def test_walk_floor1(self, gs, src, dst):
        """LCP walks between two floor-1 positions."""
        _place_lcp_at_position(gs, src)
        set_walk_target(gs, dst)
        steps = walk_to_completion(gs)
        assert steps >= 0, f"Failed to reach pos {dst} from pos {src} within step limit"
        tx, ty = house_get_position_xy(dst)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty


class TestSameFloorMovementFloor2:
    """Exhaustive same-floor movement tests for floor 2 (positions 16-31)."""

    @pytest.mark.parametrize("src,dst", _floor2_pairs,
                             ids=[f"{s}->{d}" for s, d in _floor2_pairs])
    def test_walk_floor2(self, gs, src, dst):
        """LCP walks between two floor-2 positions."""
        _place_lcp_at_position(gs, src)
        set_walk_target(gs, dst)
        steps = walk_to_completion(gs)
        assert steps >= 0, f"Failed to reach pos {dst} from pos {src} within step limit"
        tx, ty = house_get_position_xy(dst)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty


class TestSameFloorMovementFloor3:
    """Exhaustive same-floor movement tests for floor 3 (positions 32-46)."""

    @pytest.mark.parametrize("src,dst", _floor3_pairs,
                             ids=[f"{s}->{d}" for s, d in _floor3_pairs])
    def test_walk_floor3(self, gs, src, dst):
        """LCP walks between two floor-3 positions."""
        _place_lcp_at_position(gs, src)
        set_walk_target(gs, dst)
        steps = walk_to_completion(gs)
        assert steps >= 0, f"Failed to reach pos {dst} from pos {src} within step limit"
        tx, ty = house_get_position_xy(dst)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty


# ===================================================================
# 5. Cross-floor movement tests (representative paths)
# ===================================================================
class TestCrossFloorMovement:
    """Test representative paths that cross floor boundaries."""

    def test_floor1_to_floor2_pos0_to_pos17(self, gs):
        """Floor 1 pos 0 -> Floor 2 pos 17 (bed)."""
        _place_lcp_at_position(gs, 0)
        set_walk_target(gs, 17)
        steps = walk_to_completion(gs)
        assert steps >= 0, "Failed to walk from floor 1 to floor 2"
        tx, ty = house_get_position_xy(17)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty

    def test_floor1_to_floor3_pos7_to_pos39(self, gs):
        """Floor 1 pos 7 (study door) -> Floor 3 pos 39 (front door area)."""
        _place_lcp_at_position(gs, 7)
        set_walk_target(gs, 39)
        steps = walk_to_completion(gs)
        assert steps >= 0, "Failed to walk from floor 1 to floor 3"
        tx, ty = house_get_position_xy(39)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty

    def test_floor3_to_floor1_pos35_to_pos4(self, gs):
        """Floor 3 pos 35 (fridge area) -> Floor 1 pos 4 (fireplace)."""
        _place_lcp_at_position(gs, 35)
        set_walk_target(gs, 4)
        steps = walk_to_completion(gs)
        assert steps >= 0, "Failed to walk from floor 3 to floor 1"
        tx, ty = house_get_position_xy(4)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty

    def test_floor2_to_floor3_pos24_to_pos40(self, gs):
        """Floor 2 pos 24 (computer) -> Floor 3 pos 40."""
        _place_lcp_at_position(gs, 24)
        set_walk_target(gs, 40)
        steps = walk_to_completion(gs)
        assert steps >= 0, "Failed to walk from floor 2 to floor 3"
        tx, ty = house_get_position_xy(40)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty

    def test_floor2_to_floor1_pos20_to_pos12(self, gs):
        """Floor 2 pos 20 (couch) -> Floor 1 pos 12 (record shelf)."""
        _place_lcp_at_position(gs, 20)
        set_walk_target(gs, 12)
        steps = walk_to_completion(gs)
        assert steps >= 0, "Failed to walk from floor 2 to floor 1"
        tx, ty = house_get_position_xy(12)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty


# ===================================================================
# 6. Facing direction tests
# ===================================================================
class TestFacingDirection:
    """LCP should face right when moving right, left when moving left."""

    def test_faces_right_when_walking_right(self, gs):
        """LCP placed left of target should face right after a step."""
        gs.lcp_x = 50
        gs.lcp_y = 190
        gs.walk_target_x = 200
        gs.walk_target_y = 190
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        old_x = gs.lcp_x
        from conftest import _pathfind_step_no_tick
        _pathfind_step_no_tick(gs)

        assert gs.lcp_x > old_x, "LCP should have moved right"
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT

    def test_faces_left_when_walking_left(self, gs):
        """LCP placed right of target should face left after a step."""
        gs.lcp_x = 200
        gs.lcp_y = 190
        gs.walk_target_x = 50
        gs.walk_target_y = 190
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        old_x = gs.lcp_x
        from conftest import _pathfind_step_no_tick
        _pathfind_step_no_tick(gs)

        assert gs.lcp_x < old_x, "LCP should have moved left"
        assert gs.lcp_facing_direction == FACING_DIR.FACING_LEFT

    def test_facing_during_rightward_same_floor_walk(self, gs):
        """During a rightward walk across floor 1, facing should be right."""
        # pos 0 is at X=44, pos 15 is at X=292 -- both floor 1
        _place_lcp_at_position(gs, 0)
        tx, ty = house_get_position_xy(15)
        assert tx > gs.lcp_x, "Target should be to the right of start"
        set_walk_target(gs, 15)

        from conftest import _pathfind_step_no_tick
        for _ in range(5):
            _pathfind_step_no_tick(gs)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT

    def test_facing_during_leftward_same_floor_walk(self, gs):
        """During a leftward walk across floor 1, facing should be left."""
        _place_lcp_at_position(gs, 15)  # X=292
        tx, ty = house_get_position_xy(0)  # X=44
        assert tx < gs.lcp_x, "Target should be to the left of start"
        set_walk_target(gs, 0)

        from conftest import _pathfind_step_no_tick
        for _ in range(5):
            _pathfind_step_no_tick(gs)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_LEFT


# ===================================================================
# 7. Pathfind single step tests
# ===================================================================
class TestPathfindSingleStep:
    """Verify a single pathfind step moves LCP by a small increment."""

    def test_single_step_changes_position(self, gs):
        """One step should move LCP by 1-2 pixels."""
        gs.lcp_x = 100
        gs.lcp_y = 190
        gs.walk_target_x = 200
        gs.walk_target_y = 190
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        old_x = gs.lcp_x
        old_y = gs.lcp_y
        from conftest import _pathfind_step_no_tick
        _pathfind_step_no_tick(gs)

        dx = abs(gs.lcp_x - old_x)
        dy = abs(gs.lcp_y - old_y)
        total = dx + dy
        assert total >= 1, "Position should change by at least 1 pixel"
        assert total <= 3, "Position should change by at most a few pixels per step"

    def test_single_step_x_moves_toward_target(self, gs):
        """After one step, X should be closer to the target."""
        gs.lcp_x = 100
        gs.lcp_y = 190
        gs.walk_target_x = 200
        gs.walk_target_y = 190
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        old_dist = abs(gs.walk_target_x - gs.lcp_x)
        from conftest import _pathfind_step_no_tick
        _pathfind_step_no_tick(gs)
        new_dist = abs(gs.walk_target_x - gs.lcp_x)

        assert new_dist < old_dist, "Should be closer to target after one step"

    def test_multiple_steps_progress(self, gs):
        """After 10 steps, LCP should have moved significantly toward target."""
        gs.lcp_x = 50
        gs.lcp_y = 190
        gs.walk_target_x = 250
        gs.walk_target_y = 190
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        start_x = gs.lcp_x
        from conftest import _pathfind_step_no_tick
        for _ in range(10):
            _pathfind_step_no_tick(gs)

        assert gs.lcp_x > start_x + 5, "Should have progressed at least 5 pixels in 10 steps"

    def test_y_converges_toward_floor_center(self, gs):
        """When far from waypoint, Y should drift toward floor center line."""
        gs.lcp_x = 50
        gs.lcp_y = 199  # below floor 1 center (198)
        gs.walk_target_x = 250
        gs.walk_target_y = 198
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        from conftest import _pathfind_step_no_tick
        _pathfind_step_no_tick(gs)

        # Y should move toward the center (198) from 199
        assert gs.lcp_y <= 199, "Y should converge toward floor center"


# ===================================================================
# 8. Stair flag management
# ===================================================================
class TestStairFlagManagement:
    """Verify lcp_on_stairs_flag is set for cross-floor and cleared on arrival."""

    def test_stair_flag_set_for_cross_floor_target(self, gs):
        """Setting a cross-floor target should set lcp_on_stairs_flag."""
        # gs at (160, 190) = floor 1; target on floor 2
        tx, ty = house_get_position_xy(HOUSE_POS.POS_MID_BED)  # floor 2
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.lcp_on_stairs_flag == 1

    def test_stair_flag_not_set_for_same_floor_target(self, gs):
        """Setting a same-floor target should not set lcp_on_stairs_flag."""
        # gs at (160, 190) = floor 1; target also on floor 1
        tx, ty = house_get_position_xy(HOUSE_POS.POS_TOP_FIREPLACE)  # pos 4, floor 1
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.lcp_on_stairs_flag == 0

    def test_stair_flag_cleared_on_same_floor_arrival(self, gs):
        """After completing a same-floor walk, stair flag should be 0."""
        _place_lcp_at_position(gs, 0)
        set_walk_target(gs, 4)
        walk_to_completion(gs)

        assert gs.lcp_on_stairs_flag == 0

    def test_stair_flag_cleared_after_cross_floor_arrival(self, gs):
        """After completing a cross-floor walk, stair flag should be 0."""
        _place_lcp_at_position(gs, 0)  # floor 1
        set_walk_target(gs, 17)  # floor 2
        steps = walk_to_completion(gs)
        assert steps >= 0, "Walk should complete"
        assert gs.lcp_on_stairs_flag == 0, "Stair flag should be cleared on arrival"

    def test_stair_flag_active_during_cross_floor_walk(self, gs):
        """During a cross-floor walk, stair flag should be set at some point."""
        _place_lcp_at_position(gs, 0)  # floor 1

        tx, ty = house_get_position_xy(17)  # floor 2
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        # Should be set immediately after waypoint calc for cross-floor
        assert gs.lcp_on_stairs_flag == 1

        # Walk partway and verify it was set at some point
        from conftest import _pathfind_step_no_tick
        stair_flag_observed = gs.lcp_on_stairs_flag == 1
        for _ in range(500):
            if gs.lcp_x == tx and gs.lcp_y == ty:
                break
            _pathfind_step_no_tick(gs)
            if gs.lcp_on_stairs_flag == 1:
                stair_flag_observed = True

        assert stair_flag_observed, "Stair flag should be set during cross-floor movement"

    def test_stair_flag_transitions_floor1_to_floor3(self, gs):
        """Walking floor 1 -> floor 3, stair flag cleared on arrival."""
        _place_lcp_at_position(gs, 7)  # pos 7, floor 1
        set_walk_target(gs, 39)  # pos 39, floor 3
        steps = walk_to_completion(gs)
        assert steps >= 0, "Walk should complete"
        assert gs.lcp_on_stairs_flag == 0, "Stair flag should be cleared on arrival"
        tx, ty = house_get_position_xy(39)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty

    def test_stair_flag_initial_calc_floor2_to_floor3(self, gs):
        """Floor 2 -> Floor 3: initial waypoint calc sets stair flag."""
        gs.lcp_x = 100
        gs.lcp_y = 130  # floor 2
        tx, ty = house_get_position_xy(35)  # floor 3
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.lcp_on_stairs_flag == 1
        assert gs.walk_waypoint_x == STAIR_X
        assert gs.walk_waypoint_y == STAIR_UPPER

    def test_stair_flag_initial_calc_floor3_to_floor2(self, gs):
        """Floor 3 -> Floor 2: initial waypoint calc sets stair flag."""
        gs.lcp_x = 50
        gs.lcp_y = 65  # floor 3
        tx, ty = house_get_position_xy(20)  # floor 2
        gs.walk_target_x = tx
        gs.walk_target_y = ty
        gs.walk_waypoint_x = 0
        gs.walk_waypoint_y = 0
        gs.lcp_on_stairs_flag = 0
        lcp_calc_floor_waypoint(gs)

        assert gs.lcp_on_stairs_flag == 1
        assert gs.walk_waypoint_x == STAIR_X
        assert gs.walk_waypoint_y == STAIR_UPPER


# ===================================================================
# Edge case: Position 47 (Y=79, floor 2 despite POS_BTM_ name)
# ===================================================================
class TestPosition47EdgeCase:
    """Position 47 has Y=79 which falls into floor 2 despite POS_BTM_ prefix."""

    def test_pos47_is_floor2(self):
        """Position 47 should be detected as floor 2."""
        _, y = house_get_position_xy(47)
        assert get_floor_number_from_y(y) == 2

    def test_walk_between_pos47_and_floor2_position(self, gs):
        """Walking from pos 47 to another floor 2 position should work as same-floor."""
        _place_lcp_at_position(gs, 47)
        set_walk_target(gs, 20)  # pos 20, floor 2
        steps = walk_to_completion(gs)
        assert steps >= 0, "Walk from pos 47 to floor 2 pos should succeed"
        tx, ty = house_get_position_xy(20)
        assert gs.lcp_x == tx
        assert gs.lcp_y == ty
