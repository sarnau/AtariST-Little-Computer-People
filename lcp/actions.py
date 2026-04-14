"""
Action handlers for Little Computer People (Atari ST).
Translated from Ghidra decompilation of do_action() and all action_*() functions.

addr: do_action() + action_*() functions (TEXT segment)

Each action follows the same structure:
  1. Walk to destination: house_get_position_xy(POS) → lcp_walk_to_destination()
  2. Set facing direction and animation state
  3. Wait for animation: game_tick_and_animate(count)
  4. Update game state (needs satisfied, flags set, etc.)
  5. Clear interruptible flag

do_action() cases confirmed from Ghidra decompilation: 40 actions (0-39).
The ARCHITECTURE.md count of "45" includes compound routines and event handlers.
"""

import random
from .enums import (
    ACTION_ID, PLAYER_STATE, FACING_DIR, SPRITE_LAYER, SPRITE_ID,
    SICKNESS_LEVEL, NEED_LEVEL, SOUND_EFFECT_ID, DOG_BOWL_STATUS,
)
from .state import GameState
from .constants import house_get_position_xy, HOUSE_POS


# ---------------------------------------------------------------------------
# Timing helper — game_tick_and_animate stub
# Full implementation in main.py; imported lazily to avoid circular deps.
# ---------------------------------------------------------------------------
def _tick(gs: GameState, count: int = 1) -> None:
    """Wait for `count` animation frames. addr: game_tick_and_animate(count)"""
    from .main import game_tick_and_animate
    game_tick_and_animate(gs, count)


def _walk(gs: GameState, pos: int) -> int:
    """Set walk target to HOUSE_POS and walk. Returns 0 on success, -1 if interrupted."""
    gs.walk_target_x, gs.walk_target_y = house_get_position_xy(pos)
    from .movement import lcp_walk_to_destination
    return lcp_walk_to_destination(gs)


def _walk_xy(gs: GameState, x: int, y: int) -> int:
    """Walk to explicit pixel coords."""
    gs.walk_target_x = x
    gs.walk_target_y = y
    from .movement import lcp_walk_to_destination
    return lcp_walk_to_destination(gs)


def _face_right(gs: GameState) -> None:
    gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT

def _face_left(gs: GameState) -> None:
    gs.lcp_facing_direction = FACING_DIR.FACING_LEFT

def _set_state(gs: GameState, state: int) -> None:
    gs.lcp_state = state

def _head(gs: GameState, target: int) -> None:
    """Set head_anim_target and wait for head to reach it. addr: lcp_wait_head_reach_target()"""
    gs.head_anim_target = target
    _wait_head(gs)

def _wait_head(gs: GameState) -> None:
    """Wait for head animation to reach target. addr: lcp_wait_head_reach_target()"""
    # Blocking poll in original — simulate by ticking until head reaches target
    # In headless mode, head immediately snaps to target
    gs.head_anim_current = gs.head_anim_target

def _soundfx(gs: GameState, sfx_id: int, priority: int = 3) -> None:
    """Queue a sound effect. addr: soundeffect_select()"""
    gs.soundeffect_pending = sfx_id

def _random(lo: int, hi: int) -> int:
    return random.randint(lo, hi)


# ---------------------------------------------------------------------------
# Sickness recovery check
# addr: lcp_check_recovery()
# Called after satisfying a need — start recovery if sickness > 0
# ---------------------------------------------------------------------------
def lcp_check_recovery(gs: GameState) -> None:
    """
    After satisfying thirst or hunger: check if sickness should improve.
    addr: lcp_check_recovery()
    """
    from .simulation import DIR_IMPROVING, SICKNESS_RECOVER_COUNTDOWN
    if gs.lcp.sickness_level > SICKNESS_LEVEL.SICKNESS_HEALTHY:
        if (gs.lcp.thirst_level == NEED_LEVEL.NEED_SATISFIED
                and gs.lcp.hunger_level == NEED_LEVEL.NEED_SATISFIED):
            gs.lcp.sickness_direction = DIR_IMPROVING
            if gs.lcp.sickness_countdown > SICKNESS_RECOVER_COUNTDOWN:
                gs.lcp.sickness_countdown = SICKNESS_RECOVER_COUNTDOWN


# ---------------------------------------------------------------------------
# Hide / show LCP sprites (used during toilet/closet actions)
# addr: hide_lcp_sprites(), show_lcp_sprites()
# ---------------------------------------------------------------------------
def hide_lcp_sprites(gs: GameState) -> None:
    gs.lcp_sprites_hidden = 1

def show_lcp_sprites(gs: GameState) -> None:
    gs.lcp_sprites_hidden = 0


def _action_open_close_front_door(gs: GameState, close: int) -> None:
    """
    Open (close=0) or close (close=1) the front door with sound and animation.
    addr: action_open_close_front_door()
    """
    if close == 0:
        if gs.lcp_front_door_open:
            return
        _face_right(gs)
        _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)   # bend and reach
        _tick(gs, 2)
        gs.lcp_front_door_open = 1  # opening
        _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_OPEN)
        _tick(gs, 2)
        gs.lcp_front_door_open = 2  # fully open
        _tick(gs, 2)
    else:
        if not gs.lcp_front_door_open:
            return
        gs.lcp_front_door_open = 1  # closing
        _tick(gs, 2)
        gs.lcp_front_door_open = 0  # closed
        _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_CLOSE)
        _tick(gs, 2)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _tick(gs, 0)


def _action_open_close_cabinet(gs: GameState, close: int) -> None:
    """
    Open (close=0) or close (close=1) the kitchen cabinet.
    addr: action_open_close_cabinet()
    """
    if close == 0:
        if gs.lcp_cabinet_open:
            return
        gs.lcp_cabinet_open = 1
        _set_state(gs, PLAYER_STATE.STATE_CLEAN_2)   # reach into cabinet
        _tick(gs, 3)
        _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_OPEN)
        _tick(gs, 2)
        gs.lcp_cabinet_open = 2
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        _tick(gs, 2)
    else:
        if not gs.lcp_cabinet_open:
            return
        _set_state(gs, PLAYER_STATE.STATE_CLEAN_2)   # reach into cabinet
        _tick(gs, 3)
        gs.lcp_cabinet_open = 1
        _tick(gs, 2)
        gs.lcp_cabinet_open = 0
        _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_CLOSE)
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        _tick(gs, 2)


