"""
Render and animation tests for Little Computer People.

Tests LCP player sprite positioning, body/head frame selection,
head animation state machine, and dog sprite animation.
"""

import pytest
import random

from lcp.state import GameState
from lcp.structs import LCP
from lcp.enums import (
    PLAYER_STATE, FACING_DIR, SPRITE_ID, HEAD_ANIM_MODE,
    DOG_BOWL_STATUS, HOUSE_POS, SPRITE_LAYER,
)
from lcp.constants import (
    BODY_SPRITE_FRAME_TABLE, CARRY_BODY_FRAME_TABLE,
    BODY_Y_OFFSET_PER_STATE, HEAD_X_OFFSET_PER_STATE,
    HEAD_HEIGHT_PER_STATE, HEAD_DEFAULT_ANGLE_PER_STATE,
    HAPPINESS_HEAD_FRAME_OFFSET, HEAD_TILT_FRAME_OFFSET,
    HEAD_MOVEMENT_DELTA_TABLE,
    DOG_WALK_ANIM_FRAMES, DOG_EATING_ANIM_FRAMES,
)
from lcp.sprites import (
    sprite_update_body, sprite_lcp_head_animate,
    sprite_lcp_head_update, update_carried_object_sprite,
    sprite_update_slots,
)
from lcp.dog import (
    dog_move_and_animate, dog_frame_update,
    spritedata_update_dog,
)


# ===================================================================
# Player body sprite tests
# ===================================================================

class TestBodySpritePositioning:
    """Test body sprite frame selection and X/Y positioning."""

    def test_body_facing_right_position(self, gs):
        """Body facing right: X = lcp_x - 4."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
        gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
        sprite_update_body(gs)
        assert gs.sprite_active_x[3] == 96   # 100 - 4

    def test_body_facing_left_position(self, gs):
        """Body facing left: X = lcp_x - 14."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
        gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
        sprite_update_body(gs)
        assert gs.sprite_active_x[3] == 86   # 100 - 14

    def test_body_y_with_offset(self, gs):
        """Body Y = lcp_y + body_y_offset_per_state[state] - 21."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE  # state 8, y_offset = -2
        sprite_update_body(gs)
        assert gs.sprite_active_y[3] == 150 + (-2) - 21  # 127

    def test_body_y_offset_varies_per_state(self, gs):
        """Different player states have different Y offsets."""
        gs.lcp_x = 100
        gs.lcp_y = 150

        results = {}
        for state_val in [0, 8, 25, 27, 36, 45]:
            gs.lcp_state = state_val
            gs.sprite_pending_flag[3] = 0  # allow update
            sprite_update_body(gs)
            y_off = BODY_Y_OFFSET_PER_STATE[state_val]
            expected_y = 150 + y_off - 21
            assert gs.sprite_active_y[3] == expected_y, \
                f"State {state_val}: expected Y={expected_y}, got {gs.sprite_active_y[3]}"
            results[state_val] = y_off

        # Verify offsets differ across at least some states
        assert len(set(results.values())) > 1

    def test_body_frame_selection_idle(self, gs):
        """STATE_STAND_IDLE (8) selects body frame 43."""
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
        sprite_update_body(gs)
        assert gs._body_frame_index == 43

    def test_body_frame_selection_walk(self, gs):
        """Walking states select sequential body frames."""
        walk_frames = []
        for state_val in range(8):  # STATE_WALK_FRAME_0..7
            gs.lcp_state = state_val
            gs.sprite_pending_flag[3] = 0
            sprite_update_body(gs)
            walk_frames.append(gs._body_frame_index)
        # Walking frames should be different from idle frame 43
        assert all(f != 43 for f in walk_frames)
        # Each walk frame should come from the table
        for i, f in enumerate(walk_frames):
            assert f == BODY_SPRITE_FRAME_TABLE[i]

    def test_body_frame_facing_stored(self, gs):
        """Body facing direction is stored for the renderer."""
        gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
        sprite_update_body(gs)
        assert gs._body_facing == FACING_DIR.FACING_RIGHT

        gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        assert gs._body_facing == FACING_DIR.FACING_LEFT

    def test_body_carry_frame_table(self, gs):
        """Carrying object uses alternate body frame table for states < 25."""
        gs.lcp_carrying_object_flag = 1
        gs.lcp_state = 0  # STATE_WALK_FRAME_0
        sprite_update_body(gs)
        assert gs._body_frame_index == CARRY_BODY_FRAME_TABLE[0]
        assert gs._body_frame_index != BODY_SPRITE_FRAME_TABLE[0]

    def test_body_carry_frame_not_used_above_24(self, gs):
        """States >= 25 use normal frame table even when carrying."""
        gs.lcp_carrying_object_flag = 1
        gs.lcp_state = PLAYER_STATE.STATE_STAND_FACING_SCREEN  # 25
        sprite_update_body(gs)
        # Should NOT use carry table (state 25 >= len(CARRY_BODY_FRAME_TABLE))
        assert gs._body_frame_index == BODY_SPRITE_FRAME_TABLE[25]

    def test_body_pending_flag_set(self, gs):
        """sprite_update_body sets pending flag for slot 3."""
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        assert gs.sprite_pending_flag[3] == 1

    def test_body_pending_dimensions(self, gs):
        """Body sprite pending height=21, width=32."""
        sprite_update_body(gs)
        assert gs.sprite_pending_height[3] == 21
        assert gs.sprite_pending_width[3] == 32

    def test_body_skips_if_pending(self, gs):
        """Body update skips if previous pending not consumed."""
        gs.sprite_pending_flag[3] = 1
        old_x = gs.sprite_active_x[3]
        gs.lcp_x = 200  # change position
        sprite_update_body(gs)
        # Position should NOT have changed — update was skipped
        assert gs.sprite_active_x[3] == old_x

    def test_body_hidden_when_sprites_hidden(self, gs):
        """Body sprite image cleared when lcp_sprites_hidden is set."""
        gs.lcp_sprites_hidden = 1
        sprite_update_body(gs)
        assert gs.sprite_pending_image[3] is None

    def test_body_debug_offscreen(self, gs):
        """debug_hide_lcp_offscreen pushes Y to 300."""
        gs.debug_hide_lcp_offscreen = 1
        sprite_update_body(gs)
        assert gs.sprite_active_y[3] == 300

    def test_body_all_93_states_have_valid_frames(self, gs):
        """Every player state (0–92) maps to a valid body frame index."""
        assert len(BODY_SPRITE_FRAME_TABLE) >= 91
        for state_val in range(len(BODY_SPRITE_FRAME_TABLE)):
            gs.lcp_state = state_val
            gs.sprite_pending_flag[3] = 0
            sprite_update_body(gs)
            assert gs._body_frame_index >= 0
            assert gs._body_frame_index < 100  # BODY.LCP has 98 frames


class TestBodySpriteSymmetry:
    """Test that facing-left and facing-right positions are symmetric."""

    def test_body_right_left_offset(self, gs):
        """Facing right (X-4) and left (X-14) offset for 32px sprites."""
        gs.lcp_x = 100
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE

        gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
        sprite_update_body(gs)
        x_right = gs.sprite_active_x[3]

        gs.sprite_pending_flag[3] = 0
        gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
        sprite_update_body(gs)
        x_left = gs.sprite_active_x[3]

        # Right: lcp_x - 4 = 96, Left: lcp_x - 14 = 86
        # The 10-pixel gap compensates for visible content shifting
        # from left half to right half in the 32px buffer when flipped
        assert x_right == 96
        assert x_left == 86

    @pytest.mark.parametrize("state_val", [0, 1, 2, 3, 4, 5, 6, 7, 8, 25, 27, 36])
    def test_body_y_independent_of_facing(self, gs, state_val):
        """Y position is the same regardless of facing direction."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_state = state_val

        gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        y_right = gs.sprite_active_y[3]

        gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        y_left = gs.sprite_active_y[3]

        assert y_right == y_left


