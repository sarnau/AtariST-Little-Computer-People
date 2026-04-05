# Little Computer People — Dog AI & Movement System

The dog is an autonomous companion character that wanders the three-floor house
independently of the LCP (Little Computer Person). It has its own pathfinding,
stair navigation, eating behavior, and sprite animation system.

## Overview

The dog's entire AI runs inside `screen_render_8hz()` — the main 8 Hz rendering
loop. Each frame, the dog's position is updated, its target is checked, and its
sprite is drawn into one of two hardware sprite slots depending on depth relative
to the LCP character. The dog is never directly controlled by the player; it
wanders autonomously and reacts to environmental conditions (food bowl, petting).

## House Geometry

### Floor Layout

The house has three floors separated by fixed Y-coordinate thresholds:

| Floor | Number | Y Range | `get_floor_number_from_y()` |
|---|---|---|---|
| Top | 3 | Y < 78 | Bedroom, study |
| Middle | 2 | 78 ≤ Y < 141 | Living room, bathroom |
| Bottom | 1 | Y ≥ 141 | Kitchen, entry hall |

Each floor has a center Y coordinate stored in `floor_center_y_coords[]` and
a bottom edge in `floor_bottom_y_coords[]`. Characters gravitate toward the
center line when walking horizontally across a floor.

### Staircase

A single staircase column connects all three floors. Navigation between floors
requires routing through waypoint coordinates stored in `staircase_waypoint_coords[]`,
a table of (x,y) pairs marking the top and bottom of each stair segment.

The staircase has specific Y breakpoints that trigger direction changes:

| Y Value | Meaning |
|---|---|
| 161 | Bottom of lower stair (floor 1 → floor 2 transition) |
| 100 | Top of lower stair / bottom of upper stair (floor 2 landing) |
| 98 | Top of upper stair (floor 2 → floor 3 transition) |

Additional globals `stair_top_y_threshold` and `stair_bottom_y_threshold` define
the stair entry/exit coordinates for the middle floor.

## State Variables

### Position & Navigation

| Variable | Type | Purpose |
|---|---|---|
| `dog_x` | short | Current X pixel position |
| `dog_y` | short | Current Y pixel position |
| `dog_target_x` | short | Final destination X (0 = idle) |
| `dog_target_y` | short | Final destination Y (0 = idle) |
| `dog_waypoint_x` | short | Current intermediate waypoint X |
| `dog_waypoint_y` | short | Current intermediate waypoint Y |
| `dog_on_stairs_flag` | BOOL16 | YES when navigating stairs |

### Animation

| Variable | Type | Purpose |
|---|---|---|
| `dog_walk_anim_cycle` | short | Walk frame counter (0–7) |
| `dog_sprite_id` | sprite_id | Current sprite to display |
| `dog_walk_anim_frames[8]` | sprite_id[] | Walk cycle: SPRITE_DOG_WALK_RIGHT_1–8 |
| `dog_sprite_eating_anim_tab[3]` | sprite_id[] | Eat cycle: SPRITE_DOG_EATING_1–3 |

### Wandering AI

| Variable | Type | Purpose |
|---|---|---|
| `dog_idle_countdown` | short | Ticks until next wander (20–200) |
| `dog_last_target_index` | short | Last destination index (avoid repeats) |
| `dog_visible` | BOOL16 | NO restricts to top-floor destinations |
| `dog_destination_position_table[9]` | HOUSE_POS[] | Wander destination lookup |
| `dog_dest_x_offset_table[9]` | short[] | Per-destination X offset |
| `dog_dest_y_offset_table[9]` | short[] | Per-destination Y offset |

### Eating

| Variable | Type | Purpose |
|---|---|---|
| `dog_near_food_bowl` | BOOL16 | YES when dog is near bowl area |
| `dog_eating_active` | BOOL16 | YES during eating animation |
| `dog_eating_countdown` | short | Frames remaining in eat cycle (82–100) |
| `dog_food_bowl_change` | short | -1 = drain bowl one level, 0 = no change |
| `lcp_dog_bowl_status` | DOG_BOWL_STATUS | BOWL_EMPTY / BOWL_HALF / BOWL_FULL |

### Petting

