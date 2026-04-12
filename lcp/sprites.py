"""
Sprite pipeline for Little Computer People (Atari ST).
Translated from Ghidra decompilation of sprite_update_slots(), spritedata_select().

addr: sprite_update_slots(), spritedata_select(), spritedata_update_dog()

Architecture — 3-level pipeline:
  Level 1 — Definition arrays (loaded from SPRITES file at startup):
    sprite_def_image[60], sprite_def_mask[60], sprite_def_width[60], sprite_def_height[60]

  Level 2 — Pending buffer (written by game logic each frame):
    sprite_pending_image[8], sprite_pending_mask[8],
    sprite_pending_x[8], sprite_pending_y[8],
    sprite_pending_width[8], sprite_pending_height[8],
    sprite_pending_flag[8]

  Level 3 — Active buffer (consumed by renderer):
    sprite_active_image[8], sprite_active_mask[8],
    sprite_active_x[8], sprite_active_y[8],
    sprite_active_width[8], sprite_active_height[8]

8 hardware rendering slots:
  Slot 0   — special / unused
  Slots 1–2 — SPRITE_BEHIND_LCP layer (overflow: 2 → 1)
  Slot 3   — LCP body sprite
  Slot 4   — LCP head sprite
  Slots 5–6 — SPRITE_IN_FRONT layer (overflow: 6 → 5)
  Slot 7   — Dog (special, managed by dog.py)
  Slot 9   — Disabled sentinel (out-of-range → invisible)

sprite_layer_flags[logical_id]:
  SPRITE_HIDDEN (0)       → slot 9 (invisible)
  SPRITE_IN_FRONT (1)     → slot 6 (or 5 if 6 taken)
  SPRITE_BEHIND_LCP (-1)  → slot 2 (or 1 if 2 taken)
"""

from .state import GameState
from .enums import SPRITE_LAYER, SPRITE_ID


# ---------------------------------------------------------------------------
# Slot assignment
# addr: sprite_update_slots()
# ---------------------------------------------------------------------------