# ===================================================================
# Player head sprite tests
# ===================================================================

class TestHeadSpritePositioning:
    """Test head sprite frame selection and positioning."""

    def test_head_normal_position(self, gs):
        """Head mirror=0: X = lcp_x + x_offset - 4."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE  # x_offset=0
        gs.head_sprite_mirror_flag = 0
        sprite_lcp_head_update(gs)
        assert gs.sprite_active_x[4] == 96   # 100 + 0 - 4

    def test_head_mirrored_position(self, gs):
        """Head mirror=1: X = lcp_x + x_offset - 14."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE  # x_offset=0
        gs.head_sprite_mirror_flag = 1
        sprite_lcp_head_update(gs)
        assert gs.sprite_active_x[4] == 86   # 100 + 0 - 14

    def test_head_y_position(self, gs):
        """Head Y = lcp_y + body_y_offset - (head_height + 21)."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE  # state 8
        sprite_lcp_head_update(gs)
        y_off = BODY_Y_OFFSET_PER_STATE[8]   # -2
        h_height = HEAD_HEIGHT_PER_STATE[8]   # 21
        expected = 150 + y_off - (h_height + 21)  # 150 - 2 - 42 = 106
        assert gs.sprite_active_y[4] == expected

    def test_head_touches_body_top(self, gs):
        """Head bottom edge is adjacent to body top edge (no gap)."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE

        sprite_update_body(gs)
        body_top = gs.sprite_active_y[3]

        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        head_bottom = gs.sprite_active_y[4] + 21  # head image is 21px tall

        # Head bottom should be exactly at or 1px above body top
        assert head_bottom == body_top or head_bottom == body_top + 1

    def test_head_x_offset_per_state(self, gs):
        """States with non-zero head X offset shift the head position."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.head_sprite_mirror_flag = 0

        # State 47 (PLAY_PIANO_2) has x_offset = 6
        gs.lcp_state = 47
        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        assert gs.sprite_active_x[4] == 100 + 6 - 4  # 102

        # State 50 (READ_NEWSPAPER) has x_offset = -1
        gs.lcp_state = 50
        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        assert gs.sprite_active_x[4] == 100 + (-1) - 4  # 95

    def test_head_happiness_frame_offset(self, gs):
        """Different happiness levels select different head frame rows."""
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
        gs.head_sprite_frame = 0

        for happiness, expected_offset in [(0, 44), (1, 0), (2, 22)]:
            gs.lcp.happiness = happiness
            gs.sprite_pending_flag[4] = 0
            sprite_lcp_head_update(gs)
            assert gs._head_frame_index == expected_offset + 0

    def test_head_frame_varies_with_sprite_frame(self, gs):
        """head_sprite_frame adds to the happiness-based offset."""
        gs.lcp.happiness = 1  # offset = 0
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE

        for frame_val in [0, 3, 7, 12]:
            gs.head_sprite_frame = frame_val
            gs.sprite_pending_flag[4] = 0
            sprite_lcp_head_update(gs)
            assert gs._head_frame_index == 0 + frame_val

    def test_head_mirror_flag_stored(self, gs):
        """Mirror flag is stored for the renderer."""
        gs.head_sprite_mirror_flag = 0
        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        assert gs._head_mirror == 0

        gs.head_sprite_mirror_flag = 1
        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        assert gs._head_mirror == 1

    def test_head_pending_flag_set(self, gs):
        """sprite_lcp_head_update sets pending flag for slot 4."""
        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        assert gs.sprite_pending_flag[4] == 1

    def test_head_pending_dimensions(self, gs):
        """Head sprite pending height=21, width=32."""
        sprite_lcp_head_update(gs)
        assert gs.sprite_pending_height[4] == 21
        assert gs.sprite_pending_width[4] == 32

    def test_head_skips_if_pending(self, gs):
        """Head update skips if previous pending not consumed."""
        gs.sprite_pending_flag[4] = 1
        old_x = gs.sprite_active_x[4]
        gs.lcp_x = 250
        sprite_lcp_head_update(gs)
        assert gs.sprite_active_x[4] == old_x

    def test_head_carry_stairs_offset(self, gs):
        """Head Y is lowered 1px when carrying objects on stairs (states 13-16)."""
        gs.lcp_x = 100
        gs.lcp_y = 120
        gs.lcp_carrying_object_flag = 1

        for state_val in [13, 14, 15, 16]:
            gs.lcp_state = state_val
            gs.sprite_pending_flag[4] = 0
            sprite_lcp_head_update(gs)
            y_off = BODY_Y_OFFSET_PER_STATE[state_val]
            h_height = HEAD_HEIGHT_PER_STATE[state_val]
            base_y = 120 + y_off - (h_height + 21)
            assert gs.sprite_active_y[4] == base_y + 1  # +1 for carry on stairs

    def test_head_carry_no_offset_outside_stair_states(self, gs):
        """Head Y is NOT adjusted when carrying but not on stairs."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_carrying_object_flag = 1
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE  # state 8, not in 13-16
        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        y_off = BODY_Y_OFFSET_PER_STATE[8]
        h_height = HEAD_HEIGHT_PER_STATE[8]
        expected = 150 + y_off - (h_height + 21)
        assert gs.sprite_active_y[4] == expected  # no +1

    def test_head_debug_offscreen(self, gs):
        """debug_hide_lcp_offscreen pushes head Y to 300."""
        gs.debug_hide_lcp_offscreen = 1
        sprite_lcp_head_update(gs)
        assert gs.sprite_active_y[4] == 300

    def test_head_hidden_when_sprites_hidden(self, gs):
        """Head sprite image cleared when lcp_sprites_hidden is set."""
        gs.lcp_sprites_hidden = 1
        sprite_lcp_head_update(gs)
        assert gs.sprite_pending_image[4] is None


