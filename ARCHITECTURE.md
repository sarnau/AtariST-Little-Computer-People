# Architecture — Little Computer People (LCP.PRG)

This document describes the internal architecture of the Atari ST version of Little Computer People, based on reverse engineering of the `LCP.PRG` binary using Ghidra.

## Memory Layout

The program is organized into five memory segments:

| Segment       | Address Range            | Size      | Purpose                                  |
|---------------|--------------------------|-----------|------------------------------------------|
| `LOWMEM_VARS` | `0x000400` – `0x0005FF`  | 512 B     | Atari ST low-memory system variables      |
| `TEXT`         | `0x010000` – `0x0296DB`  | ~105 KB   | Executable code                           |
| `DATA`        | `0x0296DC` – `0x02C6BF`  | ~12 KB    | Initialized data (strings, tables, constants) |
| `BSS`         | `0x02C6C0` – `0x05A2F9`  | ~186 KB   | Uninitialized data (buffers, state)       |
| `IO`          | `0xFFFF0000` – `0xFFFFFDFF` | ~64 KB  | Hardware I/O register space               |

## High-Level Architecture

```
┌─────────────────────────────────────────────────────┐
│                    main()                           │
│  Init sound, VDI/AES, screen, load assets, state    │
└──────────────────────┬──────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────┐
│              endless_game_loop()                    │
│  ┌───────────────────────────────────────────────┐  │
│  │  game_tick_and_animate(0)                     │  │
│  │    ├── screen_render_8hz()                    │  │
│  │    ├── game_simulate_one_second()             │  │
│  │    ├── player_pathfind_one_step()             │  │
│  │    ├── dog_move_and_animate()                 │  │
│  │    ├── deal_with_keycode()                    │  │
│  │    └── player/dog sprite updates              │  │
│  ├───────────────────────────────────────────────┤  │
│  │  check_for_any_action_triggers()              │  │
│  │    ├── Process event queue (doorbell, etc.)   │  │
│  │    ├── Check immediate needs (bathroom, etc.) │  │
│  │    ├── Check scheduled actions (meals, sleep) │  │
│  │    ├── Process user command queue             │  │
│  │    └── Random AI action selection             │  │
│  └───────────────────────────────────────────────┘  │
│              ↻ loops forever                        │
└─────────────────────────────────────────────────────┘
```

## Subsystem Breakdown

### 1. Initialization (`main` — `0x15546`)

The `main` function performs a strict initialization sequence:

1. **Sound system** — `sound_init_timer()` configures Timer B for MIDI playback
2. **GEM setup** — `aes_vdi_jnit()` initializes the AES/VDI graphics subsystem
3. **System config** — enters Supervisor mode to modify `conterm` (keyboard repeat settings)
4. **Working directory** — sets the current path to `data/`
5. **Screen** — `vdi_init()` + `setup_screen_buffer()` allocates front/back screen buffers
6. **Bit table** — `init_build_bit_revert_table()` builds a lookup table for horizontal sprite flipping
7. **Music** — `count_songs()` scans for `*.sng` files on disk
8. **Save file** — `player_load()` attempts to load an existing game from `hyber`
9. **Title screen** — `show_title_screen_enter_name_and_date()` displays the intro and collects the player's name and date/time
10. **House scene** — loads and decompresses `house.scn` into screen memory
11. **Character data** — loads `body.lcp` and character-specific `pex.lcp` sprite files
12. **Objects and sprites** — loads 56 object graphics and 50 sprite frames with auto-generated masks
13. **Sound effects** — `soundeffects_load()` loads `sounds.lcp`
14. **Initial state** — sets door/furniture positions, food cabinet, dog bowl, clothing palette
15. **Copy protection** — `PROT_CHECK()` performs floppy disk verification
16. **Game loop** — enters `endless_game_loop()` (never returns)

### 2. Game Loop (`endless_game_loop` — `0x15C76`)

The core loop alternates between two functions in a tight infinite cycle:

**`game_tick_and_animate(0)`** handles the real-time simulation tick: rendering sprites at ~8 Hz, advancing the game clock, processing pathfinding, animating the character and dog, and polling keyboard input.

**`check_for_any_action_triggers()`** is the AI decision engine that selects what the resident does next (see section 4 below).

If the copy protection check failed, the loop instead calls `action_sleep(-1)` forever, softly disabling the game.

### 3. Simulation Engine (`game_simulate_one_second` — `0x233DA`)

Called every 8 animation frames (approximately once per game-second), this function manages all time-dependent state:

**Need timers** — Thirst and hunger each have a countdown timer. When a timer reaches zero, the corresponding need level increments (up to 3). If the level exceeds 3, the resident becomes sick via `player_become_sick()`.