| Variable | Type | Purpose |
|---|---|---|
| `dog_pettable_flag` | BOOL16 | YES when LCP is crouching to pet |
| `petting_dog_active` | BOOL16 | YES during petting animation |
| `petting_anim_frame` | short | Current frame of 11-frame petting sequence |
| `petting_last_sprite_slot` | short | Sprite slot to hide after petting ends |

### System

| Variable | Type | Purpose |
|---|---|---|
| `dog_initialized` | BOOL16 | YES suppresses rendering during intro |
| `dog_flip_image_buffer` | void* | Pre-allocated buffer for horizontally flipped sprite |
| `dog_flip_mask_buffer` | void* | Pre-allocated buffer for flipped mask |

## Dog Sprites

| ID | Enum Name | Usage |
|---|---|---|
| 33 | `SPRITE_DOG_LAY_DOWN` | Idle / resting pose |
| 34 | `SPRITE_DOG_WALK_RIGHT_1` | Walk cycle frame 1 |
| 35 | `SPRITE_DOG_WALK_RIGHT_2` | Walk cycle frame 2 |
| 36 | `SPRITE_DOG_WALK_RIGHT_3` | Walk cycle frame 3 |
| 37 | `SPRITE_DOG_WALK_RIGHT_4` | Walk cycle frame 4 |
| 38 | `SPRITE_DOG_WALK_RIGHT_5` | Walk cycle frame 5 |
| 39 | `SPRITE_DOG_WALK_RIGHT_6` | Walk cycle frame 6 |
| 40 | `SPRITE_DOG_WALK_RIGHT_7` | Walk cycle frame 7 |
| 41 | `SPRITE_DOG_WALK_RIGHT_8` | Walk cycle frame 8 |
| 42 | `SPRITE_DOG_EATING_1` | Eating animation frame 1 |
| 43 | `SPRITE_DOG_EATING_2` | Eating animation frame 2 |
| 44 | `SPRITE_DOG_EATING_3` | Eating animation frame 3 |

The walk cycle uses 8 frames indexed by `dog_walk_anim_cycle` (0–7) via
`dog_walk_anim_frames[]`. The eating animation cycles through 3 frames
via `dog_sprite_eating_anim_tab[countdown % 3]`.

Horizontal flipping is performed by `sprite_flip_horizontal()` into dedicated
buffers (`dog_flip_image_buffer`, `dog_flip_mask_buffer`) when the dog faces left.

## Sprite Rendering

The dog uses **two hardware sprite slots simultaneously**:

| Slot | Layer | When Used |
|---|---|---|
| 0 | Behind LCP (`SPRITE_BEHIND_LCP`) | Dog Y > LCP Y + 5 (dog is "further back") |
| 7 | In front of LCP (`SPRITE_IN_FRONT`) | Dog Y ≤ LCP Y + 5 (dog is "closer") |

Only one slot receives the actual image pointer; the other stays NULL. Both slots
receive the same position, size, and mask data. This dual-slot technique creates
depth sorting — the dog seamlessly transitions between appearing behind or in front
of the LCP character as they move past each other vertically.

The `spritedata_update_dog(sprite_id, depth_layer, flip_horizontal)` function
handles all sprite setup:

1. Clear both slot images (0 and 7)
2. If sprite_id is valid and dog is initialized:
   - If flipping: copy sprite data through `sprite_flip_horizontal()` into buffers
   - Set width/height from sprite definition arrays for both slots
   - Set X = `dog_x`, Y = `dog_y - 17` (sprite anchor offset) for both slots
   - Set mask pointers for both slots
   - Set image pointer for **only** the active depth slot (0 or 7)

Exception: when `dog_initialized = YES`, all rendering is skipped (used during
the intro sequence before the dog appears).

## Movement Algorithm

### Entry Point

`dog_move_and_animate()` is called once per frame from `screen_render_8hz()`.
It executes only when the dog has an active target (`dog_target_x != 0` or
`dog_target_y != 0`).

### Step 1: Animation Cycle

```
dog_walk_anim_cycle = (dog_walk_anim_cycle + 1) % 8
dog_sprite_id = dog_walk_anim_frames[dog_walk_anim_cycle]
```

The 8-frame walk animation runs continuously while the dog is moving,
regardless of direction or terrain.

### Step 2: Depth Layer Calculation

```
if dog_y + 5 > lcp_y:
    depth_layer = -1    (behind LCP → slot 0)
else:
    depth_layer = 1     (in front of LCP → slot 7)

if LCP is reading newspaper:
    depth_layer = 1     (always in front during reading)
```