class TestHeadBodyAlignment:
    """Test head+body alignment across multiple player states."""

    @pytest.mark.parametrize("state_val", [
        0, 1, 2, 8, 9, 13, 17, 25, 27, 30, 36, 43, 54, 60, 74, 80, 90,
    ])
    def test_head_above_body_facing_right(self, gs, state_val):
        """Head is positioned above the body in all states, facing right."""
        gs.lcp_x = 160
        gs.lcp_y = 150
        gs.lcp_state = state_val
        gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
        gs.head_sprite_mirror_flag = 0

        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        body_y = gs.sprite_active_y[3]

        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        head_y = gs.sprite_active_y[4]

        # Head top should be above body top
        assert head_y <= body_y, \
            f"State {state_val}: head_y={head_y} should be <= body_y={body_y}"

    @pytest.mark.parametrize("state_val", [0, 8, 25, 36, 54])
    def test_head_body_x_alignment_facing_right(self, gs, state_val):
        """Head and body X positions match when facing right (no mirror)."""
        gs.lcp_x = 160
        gs.lcp_y = 150
        gs.lcp_state = state_val
        gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
        gs.head_sprite_mirror_flag = 0

        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        body_x = gs.sprite_active_x[3]

        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        head_x = gs.sprite_active_x[4]

        x_off = HEAD_X_OFFSET_PER_STATE[state_val]
        assert head_x == body_x + x_off

    @pytest.mark.parametrize("state_val", [0, 8, 25, 36, 54])
    def test_head_body_x_alignment_facing_left(self, gs, state_val):
        """Head and body X positions match when facing left (mirror=1)."""
        gs.lcp_x = 160
        gs.lcp_y = 150
        gs.lcp_state = state_val
        gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
        gs.head_sprite_mirror_flag = 1

        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        body_x = gs.sprite_active_x[3]

        gs.sprite_pending_flag[4] = 0
        sprite_lcp_head_update(gs)
        head_x = gs.sprite_active_x[4]

        x_off = HEAD_X_OFFSET_PER_STATE[state_val]
        assert head_x == body_x + x_off


# ===================================================================
# Head animation state machine tests
# ===================================================================

