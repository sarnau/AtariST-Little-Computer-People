# Little Computer People — LCP Character Movement & Action System

The LCP (Little Computer Person) is the main character — a small person who lives
in a three-floor house. Unlike the dog which moves autonomously, the LCP's movement
is entirely **action-driven**: every walk is initiated by the AI decision engine
selecting an action, which scripts the LCP to walk to specific positions, perform
animations, and interact with objects.

## Overview

The LCP has a multi-layered rendering system:
- **Body sprite** (slot 3): selected from `body.lcp` based on `lcp_state`
- **Head sprite** (slot 4): selected from `pex.lcp` based on `head_sprite_frame` + happiness
- **Carried object sprite** (slot dependent): when carrying items
- **Door overlay sprites** (slots 5–6): walk-behind-door illusion

Movement is driven by setting `walk_target_x`/`walk_target_y` and calling
`lcp_walk_to_destination()`, which loops `lcp_pathfind_one_step()` until arrival
or interruption by a game event.

## Coordinate System

### House Floors

| Floor | Number | Y Range | `get_floor_number_from_y()` |
|---|---|---|---|
| Top | 3 | Y < 78 | Bedroom, study |
| Middle | 2 | 78 ≤ Y < 141 | Living room, bathroom |
| Bottom | 1 | Y ≥ 141 | Kitchen, entry hall |

Each floor has a center Y coordinate in `floor_center_y_coords[]` and a bottom
edge in `floor_bottom_y_coords[]`. Characters gravitate toward the center line
when walking horizontally.

### Position System

The house has 48 named positions defined in the `HOUSE_POS` enum (e.g.,
`POS_TOP_FILING_CABINET`, `POS_MID_TOILET_DOOR`, `POS_BTM_FRIDGE`). Each position
has an X coordinate in `room_position_x_table[]` (stored as half-pixels, doubled
on lookup) and a height offset in `room_position_height_table[]`.

`house_get_position_xy(position, &x, &y)` converts a HOUSE_POS to screen coordinates:
```
x = room_position_x_table[position] * 2
y = floor_y - room_position_height_table[position + 1]
```

Where `floor_y` is 77 (top), 140 (middle), or 202 (bottom) depending on the
position index range (0–15 = top, 16–31 = middle, 32–47 = bottom).

## State Variables

### Position & Navigation

| Variable | Type | Purpose |
|---|---|---|
| `lcp_x` | short | Current X pixel position |
| `lcp_y` | short | Current Y pixel position |
| `walk_target_x` | short | Final destination X (0 = no target) |
| `walk_target_y` | short | Final destination Y (0 = no target) |
| `walk_waypoint_x` | short | Current intermediate waypoint X |
| `walk_waypoint_y` | short | Current intermediate waypoint Y |
| `lcp_on_stairs_flag` | BOOL16 | YES when navigating stairs |
| `lcp_facing_direction` | FACING_DIR | FACING_RIGHT (0) or FACING_LEFT (1) |

### Character State

| Variable | Type | Purpose |
|---|---|---|
| `lcp_state` | PLAYER_STATE | Current body pose / animation state |
| `lcp_carrying_object_flag` | BOOL16 | YES when carrying an object |
| `lcp_carried_object` | sprite_id | Which object is being carried |
| `lcp_sprites_hidden` | BOOL16 | YES to hide all LCP sprites |

### Head Animation

| Variable | Type | Purpose |
|---|---|---|
| `head_sprite_frame` | short | Current head direction (5-bit encoded) |
| `head_anim_current` | short | Current head animation position |
| `head_anim_target` | short | Target head animation position |
| `head_anim_mode` | HEAD_ANIM_MODE | Bit flags controlling allowed head movement |
| `head_anim_delay_countdown` | short | Frames until next random head movement |
| `last_walk_sound_id` | short | Last head target set during walking (avoids redundant sets) |

### Sound