# ---------------------------------------------------------------------------
# do_action — main action dispatcher
# addr: do_action()
# 40 cases confirmed from Ghidra decompilation (ACTION_IDs 0–39)
# ---------------------------------------------------------------------------
def do_action(gs: GameState) -> None:
    """
    Dispatch trigger_action to the appropriate action handler.
    Wakes the LCP from sleep first if necessary.
    addr: do_action()
    """
    action_number = gs.trigger_action
    gs.last_action   = action_number
    gs.trigger_action = ACTION_ID.ACTION_NONE

    # Wake from sleep if sleeping
    if gs.lcp.is_sleeping:
        action_get_in_out_of_bed(gs)

    dispatch = {
        ACTION_ID.ACTION_SIT_AND_EXERCISE:      action_sit_and_exercise,
        ACTION_ID.ACTION_READ_NEWSPAPER:         action_read_newspaper,
        ACTION_ID.ACTION_PLAY_COMPUTER:          action_play_computer,
        ACTION_ID.ACTION_WASH_HANDS:             action_wash_hands,
        ACTION_ID.ACTION_GET_IN_OUT_OF_BED:      action_get_in_out_of_bed,
        ACTION_ID.ACTION_LISTEN_SONG:            action_listen_song,
        ACTION_ID.ACTION_PLAY_PIANO:             action_play_piano,
        ACTION_ID.ACTION_WRITE_LETTER:           action_write_letter,
        ACTION_ID.ACTION_DANCE:                  action_dance,
        ACTION_ID.ACTION_YAWN_AND_STRETCH:       action_yawn_and_stretch,
        ACTION_ID.ACTION_PACE_NERVOUSLY:         action_pace_nervously,
        ACTION_ID.ACTION_WANDER_IDLY:            action_wander_idly,
        ACTION_ID.ACTION_DRINK:                  action_drink,
        ACTION_ID.ACTION_NOD_HEAD:               action_nod_head,
        ACTION_ID.ACTION_PEEK_AROUND:            action_peek_around,
        ACTION_ID.ACTION_PLAY_A_GAME:            action_play_a_game,
        ACTION_ID.ACTION_BRUSH_TEETH:            action_brush_teeth,
        ACTION_ID.ACTION_KITCHEN_CABINET:        action_kitchen_cabinet,
        ACTION_ID.ACTION_SIT_ON_COUCH_WITH_DOG:  action_sit_on_couch_with_dog,
        ACTION_ID.ACTION_LIGHT_FIREPLACE:        action_light_fireplace,
        ACTION_ID.ACTION_USE_TOILET:             action_use_toilet,
        ACTION_ID.ACTION_TAKE_SHOWER:            action_take_shower,
        ACTION_ID.ACTION_FEED_DOG:               lambda gs: action_feed_dog(gs, 0),
        ACTION_ID.ACTION_HELLO:                  action_hello,
        ACTION_ID.ACTION_EAT_MEAL:               action_eat_meal,
        ACTION_ID.ACTION_PLAY_WITH_RECORD:       action_play_with_record,
        ACTION_ID.ACTION_OPEN_UPSTAIRS_CLOSET:   lambda gs: action_open_close_upstairs_closet(gs, 1),
        ACTION_ID.ACTION_GET_SNACK_FROM_FRIDGE:  action_get_snack_from_fridge,
        ACTION_ID.ACTION_OPEN_BEDROOM_CLOSET:    action_open_close_bedroom_closet,
        ACTION_ID.ACTION_GET_DRESSED:            action_get_dressed,
        ACTION_ID.ACTION_CLEAN_UP:               action_clean_up,
        ACTION_ID.ACTION_TIDY_HOUSE:             action_tidy_house,
        ACTION_ID.ACTION_CHECK_FRONT_DOOR:       lambda gs: action_check_front_door(gs, 40),
        ACTION_ID.ACTION_TOGGLE_TV:              action_toggle_tv,
        ACTION_ID.ACTION_CALL_DOG:               action_call_dog,
        ACTION_ID.ACTION_WAKE_FROM_ALARM:        action_wake_from_alarm,
        ACTION_ID.ACTION_PET_DOG:               action_pet_dog,
        ACTION_ID.ACTION_WAKE_UP_MORNING:        action_wake_up_morning,
        ACTION_ID.ACTION_GO_TO_BED_NIGHT:        action_go_to_bed_night,
    }

    # ACTION_SLEEP case: action_sleep(-1)
    if action_number == ACTION_ID.ACTION_SLEEP:
        action_sleep(gs, -1)
        return

    handler = dispatch.get(action_number)
    if handler:
        handler(gs)


# ---------------------------------------------------------------------------
# action_sleep
# addr: action_sleep()
# ---------------------------------------------------------------------------
def action_sleep(gs: GameState, value: int) -> None:
    """
    Sleep in place for a randomised number of cycles.
    value=-1: walk to floor centre first; value>=0: sleep for that many cycles.
    addr: action_sleep()
    """
    STATE_SLEEP_0 = PLAYER_STATE.STATE_SLEEP_IN_BED    # breathe in
    STATE_SLEEP_1 = PLAYER_STATE.STATE_SLEEP_LYING     # breathe out

    if gs.lcp_on_stairs_flag:
        return

    if value == -1:
        # Walk to centre of current floor
        from .movement import get_floor_number_from_y, FLOOR_CENTER_Y
        floor = get_floor_number_from_y(gs.lcp_y)
        gs.walk_target_x = gs.lcp_x
        gs.walk_target_y = FLOOR_CENTER_Y[floor - 1]
        from .movement import lcp_walk_to_destination
        if lcp_walk_to_destination(gs) != 0:
            return
        _face_right(gs)
        _set_state(gs, PLAYER_STATE.STATE_STAND_SIDE_VIEW)
        _head(gs, 8)

    count = _random(7, 15) if value == -1 else value
    i = 0
    while i < count and gs.triggered_event_list[0] == ACTION_ID.ACTION_NONE:
        _set_state(gs, STATE_SLEEP_0)
        _tick(gs, 1)
        _set_state(gs, STATE_SLEEP_1)
        _tick(gs, 0)
        _soundfx(gs, SOUND_EFFECT_ID.SFX_SNORING)
        _tick(gs, 1)
        _set_state(gs, STATE_SLEEP_0)
        _tick(gs, 1)
        i += 1

    if value == -1:
        _set_state(gs, PLAYER_STATE.STATE_STAND_SIDE_VIEW)
        _tick(gs, 0)


# ---------------------------------------------------------------------------
# action_drink
# addr: action_drink()
# ---------------------------------------------------------------------------
def action_drink(gs: GameState) -> None:
    """Walk to kitchen sink, pick up glass, fill at water tap, drink."""
    if _walk(gs, HOUSE_POS.POS_BTM_KITCHEN_SINK) != 0:
        return
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _head(gs, 12)
    gs.action_interruptible_flag = 1

    # Pick up glass and carry to water tap
    from .sprites import spritedata_select_carried_object_left
    spritedata_select_carried_object_left(gs, SPRITE_ID.SPRITE_GLASS)
    _walk(gs, HOUSE_POS.POS_BTM_WATER_TAP)

    gs.sprite_layer_flags[3] = SPRITE_LAYER.SPRITE_HIDDEN
    from .sprites import sprite_update_slots
    sprite_update_slots(gs)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _head(gs, 12)

    if gs.lcp_water_level != 0:
        _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)
        _face_right(gs)
        _tick(gs, 0)
        _update_water_level_bar(gs, -3)
        gs.head_anim_mode = 2  # small range (amplitude 1)
        _set_state(gs, PLAYER_STATE.STATE_DRINK_GLASS)
        _tick(gs, 0x10)
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        gs.lcp_y += 1
        _tick(gs, 3)
        # action_drink_water_animation(3) — 3 drink cycles
        _action_drink_water_animation(gs, 3)

    gs.lcp.thirst_level = NEED_LEVEL.NEED_SATISFIED
    gs.lcp.thirst_timer  = gs.lcp.thirst_timer_max
    lcp_check_recovery(gs)
    gs.sprite_layer_flags[3] = SPRITE_LAYER.SPRITE_HIDDEN
    sprite_update_slots(gs)
    gs.lcp_carrying_object_flag = 0
    gs.action_interruptible_flag = 0