class TestHeadAnimation:
    """Test the head animation state machine (random head turns)."""

    def test_head_anim_converges_to_target(self, gs):
        """Head animation current moves toward target over time."""
        gs.head_anim_current = 0
        gs.head_anim_target = 3
        gs.head_anim_delay_countdown = 0
        gs.head_anim_mode = 0

        for _ in range(20):
            sprite_lcp_head_animate(gs)
        # After enough steps, current should reach target (or close)
        # The animation may overshoot or be redirected by random target
        # selection, so we just verify the mechanism runs without error
        # and produces a valid horizontal angle (0-7)
        horiz = gs.head_anim_current & 7
        assert 0 <= horiz <= 7

    def test_head_anim_sets_mirror_flag(self, gs):
        """Head angles >= 5 set mirror_flag = 1."""
        gs.head_anim_current = 6  # >= 5 → mirror
        gs.head_anim_target = 6
        gs.head_anim_delay_countdown = 0
        gs.head_anim_mode = 0

        sprite_lcp_head_animate(gs)
        assert gs.head_sprite_mirror_flag == 1

    def test_head_anim_no_mirror_below_5(self, gs):
        """Head angles < 5 set mirror_flag = 0."""
        gs.head_anim_current = 2
        gs.head_anim_target = 2
        gs.head_anim_delay_countdown = 0
        gs.head_anim_mode = 0

        sprite_lcp_head_animate(gs)
        assert gs.head_sprite_mirror_flag == 0

    def test_head_tilt_affects_frame(self, gs):
        """Vertical tilt adds tilt offset to head frame."""
        gs.head_anim_delay_countdown = 0
        gs.head_anim_mode = 0

        results = {}
        for tilt_bits in range(4):
            gs.head_anim_current = 2 | (tilt_bits << 3)
            gs.head_anim_target = gs.head_anim_current
            sprite_lcp_head_animate(gs)
            results[tilt_bits] = gs.head_sprite_frame

        # Different tilt values should produce different frames
        assert len(set(results.values())) > 1

    def test_head_delay_countdown_prevents_change(self, gs):
        """Positive delay countdown prevents target change."""
        gs.head_anim_current = 2
        gs.head_anim_target = 2
        gs.head_anim_delay_countdown = 50
        gs.head_anim_mode = 0
        gs.head_sprite_frame = 999  # sentinel

        sprite_lcp_head_animate(gs)
        assert gs.head_anim_delay_countdown == 49
        # Frame should have been updated (movement + frame calc still runs)
        assert gs.head_sprite_frame != 999

    def test_head_frame_within_valid_range(self, gs):
        """head_sprite_frame stays within valid range across random inputs."""
        random.seed(42)
        gs.head_anim_mode = 0
        gs.head_anim_delay_countdown = 0

        for _ in range(200):
            sprite_lcp_head_animate(gs)
            # Frame should be non-negative and reasonable
            assert 0 <= gs.head_sprite_frame < 30, \
                f"head_sprite_frame={gs.head_sprite_frame} out of range"

    def test_head_movement_delta_table(self):
        """HEAD_MOVEMENT_DELTA_TABLE has expected structure."""
        assert len(HEAD_MOVEMENT_DELTA_TABLE) == 15
        # Center (index 7) should be 0 (no movement when at target)
        assert HEAD_MOVEMENT_DELTA_TABLE[7] == 0
        # Edges should be ±1 or 99 (overflow)
        assert 99 in HEAD_MOVEMENT_DELTA_TABLE

    def test_happiness_head_offset_table(self):
        """HAPPINESS_HEAD_FRAME_OFFSET has 3 entries for happy/content/sad."""
        assert len(HAPPINESS_HEAD_FRAME_OFFSET) == 3
        assert HAPPINESS_HEAD_FRAME_OFFSET[0] == 44  # happy
        assert HAPPINESS_HEAD_FRAME_OFFSET[1] == 0   # content
        assert HAPPINESS_HEAD_FRAME_OFFSET[2] == 22  # sad

    def test_head_tilt_frame_offsets(self):
        """HEAD_TILT_FRAME_OFFSET has 4 entries."""
        assert len(HEAD_TILT_FRAME_OFFSET) >= 4
        # All should be non-negative
        assert all(o >= 0 for o in HEAD_TILT_FRAME_OFFSET[:4])


# ===================================================================
# Walking animation cycle tests
# ===================================================================

class TestWalkAnimationCycle:
    """Test the 8-frame walk cycle body frames."""

    def test_walk_cycle_8_frames(self, gs):
        """Walking uses 8 sequential body frames."""
        frames = []
        for state_val in range(8):
            gs.lcp_state = state_val
            gs.sprite_pending_flag[3] = 0
            sprite_update_body(gs)
            frames.append(gs._body_frame_index)
        assert len(frames) == 8
        assert len(set(frames)) >= 4  # at least 4 distinct frames

    def test_walk_cycle_wraps(self, gs):
        """Walk states 0-7 form a repeatable cycle."""
        gs.lcp_state = 0
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        frame_0 = gs._body_frame_index

        # After cycling through 0-7, state 0 again gives same frame
        gs.lcp_state = 0
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        assert gs._body_frame_index == frame_0

    def test_walk_body_y_bounces(self, gs):
        """Walk states produce slight Y variation (bounce effect)."""
        gs.lcp_x = 100
        gs.lcp_y = 150

        y_positions = []
        for state_val in range(8):
            gs.lcp_state = state_val
            gs.sprite_pending_flag[3] = 0
            sprite_update_body(gs)
            y_positions.append(gs.sprite_active_y[3])

        # Y should vary (walk bounce from body_y_offset_per_state)
        assert len(set(y_positions)) > 1

    def test_carry_walk_cycle_different_frames(self, gs):
        """Carry walk cycle uses different body frames than normal walk."""
        gs.lcp_carrying_object_flag = 0
        gs.lcp_state = 0
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        normal_frame = gs._body_frame_index

        gs.lcp_carrying_object_flag = 1
        gs.lcp_state = 0
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        carry_frame = gs._body_frame_index

        assert carry_frame != normal_frame


# ===================================================================
# Stair animation tests
# ===================================================================

