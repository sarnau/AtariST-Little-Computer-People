"""
Screen rendering for Little Computer People (Atari ST).
Translated from Ghidra decompilation of screen_render_8hz().

addr: screen_render_8hz()

Original hardware:
  - Atari ST 320×200 4-bitplane display
  - Double-buffered: two 32000-byte screen buffers
  - Rate-limited to ~8 Hz via 200 Hz system timer (25-tick = 125ms gate)
  - Sprites blitted with mask (AND mask, XOR/OR image data)
  - Page flip via XBIOS Setscreen + Vsync

Python implementation:
  - Pygame 320×200 window (scalable)
  - Background surface loaded from assets
  - Sprites drawn as PIL Images with transparency
  - 8 Hz rate gate using time.monotonic()
  - Double-buffer: off-screen Surface + display flip

Text scroll timer modes (faithful to original):
  timer < 0 : partial background copy — only top status bar + bottom game area
  timer = 0 : full background copy (normal mode)
  timer > 0 : split — text area from front buffer, game area from back buffer;
              decremented each frame
"""

import time
from typing import Optional

from .state import GameState
from .sprites import sprite_update_slots
from .sound import soundeffect_select, soundeffects_off
from .enums import SOUND_EFFECT_ID


# ---------------------------------------------------------------------------
# Rate-limiting constants
# addr: screen_render_8hz() timer gate
# ---------------------------------------------------------------------------

FRAME_INTERVAL_S = 1.0 / 8.0   # ~125 ms between frames (~8 Hz)

# Screen geometry (Atari ST 320×200 low res)
SCREEN_W = 320
SCREEN_H = 200

# Split-copy Y thresholds (from blkcopy32 byte offsets in screen_render_8hz)
# 135 × 32 bytes = 4320 bytes → Y ≈ 4320 / (320/8*4) = 27 lines  (text area top)
# Full screen = 1000 × 32 bytes = 32000 bytes = 200 lines
TEXT_AREA_BOTTOM_Y = 27    # lines 0–26 = top text/status strip
# For timer < 0 (partial): 385 blocks = ~12.3 lines top + 615 blocks bottom
PARTIAL_TOP_LINES  = 12


# ---------------------------------------------------------------------------
# Pygame renderer (optional — gracefully absent if pygame not installed)
# ---------------------------------------------------------------------------

try:
    import pygame
    _PYGAME_AVAILABLE = True
except ImportError:
    _PYGAME_AVAILABLE = False


