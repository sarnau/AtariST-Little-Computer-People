# Little Computer People — Atari ST Reverse Engineering

**LCP.PRG** is the main executable for *Little Computer People* on the Atari ST, a life-simulation game originally developed by Activision (Rich Gold, David Crane) and released in 1985. The player observes and interacts with a virtual character — the "Little Computer Person" — who lives inside a three-story house displayed on screen.

This document describes the complete architecture as recovered through reverse engineering of the Atari ST binary using Ghidra with MCP integration.

## Game Overview

Little Computer People is one of the earliest "virtual pet" or life-simulation games, predating The Sims by over a decade. A procedurally generated character moves into a furnished house and autonomously goes about daily life — eating, sleeping, showering, exercising, playing piano, writing letters, and more. The player can interact by typing natural-language commands, ringing the doorbell (Ctrl+A), ordering deliveries, and playing card and word games.

Each copy of the game generates a unique character with randomized appearance, name, personality, and daily schedule. The character's state persists across sessions via a 128-byte save file (`hyber`), creating a sense of a living, persistent digital companion.

## Technical Specifications

| Property | Value |
|---|---|
| Platform | Atari ST/STe |
| CPU | Motorola 68000, 32-bit big-endian |
| Resolution | 320×200, 16 colors (ST low) |
| Binary format | GEMDOS PRG executable |
| Compiler | Alcyon C (Digital Research CP/M-68K toolchain) |
| Integer size | 16-bit (`int` = `short` in Ghidra) |
| Code size | ~104 KB TEXT segment (0x10000–0x296DB) |
| Total functions | 395 identified and named (100% coverage) |
| Functions with plate comments | 395 (100%) |
| Total symbols | ~3,500 labeled |
| Custom enums | 20+ (sprite_id, ACTION_ID, PLAYER_STATE, etc.) |
| Custom structs | 6+ (LCP, PSG_ENVELOPE, MFDB, FILE_IMG_DATA, etc.) |
| Save file | `hyber`, 128 bytes (LCP struct) |

### Memory Layout

| Segment | Address Range | Size | Contents |
|---|---|---|---|
| LOWMEM_VARS | 0x000400–0x0005FF | 512 B | Atari ST system variables (_hz_200, _vbclock, conterm) |
| TEXT | 0x010000–0x0296DB | 104 KB | Executable code (395 functions) |
| DATA | 0x0296DC–0x02C6BF | 12 KB | Initialized data (ROM tables, strings, animation frames) |
| BSS | 0x02C6C0–0x05A2F9 | 183 KB | Uninitialized data (screen buffers, sprite arrays, game state) |
| IO | 0xFFFF0000–0xFFFFFDFF | — | Hardware registers (YM2149 PSG, DMA, MFP, ACIA) |

### Function Categories

| Category | Prefix | Count | Description |
|---|---|---|---|
| Action handlers | `action_`/`event_` | 54 | Player behavior sequences and triggered events |
| CRT/library | `_` prefix | 33 | Alcyon C runtime (matched to original source) |
| Poker | `poker_` | 30 | Full 5-card draw poker with AI |
| MIDI sequencer | `midi_seq_`/`midi_` | 24 | Custom MIDI-like song engine |
| Sprite system | `sprite`/`spritedata` | 19 | Overlay sprite pipeline and rendering |
| LCP character | `lcp_` | 18 | Movement, pathfinding, animation, needs |
| Copy protection | `copyprot_` | 13 | Floppy disk verification |
| VDI/AES bindings | `v_`/`vs_`/`vdi_` | 9 | GEM graphics calls |
| Screen rendering | `screen_` | 8 | Double-buffered compositor |
| PSG sound | `psg_` | 4 | YM2149 envelope processor |
| Dog AI | `dog_` | 3 | Dog movement, eating, animation |

## The LCP Character

Each character is defined by a 128-byte `LCP` struct that is saved to and loaded from the `hyber` file. Key attributes:

### Appearance
- **Clothing color**: 16 outfit combinations (e.g., `OUTFIT_BLUE_GREEN`, `OUTFIT_RED_PINK`) mapped to palette entries 1–2
- **Skin color**: 8 options including realistic and fantasy tones, mapped to palette entry 6
- **Character sprite set**: 5 visual variants (PE2–PE6.LCP files)
- **Character name**: Randomly selected from a pool of 266 names

### Personality & Schedule
- **Personality type** (0–3): Affects behavior weighting
- **Activity level** (0–7): Controls action table selection via `activity_schedule_table[3][8]`
- **Daily schedule**: Configurable wake hour (6–8am), bedtime (10pm–midnight), lunch (11am–1pm), dinner (5–7pm)
- **Initiative threshold** (20–80): Lower = more proactive about autonomous actions

### Needs System

Three basic needs drive urgent behavior:
- **Thirst**: Timer decrements each game-minute; at 0, `thirst_level` increments (0→1→2→3→sick)
- **Hunger**: Same mechanic with separate timer; triggers eating or sickness
- **Bathroom**: Timer-based; sets `bathroom_need` flag, prioritized in the AI decision engine

### Mood & Health
- **Happiness**: Cycles between `MOOD_HAPPY` (0), `MOOD_CONTENT` (1), and `MOOD_SAD` (2) over configurable hourly durations
- **Sickness**: 5 levels (`SICKNESS_HEALTHY` through `SICKNESS_CRITICAL`); triggered when thirst or hunger reaches level 3+; worsens every 60 game-minutes if untreated, recovers every 5 minutes once fed and watered; forces `MOOD_SAD` at level 2+; changes skin palette to green

## AI Decision Engine

The central AI function `check_for_any_action_triggers` evaluates conditions in strict priority order:

1. **Event queue** — doorbell, food/book/record delivery, phone call
2. **Alarm** — Ctrl+A pressed → `ACTION_WAKE_FROM_ALARM`
3. **Bathroom need** → `ACTION_USE_TOILET`
4. **Thirst** (randomized by sickness) → `ACTION_DRINK`
5. **Hunger** (randomized by sickness) → `ACTION_KITCHEN_CABINET`
6. **Scheduled meals** → `ACTION_EAT_MEAL` at configured lunch/dinner hours
7. **Sleep schedule** → `ACTION_WAKE_UP_MORNING` / `ACTION_GO_TO_BED_NIGHT`
8. **Player commands** — typed word-based command queue with priority escalation
9. **Random activity** — selected from personality-weighted action tables

### Action Tables

Three action tables provide different activity mixes:

| Table | Bias | Example Actions |
|---|---|---|
| `action_table_active` | Energetic | Computer, dance, exercise, write letter |
| `action_table_moderate` | Balanced | Read newspaper, play game, brush teeth |
| `action_table_relaxed` | Low-key | Sit on couch, yawn, wander, sleep |

The `activity_schedule_table[3][8]` selects which table to use based on time-of-day and the character's `activity_level`. Weekend override: Sunday forces relaxed activities, Saturday forces moderate.

### 45 Action Handlers

The action dispatcher `do_action` switches on `ACTION_ID` (0–44) to invoke the appropriate handler. Each action is a self-contained sequence of walking to a position, playing animations, and updating state. Key actions include:

| Action | Description |
|---|---|
| `ACTION_PLAY_COMPUTER` | Walk to desk, sit, type with random clicking sounds |
| `ACTION_WRITE_LETTER` | Get paper, sit at desk, procedurally generate letter text |
| `ACTION_PLAY_PIANO` | Walk to piano, play music from .org files |
| `ACTION_DANCE` | Put on record, dance to music with left/right steps |
| `ACTION_LIGHT_FIREPLACE` | Get logs from outside, carry to fireplace, light fire |
| `ACTION_TAKE_SHOWER` | Enter bathroom, shower animation with 5 poses |
| `ACTION_FEED_DOG` | Get food from fridge, carry to bowl, fill it |
| `ACTION_PLAY_A_GAME` | Select from 5 mini-games, play at kitchen table |