The +5 offset creates a small hysteresis band to prevent flickering when
the dog and LCP are at nearly the same Y coordinate.

### Step 3: Waypoint Routing

If no waypoint is set, `dog_calc_walk_path()` computes the next waypoint:

```
target_floor = get_floor_number_from_y(dog_target_y)
current_floor = get_floor_number_from_y(dog_y)

if current_floor == target_floor:
    # Same floor: walk directly to target
    dog_on_stairs_flag = NO
    waypoint = target

else:
    # Different floor: route through staircase
    stair_entry = staircase_waypoint_coords[(current_floor - 1) * 2]

    if dog is already at stair entry:
        dog_on_stairs_flag = YES
        if going up:
            waypoint = staircase_waypoint_coords[(current_floor - 1) * 2 + 2]
        else:
            waypoint = staircase_waypoint_coords[(current_floor - 1) * 2 - 2]
    else:
        # Walk to stair entry first
        waypoint = stair_entry
```

Special cases:
- **Floor 2 going down**: uses `stair_top_y_threshold` / `stair_bottom_y_threshold`
- **Floor 3 entry**: offsets `dog_x` by -8 to align with stair column
- **Floor 1 stair entry**: overrides waypoint to threshold coordinates

### Step 4a: Flat Walking (stairs flag = NO)

Horizontal movement — 1 pixel per tick:

```
if dog_x < waypoint_x:
    dog_x += 1; flip = NO (facing right)
else if dog_x > waypoint_x:
    dog_x -= 1; flip = YES (facing left)
```

Vertical movement depends on horizontal distance to waypoint:

```
x_distance = abs(dog_x - waypoint_x)

if x_distance < 8:
    # Close to waypoint: move directly toward target Y
    if dog_y < waypoint_y: dog_y += 1
    else if dog_y > waypoint_y: dog_y -= 1
else:
    # Far from waypoint: gravitate to floor center line
    floor = get_floor_number_from_y(dog_y)
    center = floor_center_y_coords[floor - 1]
    if dog_y < center: dog_y += 1
    else if dog_y > center: dog_y -= 1
```

This two-phase approach means the dog first walks horizontally at the floor's
center line, then adjusts vertically only when close to the destination. This
produces natural-looking movement — the dog doesn't walk diagonally across
the entire room.

### Step 4b: Stair Navigation (stairs flag = YES)

Stair movement uses hardcoded Y breakpoints for the staircase geometry.
The dog moves at 1–2 pixels per tick in a diagonal pattern.

**Going up** (waypoint Y < current Y):

| Y Position | Action |
|---|---|
| Y = 161 | Landing transition: flip left, Y = 159, X -= 17 |
| Y = 100 | Landing transition: flip right, Y = 98, X += 3 |
| Y < 100 | Upper stair: flip right, Y -= 1, X += 1 or +2 |
| 101 ≤ Y ≤ 161 | Lower stair: flip left, Y -= 1, X -= 1 or -2 |
| Other | Fast vertical: Y -= 2 (landing area) |

**Going down** (waypoint Y > current Y):

| Y Position | Action |
|---|---|
| Y = 161 | Landing transition: flip right, Y = 165, X += 1 |
| Y = 100 | Landing transition: flip right, Y = 102, X += 3 |
| Y < 100 | Upper stair (down): flip left, Y += 1, X -= 1 or -2 |
| 101 ≤ Y ≤ 161 | Lower stair (down): flip right, Y += 1, X += 1 or +2 |
| Other | Fast vertical: Y += 1 (landing area) |

The "1 or 2" horizontal movement alternates based on the current sprite frame:
when `dog_sprite_id == SPRITE_DOG_WALK_RIGHT_9`, the dog moves 1 pixel; otherwise
2 pixels. This creates the diagonal zigzag pattern that mimics climbing stairs.

### Step 5: Stair Exit Detection

After each movement step, the function checks if the dog has cleared the staircase:

```
floor = get_floor_number_from_y(waypoint_y)
if dog_y <= floor_bottom_y_coords[floor - 1]:
    if floor == 3:
        dog_on_stairs_flag = NO   # reached top floor
    else if dog_y >= staircase_waypoint_coords[...]:
        dog_on_stairs_flag = NO   # cleared stair segment
```