class Renderer:
    """
    Pygame-backed renderer.
    Call init() once, then render_frame(gs) each game tick.
    addr: screen_render_8hz() — Python Pygame equivalent
    """

    def __init__(self, scale: int = 3) -> None:
        self.scale       = scale
        self.initialized = False
        self._surface: Optional['pygame.Surface'] = None
        self._screen:  Optional['pygame.Surface'] = None
        self._last_frame_time = 0.0
        self._background: Optional['pygame.Surface'] = None

    def init(self) -> None:
        if not _PYGAME_AVAILABLE:
            return
        pygame.init()
        self._screen  = pygame.display.set_mode(
            (SCREEN_W * self.scale, SCREEN_H * self.scale),
            pygame.RESIZABLE,
        )
        pygame.display.set_caption('Little Computer People')
        self._surface = pygame.Surface((SCREEN_W, SCREEN_H))
        self.initialized = True

    def load_background(self, bg_surface: 'pygame.Surface') -> None:
        """Store the house background for compositing."""
        self._background = bg_surface

    def _rate_gate(self) -> bool:
        """Return True if enough time has passed for a new frame (~8 Hz)."""
        now = time.monotonic()
        if now - self._last_frame_time >= FRAME_INTERVAL_S:
            self._last_frame_time = now
            return True
        return False

    def render_frame(self, gs: GameState) -> bool:
        """
        Composite and display one frame.
        Returns True if a frame was rendered, False if rate-limited.
        addr: screen_render_8hz()

        Order matches original:
          1. Dog AI (move, idle countdown, eating, wandering)
          2. Sound effect tick-down
          3. Background copy (3 modes)
          4. Pending→active sprite flush
          5. Sprite compositing
          6. Page flip
          7. Sound effect playback
          8. Frame counter++
        """
        if not self.initialized or not _PYGAME_AVAILABLE:
            # Headless mode — just advance the counter
            _run_per_frame_logic(gs)
            gs.animation_tick_counter += 1
            return True

        if not self._rate_gate():
            return False

        # 1. Dog AI + sound tick-down (pre-render logic)
        _run_per_frame_logic(gs)

        surf = self._surface
        assert surf is not None

        # 3. Background copy (three modes based on text_scroll_timer)
        if self._background is not None:
            if gs.text_scroll_timer < 0:
                surf.blit(self._background, (0, 0),
                          pygame.Rect(0, 0, SCREEN_W, PARTIAL_TOP_LINES))
                surf.blit(self._background,
                          (0, PARTIAL_TOP_LINES),
                          pygame.Rect(0, PARTIAL_TOP_LINES, SCREEN_W,
                                      SCREEN_H - PARTIAL_TOP_LINES))
            elif gs.text_scroll_timer == 0:
                surf.blit(self._background, (0, 0))
            else:
                surf.blit(self._background,
                          (0, TEXT_AREA_BOTTOM_Y),
                          pygame.Rect(0, TEXT_AREA_BOTTOM_Y, SCREEN_W,
                                      SCREEN_H - TEXT_AREA_BOTTOM_Y))
                gs.text_scroll_timer -= 1

        # 4–5. Pending→active flush + sprite compositing
        for slot in range(8):
            if gs.sprite_pending_flag[slot]:
                gs.sprite_pending_flag[slot] = 0
                gs.sprite_pending_x[slot] = gs.sprite_active_x[slot]
                gs.sprite_pending_y[slot] = gs.sprite_active_y[slot]
                gs.sprite_active_image[slot]  = gs.sprite_pending_image[slot]
                gs.sprite_active_mask[slot]   = gs.sprite_pending_mask[slot]
                gs.sprite_active_height[slot] = gs.sprite_pending_height[slot]
                gs.sprite_active_width[slot]  = gs.sprite_pending_width[slot]
            if gs.sprite_active_image[slot] is not None:
                _draw_sprite(surf, gs, slot)

        # 6. Scale to display and flip
        if self.scale != 1:
            scaled = pygame.transform.scale(
                surf, (SCREEN_W * self.scale, SCREEN_H * self.scale)
            )
            self._screen.blit(scaled, (0, 0))
        else:
            self._screen.blit(surf, (0, 0))

        pygame.display.flip()

        # 7. Sound effect playback
        from .sound import soundeffect_irq_play
        soundeffect_irq_play(gs)

        # 8. Frame counter
        gs.animation_tick_counter += 1
        return True

    def handle_events(self) -> list:
        """
        Process pygame events. Returns list of (type, data) tuples.
        Call once per tick from main.py.
        """
        if not _PYGAME_AVAILABLE or not self.initialized:
            return []
        events = []
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                events.append(('quit', None))
            elif event.type == pygame.KEYDOWN:
                events.append(('keydown', event.key))
            elif event.type == pygame.KEYUP:
                events.append(('keyup', event.key))
        return events

    def quit(self) -> None:
        if _PYGAME_AVAILABLE and self.initialized:
            pygame.quit()
            self.initialized = False


# ---------------------------------------------------------------------------
# Per-frame logic (dog AI + sound tick-down)
# addr: screen_render_8hz() — pre-render logic
# ---------------------------------------------------------------------------