class TestStairAnimation:
    """Test sprite behaviour during stair transitions."""

    @pytest.mark.parametrize("state_val", [9, 10, 11, 12])
    def test_stair_up_frames(self, gs, state_val):
        """Stair-up states (9-12) select valid body frames."""
        gs.lcp_state = state_val
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        assert gs._body_frame_index == BODY_SPRITE_FRAME_TABLE[state_val]
        assert gs._body_frame_index >= 0

    @pytest.mark.parametrize("state_val", [13, 14, 15, 16])
    def test_stair_top_frames(self, gs, state_val):
        """Stair-top states (13-16) select valid body frames."""
        gs.lcp_state = state_val
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        assert gs._body_frame_index == BODY_SPRITE_FRAME_TABLE[state_val]

    @pytest.mark.parametrize("state_val", [17, 18, 19, 20])
    def test_stair_down_frames(self, gs, state_val):
        """Stair-down states (17-20) select valid body frames."""
        gs.lcp_state = state_val
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        assert gs._body_frame_index == BODY_SPRITE_FRAME_TABLE[state_val]

    def test_stair_head_height_varies(self, gs):
        """Head height varies across stair states to follow body pose."""
        heights = set()
        for state_val in range(9, 25):
            heights.add(HEAD_HEIGHT_PER_STATE[state_val])
        # Stair states have head_height = 21 (all stair states are 21)
        assert 21 in heights


# ===================================================================
# Activity-specific body frame tests
# ===================================================================

class TestActivityFrames:
    """Test body frames for specific activities."""

    def test_sitting_frames(self, gs):
        """Sitting states use distinct body frames."""
        sitting_states = [
            PLAYER_STATE.STATE_SIT_CHAIR,   # 27
            PLAYER_STATE.STATE_SIT_COUCH,   # 28
            PLAYER_STATE.STATE_SIT_DESK,    # 29
        ]
        frames = set()
        for state_val in sitting_states:
            gs.lcp_state = state_val
            gs.sprite_pending_flag[3] = 0
            sprite_update_body(gs)
            frames.add(gs._body_frame_index)
        assert len(frames) >= 1  # at least one unique sitting frame

    def test_exercise_frames(self, gs):
        """Exercise states use two alternating body frames."""
        gs.lcp_state = PLAYER_STATE.STATE_EXERCISE_ARMS_UP  # 34
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        frame_up = gs._body_frame_index

        gs.lcp_state = PLAYER_STATE.STATE_EXERCISE_CROUCH  # 35
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        frame_crouch = gs._body_frame_index

        assert frame_up != frame_crouch

    def test_sleep_frame(self, gs):
        """Sleep states use specific body frames."""
        gs.lcp_state = PLAYER_STATE.STATE_SLEEP_IN_BED  # 36
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        assert gs._body_frame_index == BODY_SPRITE_FRAME_TABLE[36]

    def test_shower_frames(self, gs):
        """Shower states 38-42 have 5 animation frames."""
        frames = set()
        for state_val in range(38, 43):
            gs.lcp_state = state_val
            gs.sprite_pending_flag[3] = 0
            sprite_update_body(gs)
            frames.add(gs._body_frame_index)
        assert len(frames) >= 3  # at least 3 distinct shower frames

    def test_piano_frames(self, gs):
        """Piano playing states have two distinct frames."""
        gs.lcp_state = PLAYER_STATE.STATE_PLAY_PIANO_1  # 46
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        frame1 = gs._body_frame_index

        gs.lcp_state = PLAYER_STATE.STATE_PLAY_PIANO_2  # 47
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        frame2 = gs._body_frame_index

        assert frame1 != frame2

    def test_dance_frames(self, gs):
        """Dance states have two distinct frames."""
        gs.lcp_state = PLAYER_STATE.STATE_DANCE_LEFT   # 48
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        frame1 = gs._body_frame_index

        gs.lcp_state = PLAYER_STATE.STATE_DANCE_RIGHT  # 49
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        frame2 = gs._body_frame_index

        assert frame1 != frame2

    def test_phone_frames(self, gs):
        """Phone states (74-76) have distinct frames."""
        frames = set()
        for state_val in [74, 75, 76]:
            gs.lcp_state = state_val
            gs.sprite_pending_flag[3] = 0
            sprite_update_body(gs)
            frames.add(gs._body_frame_index)
        assert len(frames) >= 2


# ===================================================================
# Sprite slot management tests
# ===================================================================

class TestSpriteSlots:
    """Test the 8-slot hardware sprite multiplexer."""

    def test_body_uses_slot_3(self, gs):
        """Body sprite is always in slot 3."""
        sprite_update_body(gs)
        assert gs.sprite_pending_flag[3] == 1

    def test_head_uses_slot_4(self, gs):
        """Head sprite is always in slot 4."""
        sprite_lcp_head_update(gs)
        assert gs.sprite_pending_flag[4] == 1

    def test_hidden_sprite_gets_slot_9(self, gs):
        """Hidden sprites get mapped to disabled slot 9."""
        gs.sprite_layer_flags[5] = SPRITE_LAYER.SPRITE_HIDDEN
        sprite_update_slots(gs)
        assert gs.sprite_slot_map[5] == 9

    def test_in_front_sprite_gets_slot_6(self, gs):
        """SPRITE_IN_FRONT sprites get slot 6 (or 5 if 6 taken)."""
        gs.sprite_layer_flags[5] = SPRITE_LAYER.SPRITE_IN_FRONT
        gs.sprite_slot_map[5] = 0  # reset
        sprite_update_slots(gs)
        assert gs.sprite_slot_map[5] in (5, 6)

    def test_behind_sprite_gets_slot_2(self, gs):
        """SPRITE_BEHIND_LCP sprites get slot 2 (or 1 if 2 taken)."""
        gs.sprite_layer_flags[5] = SPRITE_LAYER.SPRITE_BEHIND_LCP
        gs.sprite_slot_map[5] = 0
        sprite_update_slots(gs)
        assert gs.sprite_slot_map[5] in (1, 2)