def sprite_update_slots(gs: GameState) -> None:
    """
    Multiplex 60 logical sprites onto 8 hardware rendering slots.
    addr: sprite_update_slots()

    Slot layout:
      1–2  = SPRITE_BEHIND_LCP
      3    = LCP body (managed externally)
      4    = LCP head (managed externally)
      5–6  = SPRITE_IN_FRONT
      9    = disabled
    """
    # Slots 0 and 1 — clear if the logical sprite that owns them is hidden
    if gs.sprite_layer_flags[0] == SPRITE_LAYER.SPRITE_HIDDEN:
        slot = gs.sprite_slot_map[0]
        if 0 <= slot < 8:
            gs.sprite_active_image[slot] = None

    if gs.sprite_layer_flags[1] == SPRITE_LAYER.SPRITE_HIDDEN:
        slot = gs.sprite_slot_map[0]   # original uses slot 0 again — faithful copy
        if 0 <= slot < 8:
            gs.sprite_active_image[slot] = None

    # Logical sprites 3–59 (SPRITE_GLASS = 3 is the first real sprite)
    for sprite_id in range(3, 60):
        layer = gs.sprite_layer_flags[sprite_id]

        if layer == SPRITE_LAYER.SPRITE_HIDDEN:
            gs.sprite_slot_map[sprite_id] = 9   # disabled

        elif layer == SPRITE_LAYER.SPRITE_IN_FRONT:
            old_slot = gs.sprite_slot_map[sprite_id]
            new_slot = 6

            # Check if slot 6 is already claimed by an earlier sprite
            for earlier in range(3, sprite_id):
                if gs.sprite_slot_map[earlier] == 6:
                    new_slot = 5
                    break

            # Any later sprite that already has our new_slot gets bumped to slot 5
            for later in range(sprite_id + 1, 60):
                if gs.sprite_slot_map[later] == new_slot:
                    gs.sprite_slot_map[later] = 5
                    gs.sprite_pending_x[5] = gs.sprite_pending_x[6]
                    gs.sprite_pending_y[5] = gs.sprite_pending_y[6]
                    gs.sprite_active_image[5]  = gs.sprite_active_image[6]
                    gs.sprite_active_mask[5]   = gs.sprite_active_mask[6]
                    gs.sprite_active_height[5] = gs.sprite_active_height[6]
                    gs.sprite_active_width[5]  = gs.sprite_active_width[6]

            gs.sprite_slot_map[sprite_id] = new_slot

            # Copy previous slot data to new slot if the slot changed
            if 0 <= old_slot < 8:
                gs.sprite_pending_x[new_slot] = gs.sprite_pending_x[old_slot]
                gs.sprite_pending_y[new_slot] = gs.sprite_pending_y[old_slot]
                gs.sprite_active_image[new_slot]  = gs.sprite_active_image[old_slot]
                gs.sprite_active_mask[new_slot]   = gs.sprite_active_mask[old_slot]
                gs.sprite_active_height[new_slot] = gs.sprite_active_height[old_slot]
                gs.sprite_active_width[new_slot]  = gs.sprite_active_width[old_slot]
                if new_slot != old_slot:
                    gs.sprite_active_image[old_slot] = None

        elif layer == SPRITE_LAYER.SPRITE_BEHIND_LCP:
            old_slot = gs.sprite_slot_map[sprite_id]
            new_slot = 2

            # Check if slot 2 already claimed
            for earlier in range(3, sprite_id):
                if gs.sprite_slot_map[earlier] == 2:
                    new_slot = 1
                    break

            # Bump later sprites
            for later in range(sprite_id + 1, 60):
                if gs.sprite_slot_map[later] == new_slot:
                    gs.sprite_slot_map[later] = 1
                    gs.sprite_pending_x[1] = gs.sprite_pending_x[2]
                    gs.sprite_pending_y[1] = gs.sprite_pending_y[2]
                    gs.sprite_active_image[1]  = gs.sprite_active_image[2]
                    gs.sprite_active_mask[1]   = gs.sprite_active_mask[2]
                    gs.sprite_active_height[1] = gs.sprite_active_height[2]
                    gs.sprite_active_width[1]  = gs.sprite_active_width[2]

            gs.sprite_slot_map[sprite_id] = new_slot

            if 0 <= old_slot < 8:
                gs.sprite_pending_x[new_slot] = gs.sprite_pending_x[old_slot]
                gs.sprite_pending_y[new_slot] = gs.sprite_pending_y[old_slot]
                gs.sprite_active_image[new_slot]  = gs.sprite_active_image[old_slot]
                gs.sprite_active_mask[new_slot]   = gs.sprite_active_mask[old_slot]
                gs.sprite_active_height[new_slot] = gs.sprite_active_height[old_slot]
                gs.sprite_active_width[new_slot]  = gs.sprite_active_width[old_slot]
                if new_slot != old_slot:
                    gs.sprite_active_image[old_slot] = None

    # Clear any slot not referenced by any logical sprite
    for slot in range(1, 7):
        if all(gs.sprite_slot_map[i] != slot for i in range(60)):
            gs.sprite_active_image[slot] = None


# ---------------------------------------------------------------------------
# Sprite selection (copies def → active directly, bypassing pending)
# addr: spritedata_select()
# ---------------------------------------------------------------------------

def spritedata_select(gs: GameState, sprite_id: int) -> None:
    """
    Assign a logical sprite to its hardware slot (via sprite_update_slots),
    then copy sprite definition data directly into the active arrays.
    addr: spritedata_select()
    """
    sprite_update_slots(gs)
    slot = gs.sprite_slot_map[sprite_id]
    if 0 <= slot < 8:
        gs.sprite_active_image[slot]  = gs.sprite_def_image[sprite_id]
        gs.sprite_active_mask[slot]   = gs.sprite_def_mask[sprite_id]
        gs.sprite_active_height[slot] = gs.sprite_def_height[sprite_id]
        gs.sprite_active_width[slot]  = gs.sprite_def_width[sprite_id]


# ---------------------------------------------------------------------------
# Pending-buffer helpers (used by dog, actions, etc.)
# ---------------------------------------------------------------------------

def spritedata_set_pending(gs: GameState, slot: int,
                            image, mask, x: int, y: int,
                            width: int, height: int) -> None:
    """
    Stage a sprite update in the pending buffer.
    screen_render_8hz() flushes pending → active each frame when flag is set.
    """
    if 0 <= slot < 8:
        gs.sprite_pending_image[slot]  = image
        gs.sprite_pending_mask[slot]   = mask
        gs.sprite_pending_x[slot]      = x
        gs.sprite_pending_y[slot]      = y
        gs.sprite_pending_width[slot]  = width
        gs.sprite_pending_height[slot] = height
        gs.sprite_pending_flag[slot]   = 1


