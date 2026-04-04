# Little Computer People — Atari ST Reverse Engineering

**LCP.PRG** is the main executable for *Little Computer People* on the Atari ST, a life-simulation game originally developed by Activision (Rich Gold, David Crane) and released in 1985. The player observes and interacts with a virtual character — the "Little Computer Person" — who lives inside a three-story house displayed on screen.

This repository documents a comprehensive reverse engineering effort of the Atari ST binary using Ghidra with MCP integration.

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
| Code size | ~170 KB TEXT segment (0x10000–0x296DB) |
| Total functions | 397 identified and named |
| Total symbols | 3,516 labeled |
| Save file | `hyber`, 128 bytes (LCP struct) |

### Memory Layout

| Segment | Address Range | Size | Contents |
|---|---|---|---|
| LOWMEM_VARS | 0x000400–0x0005FF | 512 B | Atari ST system variables |
| TEXT | 0x010000–0x0296DB | 104 KB | Executable code |
| DATA | 0x0296DC–0x02C6BF | 12 KB | Initialized data (tables, strings) |
| BSS | 0x02C6C0–0x05A2F9 | 183 KB | Uninitialized data (buffers, state) |
| IO | 0xFFFF0000–0xFFFFFDFF | — | Hardware registers (PSG, DMA, MFP, ACIA) |

## The LCP Character

Each character is defined by a 128-byte `LCP` struct that is saved to and loaded from the `hyber` file. Key attributes:

### Appearance
- **Clothing color** (16 outfit combinations, e.g. `OUTFIT_BLUE_GREEN`, `OUTFIT_RED_PINK`)
- **Skin color** (8 options including realistic and fantasy tones)
- **Character sprite set** (5 visual variants, PE2–PE6.LCP files)
- **Character name** (randomly selected from a pool of 266 names)

### Personality & Schedule
- **Personality type** (0–3, affects behavior weighting)
- **Activity level** (0–7, controls action table selection via `activity_schedule_table[3][8]`)
- **Daily schedule**: configurable wake hour (6–8am), bedtime (10pm–midnight), lunch (11am–1pm), dinner (5–7pm)
- **Initiative threshold** (20–80, lower = more proactive about autonomous actions)

### Needs System
Three basic needs drive urgent behavior:
- **Thirst**: timer decrements each game-minute; at 0, `thirst_level` increments (0→1→2→3→sick)
- **Hunger**: same mechanic with separate timer; triggers eating or sickness
- **Bathroom**: timer-based; sets `bathroom_need` flag, prioritized in the AI decision engine

### Mood & Health
- **Happiness**: cycles between `MOOD_HAPPY` (0), `MOOD_CONTENT` (1), and `MOOD_SAD` (2) over configurable hourly durations
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

The player types natural-language phrases that are parsed against a 161-word vocabulary (`WORD_ID` enum). A word-to-action mapping table (`enteredword_to_action`) converts recognized word bitmask patterns into game actions with priority offsets. Examples:

- "play piano" → `ACTION_PLAY_PIANO`
- "light fire" / "build fire" → `ACTION_LIGHT_FIREPLACE`
- "feed dog" / "fill bowl" → `ACTION_FEED_DOG`
- "write letter" / "write note" → `ACTION_WRITE_LETTER`
- "dance" / "boogie" → `ACTION_DANCE`
- "play computer" / "play atari" → `ACTION_PLAY_COMPUTER`

### Keyboard Shortcuts

| Key | Action | Effect |
|---|---|---|
| Ctrl+A | Alarm clock | Wakes the character, rings alarm sound |
| Ctrl+B | Book delivery | Doorbell + book package at front door |
| Ctrl+C | Phone call | Random phone rings, character answers |
| Ctrl+D | Dog food | Dog food delivery at front door |
| Ctrl+F | Food delivery | Food package delivery |
| Ctrl+R | Record delivery | New vinyl record delivery |
| Ctrl+W | Water delivery | Refills water supply |
| Ctrl+P | Pet the dog | Dog petting animation sequence |

## Mini-Games

Five card and word games are available, played at the kitchen table:

- **Anagrams** — Unscramble a word within 9 guesses; F1 reveals a letter clue
- **War** — Classic high-card game
- **Poker** — 5-card draw with betting, raising, and AI bluffing
- **Blackjack** — Casino rules with full hand evaluation
- **Word Puzzles** — Crossword-style puzzles from external template files

## House Layout

The three-story house has 48 navigable positions (`HOUSE_POS` enum) organized by floor:

### Top Floor (Living Room / Study) — Positions 0–15
Armchair, game table, record shelf, fireplace (with log area), study door, filing cabinet, desk

### Middle Floor (Bedroom / Bathroom) — Positions 16–31
Bed, dresser, bedroom closet, couch, bathroom sink, toilet, shower, computer desk, piano

### Bottom Floor (Kitchen / Entrance) — Positions 32–47
Kitchen sink, stove, fridge, food cabinet, kitchen table, dog bowl, front door

The character navigates between positions using `lcp_pathfind_one_step`, which handles flat walking (8-frame cycle), stair climbing/descending (4-frame cycles), and waypoint-based routing.

## Graphics System

### Double-Buffered Sprite Rendering

The game runs at 8 Hz with a double-buffered rendering pipeline in `screen_render_8hz`:

1. **Background copy**: `blkcopy32` copies the static house scene (32-byte aligned block copy)
2. **Dog animation**: `dog_move_and_animate` advances the dog's position and animation
3. **Sprite compositing**: 8 hardware sprite slots composited via masked blitting
4. **Page flip**: `XBIOS Setscreen` swaps the display buffer

### Two-Level Sprite System

**Static objects** (`screen_draw_object`): 56 background elements (doors, furniture, food, fire frames) drawn with direct VDI `vro_cpyfm` in `S_ONLY` mode — no transparency.

**Overlay sprites** (`screen_draw_sprite_with_mask`): 60 sprite definitions multiplexed onto 8 hardware rendering slots via `lcp_update_sprite_slots`:
- Slots 3–4: LCP body and head (always present)
- Slots 1–2: Behind-LCP layer (`SPRITE_BEHIND_LCP`)
- Slots 5–6: In-front-of-LCP layer (`SPRITE_IN_FRONT`)

Compositing uses the classic Atari ST masked blit: `NOT_S AND D` (punch transparent hole) then `S XOR D` (paint sprite).

### LCP Character Sprites

The character is assembled from separate body and head sprite sheets:
- **Body** (`body.lcp`): 91 animation states (`PLAYER_STATE` enum) — walk cycle, stairs, sitting, showering, dancing, etc.
- **Head** (`pex.lcp`): Expression frames selected by `head_sprite_frame` and `happiness` level, with random head movements controlled by `HEAD_ANIM_MODE`

Both are 2-word-wide source sprites expanded to 4-word-wide compositing buffers by `lcp_flip_sprite_horizontal`, with bit-reversal via `revert_table[256]` for horizontal mirroring.

### Color Palette

16-color Atari ST palette using `ST_COLOR` format (0x0RGB, 3 bits per channel). The game uses custom palettes with dynamic clothing/skin color swapping:

- `main_colorpalette[16]`: Active hardware palette
- `clothing_color_primary/secondary[16]`: 16 outfit color combinations mapped to palette entries 1–2
- `skin_color_palette[8]`: 8 skin tones
- Palette entry 6: `ST_PEACH` (healthy) or `ST_SICK_GREEN` (sick)

## Sound System

### MIDI Sequencer Engine

A custom MIDI-like sequencer (`midi_seq_*` functions) plays `.sng` song files with dual output:

**External MIDI** (`midi_output_enabled`): Standard MIDI messages via the Atari ST ACIA at 0xFFFC04, supporting program changes, channel remapping, and octave transposition.

**Internal PSG** (`psg_output_enabled`): YM2149 sound chip with 3 channels, software ADSR envelope processing (`PSG_ENVELOPE` struct with `ENV_ATTACK`→`ENV_DECAY`→`ENV_SUSTAIN`→`ENV_RELEASE`→`ENV_FADEOUT` phases), Bresenham-style volume interpolation, and voice stealing.