## Player Interaction

### Typed Commands

The player types natural-language phrases that are parsed against a 161-word vocabulary (`WORD_ID` enum). A word-to-action mapping table (`enteredword_to_action`) converts recognized word bitmask patterns into game actions with priority offsets.

### Keyboard Shortcuts

| Key | Action | Effect |
|---|---|---|
| Ctrl+A | Alarm clock | Wakes the character, rings alarm sound |
| Ctrl+B | Book delivery | Doorbell + book (SPRITE_BOOK) at front door |
| Ctrl+C | Phone call | Random phone rings, character answers |
| Ctrl+D | Dog food | Dog food delivery via `delivery_is_for_dog` flag |
| Ctrl+F | Food delivery | Food package (SPRITE_FOOD_PACKAGE) to kitchen cabinet |
| Ctrl+R | Record delivery | New vinyl record (SPRITE_VINYL_CARRY) delivery |
| Ctrl+W | Water delivery | Refills water supply |
| Ctrl+P | Pet the dog | 7-frame hand animation (SPRITE_PET_HAND_1–7) |

## Mini-Games

Five card and word games are available, played at the kitchen table:
- **Anagrams** — Unscramble a word within 9 guesses; F1 reveals a letter clue
- **War** — Classic high-card game
- **Poker** — 5-card draw with betting, raising, and AI bluffing (30 functions, `poker_*` prefix)
- **Blackjack** — Casino rules with full hand evaluation
- **Word Puzzles** — Crossword-style puzzles from external template files

## House Layout

The three-story house has 48 navigable positions (`HOUSE_POS` enum) organized by floor:

**Top Floor** (Living Room / Study) — Positions 0–15: Armchair, game table, record shelf, fireplace with log area, study door, filing cabinet, desk with lamp

**Middle Floor** (Bedroom / Bathroom) — Positions 16–31: Bed, dresser, bedroom closet, couch, bathroom sink, toilet, shower, computer desk, piano

**Bottom Floor** (Kitchen / Entrance) — Positions 32–47: Kitchen sink, stove, fridge, food cabinet, kitchen table, dog bowl, front door

The character navigates between positions using `lcp_pathfind_one_step`, which handles flat walking (8-frame cycle), stair climbing/descending (4-frame cycles), and waypoint-based routing. Floor baseline Y coordinates: top=77, mid=140, bottom=202.

### Static Object Positions (from main() initialization)

| Object | Position |
|---|---|
| Cabinet | (46, 140) |
| Front door | (294, 151) |
| Dresser | (97, 115) |
| Closet | (75, 87) |
| Study door | (178, 23) |
| Toilet door | (187, 87) |
| Filing cabinet | (258, 47) |
| Dog bowl | (8, 190) |
| Fridge | (24, 153) |

## Graphics System

### Double-Buffered Sprite Rendering

The game runs at ~8 Hz with a double-buffered rendering pipeline in `screen_render_8hz`:

1. **Background copy**: `blkcopy32` copies the static house scene from the offscreen buffer (32-byte aligned block copy, with three modes depending on `text_scroll_timer` for partial updates)
2. **Dog animation**: `dog_move_and_animate` advances the dog's position, handles stair navigation, and triggers eating behavior when near a full food bowl
3. **Sprite compositing**: Iterates over all 8 hardware sprite slots; for each with a non-NULL image pointer, calls `sprite_draw` to composite using masked blitting
4. **Page flip**: `XBIOS Vsync + Setscreen` swaps the display to the newly composited buffer
5. **Sound effects**: Plays any queued sound effects via `soundeffect_irq_play`

### Sprite Pipeline Architecture

The sprite system uses a three-level pipeline with a pending/active double buffer:

**Definition level** (60 sprites, loaded from `sprites` file at startup):
- `sprite_def_image[60]` / `sprite_def_mask[60]` — pointers to image/mask bitmap data
- `sprite_def_width[60]` / `sprite_def_height[60]` — pixel dimensions

**Pending buffer** (8 hardware slots, staged by game logic):
- `sprite_pending_image[8]` / `sprite_pending_mask[8]` — image/mask to render next frame
- `sprite_pending_x[8]` / `sprite_pending_y[8]` — screen coordinates
- `sprite_pending_width[8]` / `sprite_pending_height[8]` — dimensions
- `sprite_pending_flag[8]` — set to YES when slot has new data ready

**Active buffer** (8 hardware slots, consumed by renderer):
- `sprite_active_image[8]` / `sprite_active_mask[8]` — current frame's data
- `sprite_active_x[8]` / `sprite_active_y[8]` — current screen position
- `sprite_active_width[8]` / `sprite_active_height[8]` — current dimensions

Each frame in `screen_render_8hz`, slots with `sprite_pending_flag == YES` are committed from pending to active, then rendered via `sprite_draw`.

Note: `spritedata_select` bypasses the pending buffer and writes directly to the active arrays for immediate display.

### Sprite Slot Assignment

60 logical sprites are multiplexed onto 8 hardware rendering slots via `sprite_update_slots`:
- Slots 3–4: Reserved for LCP body (3) and head (4) sprites
- Slots 1–2: Behind-LCP layer (`sprite_layer_flags[n] == SPRITE_BEHIND_LCP`)
- Slots 5–6: In-front-of-LCP layer (`sprite_layer_flags[n] == SPRITE_IN_FRONT`)
- Slots 0, 7: Dog sprite (behind or in front based on Y-depth comparison with LCP)

`sprite_slot_map[60]` stores the hardware slot assigned to each logical sprite. `sprite_layer_flags[60]` stores the visibility/layer state (`SPRITE_LAYER` enum: SPRITE_HIDDEN, SPRITE_BEHIND_LCP, SPRITE_IN_FRONT).

### Sprite Compositing

Each sprite slot is rendered via two VDI `vro_cpyfm` raster operations using MFDB structures (`sprite_mfdb_image[8]`, `sprite_mfdb_mask[8]`):
1. `NOTS_AND_D`: AND the inverted mask with the destination (punches a transparent hole)
2. `S_XOR_D`: XOR the sprite image onto the cleared area (paints sprite pixels)

This is the classic Atari ST masked blit technique. The mask is auto-generated at load time by `spritedata_generate_mask_from_color`: any pixel where all 4 bitplanes are 0 (color index 0 = transparent) produces a 0 mask bit.

### sprite_id Enum (56 values)

The `sprite_id` enum maps logical sprite indices to their purpose, identified by tracing all action functions:

| ID | Name | Usage |
|---|---|---|
| 3 | SPRITE_GLASS | Carried to water tap (action_drink) |
| 4 | SPRITE_GAME_BOX | Carried to table (action_play_a_game) |
| 7 | SPRITE_VINYL_RECORD | Shown at record shelf during browsing |
| 8 | SPRITE_TYPEWRITER | Shown at desk during letter writing |
| 9 | SPRITE_FOOD_PACKAGE | Carried from fridge / food delivery |
| 12 | SPRITE_TABLE_SETTING | Placed on table during eating/games |
| 13–15 | SPRITE_DOOR_ANIM_1–3 | Door overlay animation (3 openness stages, used for toilet door and bedroom closet) |
| 16–18 | SPRITE_CLOSET_LCP_INSIDE / AJAR / WIDE_OPEN | Closet dress-change sequence |
| 19–20 | SPRITE_PET_DOG_1–2 | Petting dog hand approach |
| 21 | SPRITE_DOG_SIT | Dog sitting/idle at front door |
| 22 | SPRITE_FIREWOOD | Carried from front door to fireplace |
| 23 | SPRITE_COOKING_POT | Carried from cabinet to stove |
| 24–26 | SPRITE_DOOR_STUDY_1 / AJAR / WIDE_OPEN | Study door overlay frames |
| 27–32 | SPRITE_PET_HAND_1–6 | Petting hand animation sequence |
| 33 | SPRITE_DOG_LAY_DOWN | Dog idle/resting pose |
| 34–41 | SPRITE_DOG_WALK_RIGHT_1–8 | Dog walk cycle (from `dog_walk_anim_frames[8]`) |
| 42–44 | SPRITE_DOG_EATING_1–3 | Dog eating animation |
| 45–47 | SPRITE_READING_1–3 | Book reading animation |
| 48 | SPRITE_SUITCASE | Carried during move-in cutscene |
| 49 | SPRITE_BOOK | Carried from front door (book delivery) |
| 50 | SPRITE_VINYL_CARRY | Carried from front door (record delivery) |
| 51 | SPRITE_PET_HAND_7 | Petting animation final frame |
| 52 | SPRITE_DESK_LAMP | Desk area overlay during writing |
| 51–54 | SPRITE_TYPING_1–4 | Typing animation frames |

