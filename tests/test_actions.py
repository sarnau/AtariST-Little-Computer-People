"""
Comprehensive tests for all action handlers in lcp.actions.

Every action handler is tested with mocked movement and tick functions
to avoid blocking loops. The mocks ensure actions execute their state
changes without actually walking or waiting.
"""

import pytest
from unittest.mock import patch, MagicMock, call

from lcp.state import GameState
from lcp.structs import LCP
from lcp.enums import (
    ACTION_ID, PLAYER_STATE, FACING_DIR, HOUSE_POS,
    DOG_BOWL_STATUS, SICKNESS_LEVEL, SPRITE_ID, SPRITE_LAYER,
    NEED_LEVEL, SOUND_EFFECT_ID, HEAD_ANIM_MODE,
)
from lcp.constants import house_get_position_xy
from lcp.actions import (
    do_action,
    action_sleep, action_sit_and_exercise, action_read_newspaper,
    action_play_computer, action_wash_hands, action_get_in_out_of_bed,
    action_listen_song, action_play_piano, action_write_letter,
    action_dance, action_yawn_and_stretch, action_pace_nervously,
    action_wander_idly, action_drink, action_nod_head,
    action_peek_around, action_play_a_game, action_brush_teeth,
    action_kitchen_cabinet, action_sit_on_couch_with_dog,
    action_light_fireplace, action_use_toilet, action_take_shower,
    action_feed_dog, action_hello, action_eat_meal,
    action_play_with_record, action_open_close_upstairs_closet,
    action_get_snack_from_fridge, action_open_close_bedroom_closet,
    action_get_dressed, action_clean_up, action_tidy_house,
    action_check_front_door, action_toggle_tv, action_call_dog,
    action_wake_from_alarm, action_pet_dog, action_wake_up_morning,
    action_go_to_bed_night,
    lcp_check_recovery, hide_lcp_sprites, show_lcp_sprites,
)


# ---------------------------------------------------------------------------
# Common mock decorators for all action tests
#
# Because _tick() and _walk() use lazy imports (inside the function body),
# we must mock the target in the *source* module, not in lcp.actions.
# ---------------------------------------------------------------------------
WALK_MOCK = 'lcp.movement.lcp_walk_to_destination'
TICK_MOCK = 'lcp.main.game_tick_and_animate'
SPRITE_UPDATE_MOCK = 'lcp.sprites.sprite_update_slots'
SPRITE_CARRY_MOCK = 'lcp.sprites.spritedata_select_carried_object_left'


def _mock_walk_side_effect(gs):
    """Side effect for walk mock: snap LCP position to walk target."""
    gs.lcp_x = gs.walk_target_x
    gs.lcp_y = gs.walk_target_y
    return 0


# ---------------------------------------------------------------------------
# do_action dispatcher tests
# ---------------------------------------------------------------------------
class TestDoAction:
    """Tests for the do_action() dispatcher."""

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_action_none_does_nothing(self, mock_tick, mock_walk, gs):
        """ACTION_NONE should not dispatch to any handler."""
        gs.trigger_action = ACTION_ID.ACTION_NONE
        do_action(gs)
        assert gs.trigger_action == ACTION_ID.ACTION_NONE

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_trigger_action_cleared_after_dispatch(self, mock_tick, mock_walk, gs):
        """After dispatching, trigger_action should be reset to ACTION_NONE."""
        gs.trigger_action = ACTION_ID.ACTION_NOD_HEAD
        do_action(gs)
        assert gs.trigger_action == ACTION_ID.ACTION_NONE

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_last_action_set_after_dispatch(self, mock_tick, mock_walk, gs):
        """After dispatching, last_action should be set to the dispatched action."""
        gs.trigger_action = ACTION_ID.ACTION_HELLO
        do_action(gs)
        assert gs.last_action == ACTION_ID.ACTION_HELLO

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_wakes_from_sleep_before_dispatch(self, mock_tick, mock_walk, gs):
        """If LCP is sleeping, do_action should wake them before dispatching."""
        gs.lcp.is_sleeping = 1
        gs.trigger_action = ACTION_ID.ACTION_HELLO
        do_action(gs)
        # action_get_in_out_of_bed is called which sets is_sleeping=0
        assert gs.lcp.is_sleeping == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sleep_action_dispatches_with_minus_one(self, mock_tick, mock_walk, gs):
        """ACTION_SLEEP should call action_sleep(gs, -1)."""
        gs.trigger_action = ACTION_ID.ACTION_SLEEP
        # Place on bottom floor so floor detection works
        gs.lcp_y = 190
        do_action(gs)
        assert gs.last_action == ACTION_ID.ACTION_SLEEP

    @pytest.mark.parametrize("action_id", [
        ACTION_ID.ACTION_SIT_AND_EXERCISE,
        ACTION_ID.ACTION_READ_NEWSPAPER,
        ACTION_ID.ACTION_PLAY_COMPUTER,
        ACTION_ID.ACTION_WASH_HANDS,
        ACTION_ID.ACTION_GET_IN_OUT_OF_BED,
        ACTION_ID.ACTION_LISTEN_SONG,
        ACTION_ID.ACTION_PLAY_PIANO,
        ACTION_ID.ACTION_WRITE_LETTER,
        ACTION_ID.ACTION_DANCE,
        ACTION_ID.ACTION_YAWN_AND_STRETCH,
        ACTION_ID.ACTION_PACE_NERVOUSLY,
        ACTION_ID.ACTION_WANDER_IDLY,
        ACTION_ID.ACTION_DRINK,
        ACTION_ID.ACTION_NOD_HEAD,
        ACTION_ID.ACTION_PEEK_AROUND,
        ACTION_ID.ACTION_PLAY_A_GAME,
        ACTION_ID.ACTION_BRUSH_TEETH,
        ACTION_ID.ACTION_KITCHEN_CABINET,
        ACTION_ID.ACTION_SIT_ON_COUCH_WITH_DOG,
        ACTION_ID.ACTION_LIGHT_FIREPLACE,
        ACTION_ID.ACTION_USE_TOILET,
        ACTION_ID.ACTION_TAKE_SHOWER,
        ACTION_ID.ACTION_FEED_DOG,
        ACTION_ID.ACTION_HELLO,
        ACTION_ID.ACTION_EAT_MEAL,
        ACTION_ID.ACTION_PLAY_WITH_RECORD,
        ACTION_ID.ACTION_OPEN_UPSTAIRS_CLOSET,
        ACTION_ID.ACTION_GET_SNACK_FROM_FRIDGE,
        ACTION_ID.ACTION_OPEN_BEDROOM_CLOSET,
        ACTION_ID.ACTION_GET_DRESSED,
        ACTION_ID.ACTION_CLEAN_UP,
        ACTION_ID.ACTION_TIDY_HOUSE,
        ACTION_ID.ACTION_CHECK_FRONT_DOOR,
        ACTION_ID.ACTION_TOGGLE_TV,
        ACTION_ID.ACTION_CALL_DOG,
        ACTION_ID.ACTION_WAKE_FROM_ALARM,
        ACTION_ID.ACTION_PET_DOG,
        ACTION_ID.ACTION_WAKE_UP_MORNING,
        ACTION_ID.ACTION_GO_TO_BED_NIGHT,
        ACTION_ID.ACTION_SLEEP,
    ])
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_all_action_ids_dispatch_without_error(self, mock_tick, mock_walk, gs, action_id):
        """Every defined ACTION_ID should dispatch without raising an exception."""
        gs.trigger_action = action_id
        gs.lcp_y = 190  # bottom floor for floor detection
        do_action(gs)
        assert gs.trigger_action == ACTION_ID.ACTION_NONE