def _update_water_level_bar(gs: GameState, delta: int) -> None:
    """Update water level and redraw bar. addr: update_water_level_bar()"""
    gs.lcp_water_level = max(0, gs.lcp_water_level + delta)


def _action_drink_water_animation(gs: GameState, cycles: int) -> None:
    """Drinking animation cycles. addr: action_drink_water_animation()"""
    for _ in range(cycles):
        _set_state(gs, PLAYER_STATE.STATE_DRINK_GLASS)
        _tick(gs, 4)
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        _tick(gs, 2)


# ---------------------------------------------------------------------------
# action_eat_meal
# addr: action_eat_meal()
# ---------------------------------------------------------------------------
def action_eat_meal(gs: GameState) -> None:
    """Walk to cabinet, cook on stove, carry to table, eat."""
    if _walk(gs, HOUSE_POS.POS_BTM_CABINET) != 0:
        return
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _head(gs, 12)
    _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)
    _tick(gs, 1)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _tick(gs, 0)

    # Carry cooking pot to stove
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_COOKING_POT] = SPRITE_LAYER.SPRITE_IN_FRONT
    gs.lcp_carrying_object_flag = 1
    gs.lcp_carried_object = SPRITE_ID.SPRITE_COOKING_POT
    gs.action_interruptible_flag = 1
    _walk(gs, HOUSE_POS.POS_BTM_STOVE)

    # Cook
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_COOKING_POT] = SPRITE_LAYER.SPRITE_HIDDEN
    gs.lcp_carrying_object_flag = 0
    _face_left(gs)
    cook_time = _random(30, 50)
    for _ in range(cook_time):
        _tick(gs, 1)

    # Carry to table
    gs.lcp_carrying_object_flag = 1
    gs.action_interruptible_flag = 1
    _walk(gs, HOUSE_POS.POS_BTM_CABINET)
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_COOKING_POT] = SPRITE_LAYER.SPRITE_HIDDEN
    gs.lcp_carrying_object_flag = 0
    _tick(gs, 0)
    action_kitchen_cabinet(gs)
    gs.action_interruptible_flag = 0


# ---------------------------------------------------------------------------
# action_kitchen_cabinet
# addr: action_kitchen_cabinet()
# ---------------------------------------------------------------------------
def action_kitchen_cabinet(gs: GameState) -> None:
    """Walk to kitchen cabinet and eat — satisfies hunger."""
    if _walk(gs, HOUSE_POS.POS_BTM_CABINET) != 0:
        return
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _head(gs, 12)
    eat_count = _random(8, 20)
    for _ in range(eat_count):
        _set_state(gs, PLAYER_STATE.STATE_EAT_BITE)
        _tick(gs, 2)
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        _tick(gs, 1)
    gs.lcp.hunger_level = NEED_LEVEL.NEED_SATISFIED
    gs.lcp.hunger_timer = gs.lcp.hunger_timer_max
    # Update cabinet fill state in door_states_and_flags bits 9-11
    cabinet = (gs.lcp.door_states_and_flags >> 9) & 7
    if cabinet > 0:
        cabinet -= 1
    gs.lcp.door_states_and_flags = (gs.lcp.door_states_and_flags & ~(7 << 9)) | (cabinet << 9)
    lcp_check_recovery(gs)


# ---------------------------------------------------------------------------
# action_use_toilet
# addr: action_use_toilet()
# ---------------------------------------------------------------------------
def action_use_toilet(gs: GameState) -> None:
    """Walk to toilet door, enter, use toilet, exit."""
    if _walk(gs, HOUSE_POS.POS_MID_TOILET) != 0:
        return
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _head(gs, 12)
    # Door open animation
    if not gs.lcp.toilet_door_open:
        _face_left(gs)
        _tick(gs, 2)
        _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_OPEN)
        _tick(gs, 6)
        gs.lcp.door_states_and_flags |= 0x20  # toilet_door open

    _face_right(gs)
    save_x = gs.lcp_x
    tx, ty = house_get_position_xy(HOUSE_POS.POS_MID_TOILET)
    _walk_xy(gs, tx - 10, ty - 3)
    gs.action_interruptible_flag = 1
    # Close door behind LCP
    hide_lcp_sprites(gs)
    _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_CLOSE)
    _tick(gs, 1)
    # Use toilet
    duration = _random(45, 60)
    _tick(gs, duration)
    _soundfx(gs, SOUND_EFFECT_ID.SFX_TOILET_FLUSH)
    _tick(gs, 16)
    # Open door, exit
    show_lcp_sprites(gs)
    _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_OPEN)
    _tick(gs, 1)
    gs.lcp.door_states_and_flags |= 0x20  # toilet_door open
    gs.lcp_x = save_x
    _walk(gs, HOUSE_POS.POS_MID_TOILET)
    # Maybe close door on exit (random)
    if gs.lcp.initiative_threshold < _random(0, 100) or gs.intro_sequence_active:
        _action_close_toilet_door(gs)
    gs.lcp.bathroom_need = 0
    gs.lcp.bathroom_timer = 9999
    gs.action_interruptible_flag = 0


def _action_close_toilet_door(gs: GameState) -> None:
    """Close the toilet door after use."""
    _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_CLOSE)
    _tick(gs, 3)
    gs.lcp.door_states_and_flags &= ~0x20  # toilet_door closed


# ---------------------------------------------------------------------------
# action_take_shower
# addr: action_take_shower()
# ---------------------------------------------------------------------------
def action_take_shower(gs: GameState) -> None:
    """Walk to shower door, enter, cycle through shower animations, exit."""
    if _walk(gs, HOUSE_POS.POS_MID_SHOWER_DOOR) != 0:
        return
    gs.action_interruptible_flag = 1
    _walk(gs, HOUSE_POS.POS_MID_SHOWER_INSIDE)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_SHOWER_1)
    gs.lcp_x -= 8
    gs.lcp_y -= 0x17   # -23
    _head(gs, 8)
    gs.head_anim_mode = 3  # wider range (amplitude 2) for shower

    shower_count = _random(0x14, 0x19)   # 20–25 cycles
    for _ in range(shower_count):
        if _random(0, 1) == 0:
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_2)
            _tick(gs, 2)
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_3)
            _tick(gs, 2)
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_2)
            _tick(gs, 2)
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_3)
            _tick(gs, 2)
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_1)
            _tick(gs, 4)
        else:
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_4)
            _tick(gs, 2)
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_5)
            _tick(gs, 2)
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_4)
            _tick(gs, 2)
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_5)
            _tick(gs, 2)
            _set_state(gs, PLAYER_STATE.STATE_SHOWER_1)
            _tick(gs, 4)

    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    gs.lcp_y += 0x1d   # +29 (asymmetric with -23 — intentional per Ghidra)
    _tick(gs, 2)
    _walk(gs, HOUSE_POS.POS_MID_SHOWER_EXIT)   # position 24, not shower door
    gs.head_anim_mode = 0  # HEAD_ANIM_DISABLED
    gs.action_interruptible_flag = 0