| Variable | Type | Purpose |
|---|---|---|
| `footstep_trigger_flag` | BOOL16 | YES when a footstep should play this frame |

## Player States (PLAYER_STATE Enum)

The LCP body sprite is selected via `body_sprite_frame_table[lcp_state]`,
which maps each state to a frame offset in `body.lcp`. Key state groups:

### Walking States (0–7)

| State | Name | Description |
|---|---|---|
| 0 | `STATE_WALK_FRAME_0` | Walk cycle frame 0 |
| 1 | `STATE_WALK_FRAME_1` | Walk cycle frame 1 |
| 2 | `STATE_WALK_FRAME_2` | Walk cycle frame 2 |
| 3 | `STATE_WALK_FRAME_3_STEP` | Walk cycle frame 3 (footstep trigger) |
| 4 | `STATE_WALK_FRAME_4` | Walk cycle frame 4 |
| 5 | `STATE_WALK_FRAME_5` | Walk cycle frame 5 |
| 6 | `STATE_WALK_FRAME_6` | Walk cycle frame 6 |
| 7 | `STATE_WALK_FRAME_7_STEP` | Walk cycle frame 7 (footstep trigger) |

The walk cycle increments state by 1 each tick (`lcp_state + 1`), wrapping
from 7 back to 0. Footstep sounds trigger on frames 3 and 7 (two steps per cycle).

### Stair Climbing States (9–12)

| State | Name | Description |
|---|---|---|
| 9 | `STATE_STAIR_CLIMB_FRAME_0` | Stair climb frame 0 |
| 10 | `STATE_STAIR_CLIMB_FRAME_1` | Stair climb frame 1 |
| 11 | `STATE_STAIR_CLIMB_FRAME_2` | Stair climb frame 2 |
| 12 | `STATE_STAIR_CLIMB_FRAME_3_STEP` | Stair climb frame 3 (footstep) |

4-frame cycle, wrapping 12 → 9. The LCP moves diagonally: 1–2 pixels horizontal
+ 1 pixel vertical per tick. Footstep triggers on frame 3.

### Stair Top Landing States (13–16)

| State | Name | Description |
|---|---|---|
| 13 | `STATE_STAIR_TOP_FRAME_0` | Top landing frame 0 |
| 14 | `STATE_STAIR_TOP_FRAME_1` | Top landing frame 1 |
| 15 | `STATE_STAIR_TOP_FRAME_2` | Top landing frame 2 |
| 16 | `STATE_STAIR_TOP_FRAME_3_STEP` | Top landing frame 3 (footstep) |

Used for the flat sections between stair flights. The LCP moves vertically
(Y -= 2) on specific frames and flips facing direction when the cycle wraps.

### Stair Descending States (17–20)

| State | Name | Description |
|---|---|---|
| 17 | `STATE_STAIR_DESCEND_FRAME_0` | Stair descend frame 0 |
| 18 | `STATE_STAIR_DESCEND_FRAME_1` | Stair descend frame 1 (footstep) |
| 19 | `STATE_STAIR_DESCEND_FRAME_2` | Stair descend frame 2 |
| 20 | `STATE_STAIR_DESCEND_FRAME_3_STEP` | Stair descend frame 3 |

4-frame cycle for going down. Diagonal movement mirrors climbing but in reverse.

### Stair Bottom Landing States (21–24)

| State | Name | Description |
|---|---|---|
| 21 | `STATE_STAIR_BTM_FRAME_0` | Bottom landing frame 0 |
| 22 | `STATE_STAIR_BTM_FRAME_1` | Bottom landing frame 1 (Y += 2) |
| 23 | `STATE_STAIR_BTM_FRAME_2` | Bottom landing frame 2 (Y += 2) |
| 24 | `STATE_STAIR_BTM_FRAME_3` | Bottom landing frame 3 (footstep) |

### Idle & Action States (25+)

States 25 and above are used for non-walking poses: standing, sitting, eating,
typing, reading, exercising, sleeping, and all object interactions. These are
set directly by action functions, not by the pathfinding system. Examples:

| State | Name |
|---|---|
| `STATE_STAND_IDLE` | Standing still after walk |
| `STATE_STAND_SIDE_VIEW` | Standing, side profile |
| `STATE_STAND_FACING_SCREEN` | Standing, facing player |
| `STATE_WRITE_AT_DESK` | Seated at desk writing |
| `STATE_TYPE_AT_DESK_LEFT_HAND` | Typing animation (left hand) |
| `STATE_EAT_BITE` | Eating food |
| `STATE_DRINK_GLASS` | Drinking water |
| `STATE_EXERCISE_ARMS_UP` | Exercise animation |
| `STATE_CROUCH_DOWN` | Crouching to pet dog |
| `STATE_SLEEP_IN_BED` | Sleeping |

## High-Level Walk Interface

### `lcp_walk_to_destination()` (0x14CEA)

The primary walk function called by all action routines:

```
set walk_target_x, walk_target_y
result = lcp_walk_to_destination()
// result: 0 = arrived, -1 = interrupted
```

Implementation:
1. Set `head_anim_mode = HEAD_ANIM_WALKING` (head bobs naturally)
2. Clear `last_walk_sound_id` (reset footstep tracking)
3. Loop:
   - Call `lcp_pathfind_one_step()` — advance one pixel
   - Check interruption conditions:
     * Event triggered AND not in execute_event AND not carrying object
       AND not in intro AND not on stairs AND action is interruptible
     * If interrupted: clear target, return -1
   - If target reached (both 0): return 0

The interruption check allows urgent events (doorbell, bathroom need, alarm)
to break the LCP out of a walk when the current action is marked interruptible
(`action_interruptible_flag = YES`). Non-interruptible actions (petting dog,
mini-games, stair traversal) cannot be interrupted.

### Typical Action Pattern

Every action function follows this pattern:

```c
void action_example(void) {
    // 1. Walk to position
    house_get_position_xy(POS_TARGET, &walk_target_x, &walk_target_y);
    result = lcp_walk_to_destination();
    if (result != 0) return;  // interrupted

    // 2. Face correct direction
    lcp_facing_direction = FACING_RIGHT;  // or FACING_LEFT
    lcp_state = STATE_STAND_FACING_SCREEN;

    // 3. Animate head
    head_anim_target = 12;
    lcp_wait_head_reach_target();

    // 4. Perform action (state changes, sound effects, object draws)
    lcp_state = STATE_ACTION_POSE;
    game_tick_and_animate(duration);

    // 5. Return to idle
    lcp_state = STATE_STAND_SIDE_VIEW;
}
```

## Movement Algorithm: `lcp_pathfind_one_step()` (0x1470A)

This 351-line function advances the LCP by one pixel per call. It is called
once per frame from `lcp_walk_to_destination()`.

### Phase 1: Waypoint Check

```
if no waypoint set:
    lcp_calc_floor_waypoint()    // compute next waypoint

if on stairs and reached floor boundary:
    lcp_on_stairs_flag = NO      // exit stair mode

if at waypoint:
    if waypoint == target:
        clear target, set STATE_STAND_IDLE
        return
    else:
        lcp_calc_floor_waypoint()   // compute next segment
```

### Phase 2: Flat Walking (stairs flag = NO)

**Carried objects:** if carrying something, call `spritedata_select_carried_object_left()`
to update the held item sprite.

**Horizontal movement** — 1 pixel per tick:

```
if lcp_x < waypoint_x:
    lcp_facing_direction = FACING_RIGHT
    lcp_x += 1
    head_anim_target = 10 (look right)
else if lcp_x > waypoint_x:
    lcp_facing_direction = FACING_LEFT
    lcp_x -= 1
    head_anim_target = 14 (look left)

lcp_state = (lcp_state + 1) % 8    // cycle walk frames 0-7
```

**Vertical movement** — same two-phase approach as the dog:

```
x_distance = abs(lcp_x - waypoint_x)

if x_distance < 8:
    // Close: move directly toward target Y
    if lcp_y < waypoint_y: lcp_y += 1
    else if lcp_y > waypoint_y: lcp_y -= 1
else:
    // Far: gravitate toward floor center line
    floor = get_floor_number_from_y(lcp_y)
    center = floor_center_y_coords[floor - 1]
    if lcp_y < center: lcp_y += 1
    else if lcp_y > center: lcp_y -= 1
```

**Footstep trigger:** set `footstep_trigger_flag = YES` on walk frames 3 and 7.

### Phase 3: Stair Navigation (stairs flag = YES)

The staircase has four distinct movement phases, each using different
PLAYER_STATE ranges and movement patterns.

#### Going Up (waypoint Y < current Y)

**Y = 161 — Bottom landing entry (floor 1 → stair):**
```
state = STATE_STAIR_CLIMB_FRAME_0
facing = FACING_LEFT
lcp_x -= 6, lcp_y -= 2
head_anim_target = 14 (look left)
```

**Y = 100 — Middle landing entry (floor 2 → upper stair):**
```
state = STATE_STAIR_CLIMB_FRAME_0
facing = FACING_RIGHT
lcp_x += 3, lcp_y -= 2
head_anim_target = 10 (look right)
```

**Y < 100 — Upper stair flight (climbing to floor 3):**
```
facing = FACING_RIGHT
lcp_y -= 1
if state != STATE_STAIR_CLIMB_FRAME_3_STEP:
    lcp_x += 2 (or +1 if at waypoint X)
else:
    lcp_x += 1
state = (state + 1), wrap at 12 → 9
footstep on STATE_STAIR_CLIMB_FRAME_3_STEP
```

**101 ≤ Y ≤ 161 — Lower stair flight (climbing to floor 2):**
```
facing = FACING_LEFT
lcp_y -= 1
if state != STATE_STAIR_CLIMB_FRAME_3_STEP:
    lcp_x -= 2 (or -1 if at waypoint X)
else:
    lcp_x -= 1
state = (state + 1), wrap at 12 → 9
footstep on STATE_STAIR_CLIMB_FRAME_3_STEP
```

**Y 139–161 or Y = 162 — Top landing platform:**
```
state cycles through STATE_STAIR_TOP_FRAME_0..3 (states 13-16)
lcp_y -= 2 on frames 0 and 3
facing flips on cycle wrap
carried objects moved to SPRITE_BEHIND_LCP layer
footstep on STATE_STAIR_TOP_FRAME_3_STEP
```

#### Going Down (waypoint Y > current Y)

**Y = 161 — Bottom landing exit (stair → floor 1):**
```
state = STATE_STAIR_BTM_FRAME_0
facing = FACING_RIGHT
lcp_y = 165, lcp_x += 6
```

**Y = 100 — Middle landing exit (stair → floor 2):**
```
state = STATE_STAIR_BTM_FRAME_0
facing = FACING_RIGHT
lcp_y = 102, lcp_x -= 2
```

**Y < 100 — Upper stair descent:**
```
facing = FACING_LEFT
lcp_y += 1
lcp_x -= 2 (or -1 if at waypoint X)
state cycles through STATE_STAIR_DESCEND_FRAME_0..3 (states 17-20)
footstep on STATE_STAIR_DESCEND_FRAME_1
```

**101 ≤ Y ≤ 161 — Lower stair descent:**
```
facing = FACING_RIGHT
lcp_y += 1
lcp_x += 2 (or +1 if at waypoint X)
state cycles through STATE_STAIR_DESCEND_FRAME_0..3
footstep on STATE_STAIR_DESCEND_FRAME_1
```

**Landing platform (bottom):**
```
state cycles through STATE_STAIR_BTM_FRAME_0..3 (states 21-24)
lcp_y += 2 on frames 1 and 2
facing flips on cycle wrap
lcp_x += 2 on first entry
footstep on STATE_STAIR_BTM_FRAME_3
```