def spritedata_set_position(gs: GameState, slot: int, x: int, y: int) -> None:
    """Update only the position in the pending buffer for a given slot."""
    if 0 <= slot < 8:
        gs.sprite_pending_x[slot] = x
        gs.sprite_pending_y[slot] = y


# ---------------------------------------------------------------------------
# Carried-object sprite positioning
# addr: game_tick_and_animate() carry branch
# ---------------------------------------------------------------------------

def update_carried_object_sprite(gs: GameState) -> None:
    """
    Position the carried-object sprite relative to the LCP.
    addr: game_tick_and_animate() carry branch — lcp_carrying_object_flag == YES
    """
    from .enums import FACING_DIR
    obj = gs.lcp_carried_object
    slot = gs.sprite_slot_map[obj] if 0 <= obj < 60 else -1
    if slot < 0 or slot >= 8:
        return

    if gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT:
        gs.sprite_pending_x[slot] = gs.lcp_x + 10
    else:
        width = gs.sprite_active_width[slot]
        gs.sprite_pending_x[slot] = (gs.lcp_x - width) + 16
        if gs.sprite_pending_x[slot] < 0:
            gs.sprite_pending_x[slot] = 0


# ---------------------------------------------------------------------------
# LCP body sprite — slot 3
# addr: sprite_update_body()
# ---------------------------------------------------------------------------

def sprite_update_body(gs: GameState) -> None:
    """
    Select the correct LCP body sprite for the current lcp_state into slot 3.
    Uses body_sprite_frame_table[] to map PLAYER_STATE → frame index.
    If carrying an object and state < 25, uses carry_body_frame_table[].
    Positions body at:
      X = lcp_x - 4 (facing right) or lcp_x - 14 (facing left)
      Y = lcp_y + body_y_offset_per_state[state] - 21
    addr: sprite_update_body()
    """
    from .enums import FACING_DIR
    from .constants import (
        BODY_SPRITE_FRAME_TABLE, CARRY_BODY_FRAME_TABLE,
        BODY_Y_OFFSET_PER_STATE,
    )

    # Wait for pending slot 3 to be consumed (in original: busy-wait loop)
    if gs.sprite_pending_flag[3]:
        return

    state = gs.lcp_state
    if state < 0 or state >= len(BODY_SPRITE_FRAME_TABLE):
        state = 0

    # Select frame index from table
    frame_idx = BODY_SPRITE_FRAME_TABLE[state]
    if gs.lcp_carrying_object_flag and state < len(CARRY_BODY_FRAME_TABLE):
        frame_idx = CARRY_BODY_FRAME_TABLE[state]

    # Store the selected body frame for the renderer
    gs._body_frame_index = frame_idx
    gs._body_facing = gs.lcp_facing_direction

    # Position the body sprite in slot 3
    if gs.lcp_facing_direction == FACING_DIR.FACING_RIGHT:
        gs.sprite_active_x[3] = gs.lcp_x - 4
    else:
        gs.sprite_active_x[3] = gs.lcp_x - 14

    y_offset = BODY_Y_OFFSET_PER_STATE[state] if state < len(BODY_Y_OFFSET_PER_STATE) else 0
    gs.sprite_active_y[3] = gs.lcp_y + y_offset - 21

    if gs.debug_hide_lcp_offscreen:
        gs.sprite_active_y[3] = 300

    gs.sprite_pending_height[3] = 21
    gs.sprite_pending_width[3] = 32

    # Copy body frame image/mask into pending slot 3
    body_frames = getattr(gs, '_body_frames', None)
    if body_frames and 0 <= frame_idx < len(body_frames):
        gs.sprite_pending_image[3] = body_frames[frame_idx]
    else:
        gs.sprite_pending_image[3] = None

    if gs.lcp_sprites_hidden:
        gs.sprite_pending_image[3] = None

    gs.sprite_pending_flag[3] = 1


# ---------------------------------------------------------------------------
# LCP head animation state machine
# addr: sprite_lcp_head_animate()
# ---------------------------------------------------------------------------