# ---------------------------------------------------------------------------
# action_get_dressed
# addr: action_get_dressed()
# ---------------------------------------------------------------------------
def action_get_dressed(gs: GameState) -> None:
    """
    Cycle through outfit-change head animation.
    addr: action_get_dressed()
    """
    # Set head to neutral position based on current direction
    current = gs.head_anim_current
    h = current & 7
    if h in (0, 1, 7):
        gs.head_anim_target = 8
    elif h == 2:
        gs.head_anim_target = 9
    elif h == 6:
        gs.head_anim_target = 15
    elif h in (3, 4):
        gs.head_anim_target = 10
    elif h == 5:
        gs.head_anim_target = 14

    gs.head_anim_mode = 2  # small range (amplitude 1)
    _wait_head(gs)

    # 4 dressing cycles: alternate between normal and outfit-change head frames
    for _ in range(4):
        gs.head_anim_target = gs.head_anim_current & 7
        _wait_head(gs)
        gs.head_anim_target = gs.head_anim_current | 0x10
        _wait_head(gs)
    gs.head_anim_target = current
    _wait_head(gs)


# ---------------------------------------------------------------------------
# Remaining action handlers — ported with correct sequence logic
# ---------------------------------------------------------------------------

def action_sit_and_exercise(gs: GameState) -> None:
    """Sit and exercise on the couch/chair."""
    _walk(gs, HOUSE_POS.POS_MID_COUCH)
    _face_right(gs)
    count = _random(10, 20)
    for _ in range(count):
        _set_state(gs, PLAYER_STATE.STATE_EXERCISE_ARMS_UP)
        _tick(gs, 2)
        _set_state(gs, PLAYER_STATE.STATE_EXERCISE_CROUCH)
        _tick(gs, 2)


def action_read_newspaper(gs: GameState) -> None:
    """Sit in armchair and read newspaper.
    addr: action_read_newspaper()
    """
    from .sound import tv_turn_on, tv_turn_off
    tv_turn_on(gs)
    result = _walk(gs, HOUSE_POS.POS_TOP_ARMCHAIR)
    if result == 0:
        gs.head_anim_mode = 4  # HEAD_ANIM_READING
        _face_left(gs)
        _set_state(gs, PLAYER_STATE.STATE_SIT_CHAIR)
        gs.head_anim_target = 0x0a  # HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER
        _wait_head(gs)
        gs.lcp_y += 8
        counter = 0
        while counter < 200 and gs.triggered_event_list[0] == 0xFFFF:
            _face_left(gs)
            _set_state(gs, PLAYER_STATE.STATE_READ_NEWSPAPER)
            if _random(0, 15) == 5:
                _set_state(gs, PLAYER_STATE.STATE_WRITE_LETTER)  # page turn frame
            _tick(gs, 1)
            counter += 1
        gs.lcp_y -= 8
        _face_left(gs)
        _set_state(gs, PLAYER_STATE.STATE_SIT_CHAIR)
        _tick(gs, 2)
        tv_turn_off(gs)


def action_play_computer(gs: GameState) -> None:
    """Walk to computer desk and type."""
    _walk(gs, HOUSE_POS.POS_MID_COMPUTER)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_SIT_DESK)
    _soundfx(gs, SOUND_EFFECT_ID.SFX_TYPEWRITER_KEY)
    type_time = _random(40, 80)
    for _ in range(type_time):
        _set_state(gs, PLAYER_STATE.STATE_TYPE_LEFT)
        _tick(gs, 2)
        _set_state(gs, PLAYER_STATE.STATE_TYPE_RIGHT)
        _tick(gs, 2)
        _soundfx(gs, SOUND_EFFECT_ID.SFX_TYPEWRITER_KEY)


def action_wash_hands(gs: GameState) -> None:
    """Walk to bathroom sink and wash hands."""
    _walk(gs, HOUSE_POS.POS_MID_SINK)
    _face_right(gs)
    _soundfx(gs, SOUND_EFFECT_ID.SFX_WATER_RUNNING)
    wash_time = _random(8, 16)
    _tick(gs, wash_time)
    _soundfx(gs, SOUND_EFFECT_ID.SFX_WATER_TAP)


def action_get_in_out_of_bed(gs: GameState) -> None:
    """Get in or out of bed. addr: action_get_in_out_of_bed()"""
    if gs.lcp.is_sleeping == 0:
        # Getting into bed — walk there first
        if _walk(gs, HOUSE_POS.POS_MID_BED) != 0:
            return
        _face_right(gs)
        _head(gs, 10)
        gs.lcp.is_sleeping = 1
        gs.lcp_x -= 10
        _face_right(gs)
        _set_state(gs, PLAYER_STATE.STATE_GET_IN_BED)
        _tick(gs, 2)
        gs.lcp_x -= 8
        _set_state(gs, PLAYER_STATE.STATE_SLEEP_LYING)
        _tick(gs, 2)
        gs.lcp_x -= 2
        _set_state(gs, PLAYER_STATE.STATE_SLEEP_IN_BED)
        _tick(gs, 2)
    else:
        # Getting out of bed — already there
        _face_right(gs)
        gs.lcp_x += 10
        _set_state(gs, PLAYER_STATE.STATE_SLEEP_LYING)
        _tick(gs, 2)
        gs.lcp_x += 10
        _set_state(gs, PLAYER_STATE.STATE_GET_IN_BED)
        _tick(gs, 2)
        gs.lcp.is_sleeping = 0
        _set_state(gs, PLAYER_STATE.STATE_STAND_IDLE)
        _head(gs, 10)
        _tick(gs, 2)


def action_listen_song(gs: GameState) -> None:
    """Walk to record player area and listen to music.
    addr: action_listen_song()
    """
    if gs.lcp_record_playing:
        return
    result = _walk(gs, HOUSE_POS.POS_TOP_DANCE_FLOOR)
    if result == 0:
        _tick(gs, 2)
        _face_right(gs)
        gs.lcp_record_playing = 1
        # In original: loads random .sng file and calls song_play()
        # For now just set the flag and wait
        listen_time = _random(60, 120)
        _tick(gs, listen_time)