**Sickness** — Sick characters have a sickness level that changes over time based on a direction value (+1 getting worse, -1 recovering). While sick, happiness is locked at level 2 (unhappy) and the character's palette shifts to show visible illness.

**Bathroom** — A dedicated timer counts down to trigger a bathroom need flag.

**Happiness** — A mood cycle oscillates the resident's happiness between 0 (happy) and 2 (unhappy) with configurable durations per level. Sickness overrides this to force unhappiness.

**Random events** — Phone calls have a 2% chance per game-second of occurring between 8 AM and 10 PM.

**Clock** — Seconds advance into minutes, minutes into hours, hours into days (with midnight triggering `daily_reset_action_flags()`), and days into months/years using a proper calendar with `days_in_month()`.

### 4. AI Decision Engine (`check_for_any_action_triggers` — `0x15CE2`)

This is the central behavior-selection system. It evaluates conditions in strict priority order:

| Priority | Condition                        | Action triggered              |
|----------|----------------------------------|-------------------------------|
| 1        | Event queue not empty            | Process event (doorbell, delivery, phone) |
| 2        | Ctrl+A alarm pressed             | `ACTION_WAKE_FROM_ALARM`      |
| 3        | Bathroom need flag set           | `ACTION_USE_TOILET`           |
| 4        | Thirst level > 0 (with random)  | `ACTION_DRINK`                |
| 5        | Hunger level > 0 (with random)  | `ACTION_KITCHEN_CABINET`      |
| 6        | Lunch hour reached               | `ACTION_EAT_MEAL`            |
| 7        | Dinner hour reached              | `ACTION_EAT_MEAL`            |
| 8        | Wake hour reached                | `ACTION_WAKE_UP_MORNING`     |
| 9        | Bedtime hour reached             | `ACTION_GO_TO_BED_NIGHT`     |
| 10       | User command in queue (high-pri) | Execute typed command         |
| 11       | None of the above                | Random personality-based action |

Thirst and hunger checks include a randomization factor influenced by sickness — when sick, the character is more likely to address needs immediately (the random threshold drops to 0%).

User-typed commands enter a priority queue with escalation. Low-priority commands may be deferred if needs are urgent, while high-priority commands (≥ 8) are executed immediately.

### 5. Action System (`do_action` — `0x16038`)

Each action is implemented as a dedicated function (approximately 80 action functions). Actions are triggered by the AI engine through the `trigger_action` global variable. The `do_action()` dispatcher calls the appropriate function. Actions typically:

1. Walk the character to a specific location via `player_walk_to_destination()`
2. Play an animation sequence by cycling sprite frames
3. Update game state (need levels, object positions, door states)
4. Play associated sound effects
5. Optionally modify the screen (draw/erase objects, update UI elements)

Key action categories and representative functions:

**Daily routine** — `action_wake_up_morning`, `action_go_to_bed_night`, `action_get_dressed`, `action_brush_teeth`, `action_take_shower`, `action_use_toilet`, `action_eat_meal`, `action_drink`

**Entertainment** — `action_play_piano`, `action_listen_song`, `action_play_with_record`, `action_toggle_tv`, `action_play_computer`, `action_read_newspaper`, `action_dance`, `action_play_a_game`, `action_sit_and_exercise`

**Housekeeping** — `action_tidy_house`, `action_clean_up`, `action_light_fireplace`, `action_write_letter`

**Interaction** — `action_hello`, `action_nod_head`, `action_pet_dog`, `action_call_dog`, `action_feed_dog`, `action_peek_around`

**Furniture** — `action_open_close_front_door`, `action_open_close_fridge`, `action_open_close_dresser`, `action_open_close_cabinet`, `action_open_close_bedroom_closet`, `action_open_close_upstairs_closet`, `action_open_close_filing_cabinet`, `action_close_toilet_door`

**Events** — `event_answer_phone`, `event_receive_food_delivery`, `event_receive_dog_food`, `event_receive_book_delivery`, `event_receive_record_delivery`, `action_check_front_door`

### 6. Command Parser (`check_entered_command` — `0x26F9A`)

When the player presses Enter (Ctrl+M), the typed text is processed through a keyword-matching system. The parser:

1. Converts input to uppercase via `strupper()`
2. Validates input with `check_valid_word_input()`
3. Matches against keyword tables using `parse_command_to_action()`

The keyword vocabulary is stored in the DATA segment as a series of word groups. Each group maps one or more synonyms to a specific action. Examples from the string data:

- "PLAY", "PERFORM", "PLAYING" → play-related actions
- "FIRE", "FIREPLACE", "LIGHT", "IGNITE", "BUILD" → light fireplace
- "PIANO", "ORGAN", "IVORIES" → play piano
- "DANCE", "MOON" → dance action
- "TEETH", "BRUSH", "FLOSS", "HYGIENE" → brush teeth
- "DRINK", "IMBIBE", "WATER", "LIQUID", "GLASS" → drink water
- "COMPUTER", "ATARI", "PROGRAM" → use computer

### 7. Graphics System

#### Screen Management

The game uses a double-buffered rendering approach with GEM VDI:

- `screen_set_draw_to_backbuffer()` / `screen_set_draw_to_frontbuffer()` toggle the active drawing target
- `copy_screen()` copies between buffers
- `screen_render_8hz()` handles the ~8 Hz display refresh cycle
- `vdi_copy_rect()` and `vro_cpyfm()` perform blit operations using VDI's MFDB (Memory Form Definition Block) structures

#### Sprites

Sprites are loaded as raw planar bitmap data (Atari ST format: 4 interleaved bit-planes per 16-pixel word). Each sprite goes through `sprite_create_with_mask()` which auto-generates transparency masks. The sprite system supports:

- Horizontal flipping via `sprite_flip_horizontal()` (using the precomputed bit-reversal table)
- Masked drawing via `screen_draw_sprite_with_mask()`
- Separate head and body sprite composition for the character (`player_build_head_sprites`, `player_build_body_sprites`)
- Carried-object overlay sprites (`sprite_activate_carried_object_left/right`)

#### Objects

56 static object images are loaded from the `objects` file. Each object has width, height, and bitmap data stored in `FILE_IMG_DATA` structures (6-byte header: width, height, then pixel data). Objects are drawn via `screen_draw_object()` for furniture, doors, food bowls, etc.

### 8. Pathfinding System

The character navigates a multi-floor house using a waypoint-based pathfinding system:

- `house_get_position_xy()` maps logical positions (room/spot IDs) to pixel coordinates
- `player_calc_floor_waypoint()` determines the next waypoint on a path
- `get_floor_number_from_y()` identifies which floor a Y-coordinate falls on
- `player_pathfind_one_step()` advances the character one step along the calculated path each tick
- `dog_calc_walk_path()` / `dog_move_and_animate()` handle independent dog pathfinding

The house consists of multiple floors connected by stairs. The character must navigate horizontally within a floor and vertically between floors to reach target positions.

### 9. Sound System

#### Music Engine (`0x1012A` – `0x12284`)

The music engine is Timer B interrupt-driven and supports both PSG (YM2149) and MIDI output:

- `sound_init_timer()` / `sound_exit_timer()` — set up and tear down Timer B interrupts
- `sound_play_song()` — loads and starts a song from `*.sng` files
- `sound_parse_song_header_events()` — parses the custom song format header
- `sound_parse_channel_config()` / `sound_parse_volume_config()` — configure per-channel instrument and volume settings
- `sound_process_next_midi_event()` — processes the next event in the song data stream
- `sound_midi_playback_loop()` — main playback state machine in the Timer B callback
- `sound_midi_note_to_psg_freq()` — converts MIDI note numbers to PSG frequency register values
- `sound_MIDI_send_note_on()` / `sound_send_MIDI_event()` — output to the MIDI port
- `sound_set_psg_channel_volume()` — directly writes to YM2149 registers via `sound_register_write()`

#### Sound Effects (`0x1D9EA` – `0x1DE36`)

- `soundeffects_load()` — loads sample data from `sounds.lcp`
- `soundeffect_select()` — selects a sound effect by index
- `soundeffect_irq_play()` — plays the selected effect using interrupt-driven sample output

Common effects include `play_soundeffect_tv_click`, `play_soundeffect_speech`, `play_soundeffect_head_nod`, `play_soundeffect_greeting`, `play_doorbell_sound`, and `select_random_click_sound` (typewriter).

### 10. Mini-Game Subsystem

Each of the five mini-games is self-contained with its own game loop, graphics, and input handling. They are launched from a common menu (`action_play_a_game`).

**Shared infrastructure:**
- `minigame_setup_screen()` — prepares the mini-game display area
- `minigame_wait_for_key_with_events()` — waits for a keypress while continuing the background simulation (the resident still animates during games)
- `play_erase_rect()` / `draw_bar_color()` — mini-game drawing primitives
- `init_vdi_and_screen()` / `exit_vdi_and_screen()` — save/restore the main game screen when entering/leaving games

**Poker** (`poker_main` — `0x18D10`) has the most complex implementation with ~30 dedicated functions covering: deck shuffling, dealing, betting rounds with AI bluffing decisions, hand evaluation and ranking, card drawing with discard management, and full showdown logic.