def _run_per_frame_logic(gs: GameState) -> None:
    """
    Dog AI and sound effect tick-down, called once per rendered frame.
    addr: screen_render_8hz() — dog and sound logic before compositing
    """
    # Dog AI — delegated to dog.py
    try:
        from .dog import dog_frame_update
        dog_frame_update(gs)
    except (ImportError, AttributeError):
        pass

    # Sound effect tick-down
    if gs.soundeffect_remaining_ticks > 0:
        gs.soundeffect_remaining_ticks -= 1
        if gs.soundeffect_remaining_ticks == 0:
            sfx_id = gs.soundeffect_playing_id
            soundeffects_off(gs)
            # Chain effects: doorbell → echo, toilet flush → refill
            if sfx_id == SOUND_EFFECT_ID.SFX_DOORBELL:
                soundeffect_select(gs, SOUND_EFFECT_ID.SFX_DOORBELL_ECHO, 5)
            elif sfx_id == SOUND_EFFECT_ID.SFX_TOILET_FLUSH:
                soundeffect_select(gs, SOUND_EFFECT_ID.SFX_TOILET_REFILL, 15)


# ---------------------------------------------------------------------------
# Sprite draw helper
# addr: screen_render_8hz() → sprite_draw(index)
# ---------------------------------------------------------------------------

def _draw_sprite(surf: 'pygame.Surface', gs: GameState, slot: int) -> None:
    """
    Composite one sprite slot onto the surface.
    In the original: AND-mask blitting with XOR image data (transparent colour 0).
    In Python: use PIL Image with transparency, then blit to Pygame surface.
    addr: sprite_draw() — simplified to Pygame blit with alpha
    """
    img = gs.sprite_active_image[slot]
    if img is None:
        return

    # img can be either a PIL Image (from assets.py) or a pygame.Surface
    if _PYGAME_AVAILABLE:
        x = gs.sprite_pending_x[slot]
        y = gs.sprite_pending_y[slot]

        if hasattr(img, 'mode'):
            # PIL Image — convert to pygame Surface
            import io
            try:
                from PIL import Image as PILImage
                if img.mode != 'RGBA':
                    img = img.convert('RGBA')
                raw = img.tobytes()
                pg_img = pygame.image.fromstring(raw, img.size, 'RGBA')
                surf.blit(pg_img, (x, y))
            except Exception:
                pass
        elif isinstance(img, type(surf)):
            # Already a pygame.Surface
            surf.blit(img, (x, y))


# ---------------------------------------------------------------------------
# Headless frame advance (no Pygame)
# addr: screen_render_8hz() — rate and counter only
# ---------------------------------------------------------------------------

def screen_render_8hz_headless(gs: GameState) -> None:
    """
    Headless equivalent of screen_render_8hz — runs per-frame logic,
    commits pending sprites to active, and advances the counter.
    addr: screen_render_8hz() loop body (no display output)
    """
    # Dog AI + sound tick-down
    _run_per_frame_logic(gs)

    # Flush pending → active for any flagged slots
    for slot in range(8):
        if gs.sprite_pending_flag[slot]:
            gs.sprite_pending_flag[slot] = 0
            gs.sprite_pending_x[slot] = gs.sprite_active_x[slot]
            gs.sprite_pending_y[slot] = gs.sprite_active_y[slot]
            gs.sprite_active_image[slot]  = gs.sprite_pending_image[slot]
            gs.sprite_active_mask[slot]   = gs.sprite_pending_mask[slot]
            gs.sprite_active_height[slot] = gs.sprite_pending_height[slot]
            gs.sprite_active_width[slot]  = gs.sprite_pending_width[slot]

    # Sound effect playback
    from .sound import soundeffect_irq_play
    soundeffect_irq_play(gs)

    gs.animation_tick_counter += 1


# ---------------------------------------------------------------------------
# Text rendering helpers
# addr: string_print(), fill_top_rect_with_background()
# ---------------------------------------------------------------------------

def fill_top_rect_with_background(gs: GameState, width_chars: int = 0x1b) -> None:
    """
    Clear the top text strip with background colour.
    addr: fill_top_rect_with_background(0x1b)
    In Python: reset any text overlay on the surface's top strip.
    """
    # In headless mode this is a no-op; renderer handles it in render_frame
    gs.text_scroll_timer = 0


def screen_scroll_text_down(gs: GameState) -> None:
    """
    Scroll the text area down by one line.
    addr: screen_scroll_text_down()
    No-op in headless mode.
    """
    pass