def action_play_piano(gs: GameState) -> None:
    """Walk to record player area (top floor). addr: action_play_piano()"""
    _walk(gs, HOUSE_POS.POS_TOP_DANCE_FLOOR)
    _face_right(gs)
    play_time = _random(60, 120)
    for _ in range(play_time):
        _set_state(gs, PLAYER_STATE.STATE_PLAY_PIANO_1)
        _tick(gs, 2)
        _set_state(gs, PLAYER_STATE.STATE_PLAY_PIANO_2)
        _tick(gs, 2)


def action_write_letter(gs: GameState) -> None:
    """Walk to filing cabinet, get paper, sit at desk and write a letter.
    addr: action_write_letter()
    """
    if gs.lcp_record_playing:
        action_play_piano(gs)
    result = _walk(gs, HOUSE_POS.POS_TOP_FILING_CABINET)
    if result != 0:
        return
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    gs.head_anim_target = 8
    _wait_head(gs)
    # Walk to study door area, then sit at desk
    result = _walk(gs, HOUSE_POS.POS_TOP_STUDY_DOOR)
    if result != 0:
        return
    gs.action_interruptible_flag = 1
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_TYPEWRITER] = SPRITE_LAYER.SPRITE_IN_FRONT
    from .sprites import spritedata_select
    spritedata_select(gs, SPRITE_ID.SPRITE_TYPEWRITER)
    slot = gs.sprite_slot_map.get(SPRITE_ID.SPRITE_TYPEWRITER, -1)
    if 0 <= slot < 8:
        gs.sprite_pending_x[slot] = 201
        gs.sprite_pending_y[slot] = 51
    # Sit and type
    _set_state(gs, PLAYER_STATE.STATE_WRITE_LETTER)
    gs.head_anim_mode = 4  # HEAD_ANIM_READING
    gs.disable_key_input_flag = 1
    write_time = _random(30, 60)
    for _ in range(write_time):
        _set_state(gs, PLAYER_STATE.STATE_TYPE_LEFT)
        _tick(gs, 3)
        _soundfx(gs, SOUND_EFFECT_ID.SFX_TYPEWRITER_KEY)
        _set_state(gs, PLAYER_STATE.STATE_TYPE_RIGHT)
        _tick(gs, 3)
    gs.disable_key_input_flag = 0
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_TYPEWRITER] = SPRITE_LAYER.SPRITE_HIDDEN
    gs.head_anim_mode = 0
    gs.action_interruptible_flag = 0


def action_dance(gs: GameState) -> None:
    """Put on record and dance.
    addr: action_dance()
    """
    if not gs.lcp_record_playing:
        gs.action_interruptible_flag = 1
        action_listen_song(gs)
    gs.action_interruptible_flag = 0
    from .constants import house_get_position_xy
    x, y = house_get_position_xy(HOUSE_POS.POS_TOP_DANCE_FLOOR)
    gs.walk_target_x = x
    gs.walk_target_y = y + 8
    from .movement import lcp_walk_to_destination
    result = lcp_walk_to_destination(gs)
    if result == 0:
        _face_right(gs)
        _set_state(gs, PLAYER_STATE.STATE_STAND_IDLE)
        gs.head_anim_target = 8
        _wait_head(gs)
        counter = 0
        while getattr(gs, 'midi_is_playing', False):
            counter += 1
            if counter & 1:
                _set_state(gs, PLAYER_STATE.STATE_DANCE_RIGHT)
            else:
                _set_state(gs, PLAYER_STATE.STATE_DANCE_LEFT)
            if gs.triggered_event_list[0] != 0xFFFF:
                break
            _tick(gs, 2)
        _set_state(gs, PLAYER_STATE.STATE_STAND_IDLE)
        _tick(gs, 0)


def action_yawn_and_stretch(gs: GameState) -> None:
    """Yawn and stretch in place."""
    _set_state(gs, PLAYER_STATE.STATE_YAWN)
    _tick(gs, 4)
    _set_state(gs, PLAYER_STATE.STATE_STRETCH)
    _tick(gs, 4)
    _set_state(gs, PLAYER_STATE.STATE_STAND_IDLE)
    _tick(gs, 2)


def action_pace_nervously(gs: GameState) -> None:
    """Pace back and forth nervously on the current floor."""
    from .movement import get_floor_number_from_y
    from .constants import FLOOR_BASELINE_Y
    floor = get_floor_number_from_y(gs.lcp_y)
    y = gs.lcp_y
    pace_count = _random(3, 7)
    direction = 1
    for _ in range(pace_count):
        target_x = gs.lcp_x + (30 * direction)
        _walk_xy(gs, target_x, y)
        direction = -direction


def action_wander_idly(gs: GameState) -> None:
    """Wander to a random position on the current floor."""
    from .movement import get_floor_number_from_y
    floor = get_floor_number_from_y(gs.lcp_y)
    wander_pos = HOUSE_POS(floor * 16 + _random(0, 10))
    _walk(gs, wander_pos)
    _set_state(gs, PLAYER_STATE.STATE_STAND_IDLE)
    _tick(gs, _random(8, 20))


def action_nod_head(gs: GameState) -> None:
    """Stand and nod head."""
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_NOD_HEAD)
    _soundfx(gs, SOUND_EFFECT_ID.SFX_HEAD_NOD)
    _tick(gs, 8)
    _set_state(gs, PLAYER_STATE.STATE_STAND_IDLE)


def action_peek_around(gs: GameState) -> None:
    """Peek around a corner."""
    _set_state(gs, PLAYER_STATE.STATE_PEEK_AROUND)
    _tick(gs, 6)
    _set_state(gs, PLAYER_STATE.STATE_STAND_IDLE)
    _tick(gs, 4)


def action_play_a_game(gs: GameState) -> None:
    """Get game from filing cabinet, carry to table, play.
    addr: action_play_a_game()
    """
    gs.dog_visible = 1
    gs.dog_idle_countdown = 1
    result = _walk(gs, HOUSE_POS.POS_TOP_FILING_CABINET)
    if result != 0:
        return
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    gs.head_anim_target = 8
    _wait_head(gs)
    # Carry game box to kitchen table
    _set_state(gs, PLAYER_STATE.STATE_STAND_IDLE)
    gs.action_interruptible_flag = 1
    _walk(gs, HOUSE_POS.POS_BTM_TABLE)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_PLAY_GAME_SIT)
    # Mini-game selection and play handled separately via games/ module
    # For headless mode, just wait
    game_time = _random(60, 120)
    _tick(gs, game_time)
    # Return game to filing cabinet
    _walk(gs, HOUSE_POS.POS_TOP_FILING_CABINET)
    gs.action_interruptible_flag = 0
    gs.dog_visible = 0


def action_brush_teeth(gs: GameState) -> None:
    """Walk to bathroom sink and brush teeth."""
    _walk(gs, HOUSE_POS.POS_MID_SINK)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_BRUSH_TEETH)
    brush_time = _random(10, 20)
    _tick(gs, brush_time)
    _set_state(gs, PLAYER_STATE.STATE_WASH_HANDS)
    _tick(gs, 4)