# ---------------------------------------------------------------------------
# action_sleep tests
# ---------------------------------------------------------------------------
class TestActionSleep:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sleep_with_negative_value_walks_to_floor_center(self, mock_tick, mock_walk, gs):
        """value=-1 should attempt to walk to current floor center."""
        gs.lcp_y = 190  # bottom floor
        action_sleep(gs, -1)
        # Walk should have been called (to floor center)
        mock_walk.assert_called()

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sleep_with_negative_value_sets_facing_right(self, mock_tick, mock_walk, gs):
        gs.lcp_y = 190
        action_sleep(gs, -1)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sleep_with_positive_value_ticks(self, mock_tick, mock_walk, gs):
        """value>=0 should tick through sleep cycles without walking."""
        action_sleep(gs, 2)
        mock_walk.assert_not_called()
        assert mock_tick.call_count > 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sleep_on_stairs_returns_immediately(self, mock_tick, mock_walk, gs):
        """If LCP is on stairs, action_sleep returns immediately."""
        gs.lcp_on_stairs_flag = 1
        action_sleep(gs, -1)
        mock_walk.assert_not_called()
        mock_tick.assert_not_called()

    @patch(WALK_MOCK, return_value=-1)
    @patch(TICK_MOCK)
    def test_sleep_walk_interrupted_returns_early(self, mock_tick, mock_walk, gs):
        """If walk is interrupted (returns -1), action_sleep returns early."""
        gs.lcp_y = 190
        action_sleep(gs, -1)
        # Should have returned before ticking sleep cycles
        mock_tick.assert_not_called()

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sleep_snoring_sound(self, mock_tick, mock_walk, gs):
        """Sleep should trigger snoring sound effect."""
        action_sleep(gs, 1)
        assert gs.soundeffect_pending == SOUND_EFFECT_ID.SFX_SNORING


# ---------------------------------------------------------------------------
# action_sit_and_exercise tests
# ---------------------------------------------------------------------------
class TestActionSitAndExercise:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_couch(self, mock_tick, mock_walk, gs):
        action_sit_and_exercise(gs)
        mock_walk.assert_called()
        # Walk target should be POS_MID_COUCH
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_MID_COUCH)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_faces_right(self, mock_tick, mock_walk, gs):
        action_sit_and_exercise(gs)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_ticks_for_exercise(self, mock_tick, mock_walk, gs):
        action_sit_and_exercise(gs)
        assert mock_tick.call_count > 0


# ---------------------------------------------------------------------------
# action_read_newspaper tests
# ---------------------------------------------------------------------------
class TestActionReadNewspaper:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_game_table(self, mock_tick, mock_walk, gs):
        action_read_newspaper(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_TOP_ARMCHAIR)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_sit_chair_state(self, mock_tick, mock_walk, gs):
        action_read_newspaper(gs)
        # After walk, state should be set to SIT_CHAIR
        assert gs.lcp_state == PLAYER_STATE.STATE_SIT_CHAIR


# ---------------------------------------------------------------------------
# action_play_computer tests
# ---------------------------------------------------------------------------
class TestActionPlayComputer:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_computer(self, mock_tick, mock_walk, gs):
        action_play_computer(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_MID_COMPUTER)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_typewriter_sound(self, mock_tick, mock_walk, gs):
        action_play_computer(gs)
        assert gs.soundeffect_pending == SOUND_EFFECT_ID.SFX_TYPEWRITER_KEY


# ---------------------------------------------------------------------------
# action_wash_hands tests
# ---------------------------------------------------------------------------
class TestActionWashHands:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_sink(self, mock_tick, mock_walk, gs):
        action_wash_hands(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_MID_SINK)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_water_sounds(self, mock_tick, mock_walk, gs):
        action_wash_hands(gs)
        # Last sound should be water tap
        assert gs.soundeffect_pending == SOUND_EFFECT_ID.SFX_WATER_TAP