def sprite_lcp_head_animate(gs: GameState) -> None:
    """
    Advance the head animation state machine.
    Controls random head movements (looking left/right/up/down) with smooth
    transitions between positions. Head direction encoded in 5 bits:
    3 bits horizontal angle, 2 bits vertical tilt.
    addr: sprite_lcp_head_animate()
    """
    import random
    from .enums import FACING_DIR, HEAD_ANIM_MODE
    from .constants import (
        HEAD_DEFAULT_ANGLE_PER_STATE, HEAD_MOVEMENT_DELTA_TABLE,
        HEAD_TILT_FRAME_OFFSET,
    )

    # If head is still moving toward target, or mode is negative (disabled),
    # or delay countdown > 0, skip new target selection
    if gs.head_anim_current != gs.head_anim_target:
        pass  # fall through to movement below
    elif gs.head_anim_mode < 0:
        pass  # fall through
    else:
        gs.head_anim_delay_countdown -= 1
        if gs.head_anim_delay_countdown > 0:
            # Update movement toward target and sprite frame, then return
            _head_move_toward_target(gs)
            return
        # Pick new delay and decide on a new target
        gs.head_anim_delay_countdown = random.randint(2, 9)
        rval = random.randint(0, 255)

        state = gs.lcp_state
        if state < 0 or state >= len(HEAD_DEFAULT_ANGLE_PER_STATE):
            state = 0

        if rval & 0x10:
            # Vertical movement
            movement_mask = gs.head_anim_mode & HEAD_ANIM_MODE.HEAD_ANIM_VERTICAL_RANGE
            if movement_mask == 0xFF & HEAD_ANIM_MODE.HEAD_ANIM_VERTICAL_RANGE:
                movement_mask = random.randint(0, 255) & 0xE0
                if movement_mask == 0xE0:
                    movement_mask = 0x40
            if (gs.head_anim_mode & 0xE1) < 0x81:
                rv = random.randint(0, 255)
                movement_mask = (((movement_mask >> 5) - 1) & 1) + ((rv & 4) >> 2)
            else:
                movement_mask = 7 - (gs.head_anim_mode >> 5)
            gs.head_anim_target = (movement_mask << 3) | (gs.head_anim_target & 7)
        else:
            # Horizontal movement
            anim_mode = gs.head_anim_mode & HEAD_ANIM_MODE.HEAD_ANIM_HORIZONTAL_AMPLITUDE
            if anim_mode == 0x1F & HEAD_ANIM_MODE.HEAD_ANIM_HORIZONTAL_AMPLITUDE:
                anim_mode = (random.randint(0, 255) & 0x07) | 1
            else:
                anim_mode = gs.head_anim_mode & 0x07

            random_seed = anim_mode - 1
            horiz_range = gs.head_anim_mode & HEAD_ANIM_MODE.HEAD_ANIM_HORIZONTAL_RANGE
            if horiz_range == 0xFF & HEAD_ANIM_MODE.HEAD_ANIM_HORIZONTAL_RANGE:
                rv = random.randint(0, 255)
                if rv & 8:
                    random_seed = -random_seed
            elif horiz_range > 7:
                random_seed = -random_seed

            random_seed = (random_seed + HEAD_DEFAULT_ANGLE_PER_STATE[state]) & 7
            if gs.lcp_facing_direction == FACING_DIR.FACING_LEFT:
                random_seed = (8 - random_seed) & 7
            gs.head_anim_target = random_seed | (gs.head_anim_target & 0x18)

    # Move current toward target and update sprite frame
    _head_move_toward_target(gs)


def _head_move_toward_target(gs: GameState) -> None:
    """
    Move head_anim_current one step toward head_anim_target and compute
    head_sprite_frame + head_sprite_mirror_flag.
    addr: sprite_lcp_head_animate() — LAB_000264f6 onward
    """
    from .enums import FACING_DIR
    from .constants import (
        HEAD_DEFAULT_ANGLE_PER_STATE, HEAD_MOVEMENT_DELTA_TABLE,
        HEAD_TILT_FRAME_OFFSET,
    )

    if gs.head_anim_target < 0:
        return

    # Vertical step (bits 3-4)
    vert_diff = (gs.head_anim_target & 0x18) - (gs.head_anim_current & 0x18)
    if vert_diff > 0:
        gs.head_anim_current += 8
    elif vert_diff < 0:
        gs.head_anim_current -= 8

    # Horizontal step via delta table
    horiz_target = gs.head_anim_target & 7
    horiz_current = gs.head_anim_current & 7
    delta_idx = (horiz_target - horiz_current) + 7
    if 0 <= delta_idx < len(HEAD_MOVEMENT_DELTA_TABLE):
        target_frame = HEAD_MOVEMENT_DELTA_TABLE[delta_idx]
    else:
        target_frame = 0

    state = gs.lcp_state
    if state < 0 or state >= len(HEAD_DEFAULT_ANGLE_PER_STATE):
        state = 0

    if target_frame == 99:
        # Overflow — try default angle
        default_angle = (gs.lcp_facing_direction * 4 + HEAD_DEFAULT_ANGLE_PER_STATE[state]) & 7
        delta_idx2 = (default_angle - horiz_current) + 7
        if 0 <= delta_idx2 < len(HEAD_MOVEMENT_DELTA_TABLE):
            target_frame = HEAD_MOVEMENT_DELTA_TABLE[delta_idx2]
        else:
            target_frame = 0
    if target_frame == 99:
        target_frame = -1

    gs.head_anim_current = ((target_frame + gs.head_anim_current) & 7) + (gs.head_anim_current & 0x18)

    # Compute sprite frame and mirror flag from head_anim_current
    if gs.head_anim_current >= 0 and gs.head_anim_current < 0x80:
        horiz_val = gs.head_anim_current & 7
        tilt_idx = (gs.head_anim_current & 0x18) >> 3
        if tilt_idx >= len(HEAD_TILT_FRAME_OFFSET):
            tilt_idx = 0
        if horiz_val < 5:
            gs.head_sprite_frame = horiz_val + HEAD_TILT_FRAME_OFFSET[tilt_idx]
            gs.head_sprite_mirror_flag = 0
        else:
            gs.head_sprite_frame = (8 - horiz_val) + HEAD_TILT_FRAME_OFFSET[tilt_idx]
            gs.head_sprite_mirror_flag = 1