def action_sit_on_couch_with_dog(gs: GameState) -> None:
    """Walk to couch and sit with the dog."""
    _walk(gs, HOUSE_POS.POS_MID_COUCH)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_SIT_COUCH)
    sit_time = _random(20, 40)
    _tick(gs, sit_time)


def action_light_fireplace(gs: GameState) -> None:
    """Get firewood from outside and light the fireplace.
    addr: action_light_fireplace()
    """
    if gs.fire_active_flag:
        return
    result = _walk(gs, HOUSE_POS.POS_BTM_FRONT_DOOR)
    if result != 0:
        return
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    gs.head_anim_target = 8
    _wait_head(gs)
    # Open front door, go outside, get firewood
    gs.action_interruptible_flag = 1
    from .sprites import spritedata_select_carried_object_left
    spritedata_select_carried_object_left(gs, SPRITE_ID.SPRITE_FIREWOOD)
    # Carry firewood to fireplace area (bottom floor, pos 40 ≈ fireplace hearth)
    _walk(gs, HOUSE_POS.POS_BTM_40)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    gs.head_anim_target = 8
    from .sprites import sprite_update_slots
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_FIREWOOD] = SPRITE_LAYER.SPRITE_HIDDEN
    sprite_update_slots(gs)
    gs.lcp_carrying_object_flag = 0
    _wait_head(gs)
    # Stoke fire
    _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)
    _tick(gs, 1)
    _set_state(gs, PLAYER_STATE.STATE_PICK_UP_OBJECT)
    _tick(gs, 1)
    _set_state(gs, PLAYER_STATE.STATE_LIGHT_FIRE_1)
    _tick(gs, 1)
    for _ in range(10):
        gs.lcp_facing_direction = _random(0, 1)
        _tick(gs, 0)
    gs.fire_active_flag = 1
    gs.fire_duration_countdown = _random(0x9c4, 5000)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _tick(gs, 0)
    gs.action_interruptible_flag = 0


def action_hello(gs: GameState) -> None:
    """Wave hello to the player."""
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_HELLO)
    _soundfx(gs, SOUND_EFFECT_ID.SFX_GREETING)
    _tick(gs, 8)
    _set_state(gs, PLAYER_STATE.STATE_STAND_IDLE)


def action_play_with_record(gs: GameState) -> None:
    """Browse the record shelf and put on a record."""
    _walk(gs, HOUSE_POS.POS_TOP_RECORD_SHELF)
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_VINYL_RECORD] = SPRITE_LAYER.SPRITE_IN_FRONT
    _set_state(gs, PLAYER_STATE.STATE_PLAY_RECORD_1)
    _tick(gs, 4)
    _set_state(gs, PLAYER_STATE.STATE_PLAY_RECORD_2)
    _tick(gs, 4)
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_VINYL_RECORD] = SPRITE_LAYER.SPRITE_HIDDEN
    gs.record_player_on = 1


def action_open_close_upstairs_closet(gs: GameState, value: int) -> None:
    """Open or close the upstairs filing cabinet closet."""
    _walk(gs, HOUSE_POS.POS_TOP_FILING_CAB)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_OPEN_CLOSET if value else PLAYER_STATE.STATE_CLOSE_CLOSET)
    _tick(gs, 4)


def action_get_snack_from_fridge(gs: GameState) -> None:
    """Walk to fridge, get a snack, eat it."""
    _walk(gs, HOUSE_POS.POS_BTM_FRIDGE)
    _face_right(gs)
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_FOOD_PACKAGE] = SPRITE_LAYER.SPRITE_IN_FRONT
    gs.lcp_carrying_object_flag = 1
    gs.lcp_carried_object = SPRITE_ID.SPRITE_FOOD_PACKAGE
    _walk(gs, HOUSE_POS.POS_BTM_TABLE)
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_FOOD_PACKAGE] = SPRITE_LAYER.SPRITE_HIDDEN
    gs.lcp_carrying_object_flag = 0
    eat_count = _random(5, 10)
    for _ in range(eat_count):
        _set_state(gs, PLAYER_STATE.STATE_EAT_BITE)
        _tick(gs, 3)
    gs.lcp.hunger_level = NEED_LEVEL.NEED_SATISFIED
    gs.lcp.hunger_timer = gs.lcp.hunger_timer_max
    lcp_check_recovery(gs)


def action_open_close_bedroom_closet(gs: GameState) -> None:
    """Open the bedroom closet (get dressed)."""
    _walk(gs, HOUSE_POS.POS_MID_CLOSET)
    _face_right(gs)
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_CLOSET_LCP_INSIDE] = SPRITE_LAYER.SPRITE_IN_FRONT
    hide_lcp_sprites(gs)
    _tick(gs, _random(10, 20))
    show_lcp_sprites(gs)
    gs.sprite_layer_flags[SPRITE_ID.SPRITE_CLOSET_LCP_INSIDE] = SPRITE_LAYER.SPRITE_HIDDEN
    action_get_dressed(gs)


def action_clean_up(gs: GameState) -> None:
    """Clean up the house."""
    _walk(gs, HOUSE_POS.POS_BTM_0)
    for pos in [HOUSE_POS.POS_BTM_SINK, HOUSE_POS.POS_MID_SINK, HOUSE_POS.POS_MID_SHOWER]:
        _walk(gs, pos)
        _set_state(gs, PLAYER_STATE.STATE_TIDY_1)
        _tick(gs, 4)
        _set_state(gs, PLAYER_STATE.STATE_TIDY_2)
        _tick(gs, 4)


def action_tidy_house(gs: GameState) -> None:
    """Tidy the house rooms."""
    for pos in [HOUSE_POS.POS_TOP_0, HOUSE_POS.POS_MID_0, HOUSE_POS.POS_BTM_0]:
        _walk(gs, pos)
        _set_state(gs, PLAYER_STATE.STATE_CLEAN_1)
        _tick(gs, 4)
        _set_state(gs, PLAYER_STATE.STATE_CLEAN_2)
        _tick(gs, 4)


def action_check_front_door(gs: GameState, count: int) -> None:
    """
    Walk to front door, open it, look outside, optionally close.
    count: ticks to stand at open door looking outside.
    addr: action_check_front_door()
    """
    from .sprites import sprite_update_slots
    if _walk(gs, HOUSE_POS.POS_BTM_FRONT_DOOR) != 0:
        return
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _head(gs, 12)
    # Open door if closed
    if not gs.lcp.front_door_open:
        _action_open_close_front_door(gs, 0)
    gs.action_interruptible_flag = 1
    # Walk to door position -10 X
    tx, ty = house_get_position_xy(HOUSE_POS.POS_BTM_FRONT_DOOR)
    _walk_xy(gs, tx - 10, ty)
    # Walk to door
    _walk(gs, HOUSE_POS.POS_BTM_FRONT_DOOR)
    # Hide LCP, look outside
    hide_lcp_sprites(gs)
    _tick(gs, count)
    show_lcp_sprites(gs)
    # Walk back
    _walk_xy(gs, tx - 10, ty)
    # Random check: close door based on initiative_threshold
    if _random(0, 100) > gs.lcp.initiative_threshold:
        gs.action_interruptible_flag = 1
        _walk(gs, HOUSE_POS.POS_BTM_FRONT_DOOR)
        _face_right(gs)
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        _head(gs, 12)
        _action_open_close_front_door(gs, 1)
    gs.action_interruptible_flag = 0


