"""
Main game loop for Little Computer People (Atari ST).
Translated from Ghidra decompilation of endless_game_loop() and
game_tick_and_animate().

addr: endless_game_loop(), game_tick_and_animate()

endless_game_loop():
  - If lcp_loaded: position LCP at study door and save
  - Main loop: game_tick_and_animate(0) → check_for_any_action_triggers()
  - Copy-protection always passes in Python (copyprot_check_return = 1)

game_tick_and_animate(counter):
  - Waits for (counter + 1) animation frames
  - Per frame:
      screen_render_8hz → clock animation → game_simulate_one_second
      → dog bowl / fire / alarm / phone / TV
      → sprite_update_body + sprite_lcp_head_animate + sprite_lcp_head_update
      → keyboard input
  - Also handles carried-object sprite positioning (separate path)
"""

from .state import GameState
from .enums import ACTION_ID, FACING_DIR, DOG_BOWL_STATUS, SOUND_EFFECT_ID


# ---------------------------------------------------------------------------
# game_tick_and_animate — wait for N render frames
# addr: game_tick_and_animate()
# ---------------------------------------------------------------------------

def game_tick_and_animate(gs: GameState, counter: int) -> None:
    """
    Advance the simulation by (counter + 1) animation frames.
    Each frame calls screen_render_8hz() and all per-frame subsystems.
    addr: game_tick_and_animate()

    counter=0 → wait exactly 1 frame
    counter=N → wait N+1 frames
    """
    from .render import screen_render_8hz_headless
    from .sprites import sprite_update_body, sprite_lcp_head_animate, sprite_lcp_head_update, update_carried_object_sprite
    from .simulation import game_simulate_one_second
    from .sound import soundeffect_select, soundeffects_off

    # Carried-object path: update sprite position and return
    if gs.lcp_carrying_object_flag:
        update_carried_object_sprite(gs)
        return

    count = gs.animation_tick_counter
    for _ in range(counter + 1):
        # Wait for next render frame
        while count == gs.animation_tick_counter:
            _screen_render_8hz(gs)
        count = gs.animation_tick_counter

        # Sub-frame animation counter (used for fire/alarm/phone animations)
        gs.sub_animation_frame_counter += 1

        # Clock animation (4-frame cycle at 1/4 frame rate)
        # addr: object_draw(_object_clock_animation[...], 271, 92)
        _clock_frame = (gs.sub_animation_frame_counter >> 2) & 3
        gs.clock_animation_frame = _clock_frame

        # Time simulation
        game_simulate_one_second(gs)

        # Petting animation
        # addr: petting_dog_active branch in game_tick_and_animate
        if gs.petting_dog_active:
            if gs.petting_anim_frame < 11:
                from .sprites import spritedata_select
                from .enums import SPRITE_LAYER
                # Hide previous petting frame
                if gs.petting_anim_frame > 0:
                    prev = gs.petting_anim_frame - 1
                    if prev < 60:
                        gs.sprite_layer_flags[prev] = SPRITE_LAYER.SPRITE_HIDDEN
                # Show current petting frame
                frame_id = gs.petting_anim_frame
                if frame_id < 60:
                    gs.sprite_layer_flags[frame_id] = SPRITE_LAYER.SPRITE_BEHIND_LCP
                    spritedata_select(gs, frame_id)
                    slot = gs.sprite_slot_map[frame_id]
                    if 0 <= slot < 8:
                        gs.sprite_pending_x[slot] = 192
                        gs.sprite_pending_y[slot] = 165
                gs.petting_anim_frame += 1
            else:
                slot = gs.petting_last_sprite_slot
                if 0 <= slot < 60:
                    gs.sprite_layer_flags[slot] = 0
                from .sprites import sprite_update_slots
                sprite_update_slots(gs)
                gs.petting_dog_active = 0

        # Dog bowl animation (3 states: EMPTY=0, HALF=1, FULL=2)
        # addr: object_draw(_object_dog_eating_animation[lcp_dog_bowl_status], 8, 190)
        if gs.dog_food_bowl_change < 0:
            if gs.dog_bowl_status != DOG_BOWL_STATUS.BOWL_EMPTY:
                gs.dog_bowl_status -= 1
            if gs.dog_bowl_status < 0:
                gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_EMPTY
        elif gs.dog_food_bowl_change > 0:
            gs.dog_bowl_status += 1
            if gs.dog_bowl_status > DOG_BOWL_STATUS.BOWL_FULL:
                gs.dog_bowl_status = DOG_BOWL_STATUS.BOWL_FULL

        # Fire animation and countdown
        # addr: fire_active_flag branch in game_tick_and_animate
        if gs.fire_active_flag:
            gs.fire_animation_frame = gs.sub_animation_frame_counter & 3
            gs.fire_duration_countdown -= 1
            if gs.fire_duration_countdown == 0:
                gs.fire_extinguish_flag = 1

        if gs.fire_extinguish_flag:
            gs.fire_extinguish_flag = 0
            gs.fire_active_flag = 0

        # Alarm animation and sound
        # addr: ctrl_a_alarm_pressed_flag branch
        if gs.ctrl_a_alarm_pressed_flag:
            if not gs.alarm_sound_started:
                soundeffect_select(gs, SOUND_EFFECT_ID.SFX_ALARM_CLOCK, 100000)
                gs.alarm_sound_started = 1
            elif not gs.soundeffect_playing_flag:
                soundeffect_select(gs, SOUND_EFFECT_ID.SFX_ALARM_CLOCK, 100000)
            gs.alarm_active = gs.sub_animation_frame_counter & 1
        else:
            gs.alarm_sound_started = 0
            if (gs.soundeffect_playing_flag and
                    gs.soundeffect_playing_id == SOUND_EFFECT_ID.SFX_ALARM_CLOCK):
                soundeffects_off(gs)

        # Phone ring animation and sound
        # addr: phone_call_active_flag branch
        if gs.phone_call_active_flag:
            if gs.phone_ring_countdown == 0:
                soundeffect_select(gs, SOUND_EFFECT_ID.SFX_PHONE_RING, 10000)
                gs.phone_ring_countdown = 0x1a   # 26 ticks
            gs.phone_ring_countdown -= 1

        # Phone hangup cleanup
        if gs.phone_hangup_flag:
            gs.phone_hangup_flag = 0
            gs.phone_ring_countdown = 0
            if (gs.soundeffect_playing_flag and
                    gs.soundeffect_playing_id == SOUND_EFFECT_ID.SFX_PHONE_RING):
                soundeffects_off(gs)

        # Sound effect tick countdown is handled in screen_render_8hz() (render.py),
        # NOT here — it runs at render frame rate, not game tick rate.

        # Record player and TV animations
        # addr: lcp_record_playing, lcp_tv_on branches
        # (Full animation handling requires object sprites from assets)

        # LCP body and head sprite update
        sprite_update_body(gs)
        sprite_lcp_head_animate(gs)
        sprite_lcp_head_update(gs)

        # Keyboard input
        # addr: disable_key_input_flag / intro_sequence_active / game_input_mode_flag
        if gs.screen_scroll_down_count < 1:
            if not gs.disable_key_input_flag and not gs.intro_sequence_active:
                _process_keyboard(gs)
            elif gs.game_input_mode_flag:
                # Minigame mode: still process keys for game input
                _process_keyboard(gs)
        else:
            gs.screen_scroll_down_count -= 1

        # Second render call at end of frame (matches Ghidra: screen_render_8hz at loop tail)
        _screen_render_8hz(gs)


