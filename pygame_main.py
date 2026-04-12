#!/usr/bin/env python3
"""
Pygame frontend for Little Computer People.
Run: python3 pygame_main.py

Creates a Pygame window and runs the game loop with full rendering.
Keyboard controls:
  Ctrl+A  Alarm clock
  Ctrl+B  Deliver book
  Ctrl+C  Phone call
  Ctrl+D  Dog food
  Ctrl+F  Deliver food
  Ctrl+P  Pet the dog
  Ctrl+R  Deliver record
  Ctrl+W  Deliver water
  Type normally to enter text commands, press Enter to submit.
"""

import sys
import threading

import pygame

from lcp.state import GameState
from lcp.assets import load_all_assets
from lcp.render import Renderer
from lcp.main import game_tick_and_animate
from lcp.ai import check_for_any_action_triggers
from lcp.enums import HOUSE_POS, PLAYER_STATE, FACING_DIR


def main() -> None:
    gs = GameState()
    load_all_assets(gs)

    renderer = Renderer(scale=3)
    renderer.init()

    # Convert house background from PIL Image to pygame.Surface
    if hasattr(gs, '_house_background') and gs._house_background is not None:
        bg = gs._house_background
        if hasattr(bg, 'mode'):
            if bg.mode != 'RGBA':
                bg = bg.convert('RGBA')
            raw = bg.tobytes()
            bg_surf = pygame.image.fromstring(raw, bg.size, 'RGBA')
            renderer.load_background(bg_surf)

    # Attach the renderer so _screen_render_8hz() uses it.
    # The renderer's render_frame() blocks (sleeps) until the next
    # 8 Hz frame boundary, matching the original Atari ST timing:
    # 1 game-second = 8 frames × 125ms = 1 real second.
    gs._renderer = renderer

    # Position LCP at the study door if a save was loaded
    if gs.lcp_loaded:
        from lcp.constants import house_get_position_xy
        x, y = house_get_position_xy(HOUSE_POS.POS_TOP_STUDY_DOOR)
        gs.lcp_x = x - 10
        gs.lcp_y = y - 3

    gs.copyprot_check_return = 1
    gs.game_speed_counter = 5
    gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
    gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT

    # Run the game logic in a daemon thread so the main thread can
    # service Pygame events without blocking on game_tick_and_animate().
    running = True

    def game_thread() -> None:
        while running:
            try:
                game_tick_and_animate(gs, 0)
                check_for_any_action_triggers(gs)
            except Exception as exc:
                print(f"Game error: {exc}")
                import traceback
                traceback.print_exc()

    t = threading.Thread(target=game_thread, daemon=True)
    t.start()

    # Main thread: Pygame event loop
    clock = pygame.time.Clock()
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                _handle_keydown(gs, event)
        clock.tick(60)

    renderer.quit()
    sys.exit(0)


def _handle_keydown(gs: GameState, event: pygame.event.Event) -> None:
    """Map pygame key events to game control flags and text input."""
    mods = pygame.key.get_mods()
    if mods & pygame.KMOD_CTRL:
        ctrl_map = {
            pygame.K_a: 'ctrl_a_alarm_pressed_flag',
            pygame.K_b: 'ctrl_b_book_flag',
            pygame.K_c: 'ctrl_c_phone_flag',
            pygame.K_d: 'ctrl_d_dog_food_flag',
            pygame.K_f: 'ctrl_f_food_flag',
            pygame.K_p: 'ctrl_p_pet_flag',
            pygame.K_r: 'ctrl_r_record_flag',
            pygame.K_w: 'ctrl_w_water_flag',
        }
        if event.key in ctrl_map:
            setattr(gs, ctrl_map[event.key], 1)
    elif event.key == pygame.K_RETURN:
        gs.keyboard_input_buffer += '\n'
    elif event.key == pygame.K_BACKSPACE:
        if gs.keyboard_input_buffer:
            gs.keyboard_input_buffer = gs.keyboard_input_buffer[:-1]
    elif event.unicode and event.unicode.isprintable():
        gs.keyboard_input_buffer += event.unicode


if __name__ == '__main__':
    main()