def action_toggle_tv(gs: GameState) -> None:
    """Walk to TV and toggle it on/off."""
    _walk(gs, HOUSE_POS.POS_MID_COUCH)
    _face_right(gs)
    gs.tv_on = 1 - gs.tv_on
    _soundfx(gs, SOUND_EFFECT_ID.SFX_TV_CLICK)
    _set_state(gs, PLAYER_STATE.STATE_WATCH_TV if gs.tv_on else PLAYER_STATE.STATE_SIT_COUCH)
    _tick(gs, _random(30, 60) if gs.tv_on else 4)


def action_call_dog(gs: GameState) -> None:
    """Call the dog."""
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_HELLO)
    _soundfx(gs, SOUND_EFFECT_ID.SFX_GREETING)
    _tick(gs, 4)
    # Signal dog to come to LCP position
    gs.dog_target_x = gs.lcp_x
    gs.dog_target_y = gs.lcp_y


def action_wake_from_alarm(gs: GameState) -> None:
    """React to alarm clock."""
    gs.ctrl_a_alarm_pressed_flag = 0
    gs.alarm_active = 0
    _soundfx(gs, SOUND_EFFECT_ID.SFX_ALARM_CLOCK)
    _set_state(gs, PLAYER_STATE.STATE_WAKE_FROM_ALARM)
    _tick(gs, 8)


def action_pet_dog(gs: GameState) -> None:
    """Walk to dog and pet it."""
    _walk_xy(gs, gs.dog_x, gs.dog_y)
    _face_right(gs)
    for i in range(1, 8):
        sprite_id = getattr(SPRITE_ID, f'SPRITE_PET_HAND_{i}', SPRITE_ID.SPRITE_PET_HAND_1)
        gs.sprite_layer_flags[sprite_id] = SPRITE_LAYER.SPRITE_IN_FRONT
        _tick(gs, 3)
        gs.sprite_layer_flags[sprite_id] = SPRITE_LAYER.SPRITE_HIDDEN


def action_wake_up_morning(gs: GameState) -> None:
    """Morning routine: delay, get up, shower, brush teeth, get dressed, eat."""
    _tick(gs, _random(40, 100))
    action_get_in_out_of_bed(gs)
    action_wake_from_alarm(gs)
    gs.action_interruptible_flag = 1
    action_take_shower(gs)
    gs.action_interruptible_flag = 1
    action_brush_teeth(gs)
    gs.action_interruptible_flag = 1
    action_open_close_bedroom_closet(gs)
    gs.action_interruptible_flag = 1
    action_eat_meal(gs)
    gs.action_interruptible_flag = 0


def action_go_to_bed_night(gs: GameState) -> None:
    """Evening routine: shower, change, snack, brush teeth, get in bed."""
    action_take_shower(gs)
    gs.action_interruptible_flag = 1
    action_open_close_bedroom_closet(gs)
    gs.action_interruptible_flag = 1
    action_kitchen_cabinet(gs)
    gs.action_interruptible_flag = 1
    action_brush_teeth(gs)
    gs.action_interruptible_flag = 1
    action_get_in_out_of_bed(gs)
    gs.action_interruptible_flag = 0


def action_feed_dog(gs: GameState, value: int) -> None:
    """
    Get dog food from fridge and fill the dog bowl.
    value=0: full sequence (walk to fridge, get food, carry to bowl, return).
    value!=0: skip fridge walk, go straight to bowl.
    addr: action_feed_dog()
    """
    from .sprites import sprite_update_slots
    from .sprites import sprite_update_slots, spritedata_select_carried_object_left
    if value == 0:
        if _walk(gs, HOUSE_POS.POS_BTM_FRIDGE) != 0:
            return
        _face_right(gs)
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        _head(gs, 12)
        # Fridge open/close animation (simplified — object_draw calls omitted)
        _face_left(gs)
        _set_state(gs, PLAYER_STATE.STATE_PICK_UP_OBJECT)
        _tick(gs, 1)
        _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_OPEN)
        _tick(gs, 2)
        _face_right(gs)
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        _tick(gs, 2)
        _face_left(gs)
        _set_state(gs, PLAYER_STATE.STATE_PICK_UP_OBJECT)
        _tick(gs, 3)
        _face_right(gs)
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        _tick(gs, 2)
        _tick(gs, 1)
        _soundfx(gs, SOUND_EFFECT_ID.SFX_DOOR_OPEN)
        _tick(gs, 1)
        spritedata_select_carried_object_left(gs, SPRITE_ID.SPRITE_FOOD_PACKAGE)

    # Walk to dog bowl
    gs.action_interruptible_flag = 1
    _walk(gs, HOUSE_POS.POS_BTM_DOG_BOWL)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _head(gs, 12)
    gs.sprite_layer_flags[9] = SPRITE_LAYER.SPRITE_HIDDEN
    sprite_update_slots(gs)
    gs.lcp_carrying_object_flag = 0
    _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)
    _tick(gs, 1)
    _set_state(gs, PLAYER_STATE.STATE_FEED_DOG)
    _tick(gs, 2)
    _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)
    _tick(gs, 1)

    # Fill bowl
    gs.dog_food_bowl_change = 1
    gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_FULL
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _tick(gs, 0)

    # Return food to fridge
    spritedata_select_carried_object_left(gs, SPRITE_ID.SPRITE_FOOD_PACKAGE)
    gs.action_interruptible_flag = 1
    _walk(gs, HOUSE_POS.POS_BTM_FRIDGE)
    gs.sprite_layer_flags[9] = SPRITE_LAYER.SPRITE_HIDDEN
    sprite_update_slots(gs)
    gs.lcp_carrying_object_flag = 0
    gs.action_interruptible_flag = 0


# ---------------------------------------------------------------------------
# Event handlers (triggered by Ctrl key combos via put_event_to_list)
# These are dispatched by execute_event() in ai.py, NOT by do_action().
# addr: event_receive_* functions
# ---------------------------------------------------------------------------

def _walk_to_front_door(gs: GameState) -> None:
    """Walk to front door. addr: walk_to_front_door()"""
    _walk(gs, HOUSE_POS.POS_BTM_FRONT_DOOR)


def _open_door_pick_up_item(gs: GameState) -> None:
    """Common doorbell sequence: open door, bend down, pick up item."""
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _head(gs, 12)  # HEAD_ANIM_HORIZONTAL_RANGE
    _action_open_close_front_door(gs, 0)
    _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)   # bend down
    _tick(gs, 1)
    _set_state(gs, PLAYER_STATE.STATE_PICK_UP_OBJECT)    # reach forward
    _tick(gs, 2)
    _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)   # bend down
    _tick(gs, 1)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _tick(gs, 0)
    # Random chance to close door
    if _random(0, 100) > gs.lcp.initiative_threshold:
        _action_open_close_front_door(gs, 1)