def _screen_render_8hz(gs: GameState) -> None:
    """
    Call either the Pygame renderer or the headless fallback.
    addr: screen_render_8hz() call sites

    Timing (faithful to original Atari ST):
      The original screen_render_8hz() checks the 200 Hz system timer
      and returns immediately if < 125ms since last frame.  The caller
      (game_tick_and_animate) busy-waits until the counter increments.
      In Python:
        - Pygame renderer: render_frame() sleeps until frame is due,
          then renders and increments counter.  No busy-wait needed.
        - Headless + _realtime: sleeps to maintain 8 Hz.
        - Headless (default): no sleep — runs at full speed (for tests).
    """
    if hasattr(gs, '_renderer') and gs._renderer is not None and gs._renderer.initialized:
        gs._renderer.render_frame(gs)
    else:
        from .render import screen_render_8hz_headless
        screen_render_8hz_headless(gs)


def _process_keyboard(gs: GameState) -> None:
    """
    Handle typed keyboard input and Ctrl shortcuts.
    addr: game_tick_and_animate() keyboard input section
    Integrates with nlp.py command parsing.
    """
    # Input is delivered externally (e.g. Pygame events → gs.keyboard_input_buffer)
    # Here we drain the buffer and parse any complete commands.
    if not gs.keyboard_input_buffer:
        return

    from .nlp import parse_and_queue_command
    from .ai import add_command_to_queue

    # Ctrl shortcuts
    key = gs.keyboard_input_buffer
    gs.keyboard_input_buffer = ''

    ctrl_map = {
        '\x01': 'ctrl_a_alarm_pressed_flag',  # Ctrl-A alarm
        '\x02': 'ctrl_b_book_flag',            # Ctrl-B book
        '\x03': 'ctrl_c_phone_flag',           # Ctrl-C call
        '\x04': 'ctrl_d_dog_food_flag',        # Ctrl-D dog food
        '\x06': 'ctrl_f_food_flag',            # Ctrl-F food
        '\x10': 'ctrl_p_pet_flag',             # Ctrl-P pet
        '\x12': 'ctrl_r_record_flag',          # Ctrl-R record
        '\x17': 'ctrl_w_water_flag',           # Ctrl-W water
    }
    if key in ctrl_map:
        setattr(gs, ctrl_map[key], 1)
        return

    # Normal text — try NLP parse when Enter-terminated
    if '\r' in key or '\n' in key:
        sentence = key.rstrip('\r\n')
        if sentence:
            parse_and_queue_command(gs, sentence)
        gs.text_scroll_timer = 0
    else:
        # Accumulate into display buffer; reset scroll timer
        if gs.text_scroll_timer == 0:
            gs.command_input_buffer_pos = 0
        gs.text_scroll_timer = 160