**Anagram** (`anagram_main` — `0x181AE`) loads a dictionary of 150 words, scrambles a random selection, and tracks up to 9 guesses with an optional clue system that progressively unscrambles letters.

**Word Puzzles** (`word_puzzle_main` — `0x176F8`) loads template definitions from `wordpz.txt` and presents crossword-style fill-in puzzles.

### 11. Persistence (`player_save` / `player_load`)

The game state is serialized to and from the `hyber` file. The save data includes the `resident` structure containing: physical traits (sprite ID, clothing/skin colors), personality parameters (happiness direction, durations), need states (hunger, thirst, bathroom timers and levels), sickness state, scheduled hours (wake, lunch, dinner, bedtime), door/furniture states (packed into bitfields), and food supply level.

### 12. Copy Protection (`PROT_CHECK` — `0x122C0`)

The protection system directly accesses the WD1772 Floppy Disk Controller hardware registers at `0xFF8604`/`0xFF8606`. It:

1. Selects drive A via `PROT_SELECT_DRIVE()`
2. Seeks to a specific track
3. Reads raw track data via `PROT_FDC_READ_TRACK()`
4. Decrypts verification data with `PROT_DECRYPT()` using an XOR-based scheme
5. Compares decrypted values against expected constants (`PROT_DATA_WORD_A` through `PROT_DATA_WORD_D`)

The result is stored in `copyprot_check_return` and checked in `endless_game_loop()`.

### 13. Runtime Library

The binary includes a statically linked C runtime (from the CP/M-68K compiler toolchain by Digital Research). This covers standard library functions: `sprintf`, `strlen`, `strcpy`, `strcat`, `strcmpi`, `toupper`, `alloc`, `strtol`, printf formatting, file I/O wrappers (`read`, `write`, `runtime_fseek`), and process management (`exit`, `runtime_init`). The runtime initializes via `runtime_init()` which configures the heap, parses the command line, and sets up file descriptor tables before calling `main()`.

## Data Structures

Key custom types defined in the Ghidra analysis:

| Type              | Size    | Purpose                                                |
|-------------------|---------|--------------------------------------------------------|
| `MFDB`            | 20 B    | GEM Memory Form Definition Block (bitmap descriptor)    |
| `AESPB`           | 24 B    | GEM AES Parameter Block                                 |
| `FILE_IMG_DATA`   | 6 B+    | Image file header (width, height, then planar data)     |
| `CARD_TYPE`       | 2 B     | Card value for poker/blackjack/war                      |
| `MIDI_Note_Struct`| varies  | MIDI event data for the music engine                    |
| `DTA`             | 44 B    | GEMDOS Disk Transfer Address (file search)              |
| `action_id`       | 2 B     | Enumeration of all possible character actions            |
| `color_enum`      | 2 B     | VDI color index enumeration                             |
| `keycode_enum`    | 2 B     | Keyboard scancode/ASCII mapping                         |
| `BIOS_FUNCTION`   | 2 B     | BIOS trap function numbers                              |
| `GEMDOS_FUNCTION` | 2 B     | GEMDOS trap function numbers                            |

## Function Organization by Address

The code is laid out roughly by subsystem in the TEXT segment:

| Address Range              | Subsystem                           | ~Count |
|----------------------------|-------------------------------------|--------|
| `0x1006E` – `0x100C4`     | Runtime / startup                   | 5      |
| `0x100FA` – `0x1011A`     | OS trap wrappers (XBIOS, BIOS, GEMDOS) | 3  |
| `0x1012A` – `0x12284`     | Sound / music engine                | 35     |
| `0x122C0` – `0x125F4`     | Copy protection                     | 18     |
| `0x1400C` – `0x15BDC`     | Sprites, pathfinding, file loading, main | 15 |
| `0x15C76` – `0x163CC`     | Game loop, AI engine, events        | 10     |
| `0x163CC` – `0x176F8`     | VDI wrappers, screen, UI, input     | 30     |
| `0x176F8` – `0x186E0`     | Mini-games: word puzzle, anagram    | 20     |
| `0x186E0` – `0x1D9EA`     | Mini-games: poker, blackjack, war   | 45     |
| `0x1D9EA` – `0x1E338`     | Sound effects, mouse, game-table    | 10     |
| `0x1E338` – `0x247A0`     | Action functions (80+)              | 80     |
| `0x247A0` – `0x26D5A`     | Save/load, sprite update, rendering | 15     |
| `0x26D5A` – `0x27310`     | Text scrolling, printing, command parser | 12 |
| `0x27310` – `0x27968`     | VDI/AES bindings                    | 20     |
| `0x27968` – `0x2969A`     | C runtime library                   | 60     |