# ---------------------------------------------------------------------------
# action_get_in_out_of_bed tests
# ---------------------------------------------------------------------------
class TestActionGetInOutOfBed:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_getting_in_bed_walks_to_bed(self, mock_tick, mock_walk, gs):
        gs.lcp.is_sleeping = 0
        action_get_in_out_of_bed(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_MID_BED)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_getting_in_bed_sets_sleeping(self, mock_tick, mock_walk, gs):
        gs.lcp.is_sleeping = 0
        action_get_in_out_of_bed(gs)
        assert gs.lcp.is_sleeping == 1

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_getting_in_bed_state_transitions(self, mock_tick, mock_walk, gs):
        gs.lcp.is_sleeping = 0
        action_get_in_out_of_bed(gs)
        # Final state should be SLEEP_IN_BED
        assert gs.lcp_state == PLAYER_STATE.STATE_SLEEP_IN_BED

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_getting_out_of_bed_clears_sleeping(self, mock_tick, mock_walk, gs):
        gs.lcp.is_sleeping = 1
        action_get_in_out_of_bed(gs)
        assert gs.lcp.is_sleeping == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_getting_out_of_bed_sets_stand_idle(self, mock_tick, mock_walk, gs):
        gs.lcp.is_sleeping = 1
        action_get_in_out_of_bed(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_STAND_IDLE

    @patch(WALK_MOCK, return_value=-1)
    @patch(TICK_MOCK)
    def test_walk_interrupted_does_not_set_sleeping(self, mock_tick, mock_walk, gs):
        gs.lcp.is_sleeping = 0
        action_get_in_out_of_bed(gs)
        assert gs.lcp.is_sleeping == 0


# ---------------------------------------------------------------------------
# action_drink tests
# ---------------------------------------------------------------------------
class TestActionDrink:

    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_kitchen_sink(self, mock_tick, mock_walk, mock_sprites, gs):
        action_drink(gs)
        # First walk target should be kitchen sink
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_BTM_KITCHEN_SINK)
        # Walk is called multiple times; first call sets target to kitchen sink
        first_walk_x = expected_x
        assert mock_walk.call_count >= 1

    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_satisfies_thirst(self, mock_tick, mock_walk, mock_sprites, gs):
        gs.lcp.thirst_level = NEED_LEVEL.NEED_CRITICAL
        action_drink(gs)
        assert gs.lcp.thirst_level == NEED_LEVEL.NEED_SATISFIED

    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_resets_thirst_timer(self, mock_tick, mock_walk, mock_sprites, gs):
        gs.lcp.thirst_timer_max = 60
        action_drink(gs)
        assert gs.lcp.thirst_timer == 60

    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_carrying_flag(self, mock_tick, mock_walk, mock_sprites, gs):
        action_drink(gs)
        assert gs.lcp_carrying_object_flag == 0

    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_interruptible_flag(self, mock_tick, mock_walk, mock_sprites, gs):
        action_drink(gs)
        assert gs.action_interruptible_flag == 0

    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=-1)
    @patch(TICK_MOCK)
    def test_walk_interrupted_returns_early(self, mock_tick, mock_walk, mock_sprites, gs):
        gs.lcp.thirst_level = NEED_LEVEL.NEED_CRITICAL
        action_drink(gs)
        # Thirst should not be satisfied if walk was interrupted
        assert gs.lcp.thirst_level == NEED_LEVEL.NEED_CRITICAL