def event_receive_food_delivery(gs: GameState) -> None:
    """
    Ctrl+F: Walk to front door, pick up groceries, carry to kitchen cabinet.
    addr: event_receive_food_delivery()
    """
    from .sprites import sprite_update_slots
    from .movement import spritedata_select_carried_object_left

    gs.action_interruptible_flag = 1
    _walk_to_front_door(gs)
    _open_door_pick_up_item(gs)

    if not gs.delivery_is_for_dog:
        spritedata_select_carried_object_left(gs, SPRITE_ID.SPRITE_FOOD_PACKAGE)
        _walk(gs, HOUSE_POS.POS_BTM_CABINET)
        gs.sprite_layer_flags[9] = SPRITE_LAYER.SPRITE_HIDDEN
        sprite_update_slots(gs)
        gs.lcp_carrying_object_flag = 0
        _face_right(gs)
        _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
        _head(gs, 12)
        _action_open_close_cabinet(gs, 0)
        # Stock cabinet (up to 4 food items)
        food_count = (gs.lcp.door_states_and_flags >> 9) & 7
        while food_count + 1 < 5:
            food_count += 1
            gs.lcp.door_states_and_flags = (
                (food_count * 0x200) | (gs.lcp.door_states_and_flags & ~0xE00)
            )
            _set_state(gs, PLAYER_STATE.STATE_CLEAN_2)   # reach into cabinet
            _tick(gs, 3)
            _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
            _tick(gs, 1)
        if _random(0, 100) > gs.lcp.initiative_threshold:
            _action_open_close_cabinet(gs, 1)
        gs.action_interruptible_flag = 0
    else:
        spritedata_select_carried_object_left(gs, SPRITE_ID.SPRITE_FOOD_PACKAGE)
        if gs.dog_bowl_status == DOG_BOWL_STATUS.BOWL_EMPTY:
            action_feed_dog(gs, 1)
        else:
            action_get_snack_from_fridge(gs)
            gs.sprite_layer_flags[9] = SPRITE_LAYER.SPRITE_HIDDEN
            sprite_update_slots(gs)
            gs.lcp_carrying_object_flag = 0


def event_receive_dog_food(gs: GameState) -> None:
    """
    Ctrl+D: Dog food delivery. Sets delivery_is_for_dog, then does food delivery.
    addr: event_receive_dog_food()
    """
    gs.delivery_is_for_dog = 1
    event_receive_food_delivery(gs)
    gs.delivery_is_for_dog = 0


def event_receive_book_delivery(gs: GameState) -> None:
    """
    Ctrl+B: Walk to front door, pick up book, carry to bookshelf.
    addr: event_receive_book_delivery()
    """
    from .sprites import sprite_update_slots
    from .movement import spritedata_select_carried_object_left

    gs.action_interruptible_flag = 1
    _walk_to_front_door(gs)
    _open_door_pick_up_item(gs)
    spritedata_select_carried_object_left(gs, SPRITE_ID.SPRITE_BOOK)
    _walk(gs, HOUSE_POS.POS_MID_BATHROOM_ENTRANCE)
    gs.sprite_layer_flags[0x31] = SPRITE_LAYER.SPRITE_HIDDEN
    sprite_update_slots(gs)
    gs.lcp_carrying_object_flag = 0
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _head(gs, 12)
    _set_state(gs, PLAYER_STATE.STATE_CLEAN_2)   # reach into cabinet
    _tick(gs, 3)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _tick(gs, 2)
    gs.action_interruptible_flag = 0


def event_receive_record_delivery(gs: GameState) -> None:
    """
    Ctrl+R: Walk to front door, pick up record, carry to record player shelf.
    addr: event_receive_record_delivery()
    """
    from .sprites import sprite_update_slots
    from .movement import spritedata_select_carried_object_left

    gs.action_interruptible_flag = 1
    _walk_to_front_door(gs)
    _open_door_pick_up_item(gs)
    spritedata_select_carried_object_left(gs, SPRITE_ID.SPRITE_VINYL_CARRY)
    _walk(gs, HOUSE_POS.POS_TOP_DANCE_FLOOR)
    _face_right(gs)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    gs.sprite_layer_flags[0x32] = SPRITE_LAYER.SPRITE_HIDDEN
    sprite_update_slots(gs)
    gs.lcp_carrying_object_flag = 0
    _head(gs, 12)
    _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)   # bend down
    _tick(gs, 1)
    _set_state(gs, PLAYER_STATE.STATE_PICK_UP_OBJECT)    # reach forward
    _tick(gs, 2)
    _set_state(gs, PLAYER_STATE.STATE_PUT_DOWN_OBJECT)   # bend down
    _tick(gs, 1)
    _set_state(gs, PLAYER_STATE.STATE_STAND_FACING_SCREEN)
    _tick(gs, 0)
    gs.lcp_food_count = getattr(gs, 'lcp_food_count', 0) + 1
    gs.action_interruptible_flag = 0


def event_answer_phone(gs: GameState) -> None:
    """
    Phone call event: walk to phone, pick up, talk, hang up.
    addr: event_answer_phone()
    """
    gs.action_interruptible_flag = 1
    action_call_dog(gs)
    gs.action_interruptible_flag = 0
    gs.head_anim_mode = 0  # HEAD_ANIM_DISABLED
    _head(gs, 8)
    gs.lcp_y += 6
    _set_state(gs, PLAYER_STATE.STATE_PHONE_ANSWER)
    _tick(gs, 1)
    gs.phone_answered_flag = 1
    gs.phone_call_active_flag = 0
    gs.phone_hangup_flag = 1
    _tick(gs, 0)
    _set_state(gs, PLAYER_STATE.STATE_PHONE_TALK)
    _tick(gs, 1)
    # Talk for 40-50 ticks with random head animations
    talk_count = _random(0x28, 0x32)
    for _ in range(talk_count):
        r = _random(0, 2)
        if r == 0:
            gs.head_sprite_frame = 5
        elif r == 1:
            gs.head_sprite_frame = 6
        else:
            gs.head_sprite_frame = getattr(gs, '_phone_head_frame', 0)
        wait = _random(1, 2)
        _tick(gs, wait)
    gs.phone_hangup_flag = 1
    _set_state(gs, PLAYER_STATE.STATE_PHONE_ANSWER)
    _tick(gs, 1)
    gs.lcp_y -= 6
    _set_state(gs, PLAYER_STATE.STATE_EXERCISE_CROUCH)
    _tick(gs, 1)
    # Wait for petting to finish
    while gs.petting_dog_active:
        _tick(gs, 0)
    gs.dog_pettable_flag = 0
    gs.lcp_y -= 2
    gs.head_anim_target = 8
    gs.head_anim_current = 8
    _set_state(gs, PLAYER_STATE.STATE_STAND_SIDE_VIEW)
    _wait_head(gs)
    _tick(gs, 0)
    gs.phone_answered_flag = 0