### Door Overlay Technique

Sprites 13–15 (SPRITE_DOOR_ANIM_1–3) are door overlay sprites at three stages of openness, rendered IN FRONT of the LCP sprite to create the illusion of walking behind/through a door. When the LCP walks toward a door, the door sprite (fully open) is placed in the SPRITE_IN_FRONT layer so the character appears to walk behind it. As the door closes, progressively narrower door sprites are swapped in. This technique is used for both the toilet door (187,87) and the bedroom closet (75,87).

### LCP Character Sprites

The character is assembled from separate body and head sprite sheets:
- **Body** (`body.lcp`): 91 animation states (`PLAYER_STATE` enum) — walk cycle, stairs, sitting, showering, dancing, etc.
- **Head** (`pex.lcp`): Expression frames selected by `head_sprite_frame` and `happiness` level, with random head movements controlled by `HEAD_ANIM_MODE`

Both are 2-word-wide source sprites expanded to 4-word-wide compositing buffers by `lcp_flip_sprite_horizontal`, with bit-reversal via `revert_table[256]` for horizontal mirroring.

### Dog Sprite System

The dog uses a separate rendering path via `spritedata_update_dog`:
- Uses hardware slots 0 and 7 (behind and in front of LCP based on Y-depth comparison)
- Walk animation: 8-frame cycle from `dog_walk_anim_frames[8]` (SPRITE_DOG_WALK_RIGHT_1–8)
- Eating animation: 3-frame cycle from `dog_sprite_eating_anim_tab[3]` (SPRITE_DOG_EATING_1–3)
- Horizontal flip: bit-reversal into `dog_flip_image_buffer` / `dog_flip_mask_buffer` (32×15 pixel scratch buffers)

### Color Palette

16-color Atari ST palette using `ST_COLOR` format (0x0RGB, 3 bits per channel):
- `main_colorpalette[16]`: Active hardware palette
- `clothing_color_primary/secondary[16]`: 16 outfit color combinations mapped to palette entries 1–2
- `skin_color_palette[8]`: 8 skin tones
- Palette entry 6: `ST_PEACH` (healthy) or `ST_SICK_GREEN` (sick)

### HOUSE.SCN Decompression

The house background is stored as `HOUSE.SCN`, a compressed 320×200 4-bitplane image using a nibble-based RLE scheme with a 15-entry common-word lookup table. The decompression algorithm (`decompress_scn`) processes nibbles from the compressed stream: values 0x1–0xF index into a preloaded common-word table, while 0x0 signals a literal word follows. Run-length encoding uses a repeat-count nibble for consecutive identical words.

## Sound System

### MIDI Sequencer Engine

A custom MIDI-like sequencer (`midi_seq_*` functions, 24 total) plays `.sng` song files with dual output:

**External MIDI** (`midi_output_enabled`): Standard MIDI messages via the Atari ST ACIA at 0xFFFC04, supporting program changes, channel remapping, and octave transposition.