# ---------------------------------------------------------------------------
# endless_game_loop — top-level entry point
# addr: endless_game_loop()
# ---------------------------------------------------------------------------

def endless_game_loop(gs: GameState) -> None:
    """
    Top-level game loop. Runs until process is killed.
    addr: endless_game_loop()

    Flow:
      1. If save file was loaded: position LCP at study door and enter study.
      2. Loop forever: game_tick_and_animate(0) + check_for_any_action_triggers()
      3. Copy protection always passes (copyprot_check_return = 1).
    """
    from .ai import check_for_any_action_triggers
    from .constants import house_get_position_xy
    from .enums import HOUSE_POS

    # If a save file was loaded: start the LCP at the study door
    if gs.lcp_loaded:
        x, y = house_get_position_xy(HOUSE_POS.POS_TOP_STUDY_DOOR)
        gs.lcp_x = x - 10
        gs.lcp_y = y - 3
        # lcp_enter_study_and_save(NO, NO) — stubbed out; just set position

    if not gs.copyprot_check_return:
        # Copy protection failed — sleep forever (never happens in Python)
        while True:
            from .actions import action_sleep
            action_sleep(gs, -1)

    gs.game_speed_counter = 5

    while True:
        game_tick_and_animate(gs, 0)
        check_for_any_action_triggers(gs)