### Phase 4: Sickness Speed Penalty

After movement, the footstep sound is played. If the LCP is sick
(`lcp.sickness_level != SICKNESS_HEALTHY`), an extra `game_tick_and_animate(0)`
call is inserted **before** the footstep sound, effectively doubling the
time per step and halving walking speed.

```
if sick:
    game_tick_and_animate(0)     // extra frame delay
    lcp_play_footstep_sound()
game_tick_and_animate(0)         // normal frame
if healthy:
    lcp_play_footstep_sound()
```

## Waypoint Routing: `lcp_calc_floor_waypoint()` (0x150BC)

Computes the next intermediate waypoint for multi-floor navigation:

```
target_floor = get_floor_number_from_y(walk_target_y)
current_floor = get_floor_number_from_y(lcp_y)

if same floor:
    lcp_on_stairs_flag = NO
    waypoint = target (direct walk)

if different floor:
    // Route to staircase entry for current floor
    stair_index = (current_floor - 1) * 2
    waypoint = staircase_waypoint_coords[stair_index, stair_index+1]

    // Special case: floor 2 going down
    if current_floor == 2 and target_floor < current_floor:
        waypoint = (stair_top_y_threshold, stair_bottom_y_threshold)

    if already at stair entry:
        lcp_on_stairs_flag = YES
        if going up:
            waypoint = staircase_waypoint_coords[stair_index+2, stair_index+3]
        else:
            waypoint = staircase_waypoint_coords[stair_index-2, stair_index-1]

        // Special case: floor 1 stair entry
        if current_floor == 1:
            waypoint = (stair_top_y_threshold, stair_bottom_y_threshold)
```

The staircase coordinate table stores 6 (x,y) pairs: entry and exit points
for each of the three floor-to-floor stair segments. Navigation between
non-adjacent floors (e.g., floor 1 to floor 3) requires multiple
waypoint segments — the function is called repeatedly as each segment completes.

## Footstep Sound System

`lcp_play_footstep_sound()` plays different sounds based on floor surface:

| Floor | X Range | Sound Effect |
|---|---|---|
| 1 (bottom) | X < 166 | `SFX_FOOTSTEP_CARPET` |
| 1 (bottom) | X ≥ 166 | `SFX_FOOTSTEP_WOOD` |
| 2 (middle) | 146 < X < 234 | `SFX_FOOTSTEP_CARPET` |
| 2 (middle) | other | (silent) |
| 3 (top) | X > 136 | `SFX_FOOTSTEP_WOOD` |
| 3 (top) | X ≤ 136 | (silent) |
| Stairs | any | `SFX_FOOTSTEP_STAIRS` |

This creates the impression of different floor surfaces: carpet in living areas,
wood in hallways and the study, and a distinct stair sound. Some positions are
deliberately silent (e.g., the bathroom area on floor 2).

## Head Animation System

The head is a separate sprite (slot 4) that animates independently of the body.
It creates the impression of the LCP looking around, reacting to events, and
showing emotion.

### Head Direction Encoding

The head direction is encoded in 5 bits:

```
head_sprite_frame = (vertical << 3) | horizontal
```

- **Bits 0–2** (horizontal): 0=far left, 4=center, 7=far right
- **Bits 3–4** (vertical): 0=looking up, 1=center, 2=looking down

### Random Head Movement (`sprite_lcp_head_animate`, 0x26368)

Called every frame. Uses a countdown timer (`head_anim_delay_countdown`) that
triggers a random head position change every 2–9 frames:

1. Random coin flip (bit 4 of XBIOS Random):
   - **Vertical change**: pick random vertical position within allowed range
   - **Horizontal change**: pick random horizontal position within amplitude
2. Set `head_anim_target` with new direction
3. The smooth transition between current and target happens in `sprite_lcp_head_update()`