**Internal PSG** (`psg_output_enabled`): YM2149 sound chip with 3 channels, software ADSR envelope processing via the `PSG_ENVELOPE` struct (14 bytes per channel, 3 channels = `psg_envelope[3]`).

### PSG_ENVELOPE Struct (14 bytes)

| Offset | Field | Description |
|---|---|---|
| 0 | phase | Current envelope phase (ENV_PHASE: IDLE→ATTACK→DECAY→SUSTAIN→RELEASE→FADEOUT) |
| 1 | attack_start_vol | Initial volume at note-on |
| 2 | attack_duration | Ticks to reach attack target |
| 3 | attack_target_vol | Peak volume after attack |
| 4 | decay_duration | Ticks from peak to sustain |
| 5 | decay_target_vol | Volume at end of decay |
| 6 | sustain_duration | Ticks to hold sustain |
| 7 | sustain_target_vol | Volume during sustain |
| 8 | release_duration | Ticks to fade to silence |
| 9 | max_volume | Clamp ceiling for output |
| 10 | phase_timer | Countdown within current phase |
| 11 | current_volume | Current output volume (0–15) |
| 12 | ramp_direction | +1 or -1 for volume interpolation |

Envelope processing (`psg_process_envelopes`, called at 50 Hz) uses Bresenham-style integer interpolation for smooth volume ramping.

### Timing

Interrupt-driven via MFP Timer A at 200 Hz (`midi_seq_tick_handler`). The sequencer state machine (`MIDI_SEQ_PHASE` enum: WAIT_NOTE_EXPIRE, PARSE_NEXT_EVENT, SONG_ENDING) manages event parsing and note expiration.

### .SNG File Format

| Section | Content |
|---|---|
| Header (10 bytes) | File identifier (skipped) |
| Channel map (30 bytes) | MIDI channel/program mappings |
| PSG envelopes (360 bytes) | ADSR parameters for 3 PSG channels |
| Header commands | Tempo, volume, scale table setup (parsed by `midi_seq_parse_header` via `midi_header_cmd_values/handlers` jump table) |
| Event stream | Compact 3-byte note format + control events |

### Sound Effects

Separate from the music engine, a sound effect system (`soundeffect_*`) plays ambient sounds via the Atari ST's XBIOS DoSound interface: footsteps (carpet, wood, stairs), doorbell, door open/close, water running, toilet flush, phone ring, typewriter clicks, speech, snoring, alarm clock, and more. 26 effects defined in the `SOUND_EFFECT_ID` enum.

## Copy Protection

The binary includes floppy-disk-based copy protection (`copyprot_*`, 13 functions). Key characteristics:

- **Self-modifying code**: XOR encryption with key 0x1567 (`copyprot_decrypt_code_block` / `copyprot_reencrypt_code_block`)
- **Direct hardware access**: WD1772 FDC via DMA controller at 0xFF8604
- **Raw track reading**: Reads MFM track data and searches for non-standard sector format
- **Gap byte validation**: Checks gap byte counts in two ranges (< 16 and >= 80); both must be found
- **Failure mode**: Sets `copyprot_check_return = 0` → character enters infinite `action_sleep(-1)` loop (permanently sleeps, never performs autonomous actions)

## Data Files

| File | Purpose |
|---|---|
| `house.scn` | Compressed house background scene (320×200, nibble-based RLE) |
| `title.scn` | Title screen scene (same compression) |
| `body.lcp` | Body sprite sheet (91 states × 21 scanlines × 2 word-groups) |
| `PE*.lcp` | Head sprite sheets (per-character expressions + happiness variants) |
| `objects` | 56 static object graphics with FILE_IMG_DATA headers (height, width, pixel data) |
| `sprites` | 50 overlay sprite definitions (mapped to 56 sprite_id slots via `spritedata_index_table[50]`) |
| `sounds.lcp` | Sound effect data (26 DoSound sequences) |
| `cards` | Card game graphics (52 cards + backs) |
| `words` | Anagram dictionary (150 words) |
| `wordpz.txt` | Word puzzle templates |
| `names` | Character name pool (266 × 10 bytes) |
| `letter.txt` | Letter writing templates (4 categories × happiness variants) |
| `hyber` | Save file (128-byte LCP struct) |
| `*.sng` | Music song files (custom MIDI format with channel mapping + PSG envelope data) |
| `*.org` | Organ music files for piano/record playback |