# ---------------------------------------------------------------------------
# LCP head sprite — slot 4
# addr: sprite_lcp_head_update()
# ---------------------------------------------------------------------------

def sprite_lcp_head_update(gs: GameState) -> None:
    """
    Build and position the LCP head sprite for the current frame.
    Selects head frame from PE*.LCP based on head_sprite_frame and
    happiness level. Positions head relative to body using per-state
    offset tables.
    addr: sprite_lcp_head_update()
    """
    from .constants import (
        HAPPINESS_HEAD_FRAME_OFFSET, HEAD_X_OFFSET_PER_STATE,
        BODY_Y_OFFSET_PER_STATE, HEAD_HEIGHT_PER_STATE,
    )

    # Wait for pending slot 4
    if gs.sprite_pending_flag[4]:
        return

    state = gs.lcp_state
    if state < 0 or state >= len(HEAD_X_OFFSET_PER_STATE):
        state = 0

    # Compute head frame index from happiness + head_sprite_frame
    happiness = gs.lcp.happiness
    if happiness < 0 or happiness >= len(HAPPINESS_HEAD_FRAME_OFFSET):
        happiness = 0
    head_idx = HAPPINESS_HEAD_FRAME_OFFSET[happiness] + (gs.head_sprite_frame & 0x7F)

    # Store for renderer
    gs._head_frame_index = head_idx
    gs._head_mirror = gs.head_sprite_mirror_flag

    # Position the head in slot 4
    x_offset = HEAD_X_OFFSET_PER_STATE[state] if state < len(HEAD_X_OFFSET_PER_STATE) else 0
    if gs.head_sprite_mirror_flag == 0:
        gs.sprite_active_x[4] = gs.lcp_x + x_offset - 4
    else:
        gs.sprite_active_x[4] = gs.lcp_x + x_offset - 14

    y_body = BODY_Y_OFFSET_PER_STATE[state] if state < len(BODY_Y_OFFSET_PER_STATE) else 0
    y_head = HEAD_HEIGHT_PER_STATE[state] if state < len(HEAD_HEIGHT_PER_STATE) else 21
    gs.sprite_active_y[4] = gs.lcp_y + y_body - (y_head + 21)

    if gs.debug_hide_lcp_offscreen:
        gs.sprite_active_y[4] = 300

    # Special case: carrying objects on stairs (states 13–16), head lowered 1px
    if gs.lcp_carrying_object_flag and 12 < state < 17:
        gs.sprite_active_y[4] += 1

    gs.sprite_pending_height[4] = 21
    gs.sprite_pending_width[4] = 32

    # Copy head frame into pending slot 4
    head_frames = getattr(gs, '_head_frames', {})
    char_variant = gs.lcp.character_sprite_id
    variant_frames = head_frames.get(char_variant, None)
    if variant_frames and 0 <= head_idx < len(variant_frames):
        gs.sprite_pending_image[4] = variant_frames[head_idx]
    else:
        gs.sprite_pending_image[4] = None

    if gs.lcp_sprites_hidden:
        gs.sprite_pending_image[4] = None

    gs.sprite_pending_flag[4] = 1