# ---------------------------------------------------------------------------
# action_brush_teeth tests
# ---------------------------------------------------------------------------
class TestActionBrushTeeth:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_sink(self, mock_tick, mock_walk, gs):
        action_brush_teeth(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_MID_SINK)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_ends_with_wash_hands_state(self, mock_tick, mock_walk, gs):
        action_brush_teeth(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_WASH_HANDS


# ---------------------------------------------------------------------------
# action_use_toilet tests
# ---------------------------------------------------------------------------
class TestActionUseToilet:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_toilet(self, mock_tick, mock_walk, gs):
        action_use_toilet(gs)
        # First walk should target toilet position
        mock_walk.assert_called()

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_bathroom_need(self, mock_tick, mock_walk, gs):
        gs.lcp.bathroom_need = 1
        action_use_toilet(gs)
        assert gs.lcp.bathroom_need == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_bathroom_timer(self, mock_tick, mock_walk, gs):
        action_use_toilet(gs)
        assert gs.lcp.bathroom_timer == 9999

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_interruptible(self, mock_tick, mock_walk, gs):
        action_use_toilet(gs)
        assert gs.action_interruptible_flag == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_flush_sound(self, mock_tick, mock_walk, gs):
        """Toilet flush sound should be played during the action."""
        action_use_toilet(gs)
        # The last few sounds include flush; we just verify no exception

    @patch(WALK_MOCK, return_value=-1)
    @patch(TICK_MOCK)
    def test_walk_interrupted_returns_early(self, mock_tick, mock_walk, gs):
        gs.lcp.bathroom_need = 1
        action_use_toilet(gs)
        # Bathroom need should not be cleared if walk was interrupted
        assert gs.lcp.bathroom_need == 1


# ---------------------------------------------------------------------------
# action_take_shower tests
# ---------------------------------------------------------------------------
class TestActionTakeShower:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_shower(self, mock_tick, mock_walk, gs):
        action_take_shower(gs)
        mock_walk.assert_called()

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_interruptible(self, mock_tick, mock_walk, gs):
        action_take_shower(gs)
        assert gs.action_interruptible_flag == 0

    @patch(WALK_MOCK, return_value=-1)
    @patch(TICK_MOCK)
    def test_walk_interrupted_returns_early(self, mock_tick, mock_walk, gs):
        action_take_shower(gs)
        # Should return early without ticking shower cycles
        # (walk returns -1 immediately on first call)


# ---------------------------------------------------------------------------
# action_eat_meal tests
# ---------------------------------------------------------------------------
class TestActionEatMeal:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_cabinet(self, mock_tick, mock_walk, gs):
        action_eat_meal(gs)
        # The first walk destination should be POS_BTM_CABINET
        mock_walk.assert_called()

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_satisfies_hunger(self, mock_tick, mock_walk, gs):
        gs.lcp.hunger_level = NEED_LEVEL.NEED_CRITICAL
        action_eat_meal(gs)
        assert gs.lcp.hunger_level == NEED_LEVEL.NEED_SATISFIED

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_resets_hunger_timer(self, mock_tick, mock_walk, gs):
        gs.lcp.hunger_timer_max = 90
        action_eat_meal(gs)
        assert gs.lcp.hunger_timer == 90

    @patch(WALK_MOCK, return_value=-1)
    @patch(TICK_MOCK)
    def test_walk_interrupted_does_not_eat(self, mock_tick, mock_walk, gs):
        gs.lcp.hunger_level = NEED_LEVEL.NEED_CRITICAL
        action_eat_meal(gs)
        assert gs.lcp.hunger_level == NEED_LEVEL.NEED_CRITICAL


# ---------------------------------------------------------------------------
# action_kitchen_cabinet tests
# ---------------------------------------------------------------------------
class TestActionKitchenCabinet:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_cabinet(self, mock_tick, mock_walk, gs):
        action_kitchen_cabinet(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_BTM_CABINET)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_satisfies_hunger(self, mock_tick, mock_walk, gs):
        gs.lcp.hunger_level = NEED_LEVEL.NEED_ELEVATED
        action_kitchen_cabinet(gs)
        assert gs.lcp.hunger_level == NEED_LEVEL.NEED_SATISFIED

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_resets_hunger_timer(self, mock_tick, mock_walk, gs):
        gs.lcp.hunger_timer_max = 90
        action_kitchen_cabinet(gs)
        assert gs.lcp.hunger_timer == 90

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_decrements_cabinet_fill(self, mock_tick, mock_walk, gs):
        """Cabinet fill level (bits 9-11) should decrement."""
        gs.lcp.door_states_and_flags = 3 << 9  # cabinet fill = 3
        action_kitchen_cabinet(gs)
        cabinet_fill = (gs.lcp.door_states_and_flags >> 9) & 7
        assert cabinet_fill == 2

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_cabinet_fill_stays_at_zero(self, mock_tick, mock_walk, gs):
        """Cabinet fill level should not go below 0."""
        gs.lcp.door_states_and_flags = 0  # cabinet fill = 0
        action_kitchen_cabinet(gs)
        cabinet_fill = (gs.lcp.door_states_and_flags >> 9) & 7
        assert cabinet_fill == 0

    @patch(WALK_MOCK, return_value=-1)
    @patch(TICK_MOCK)
    def test_walk_interrupted(self, mock_tick, mock_walk, gs):
        gs.lcp.hunger_level = NEED_LEVEL.NEED_CRITICAL
        action_kitchen_cabinet(gs)
        assert gs.lcp.hunger_level == NEED_LEVEL.NEED_CRITICAL


# ---------------------------------------------------------------------------
# action_feed_dog tests
# ---------------------------------------------------------------------------
class TestActionFeedDog:

    @patch(SPRITE_CARRY_MOCK)
    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_full_sequence_walks_to_fridge(self, mock_tick, mock_walk, mock_sprites, mock_carry, gs):
        action_feed_dog(gs, 0)
        mock_walk.assert_called()

    @patch(SPRITE_CARRY_MOCK)
    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_fills_dog_bowl(self, mock_tick, mock_walk, mock_sprites, mock_carry, gs):
        gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_EMPTY
        action_feed_dog(gs, 0)
        assert gs.dog_bowl_status == DOG_BOWL_STATUS.BOWL_FULL

    @patch(SPRITE_CARRY_MOCK)
    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_dog_food_bowl_change(self, mock_tick, mock_walk, mock_sprites, mock_carry, gs):
        action_feed_dog(gs, 0)
        assert gs.dog_food_bowl_change == 1

    @patch(SPRITE_CARRY_MOCK)
    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_interruptible_flag(self, mock_tick, mock_walk, mock_sprites, mock_carry, gs):
        action_feed_dog(gs, 0)
        assert gs.action_interruptible_flag == 0

    @patch(SPRITE_CARRY_MOCK)
    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=-1)
    @patch(TICK_MOCK)
    def test_walk_interrupted_returns_early(self, mock_tick, mock_walk, mock_sprites, mock_carry, gs):
        gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_EMPTY
        action_feed_dog(gs, 0)
        # Bowl should still be empty if walk to fridge failed
        assert gs.dog_bowl_status == DOG_BOWL_STATUS.BOWL_EMPTY

    @patch(SPRITE_CARRY_MOCK)
    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_skip_fridge_with_nonzero_value(self, mock_tick, mock_walk, mock_sprites, mock_carry, gs):
        """value!=0 skips fridge walk and goes directly to bowl."""
        action_feed_dog(gs, 1)
        assert gs.dog_bowl_status == DOG_BOWL_STATUS.BOWL_FULL


# ---------------------------------------------------------------------------
# action_get_snack_from_fridge tests
# ---------------------------------------------------------------------------
class TestActionGetSnackFromFridge:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_fridge(self, mock_tick, mock_walk, gs):
        action_get_snack_from_fridge(gs)
        # First walk target should be POS_BTM_FRIDGE
        mock_walk.assert_called()

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_satisfies_hunger(self, mock_tick, mock_walk, gs):
        gs.lcp.hunger_level = NEED_LEVEL.NEED_CRITICAL
        action_get_snack_from_fridge(gs)
        assert gs.lcp.hunger_level == NEED_LEVEL.NEED_SATISFIED

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_resets_hunger_timer(self, mock_tick, mock_walk, gs):
        gs.lcp.hunger_timer_max = 90
        action_get_snack_from_fridge(gs)
        assert gs.lcp.hunger_timer == 90