### Step 6: Arrival

```
if dog reached waypoint:
    if waypoint == target:
        # Final destination reached
        clear target and waypoint
        dog_sprite_id = SPRITE_DOG_LAY_DOWN
    else:
        # Intermediate waypoint (stair entry/exit)
        dog_calc_walk_path()   # compute next segment
```

## Wandering AI

The wandering logic runs in `screen_render_8hz()` when the dog is idle
(target = 0,0) and not eating.

### Idle Timer

```
if dog is idle and not eating:
    dog_idle_countdown -= 1

if dog_idle_countdown reaches 0:
    pick new random destination
    dog_idle_countdown = randomRange(20, 200)   # 2.5 to 25 seconds
```

### Destination Selection

```
if dog_visible == NO:
    range = 0 to 2    (top floor destinations only)
else:
    range = 3 to 8    (all destinations)

repeat:
    index = randomRange(range_min, 8)
until index != dog_last_target_index    # avoid repeating

position = dog_destination_position_table[index]
house_get_position_xy(position, &dog_target_x, &dog_target_y)
dog_target_x += dog_dest_x_offset_table[index]
dog_target_y += dog_dest_y_offset_table[index]

if position == POS_BTM_STAIR_LANDING:
    dog_near_food_bowl = YES

dog_last_target_index = index
```