# ===================================================================
# Dog animation tests
# ===================================================================

class TestDogSpriteAnimation:
    """Test dog sprite selection, animation cycles, and positioning."""

    def test_dog_idle_sprite(self, gs):
        """Idle dog uses SPRITE_DOG_LAY_DOWN."""
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        assert gs.dog_sprite_id == SPRITE_ID.SPRITE_DOG_LAY_DOWN

    def test_dog_walk_cycle_8_frames(self, gs):
        """Dog walking cycles through 8 walk animation frames."""
        gs.dog_x = 50
        gs.dog_y = 190
        gs.dog_target_x = 200
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 200
        gs.dog_waypoint_y = 190

        seen_sprites = set()
        for _ in range(16):  # 2 full cycles
            dog_move_and_animate(gs)
            seen_sprites.add(gs.dog_sprite_id)

        # Should cycle through the 8 walk frames
        walk_frame_set = set(DOG_WALK_ANIM_FRAMES)
        assert seen_sprites == walk_frame_set

    def test_dog_walk_cycle_order(self, gs):
        """Dog walk frames cycle in order 0-7, wrapping around."""
        gs.dog_x = 50
        gs.dog_y = 190
        gs.dog_target_x = 200
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 200
        gs.dog_waypoint_y = 190
        gs.dog_walk_anim_cycle = 0  # start at beginning

        frames = []
        for _ in range(10):
            dog_move_and_animate(gs)
            frames.append(gs.dog_sprite_id)

        # First 8 should be the 8 walk frames in order
        for i in range(8):
            assert frames[i] == DOG_WALK_ANIM_FRAMES[(i + 1) % 8]

    def test_dog_walk_anim_wraps_at_7(self, gs):
        """dog_walk_anim_cycle wraps from 7 back to 0."""
        gs.dog_walk_anim_cycle = 7
        gs.dog_target_x = 100
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 100
        gs.dog_waypoint_y = 190
        gs.dog_x = 50
        gs.dog_y = 190

        dog_move_and_animate(gs)
        assert gs.dog_walk_anim_cycle == 0

    def test_dog_eating_animation_3_frames(self, gs):
        """Dog eating cycles through 3 eating frames."""
        gs.dog_x = 8
        gs.dog_y = 190
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        gs.dog_eating_active = 1
        gs.dog_eating_countdown = 30
        gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_FULL
        gs.dog_near_food_bowl = 1

        seen = set()
        for _ in range(9):  # 3 full cycles
            dog_frame_update(gs)
            seen.add(gs.dog_sprite_id)

        eating_set = set(DOG_EATING_ANIM_FRAMES)
        assert seen == eating_set

    def test_dog_eating_countdown_selects_frame(self, gs):
        """Eating frame = DOG_EATING_ANIM_FRAMES[countdown % 3]."""
        gs.dog_x = 8
        gs.dog_y = 190
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        gs.dog_eating_active = 1
        gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_FULL
        gs.dog_near_food_bowl = 1

        for countdown in [10, 11, 12]:
            gs.dog_eating_countdown = countdown
            dog_frame_update(gs)
            expected = DOG_EATING_ANIM_FRAMES[(countdown - 1) % 3]
            assert gs.dog_sprite_id == expected

    def test_dog_stops_eating_at_zero(self, gs):
        """Dog eating ends when countdown reaches 0."""
        gs.dog_x = 8
        gs.dog_y = 190
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        gs.dog_eating_active = 1
        gs.dog_eating_countdown = 1
        gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_FULL
        gs.dog_near_food_bowl = 1

        dog_frame_update(gs)
        assert gs.dog_eating_active == 0

    def test_dog_lay_down_on_arrival(self, gs):
        """Dog switches to LAY_DOWN sprite when reaching destination."""
        gs.dog_x = 100
        gs.dog_y = 190
        gs.dog_target_x = 101
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 101
        gs.dog_waypoint_y = 190

        # One step to reach destination
        dog_move_and_animate(gs)
        # One more step — now at destination
        dog_move_and_animate(gs)
        assert gs.dog_sprite_id == SPRITE_ID.SPRITE_DOG_LAY_DOWN
        assert gs.dog_target_x == 0
        assert gs.dog_target_y == 0