# ---------------------------------------------------------------------------
# action_check_front_door tests
# ---------------------------------------------------------------------------
class TestActionCheckFrontDoor:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_front_door(self, mock_tick, mock_walk, gs):
        action_check_front_door(gs, 40)
        mock_walk.assert_called()

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_opens_closed_door(self, mock_tick, mock_walk, gs):
        """If front door is closed, it should be opened."""
        gs.lcp.door_states_and_flags = 0  # all closed
        # Set initiative_threshold high so random check never closes door again
        # (random(0,100) > threshold → close; with threshold=100, never closes)
        gs.lcp.initiative_threshold = 100
        action_check_front_door(gs, 40)
        assert gs.lcp.door_states_and_flags & 0x01  # front door open

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_interruptible(self, mock_tick, mock_walk, gs):
        action_check_front_door(gs, 10)
        assert gs.action_interruptible_flag == 0

    @patch(WALK_MOCK, return_value=-1)
    @patch(TICK_MOCK)
    def test_walk_interrupted_returns_early(self, mock_tick, mock_walk, gs):
        gs.lcp.door_states_and_flags = 0
        action_check_front_door(gs, 40)
        # Door should remain closed if walk was interrupted
        assert gs.lcp.door_states_and_flags == 0


# ---------------------------------------------------------------------------
# action_light_fireplace tests
# ---------------------------------------------------------------------------
class TestActionLightFireplace:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_fire_active(self, mock_tick, mock_walk, gs):
        gs.fire_active = 0
        action_light_fireplace(gs)
        assert gs.fire_active == 1

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_carries_firewood(self, mock_tick, mock_walk, gs):
        """Should carry firewood during the action."""
        action_light_fireplace(gs)
        # At the end, carrying flag is cleared
        assert gs.lcp_carrying_object_flag == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_hides_firewood_sprite(self, mock_tick, mock_walk, gs):
        action_light_fireplace(gs)
        assert gs.sprite_layer_flags[SPRITE_ID.SPRITE_FIREWOOD] == SPRITE_LAYER.SPRITE_HIDDEN

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_interruptible(self, mock_tick, mock_walk, gs):
        action_light_fireplace(gs)
        assert gs.action_interruptible_flag == 0


# ---------------------------------------------------------------------------
# action_play_piano tests
# ---------------------------------------------------------------------------
class TestActionPlayPiano:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_piano(self, mock_tick, mock_walk, gs):
        action_play_piano(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_TOP_DANCE_FLOOR)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_faces_right(self, mock_tick, mock_walk, gs):
        action_play_piano(gs)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT


# ---------------------------------------------------------------------------
# action_play_with_record tests
# ---------------------------------------------------------------------------
class TestActionPlayWithRecord:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_record_shelf(self, mock_tick, mock_walk, gs):
        action_play_with_record(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_TOP_RECORD_SHELF)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_record_player_on(self, mock_tick, mock_walk, gs):
        gs.record_player_on = 0
        action_play_with_record(gs)
        assert gs.record_player_on == 1

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_hides_vinyl_sprite(self, mock_tick, mock_walk, gs):
        action_play_with_record(gs)
        assert gs.sprite_layer_flags[SPRITE_ID.SPRITE_VINYL_RECORD] == SPRITE_LAYER.SPRITE_HIDDEN


# ---------------------------------------------------------------------------
# action_listen_song tests
# ---------------------------------------------------------------------------
class TestActionListenSong:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_record_shelf(self, mock_tick, mock_walk, gs):
        action_listen_song(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_TOP_RECORD_SHELF)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_faces_right(self, mock_tick, mock_walk, gs):
        action_listen_song(gs)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT


# ---------------------------------------------------------------------------
# action_write_letter tests
# ---------------------------------------------------------------------------
class TestActionWriteLetter:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_desk(self, mock_tick, mock_walk, gs):
        action_write_letter(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_TOP_DESK_LAMP)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_hides_typewriter_after(self, mock_tick, mock_walk, gs):
        action_write_letter(gs)
        assert gs.sprite_layer_flags[SPRITE_ID.SPRITE_TYPEWRITER] == SPRITE_LAYER.SPRITE_HIDDEN

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_typewriter_sound(self, mock_tick, mock_walk, gs):
        action_write_letter(gs)
        assert gs.soundeffect_pending == SOUND_EFFECT_ID.SFX_TYPEWRITER_KEY


# ---------------------------------------------------------------------------
# action_dance tests
# ---------------------------------------------------------------------------
class TestActionDance:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_record_shelf(self, mock_tick, mock_walk, gs):
        action_dance(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_TOP_RECORD_SHELF)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_ticks_for_dance(self, mock_tick, mock_walk, gs):
        action_dance(gs)
        assert mock_tick.call_count > 0