### Head Animation Modes (`HEAD_ANIM_MODE`)

Bit flags control the allowed range of head movement:

| Flag | Meaning |
|---|---|
| `HEAD_ANIM_WALKING` | Natural walking head bob |
| `HEAD_ANIM_VERTICAL_RANGE` | Allowed vertical range mask |
| `HEAD_ANIM_HORIZONTAL_AMPLITUDE` | Allowed horizontal swing |
| `HEAD_ANIM_HORIZONTAL_RANGE` | Horizontal direction bias |
| `HEAD_ANIM_VERTICAL_OVERRIDE` | Force specific vertical position |
| `HEAD_ANIM_DISABLED` | Head movement frozen |

Different activities set different modes:
- Walking: natural bob with wide horizontal range
- Reading: fixed downward gaze, narrow horizontal
- Typing: slight downward, medium horizontal
- Sleeping: fully disabled (head hidden)

### Happiness Effect on Head Sprites

The head sprite selection includes a happiness offset:

```
headIndex = happiness_head_frame_offset[lcp.happiness] + (head_sprite_frame & 0x7F)
```

Different happiness levels select different rows in `pex.lcp`, providing
happy, neutral, and sad facial expressions for each head direction.

## Body Sprite System

### `sprite_update_body()` (0x26244)

Called every frame to render the body:

1. Look up body frame: `body_sprite_frame_table[lcp_state]`
   - If carrying object and state < 25: use `carry_body_frame_table[lcp_state]` instead
2. Call `sprite_lcp_flip()` to expand and optionally mirror the sprite
3. Position: X = `lcp_x - 4` (right) or `lcp_x - 14` (left), Y = `lcp_y + body_y_offset - 21`
4. Set sprite slot 3 with 32×21 pixel dimensions

### `sprite_lcp_head_update()` (0x2664C)

Called every frame to render the head:

1. Compute head frame from `head_sprite_frame` + happiness offset
2. Call `sprite_lcp_flip()` with `head_sprite_mirror_flag` for horizontal flip
3. Position relative to body using per-state offset tables:
   - X offset: `head_x_offset_per_state[lcp_state]`
   - Y offset: `body_y_offset_per_state[lcp_state] - head_height_per_state[lcp_state]`
4. Special: carrying objects on stairs (states 13–16) lowers head by 1 pixel
5. Set sprite slot 4

### Horizontal Flipping

`sprite_lcp_flip()` handles the mirroring of body and head sprites based on
`lcp_facing_direction` (`FACING_DIR` enum). The source sprite data in `body.lcp` / `pex.lcp` is
stored as right-facing (`FACING_RIGHT`); left-facing (`FACING_LEFT`) is generated at runtime by
flipping the pixel data horizontally and swapping the left/right halves.

### Sprite Data Sizes

Each source sprite frame is **168 bytes** (21 rows × 4 bytes/row × 2 bit-
planes; a 16×21-pixel 2-plane image).  `sprite_lcp_flip()` expands each
frame in place to a 336-byte destination (168 shorts, 32-pixel-wide 4-plane
MFDB layout) that the compositor then blits.

| Constant (in `source/include/sprites.h`) | Value | Meaning |
|---|---|---|
| `LCP_BODY_FRAME_SIZE` | `21 * 4 * 2` = 168 bytes | Source frame stride in BODY.LCP / PE*.LCP |
| `LCP_BODY_SHAPE_SIZE` | `21 * 4` = 84 bytes | Dilated silhouette stride in `body_shp` / `hd_shp` |
| `LCP_BODY_DEST_WORDS` | `21 * 4 * 2` = 168 shorts | Expanded destination in `g_lsimg` / `g_lsmas` / `g_hsbuf` / `g_hsmas` |

BODY.LCP holds 98 frames = 16,464 bytes of image data; each PE*.LCP holds 66
frames = 11,088 bytes.  Ghidra's ROM-allocated buffers are slightly larger
(120 and 66 slots, per the padded reservation the 1985 build set aside).