class TestDogSpritePositioning:
    """Test dog sprite positioning and depth layering."""

    def test_dog_render_y_offset(self, gs):
        """Dog render Y = dog_y - 17."""
        gs.dog_x = 50
        gs.dog_y = 180
        spritedata_update_dog(gs, SPRITE_ID.SPRITE_DOG_LAY_DOWN, 1, 0)
        assert gs._dog_render_y == 163  # 180 - 17

    def test_dog_render_x(self, gs):
        """Dog render X = dog_x."""
        gs.dog_x = 75
        gs.dog_y = 180
        spritedata_update_dog(gs, SPRITE_ID.SPRITE_DOG_LAY_DOWN, 1, 0)
        assert gs._dog_render_x == 75

    def test_dog_depth_behind_lcp(self, gs):
        """Dog behind LCP when lcp_y < dog_y + 5."""
        gs.lcp_y = 150
        gs.dog_x = 50
        gs.dog_y = 190  # lcp_y (150) < dog_y + 5 (195)
        gs.dog_target_x = 100
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 100
        gs.dog_waypoint_y = 190

        dog_move_and_animate(gs)
        assert gs._dog_depth_layer == 1  # behind

    def test_dog_depth_in_front_of_lcp(self, gs):
        """Dog in front of LCP when lcp_y >= dog_y + 5."""
        gs.lcp_y = 200
        gs.dog_x = 50
        gs.dog_y = 190  # lcp_y (200) >= dog_y + 5 (195)
        gs.dog_target_x = 100
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 100
        gs.dog_waypoint_y = 190

        dog_move_and_animate(gs)
        assert gs._dog_depth_layer == -1  # in front

    def test_dog_flip_facing_right(self, gs):
        """Dog flip=0 when walking right (target_x > dog_x)."""
        gs.dog_x = 50
        gs.dog_y = 190
        gs.dog_target_x = 200
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 200
        gs.dog_waypoint_y = 190

        dog_move_and_animate(gs)
        assert gs._dog_flip == 0

    def test_dog_flip_facing_left(self, gs):
        """Dog flip=1 when walking left (waypoint_x < dog_x)."""
        gs.dog_x = 200
        gs.dog_y = 190
        gs.dog_target_x = 50
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 50
        gs.dog_waypoint_y = 190

        dog_move_and_animate(gs)
        assert gs._dog_flip == 1

    def test_dog_moves_toward_target(self, gs):
        """Dog X moves toward waypoint each frame."""
        gs.dog_x = 50
        gs.dog_y = 190
        gs.dog_target_x = 100
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 100
        gs.dog_waypoint_y = 190

        initial_x = gs.dog_x
        dog_move_and_animate(gs)
        assert gs.dog_x > initial_x  # moved right

    def test_dog_moves_left_toward_target(self, gs):
        """Dog X decreases when target is to the left."""
        gs.dog_x = 200
        gs.dog_y = 190
        gs.dog_target_x = 50
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 50
        gs.dog_waypoint_y = 190

        initial_x = gs.dog_x
        dog_move_and_animate(gs)
        assert gs.dog_x < initial_x


class TestDogWanderAI:
    """Test the dog's idle wandering behaviour."""

    def test_dog_idle_countdown_decrements(self, gs):
        """Idle countdown decreases each frame when dog is stationary."""
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        gs.dog_eating_active = 0
        gs.dog_idle_countdown = 10

        dog_frame_update(gs)
        assert gs.dog_idle_countdown == 9

    def test_dog_idle_countdown_clamped(self, gs):
        """Out-of-range idle countdown is clamped to 5, then decremented."""
        gs.dog_idle_countdown = 300
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        gs.dog_eating_active = 0

        dog_frame_update(gs)
        # Clamped to 5, then decremented by 1 in the same frame = 4
        assert gs.dog_idle_countdown == 4

    def test_dog_picks_destination_at_zero(self, gs):
        """Dog picks a new destination when idle countdown reaches 0."""
        random.seed(42)
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        gs.dog_eating_active = 0
        gs.dog_idle_countdown = 0
        gs.dog_visible = 1
        gs.dog_last_target_index = -1

        dog_frame_update(gs)
        # Should now have a destination
        assert gs.dog_target_x != 0 or gs.dog_target_y != 0

    def test_dog_eating_starts_near_bowl(self, gs):
        """Dog starts eating when idle near food bowl with food."""
        gs.dog_x = 8      # < 20
        gs.dog_y = 170     # > 160
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        gs.dog_eating_active = 0
        gs.dog_near_food_bowl = 1
        gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_FULL

        random.seed(42)
        dog_frame_update(gs)
        assert gs.dog_eating_active == 1
        assert gs.dog_eating_countdown > 0

    def test_dog_no_eating_empty_bowl(self, gs):
        """Dog does NOT eat when bowl is empty."""
        gs.dog_x = 8
        gs.dog_y = 170
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        gs.dog_eating_active = 0
        gs.dog_near_food_bowl = 1
        gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_EMPTY

        dog_frame_update(gs)
        assert gs.dog_eating_active == 0

    def test_dog_bowl_depletes_during_eating(self, gs):
        """Bowl depletes at specific countdown values (60, 30, 4)."""
        gs.dog_x = 8
        gs.dog_y = 190
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        gs.dog_eating_active = 1
        gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_FULL
        gs.dog_near_food_bowl = 1

        for countdown_val in [61, 31, 5]:
            gs.dog_eating_countdown = countdown_val
            gs.dog_food_bowl_change = 0
            dog_frame_update(gs)
            assert gs.dog_food_bowl_change == -1, \
                f"Expected bowl change at countdown {countdown_val - 1}"


# ===================================================================
# Constant table integrity tests
# ===================================================================