Timing is interrupt-driven via MFP Timer A at 200 Hz (`midi_seq_tick_handler`), with the sequencer state machine (`MIDI_SEQ_PHASE`) managing event parsing and note expiration.

### .SNG File Format

| Offset | Content |
|---|---|
| 0–9 | File header (skipped) |
| 10–... | Channel/program mapping (30 bytes) + ADSR envelope data (360 bytes) |
| 0x1FE+ | MIDI event stream (compact 3-byte note format + control events) |

### Sound Effects

Separate from the music engine, a sound effect system (`soundeffect_*`) plays ambient sounds: footsteps (carpet, wood, stairs), doorbell, door open/close, water running, toilet flush, phone ring, typewriter clicks, speech, snoring, alarm clock, and more.

## Data Files

| File | Purpose |
|---|---|
| `house.scn` | Compressed house background scene (320×200, 4-bitplane) |
| `title.scn` | Title screen scene |
| `body.lcp` | Body sprite sheet (91 states × 21 scanlines × 2 words) |
| `PE*.lcp` | Head sprite sheets (per-character expressions + happiness variants) |
| `objects` | 56 static object graphics with MFDBs |
| `sprites` | 60 overlay sprite definitions |
| `sounds.lcp` | Sound effect PCM sample data |
| `cards` | Card game graphics (52 cards + backs) |
| `words` | Anagram dictionary (150 words) |
| `wordpz.txt` | Word puzzle templates |
| `names` | Character name pool (266 × 10 bytes) |
| `letter.txt` | Letter writing templates (4 categories × happiness variants) |
| `hyber` | Save file (128-byte LCP struct) |
| `*.sng` | Music song files (custom MIDI format) |
| `*.org` | Organ music files for piano playback |

## Copy Protection

The binary includes floppy-disk-based copy protection (`PROT_CHECK`). It reads specific tracks via direct FDC (Floppy Disk Controller) hardware access at 0xFF8604, decrypts verification data using DMA transfers, and compares checksums. If the check fails, the game enters an infinite `action_sleep(-1)` loop — the character permanently sleeps and never performs autonomous actions.

## Reverse Engineering Status

### Completed
- All 397 functions identified, named, and documented with plate comments
- All auto-generated variable names resolved (0 remaining)
- 128-byte LCP save file struct fully mapped (49 fields)
- 20+ custom enums applied (ACTION_ID, PLAYER_STATE, HOUSE_POS, WORD_ID, HEAD_ANIM_MODE, ST_COLOR, CLOTHING_COLOR_ID, SKIN_COLOR_ID, SICKNESS_LEVEL, HAPPINESS_LEVEL, NEED_LEVEL, MOOD_DIRECTION, PERSONALITY_TYPE, DOOR_STATE_FLAGS, DOG_BOWL_STATUS, MIDI_SEQ_PHASE, ENV_PHASE, SPRITE_LAYER, VDI_COPY_MODE, ST_RESOLUTION, VDI_FILL_STYLE, VDI_COLOR, etc.)
- 5+ custom structs defined (LCP, PSG_ENVELOPE, CCB, _iobuf/FILE, fcbtab, WORD2ACTION)
- Alcyon C runtime library fully identified (~60 functions matched to original source code)
- Complete sound engine documented (MIDI sequencer + PSG envelope + SFX system)
- Complete sprite rendering pipeline documented
- AI decision engine and action system fully analyzed

### Tools Used
- **Ghidra** with MCP (Model Context Protocol) integration for interactive analysis
- **Alcyon C compiler source code** (from Digital Research CP/M-68K) for runtime library identification
- Original game running in **Hatari** emulator for behavioral verification

## Running the Game

1. Use an Atari ST emulator (Hatari, Steem) configured for **ST low resolution** (320×200, 16 colors)
2. Place `LCP.PRG` and all data files on a virtual floppy or hard disk image
3. Launch `LCP.PRG` from the GEM desktop
4. The game will display an error alert if not in low resolution mode

## License

This is a reverse engineering documentation project. The original game is © 1985 Activision. All analysis is for educational and preservation purposes.