The destination table contains 9 `HOUSE_POS` entries pointing to various
locations throughout the house. The per-destination X/Y offset tables allow
fine-tuning the exact stopping position (e.g., slightly in front of the couch
rather than exactly at the room's anchor point).

When `dog_visible = NO` (set during certain actions), the dog is restricted to
indices 0–2, which correspond to top-floor positions only, keeping the dog
out of the way during cutscenes or events.

## Eating Behavior

### Trigger Conditions

All of these must be true simultaneously:

1. Dog is idle: `dog_target_x == 0 && dog_target_y == 0`
2. Food bowl is not empty: `lcp_dog_bowl_status != BOWL_EMPTY`
3. Dog is flagged near bowl: `dog_near_food_bowl == YES`
4. Dog is not already eating: `dog_eating_active == NO`
5. Dog position is near food bowl: `dog_x < 20` and `dog_y > 160`

The food bowl is located in the bottom-left corner of the house (near x=8, y=190).
The `dog_near_food_bowl` flag is set when the wandering AI selects
`POS_BTM_STAIR_LANDING` as the destination, which routes the dog near the bowl area.

### Eating Sequence

```
dog_eating_active = YES
dog_eating_countdown = randomRange(82, 100)   # ~10-12 seconds

each tick:
    dog_eating_countdown -= 1

    if countdown == 0:
        dog_eating_active = NO
        dog_near_food_bowl = NO
        dog_food_bowl_change = -1    # final bowl drain

    else if countdown == 60, 30, or 4:
        dog_food_bowl_change = -1    # drain bowl one level

    else:
        dog_food_bowl_change = 0     # no change this tick

    dog_sprite_id = dog_sprite_eating_anim_tab[countdown % 3]
    spritedata_update_dog(dog_sprite_id, 1, NO)
```

The eating animation cycles through 3 sprites (SPRITE_DOG_EATING_1/2/3) using
modulo indexing. The food bowl drains at three specific moments during eating
(countdown = 60, 30, 4), plus one final drain when eating ends. This means a
full bowl (BOWL_FULL) can be emptied to BOWL_EMPTY in one eating session if
the dog eats long enough.

### Food Bowl Display

The bowl visual is updated in `game_tick_and_animate()`:

```
object_draw(_object_dog_eating_animation[lcp_dog_bowl_status], 8, 190)
```

The bowl has 3 visual states mapped to `DOG_BOWL_STATUS`:
- `BOWL_EMPTY` (0): empty bowl sprite
- `BOWL_HALF` (1): half-full bowl sprite
- `BOWL_FULL` (2): full bowl sprite

Bowl changes are clamped to the valid range (0–2) and applied per-tick in
`game_tick_and_animate()` based on `dog_food_bowl_change`.

### Feeding the Dog

The player feeds the dog via `action_feed_dog()` (triggered by Ctrl+D delivery):

1. LCP walks to fridge (`POS_BTM_FRIDGE`)
2. Opens fridge door (SFX_DOOR_OPEN)
3. Carries food bowl (STATE_CARRY_WALK) to feeding area
4. Sets `lcp_dog_bowl_status = BOWL_FULL`
5. Sets `dog_food_bowl_change = 1` (triggers bowl fill animation)
6. Returns bowl to fridge

## Petting Interaction

### Trigger

The player presses Ctrl+P, which queues `ACTION_PET_DOG`.

### Sequence

1. **Call dog** (`action_call_dog()`):
   - LCP walks to `POS_BTM_DOG_FOOD`
   - Faces right, crouches (STATE_CROUCH_DOWN)
   - Sets `dog_pettable_flag = YES`

2. **Wait for dog** (`action_pet_dog()`):
   - Waits 100–200 frames (12–25 seconds) for the dog to wander near
   - During intro sequence: reduced to 10 frames
   - Exits early if a game event triggers

3. **Petting animation** (in `game_tick_and_animate()`):
   - When `petting_dog_active = YES`: plays 11-frame animation sequence
   - Uses `object_id_ARRAY_0002b93e[]` — a table of sprite IDs for the petting overlay
   - Each frame rendered at fixed position (192, 165) in SPRITE_BEHIND_LCP layer
   - Previous frame hidden, current frame shown
   - After 11 frames: hide last sprite, clear `petting_dog_active`

Note: the petting system does not explicitly move the dog to the LCP. Instead,
`dog_pettable_flag` is set and the normal wandering AI is expected to eventually
bring the dog near the LCP's position. The petting animation is a separate
overlay sprite sequence, not the dog sprite itself.

## Comparison: Dog vs LCP Pathfinding

Both the dog and LCP use the same fundamental algorithm — waypoint-based
pathfinding through the staircase coordinate table. The key differences:

| Feature | LCP | Dog |
|---|---|---|
| Pathfinding function | `lcp_calc_floor_waypoint` | `dog_calc_walk_path` |
| Movement function | `lcp_pathfind_one_step` | `dog_move_and_animate` |
| Walking speed (flat) | 2 pixels/tick | 1 pixel/tick |
| Walking speed (stairs) | Variable | 1–2 pixels/tick |
| Stair coordinate table | `staircase_waypoint_coords[]` | Same table |
| Floor detection | `get_floor_number_from_y()` | Same function |
| Sprite slots | 3 (body), 4 (carried object), 5–6 (head) | 0 and 7 |
| Walk animation | 8 LCP body states | 8 dog sprites (34–41) |
| Autonomous | No (action-driven) | Yes (idle timer) |
| Depth sorting | Always rendered | Dual-slot 0/7 behind/front toggle |
| Horizontal flip | Via LCP sprite system | `sprite_flip_horizontal()` to buffer |
| Sprite anchor Y offset | -21 | -17 |

The LCP's movement is **action-driven** — it walks to specific positions as part
of scripted action sequences (`lcp_walk_to_destination()`). The dog's movement
is **autonomous** — it wanders randomly with idle timers, gravitating toward the
food bowl when hungry.

Both characters share the same floor geometry (`get_floor_number_from_y()`),
staircase waypoints (`staircase_waypoint_coords[]`), and floor center lines
(`floor_center_y_coords[]`). The pathfinding functions are structurally
identical — `dog_calc_walk_path` is a simplified mirror of `lcp_calc_floor_waypoint`.

## Function Reference

| Address | Function | Purpose |
|---|---|---|
| 0x1412C | `dog_move_and_animate` | Main per-tick movement and animation |
| 0x14586 | `dog_calc_walk_path` | Calculate next waypoint for multi-floor routing |
| 0x248FE | `spritedata_update_dog` | Update dog sprite slots 0 and 7 |
| 0x15224 | `get_floor_number_from_y` | Convert Y coordinate to floor number (shared) |
| 0x150BC | `lcp_calc_floor_waypoint` | LCP equivalent of dog_calc_walk_path (shared stair table) |
| 0x25138 | `screen_render_8hz` | Contains wandering AI, eating trigger, food bowl update |
| 0x256A6 | `game_tick_and_animate` | Contains petting animation, food bowl display |
| 0x20C9E | `action_pet_dog` | Player-initiated petting action |
| 0x20C50 | `action_call_dog` | LCP crouches and calls dog over |
| 0x20AF8 | `action_feed_dog` | Fill food bowl via fridge |