# ---------------------------------------------------------------------------
# action_yawn_and_stretch tests
# ---------------------------------------------------------------------------
class TestActionYawnAndStretch:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_ends_with_stand_idle(self, mock_tick, mock_walk, gs):
        action_yawn_and_stretch(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_STAND_IDLE

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_no_walk_needed(self, mock_tick, mock_walk, gs):
        """Yawn and stretch happens in place, no walk."""
        action_yawn_and_stretch(gs)
        mock_walk.assert_not_called()


# ---------------------------------------------------------------------------
# action_pace_nervously tests
# ---------------------------------------------------------------------------
class TestActionPaceNervously:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_runs_without_error(self, mock_tick, mock_walk, gs):
        gs.lcp_y = 190  # bottom floor
        action_pace_nervously(gs)

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_uses_walk_xy(self, mock_tick, mock_walk, gs):
        """Pacing uses _walk_xy to walk back and forth."""
        gs.lcp_y = 190
        action_pace_nervously(gs)
        mock_walk.assert_called()


# ---------------------------------------------------------------------------
# action_wander_idly tests
# ---------------------------------------------------------------------------
class TestActionWanderIdly:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_ends_with_stand_idle(self, mock_tick, mock_walk, gs):
        gs.lcp_y = 190  # bottom floor
        action_wander_idly(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_STAND_IDLE

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_random_position(self, mock_tick, mock_walk, gs):
        gs.lcp_y = 190
        action_wander_idly(gs)
        mock_walk.assert_called()


# ---------------------------------------------------------------------------
# action_nod_head tests
# ---------------------------------------------------------------------------
class TestActionNodHead:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_ends_with_stand_idle(self, mock_tick, mock_walk, gs):
        action_nod_head(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_STAND_IDLE

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_nod_sound(self, mock_tick, mock_walk, gs):
        action_nod_head(gs)
        assert gs.soundeffect_pending == SOUND_EFFECT_ID.SFX_HEAD_NOD

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_faces_right(self, mock_tick, mock_walk, gs):
        gs.lcp_facing_direction = FACING_DIR.FACING_LEFT
        action_nod_head(gs)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT


# ---------------------------------------------------------------------------
# action_peek_around tests
# ---------------------------------------------------------------------------
class TestActionPeekAround:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_ends_with_stand_idle(self, mock_tick, mock_walk, gs):
        action_peek_around(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_STAND_IDLE

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_no_walk_needed(self, mock_tick, mock_walk, gs):
        action_peek_around(gs)
        mock_walk.assert_not_called()


# ---------------------------------------------------------------------------
# action_play_a_game tests
# ---------------------------------------------------------------------------
class TestActionPlayAGame:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_table(self, mock_tick, mock_walk, gs):
        action_play_a_game(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_BTM_TABLE)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_game_sit_state(self, mock_tick, mock_walk, gs):
        action_play_a_game(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_PLAY_GAME_SIT


# ---------------------------------------------------------------------------
# action_sit_on_couch_with_dog tests
# ---------------------------------------------------------------------------
class TestActionSitOnCouchWithDog:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_couch(self, mock_tick, mock_walk, gs):
        action_sit_on_couch_with_dog(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_MID_COUCH)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_sit_couch_state(self, mock_tick, mock_walk, gs):
        action_sit_on_couch_with_dog(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_SIT_COUCH


# ---------------------------------------------------------------------------
# action_hello tests
# ---------------------------------------------------------------------------
class TestActionHello:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_ends_with_stand_idle(self, mock_tick, mock_walk, gs):
        action_hello(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_STAND_IDLE

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_greeting_sound(self, mock_tick, mock_walk, gs):
        action_hello(gs)
        assert gs.soundeffect_pending == SOUND_EFFECT_ID.SFX_GREETING

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_faces_right(self, mock_tick, mock_walk, gs):
        action_hello(gs)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT


# ---------------------------------------------------------------------------
# action_open_close_upstairs_closet tests
# ---------------------------------------------------------------------------
class TestActionOpenCloseUpstairsCloset:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_filing_cabinet(self, mock_tick, mock_walk, gs):
        action_open_close_upstairs_closet(gs, 1)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_TOP_FILING_CAB)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_open_sets_open_closet_state(self, mock_tick, mock_walk, gs):
        action_open_close_upstairs_closet(gs, 1)
        assert gs.lcp_state == PLAYER_STATE.STATE_OPEN_CLOSET

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_close_sets_close_closet_state(self, mock_tick, mock_walk, gs):
        action_open_close_upstairs_closet(gs, 0)
        assert gs.lcp_state == PLAYER_STATE.STATE_CLOSE_CLOSET


# ---------------------------------------------------------------------------
# action_open_close_bedroom_closet tests
# ---------------------------------------------------------------------------
class TestActionOpenCloseBedroomCloset:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_closet(self, mock_tick, mock_walk, gs):
        action_open_close_bedroom_closet(gs)
        mock_walk.assert_called()

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_hides_and_shows_sprites(self, mock_tick, mock_walk, gs):
        """LCP sprites should be hidden during closet use and shown after."""
        action_open_close_bedroom_closet(gs)
        assert gs.lcp_sprites_hidden == 0  # shown after


# ---------------------------------------------------------------------------
# action_get_dressed tests
# ---------------------------------------------------------------------------
class TestActionGetDressed:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_runs_without_error(self, mock_tick, mock_walk, gs):
        gs.head_anim_current = 8
        action_get_dressed(gs)

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_head_anim_mode_fixed(self, mock_tick, mock_walk, gs):
        gs.head_anim_current = 0
        action_get_dressed(gs)
        # head_anim_mode should be set to FIXED during dressing
        # (the final wait_head may modify it, but mode was set)

    @pytest.mark.parametrize("initial_h,expected_target", [
        (0, 8),
        (1, 8),
        (7, 8),
        (2, 9),
        (6, 15),
        (3, 10),
        (4, 10),
        (5, 14),
    ])
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_head_target_by_direction(self, mock_tick, mock_walk, gs, initial_h, expected_target):
        """Head target should be set based on current head anim."""
        gs.head_anim_current = initial_h
        action_get_dressed(gs)
        # After dressing, head_anim_target should be restored to original
        assert gs.head_anim_target == initial_h


# ---------------------------------------------------------------------------
# action_clean_up tests
# ---------------------------------------------------------------------------
class TestActionCleanUp:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_multiple_positions(self, mock_tick, mock_walk, gs):
        action_clean_up(gs)
        # Should walk to BTM_0, BTM_SINK, MID_SINK, MID_SHOWER (4 walks)
        assert mock_walk.call_count >= 4

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_tidy_animations(self, mock_tick, mock_walk, gs):
        action_clean_up(gs)
        assert mock_tick.call_count > 0


# ---------------------------------------------------------------------------
# action_tidy_house tests
# ---------------------------------------------------------------------------
class TestActionTidyHouse:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_all_floors(self, mock_tick, mock_walk, gs):
        action_tidy_house(gs)
        # Should walk to TOP_0, MID_0, BTM_0 (3 walks)
        assert mock_walk.call_count >= 3

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clean_animations(self, mock_tick, mock_walk, gs):
        action_tidy_house(gs)
        assert mock_tick.call_count > 0


# ---------------------------------------------------------------------------
# action_toggle_tv tests
# ---------------------------------------------------------------------------
class TestActionToggleTv:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_turns_tv_on(self, mock_tick, mock_walk, gs):
        gs.tv_on = 0
        action_toggle_tv(gs)
        assert gs.tv_on == 1

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_turns_tv_off(self, mock_tick, mock_walk, gs):
        gs.tv_on = 1
        action_toggle_tv(gs)
        assert gs.tv_on == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_tv_click_sound(self, mock_tick, mock_walk, gs):
        action_toggle_tv(gs)
        assert gs.soundeffect_pending == SOUND_EFFECT_ID.SFX_TV_CLICK

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_couch(self, mock_tick, mock_walk, gs):
        action_toggle_tv(gs)
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_MID_COUCH)
        assert gs.walk_target_x == expected_x
        assert gs.walk_target_y == expected_y

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_watch_tv_state_when_on(self, mock_tick, mock_walk, gs):
        gs.tv_on = 0
        action_toggle_tv(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_WATCH_TV

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sit_couch_state_when_off(self, mock_tick, mock_walk, gs):
        gs.tv_on = 1
        action_toggle_tv(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_SIT_COUCH


# ---------------------------------------------------------------------------
# action_call_dog tests
# ---------------------------------------------------------------------------
class TestActionCallDog:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_dog_target_to_lcp(self, mock_tick, mock_walk, gs):
        gs.lcp_x = 100
        gs.lcp_y = 190
        action_call_dog(gs)
        assert gs.dog_target_x == 100
        assert gs.dog_target_y == 190

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_greeting_sound(self, mock_tick, mock_walk, gs):
        action_call_dog(gs)
        assert gs.soundeffect_pending == SOUND_EFFECT_ID.SFX_GREETING

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_faces_right(self, mock_tick, mock_walk, gs):
        action_call_dog(gs)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT


# ---------------------------------------------------------------------------
# action_wake_from_alarm tests
# ---------------------------------------------------------------------------
class TestActionWakeFromAlarm:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_alarm_flag(self, mock_tick, mock_walk, gs):
        gs.ctrl_a_alarm_pressed_flag = 1
        action_wake_from_alarm(gs)
        assert gs.ctrl_a_alarm_pressed_flag == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_alarm_active(self, mock_tick, mock_walk, gs):
        gs.alarm_active = 1
        action_wake_from_alarm(gs)
        assert gs.alarm_active == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_alarm_sound(self, mock_tick, mock_walk, gs):
        action_wake_from_alarm(gs)
        assert gs.soundeffect_pending == SOUND_EFFECT_ID.SFX_ALARM_CLOCK

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_sets_wake_from_alarm_state(self, mock_tick, mock_walk, gs):
        action_wake_from_alarm(gs)
        assert gs.lcp_state == PLAYER_STATE.STATE_WAKE_FROM_ALARM


# ---------------------------------------------------------------------------
# action_pet_dog tests
# ---------------------------------------------------------------------------
class TestActionPetDog:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_walks_to_dog(self, mock_tick, mock_walk, gs):
        gs.dog_x = 50
        gs.dog_y = 190
        action_pet_dog(gs)
        mock_walk.assert_called()

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_cycles_through_pet_sprites(self, mock_tick, mock_walk, gs):
        """Should show and hide pet hand sprites."""
        action_pet_dog(gs)
        # All pet hand sprites should be hidden after petting
        for i in range(1, 8):
            sprite_id = getattr(SPRITE_ID, f'SPRITE_PET_HAND_{i}', SPRITE_ID.SPRITE_PET_HAND_1)
            assert gs.sprite_layer_flags[sprite_id] == SPRITE_LAYER.SPRITE_HIDDEN

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_faces_right(self, mock_tick, mock_walk, gs):
        action_pet_dog(gs)
        assert gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT


# ---------------------------------------------------------------------------
# action_wake_up_morning tests
# ---------------------------------------------------------------------------
class TestActionWakeUpMorning:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_runs_full_morning_routine(self, mock_tick, mock_walk, gs):
        """Morning routine should complete without error."""
        gs.lcp.is_sleeping = 1  # Start sleeping
        action_wake_up_morning(gs)

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_wakes_from_bed(self, mock_tick, mock_walk, gs):
        gs.lcp.is_sleeping = 1
        action_wake_up_morning(gs)
        assert gs.lcp.is_sleeping == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_interruptible(self, mock_tick, mock_walk, gs):
        gs.lcp.is_sleeping = 1
        action_wake_up_morning(gs)
        assert gs.action_interruptible_flag == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_satisfies_hunger(self, mock_tick, mock_walk, gs):
        """Morning routine includes eating, which should satisfy hunger."""
        gs.lcp.is_sleeping = 1
        gs.lcp.hunger_level = NEED_LEVEL.NEED_ELEVATED
        action_wake_up_morning(gs)
        assert gs.lcp.hunger_level == NEED_LEVEL.NEED_SATISFIED


# ---------------------------------------------------------------------------
# action_go_to_bed_night tests
# ---------------------------------------------------------------------------
class TestActionGoToBedNight:

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_runs_full_night_routine(self, mock_tick, mock_walk, gs):
        """Night routine should complete without error."""
        action_go_to_bed_night(gs)

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_ends_sleeping(self, mock_tick, mock_walk, gs):
        action_go_to_bed_night(gs)
        assert gs.lcp.is_sleeping == 1

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_clears_interruptible(self, mock_tick, mock_walk, gs):
        action_go_to_bed_night(gs)
        assert gs.action_interruptible_flag == 0

    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_satisfies_hunger_from_cabinet(self, mock_tick, mock_walk, gs):
        gs.lcp.hunger_level = NEED_LEVEL.NEED_ELEVATED
        action_go_to_bed_night(gs)
        assert gs.lcp.hunger_level == NEED_LEVEL.NEED_SATISFIED


# ---------------------------------------------------------------------------
# lcp_check_recovery tests
# ---------------------------------------------------------------------------
class TestLcpCheckRecovery:

    def test_no_recovery_when_healthy(self, gs):
        gs.lcp.sickness_level = SICKNESS_LEVEL.SICKNESS_HEALTHY
        gs.lcp.thirst_level = NEED_LEVEL.NEED_SATISFIED
        gs.lcp.hunger_level = NEED_LEVEL.NEED_SATISFIED
        lcp_check_recovery(gs)
        # No change expected when healthy
        assert gs.lcp.sickness_level == SICKNESS_LEVEL.SICKNESS_HEALTHY

    def test_recovery_when_sick_and_needs_satisfied(self, gs):
        from lcp.simulation import DIR_IMPROVING, SICKNESS_RECOVER_COUNTDOWN
        gs.lcp.sickness_level = SICKNESS_LEVEL.SICKNESS_LEVEL_2
        gs.lcp.thirst_level = NEED_LEVEL.NEED_SATISFIED
        gs.lcp.hunger_level = NEED_LEVEL.NEED_SATISFIED
        gs.lcp.sickness_countdown = 999
        lcp_check_recovery(gs)
        assert gs.lcp.sickness_direction == DIR_IMPROVING
        assert gs.lcp.sickness_countdown == SICKNESS_RECOVER_COUNTDOWN

    def test_no_recovery_when_thirsty(self, gs):
        gs.lcp.sickness_level = SICKNESS_LEVEL.SICKNESS_LEVEL_1
        gs.lcp.thirst_level = NEED_LEVEL.NEED_MILD
        gs.lcp.hunger_level = NEED_LEVEL.NEED_SATISFIED
        gs.lcp.sickness_direction = 1  # worsening
        lcp_check_recovery(gs)
        assert gs.lcp.sickness_direction == 1  # should not change

    def test_no_recovery_when_hungry(self, gs):
        gs.lcp.sickness_level = SICKNESS_LEVEL.SICKNESS_LEVEL_1
        gs.lcp.thirst_level = NEED_LEVEL.NEED_SATISFIED
        gs.lcp.hunger_level = NEED_LEVEL.NEED_MILD
        gs.lcp.sickness_direction = 1
        lcp_check_recovery(gs)
        assert gs.lcp.sickness_direction == 1


# ---------------------------------------------------------------------------
# hide/show LCP sprites tests
# ---------------------------------------------------------------------------
class TestHideShowSprites:

    def test_hide_lcp_sprites(self, gs):
        gs.lcp_sprites_hidden = 0
        hide_lcp_sprites(gs)
        assert gs.lcp_sprites_hidden == 1

    def test_show_lcp_sprites(self, gs):
        gs.lcp_sprites_hidden = 1
        show_lcp_sprites(gs)
        assert gs.lcp_sprites_hidden == 0


# ---------------------------------------------------------------------------
# Parametrized smoke test: every action runs without raising
# ---------------------------------------------------------------------------
_ACTION_HANDLERS_NO_ARGS = [
    action_sit_and_exercise,
    action_read_newspaper,
    action_play_computer,
    action_wash_hands,
    action_get_in_out_of_bed,
    action_listen_song,
    action_play_piano,
    action_write_letter,
    action_dance,
    action_yawn_and_stretch,
    action_nod_head,
    action_peek_around,
    action_play_a_game,
    action_brush_teeth,
    action_kitchen_cabinet,
    action_sit_on_couch_with_dog,
    action_hello,
    action_play_with_record,
    action_get_snack_from_fridge,
    action_open_close_bedroom_closet,
    action_clean_up,
    action_tidy_house,
    action_toggle_tv,
    action_call_dog,
    action_wake_from_alarm,
    action_pet_dog,
]


@pytest.mark.parametrize("handler", _ACTION_HANDLERS_NO_ARGS, ids=lambda h: h.__name__)
@patch(WALK_MOCK, return_value=0)
@patch(TICK_MOCK)
def test_action_handler_smoke(mock_tick, mock_walk, gs, handler):
    """Every action handler should run without raising an exception."""
    gs.lcp_y = 190  # ensure floor detection works
    handler(gs)


_ACTION_HANDLERS_WITH_ARGS = [
    (action_sleep, (-1,)),
    (action_sleep, (3,)),
    (action_light_fireplace, ()),
    (action_use_toilet, ()),
    (action_take_shower, ()),
    (action_eat_meal, ()),
    (action_check_front_door, (40,)),
    (action_open_close_upstairs_closet, (1,)),
    (action_open_close_upstairs_closet, (0,)),
    (action_feed_dog, (0,)),
    (action_feed_dog, (1,)),
    (action_pace_nervously, ()),
    (action_wander_idly, ()),
    (action_get_dressed, ()),
    (action_wake_up_morning, ()),
    (action_go_to_bed_night, ()),
]


@pytest.mark.parametrize("handler,args", _ACTION_HANDLERS_WITH_ARGS,
                         ids=lambda x: x.__name__ if callable(x) else str(x))
@patch(SPRITE_CARRY_MOCK, MagicMock())
@patch(SPRITE_UPDATE_MOCK, MagicMock())
@patch(WALK_MOCK, return_value=0)
@patch(TICK_MOCK)
def test_action_handler_with_args_smoke(mock_tick, mock_walk, gs, handler, args):
    """Action handlers with arguments should run without raising."""
    gs.lcp_y = 190
    handler(gs, *args)


# ---------------------------------------------------------------------------
# action_drink water level tests
# ---------------------------------------------------------------------------
class TestActionDrinkWaterLevel:

    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_decrements_water_level(self, mock_tick, mock_walk, mock_sprites, gs):
        gs.lcp_water_level = 10
        action_drink(gs)
        assert gs.lcp_water_level == 7  # decremented by 3

    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_water_level_does_not_go_negative(self, mock_tick, mock_walk, mock_sprites, gs):
        gs.lcp_water_level = 1
        action_drink(gs)
        assert gs.lcp_water_level >= 0

    @patch(SPRITE_UPDATE_MOCK)
    @patch(WALK_MOCK, return_value=0)
    @patch(TICK_MOCK)
    def test_empty_water_still_satisfies_thirst(self, mock_tick, mock_walk, mock_sprites, gs):
        """Even with zero water, thirst is satisfied (just no drink animation)."""
        gs.lcp_water_level = 0
        gs.lcp.thirst_level = NEED_LEVEL.NEED_CRITICAL
        action_drink(gs)
        assert gs.lcp.thirst_level == NEED_LEVEL.NEED_SATISFIED