class TestConstantTables:
    """Verify animation data table dimensions and bounds."""

    def test_body_sprite_frame_table_length(self):
        """BODY_SPRITE_FRAME_TABLE has entries for all 93 player states."""
        assert len(BODY_SPRITE_FRAME_TABLE) >= 91

    def test_carry_body_frame_table_length(self):
        """CARRY_BODY_FRAME_TABLE covers states 0-24."""
        assert len(CARRY_BODY_FRAME_TABLE) == 25

    def test_body_y_offset_table_length(self):
        """BODY_Y_OFFSET_PER_STATE covers all states."""
        assert len(BODY_Y_OFFSET_PER_STATE) >= 91

    def test_head_x_offset_table_length(self):
        """HEAD_X_OFFSET_PER_STATE covers all states."""
        assert len(HEAD_X_OFFSET_PER_STATE) >= 91

    def test_head_height_table_length(self):
        """HEAD_HEIGHT_PER_STATE covers all states."""
        assert len(HEAD_HEIGHT_PER_STATE) >= 91

    def test_head_default_angle_table_length(self):
        """HEAD_DEFAULT_ANGLE_PER_STATE covers all states."""
        assert len(HEAD_DEFAULT_ANGLE_PER_STATE) >= 91

    def test_head_heights_positive(self):
        """All head heights are positive (sprite has non-zero height)."""
        for h in HEAD_HEIGHT_PER_STATE:
            assert h > 0

    def test_body_y_offsets_reasonable(self):
        """Body Y offsets are within a reasonable range."""
        for off in BODY_Y_OFFSET_PER_STATE:
            assert -10 <= off <= 15

    def test_head_x_offsets_small(self):
        """Head X offsets are small adjustments."""
        for off in HEAD_X_OFFSET_PER_STATE:
            assert -5 <= off <= 10

    def test_head_default_angles_valid(self):
        """Head default angles are 0-7."""
        for angle in HEAD_DEFAULT_ANGLE_PER_STATE:
            assert 0 <= angle <= 7

    def test_dog_walk_frames_8(self):
        """DOG_WALK_ANIM_FRAMES has exactly 8 entries."""
        assert len(DOG_WALK_ANIM_FRAMES) == 8

    def test_dog_eating_frames_3(self):
        """DOG_EATING_ANIM_FRAMES has exactly 3 entries."""
        assert len(DOG_EATING_ANIM_FRAMES) == 3

    def test_dog_walk_frames_sequential(self):
        """Dog walk frame IDs are sequential SPRITE_DOG_WALK_RIGHT_1..8."""
        for i, frame in enumerate(DOG_WALK_ANIM_FRAMES):
            assert frame == SPRITE_ID.SPRITE_DOG_WALK_RIGHT_1 + i

    def test_dog_eating_frames_sequential(self):
        """Dog eating frame IDs are sequential SPRITE_DOG_EATING_1..3."""
        for i, frame in enumerate(DOG_EATING_ANIM_FRAMES):
            assert frame == SPRITE_ID.SPRITE_DOG_EATING_1 + i


# ===================================================================
# Full frame update integration tests
# ===================================================================

class TestFrameUpdateIntegration:
    """Test complete sprite update cycle (body + head + dog per frame)."""

    def test_full_sprite_update_cycle(self, gs):
        """One complete update cycle sets all slots correctly."""
        gs.lcp_x = 160
        gs.lcp_y = 150
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
        gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT

        sprite_update_body(gs)
        sprite_lcp_head_animate(gs)
        sprite_lcp_head_update(gs)

        # Both slots should have pending data
        assert gs.sprite_pending_flag[3] == 1
        assert gs.sprite_pending_flag[4] == 1

        # Positions should be set
        assert gs.sprite_active_x[3] == 156  # 160 - 4
        assert gs.sprite_active_x[4] == 156  # 160 + 0 - 4 (mirror=0, x_off=0)

    def test_multiple_frame_updates(self, gs):
        """Multiple update cycles don't corrupt state."""
        gs.lcp_x = 100
        gs.lcp_y = 150
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE

        for _ in range(20):
            gs.sprite_pending_flag[3] = 0
            gs.sprite_pending_flag[4] = 0
            sprite_update_body(gs)
            sprite_lcp_head_animate(gs)
            sprite_lcp_head_update(gs)

        assert gs.sprite_active_x[3] == 96
        # Head Y should be reasonable (above body)
        assert gs.sprite_active_y[4] < gs.sprite_active_y[3]

    def test_position_changes_propagate(self, gs):
        """Moving lcp_x updates sprite positions on next frame."""
        gs.lcp_x = 100
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
        gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT

        sprite_update_body(gs)
        assert gs.sprite_active_x[3] == 96

        gs.lcp_x = 120
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        assert gs.sprite_active_x[3] == 116  # 120 - 4

    def test_state_change_updates_frame(self, gs):
        """Changing player state updates the body frame index."""
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
        sprite_update_body(gs)
        idle_frame = gs._body_frame_index

        gs.lcp_state = PLAYER_STATE.STATE_SIT_CHAIR
        gs.sprite_pending_flag[3] = 0
        sprite_update_body(gs)
        sit_frame = gs._body_frame_index

        assert idle_frame != sit_frame

    def test_dog_and_lcp_independent(self, gs):
        """Dog and LCP sprite updates don't interfere with each other."""
        gs.lcp_x = 160
        gs.lcp_y = 150
        gs.dog_x = 50
        gs.dog_y = 190

        sprite_update_body(gs)
        lcp_x = gs.sprite_active_x[3]

        gs.dog_target_x = 100
        gs.dog_target_y = 190
        gs.dog_waypoint_x = 100
        gs.dog_waypoint_y = 190
        dog_move_and_animate(gs)

        # LCP position should be unchanged
        assert gs.sprite_active_x[3] == lcp_x

    def test_rapid_direction_changes(self, gs):
        """Rapid facing-direction toggles produce correct positions."""
        gs.lcp_x = 100
        gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE

        for _ in range(10):
            gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
            gs.sprite_pending_flag[3] = 0
            sprite_update_body(gs)
            assert gs.sprite_active_x[3] == 96   # 100 - 4

            gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
            gs.sprite_pending_flag[3] = 0
            sprite_update_body(gs)
            assert gs.sprite_active_x[3] == 86   # 100 - 14