### Carried Object Rendering

When `lcp_carrying_object_flag = YES`:

- **Facing right**: `spritedata_select_carried_object_right(lcp_carried_object)`
  - Object positioned at `lcp_x + 10`
- **Facing left**: `spritedata_select_carried_object_left(lcp_carried_object)`
  - Object positioned at `lcp_x - sprite_width + 8`

The carried object sprite is drawn in a dedicated slot and follows the LCP's
position. On stairs, the object transitions between `SPRITE_IN_FRONT` and
`SPRITE_BEHIND_LCP` layers depending on the stair section.

## Interaction with the Action System

The AI decision engine selects actions based on time of day, needs, and
personality. Each action follows a scripted sequence of walks and poses.

### Action Dispatch

```
main loop:
    action = check_time_based_actions()   // AI selects action
    execute_action(action)                 // dispatch to handler
```

### Example: `action_write_letter()`

```
1. Walk to filing cabinet (POS_TOP_FILING_CABINET)
2. Face screen, look down (head_anim_target = 12)
3. Maybe open filing cabinet (random chance based on initiative)
4. Walk to study door, walk to desk
5. Sit down (STATE_WRITE_AT_DESK)
6. Type date header, greeting, body paragraphs
7. Each character: STATE_TYPE_AT_DESK_LEFT_HAND/RIGHT_HAND + SFX_TYPEWRITER_KEY
8. Stand up, walk away
```

### Walk Interruption

During interruptible actions, urgent events can break the walk:

| Condition | Effect |
|---|---|
| Doorbell rings | LCP stops, handles delivery, resumes |
| Bathroom need | LCP stops, uses bathroom, resumes |
| Alarm goes off | LCP stops, handles alarm |
| Phone rings | LCP stops, answers phone |

Non-interruptible states: on stairs, carrying objects, during intro,
inside event handlers, mini-games.

## Function Reference

### Core Movement

| Address | Function | Purpose |
|---|---|---|
| 0x14CEA | `lcp_walk_to_destination` | High-level walk loop (target → arrival) |
| 0x1470A | `lcp_pathfind_one_step` | Advance one pixel per frame (351 lines) |
| 0x150BC | `lcp_calc_floor_waypoint` | Compute next waypoint for multi-floor routing |
| 0x15224 | `get_floor_number_from_y` | Convert Y coordinate to floor number |
| 0x14FEC | `lcp_play_footstep_sound` | Surface-dependent footstep SFX |
| 0x1635E | `house_get_position_xy` | Convert HOUSE_POS to screen coordinates |

### Sprite Rendering

| Address | Function | Purpose |
|---|---|---|
| 0x26244 | `sprite_update_body` | Build and position body sprite (slot 3) |
| 0x2664C | `sprite_lcp_head_update` | Build and position head sprite (slot 4) |
| 0x26368 | `sprite_lcp_head_animate` | Random head movement state machine |
| 0x267B0 | `sprite_lcp_build_all` | Build all LCP sprite components |

### Head Animation

| Address | Function | Purpose |
|---|---|---|
| 0x26368 | `sprite_lcp_head_animate` | Head direction randomizer |
| 0x2664C | `sprite_lcp_head_update` | Head sprite rendering |
| 0x26530 | `lcp_wait_head_reach_target` | Block until head reaches target angle |

### Carried Objects

| Address | Function | Purpose |
|---|---|---|
| 0x24A26 | `spritedata_select_carried_object_left` | Set carried object for left-facing |
| 0x24A94 | `spritedata_select_carried_object_right` | Set carried object for right-facing |

### Walk Helpers

| Address | Function | Purpose |
|---|---|---|
| 0x1DF06 | `action_walk_to_and_turn` | Walk to position and face screen |
| 0x1DEBC | `hide_lcp_sprites` | Hide all LCP sprites |
| 0x1DEDC | `show_lcp_sprites` | Show all LCP sprites |