## Reverse Engineering Status

### Completed
- All 395 functions identified, named, and documented with plate comments (100% coverage)
- All auto-generated function names resolved (0 `FUN_*` remaining)
- 128-byte LCP save file struct fully mapped (49 fields)
- 20+ custom enums applied (sprite_id, ACTION_ID, PLAYER_STATE, HOUSE_POS, WORD_ID, HEAD_ANIM_MODE, ST_COLOR, CLOTHING_COLOR_ID, SKIN_COLOR_ID, SICKNESS_LEVEL, HAPPINESS_LEVEL, NEED_LEVEL, MOOD_DIRECTION, PERSONALITY_TYPE, DOOR_STATE_FLAGS, DOG_BOWL_STATUS, MIDI_SEQ_PHASE, ENV_PHASE, SPRITE_LAYER, VDI_COPY_MODE, ST_RESOLUTION, VDI_FILL_STYLE, VDI_COLOR, SOUND_EFFECT_ID, CARD_TYPE)
- 6+ custom structs defined (LCP, PSG_ENVELOPE, MFDB, FILE_IMG_DATA, CCB, _iobuf/FILE, fcbtab)
- Alcyon C runtime library fully identified (~33 functions matched to original CP/M-68K source)
- Complete sound engine documented (24 MIDI sequencer + 4 PSG envelope + SFX functions)
- Complete sprite rendering pipeline documented with three-level architecture and corrected width/height naming
- sprite_id enum fully traced (56 values mapped to actual game usage via action function analysis)
- AI decision engine and action system fully analyzed (54 action/event handlers)
- Copy protection fully analyzed (13 functions, self-modifying XOR encryption, WD1772 FDC access)
- HOUSE.SCN decompressed and rendered with annotated object positions
- Dog AI and animation system fully documented (movement, stair navigation, eating, petting)
- Poker AI fully documented (30 functions with hand evaluation, bluffing, and draw strategy)
- Global variable naming conventions unified: `midi_*` for MIDI sequencer, `psg_*` for YM2149 PSG, `soundeffect_*` for DoSound effects, `sprite_def_*`/`sprite_pending_*`/`sprite_active_*` for the three-level sprite pipeline

### Known Remaining Issues
- ~2,210 unnamed global labels (mostly auto-generated `DAT_`, `LAB_`, `BYTE_` prefixes for intermediate data values, padding, and array elements that don't require meaningful names)
- 1 remaining `extraout_D0` decompiler artifact: `copyprot_fdc_read_status` — direct WD1772 FDC hardware register read via DMA port at 0xFF8604; D0 is set by memory-mapped I/O that Ghidra cannot model
- Some local variables in complex functions still have auto-generated names

### Tools Used
- **Ghidra 11** with MCP (Model Context Protocol) integration for interactive analysis
- **Alcyon C compiler source code** (from Digital Research CP/M-68K, uploaded as reference) for runtime library identification
- Original game running in **Hatari** emulator for behavioral verification

## Running the Game

1. Use an Atari ST emulator (Hatari, Steem) configured for **ST low resolution** (320×200, 16 colors)
2. Place `LCP.PRG` and all data files in a `data/` subdirectory on a virtual floppy or hard disk image
3. Launch `LCP.PRG` from the GEM desktop
4. The game will display an error alert if not in low resolution mode

## License

This is a reverse engineering documentation project. The original game is © 1985 Activision. All analysis is for educational and preservation purposes.
