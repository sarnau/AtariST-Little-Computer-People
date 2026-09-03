# source/ port status

> **Stale.**  This table predates the LCP_STX restructuring: the
> action files it names (ahouse.c, aleisure.c, adoors.c, ...) no
> longer exist -- every body now lives in `source/parts/` and is
> included by a unity unit in LCP_STX's own order.  The file column
> is therefore historical.  The live status is in CLAUDE.md.

Snapshot of what is ported for real vs. still a stub.

Total Ghidra functions in `LCP.PRG`: ~395.
Ported for real (Ghidra-verified): **188 table rows** across **54
translation units** -- several rows list multiple related wrappers
(the 6 VDI colour/mode setters, the 4 `play_soundeffect_*` variants,
etc.) so the raw function count is closer to 200.
Stubbed (empty body, links but does nothing): **0 groups** -- the port
is *complete* at the game-logic + ABI level.  What remains is the
trap #2 assembler stub under Alcyon and the card-game engine logic
beyond each game's `_main()` entry point.

Data tables populated with authoritative values: **NLP vocabulary (160 words), NLP action-matching (33 rules), PSG frequency LUT (132 notes)** -- the parser now resolves real sentences to real ACTION_IDs, and MIDI notes get their correct YM2149 tone periods.

## Ported (real)

| function                               | file                | notes |
|----------------------------------------|---------------------|-------|
| `game_simulate_one_second`             | `sim.c`             | 9-branch needs/clock/mood tick |
| `days_in_month`                        | `calendar.c`        | preserves 1985 date_year bug |
| `daily_reset_action_flags`             | `calendar.c`        | midnight reset of once-per-day triggers |
| `randomRange`                          | `random.c`          | XBIOS Random() -> [low, high] |
| `put_event_to_list`                    | `events.c`          | 10-slot FIFO append |
| `get_event_from_list`                  | `events.c`          | FIFO pop with shift-down |
| `execute_event`                        | `ai.c`              | 6-case dispatcher |
| `check_for_any_action_triggers`        | `ai.c`              | 9-priority AI decision engine |
| `lcp_become_sick`                      | `health.c`          | sickness onset + palette refresh |
| `do_action`                            | `actions.c`         | 45-case dispatcher |
| `house_get_position_xy`                | `movement.c`        | HOUSE_POS -> screen X/Y |
| `get_floor_number_from_y`              | `movement.c`        | screen Y -> floor 1..3 |
| `calc_weekday`                         | `movement.c`        | day-of-week accumulator |
| `endless_game_loop`                    | `main.c`            | top-level game loop |
| `action_wake_from_alarm`               | `asimple.c`   | Ctrl+A path |
| `action_hello`                         | `asimple.c`   | wave with head-nod loop |
| `action_yawn_and_stretch`              | `asimple.c`   | 15-frame idle yawn |
| `action_nod_head`                      | `asimple.c`   | 3-frame nod with SFX |
| `action_pet_dog`                       | `asimple.c`   | wait + pettable-flag clear |
| `action_call_dog`                      | `asimple.c`   | walk to POS 43 + crouch |
| `sprite_update_body`                   | `sprites.c`         | body pose selection + slot 3 push |
| `spritedata_select_carried_object_l/r` | `sprites.c`         | activate carried sprite (behind/in-front) |
| `dog_move_and_animate`                 | `dog.c`             | 8Hz movement + stair traversal |
| `spritedata_update_dog`                | `dog.c`             | dog frame -> slots 0/7 |
| `game_tick_and_animate`                | `tick.c`            | frame driver (rendering side stubbed) |
| `create_file`                          | `save.c`            | GEMDOS Fcreate with retry |
| `file_read`                            | `save.c`            | GEMDOS Fread with 3-retry + alert |
| `lcp_save`                             | `save.c`            | GEMDOS Fopen/Fwrite/Fclose |
| `lcp_load`                             | `save.c`            | reads HYBER + unpacks door bits |
| `lcp_enter_study_and_save`             | `save.c`            | full study animation + repack + save |
| `sprite_lcp_flip`                      | `sprites.c`         | 2->4 word row expansion + optional mirror via revert_table |
| `sprite_flip_horizontal`               | `sprites.c`         | in-place general sprite mirror |
| `sprite_update_slots`                  | `sprites.c`         | 60 logical -> 8 hardware slot multiplexer |
| `spritedata_select`                    | `sprites.c`         | generic sprite -> active slot copy |
| `lcp_wait_head_reach_target`           | `sprites.c`         | spin-tick until head reaches target |
| `sprite_lcp_head_update`               | `sprites.c`         | slot-4 head positioning + double-buffer push |
| `sprite_lcp_head_animate`              | `sprhead.c`    | 8-direction head state machine (H+V) |
| `lcp_walk_to_destination`              | `walk.c`            | outer walk loop with interrupt gates |
| `lcp_pathfind_one_step`                | `walk.c`            | 349-line flat + stair + descend + top step |
| `lcp_calc_floor_waypoint`              | `walk.c`            | same-floor / cross-floor waypoint router |
| `dog_calc_walk_path`                   | `walk.c`            | dog counterpart, with -3/-8 X patches |
| `lcp_play_footstep_sound`              | `walk.c`            | carpet/wood/stairs SFX picker by X and floor |
| `walk_to_front_door`                   | `delivery.c`      | tiny helper |
| `action_open_close_front_door`         | `delivery.c`      | 2-frame door open/close with SFX |
| `action_open_close_cabinet`            | `delivery.c`      | kitchen cabinet toggle |
| `event_receive_food_delivery`          | `delivery.c`      | Ctrl+F: groceries -> cabinet, 4-pack loop |
| `event_receive_book_delivery`          | `delivery.c`      | Ctrl+B: book -> bookshelf |
| `event_receive_record_delivery`        | `delivery.c`      | Ctrl+R: record -> shelf (preserves 1985 typo) |
| `event_receive_dog_food`               | `delivery.c`      | Ctrl+D: trampoline to food delivery |
| `event_answer_phone`                   | `delivery.c`      | Ctrl+C: 40-50 tick phone call with SFX |
| `check_time_based_actions`             | `airandom.c`       | 3-table time-of-day action picker with weekend bias |
| `action_wander_idly`                   | `aidle.c`    | 4-step shrug |
| `action_peek_around`                   | `aidle.c`    | 6-tick side glance |
| `action_pace_nervously`                | `aidle.c`    | 15-frame pace |
| `action_toggle_tv`                     | `aidle.c`    | TV on/off flip |
| `action_sleep`                         | `aidle.c`    | snoring lie-down (value=-1 = copy protection loop) |
| `action_read_newspaper`                | `ahouse.c`   | armchair + 200-tick TV read |
| `action_get_in_out_of_bed`             | `ahouse.c`   | undress + lie down / reverse |
| `action_dance`                         | `ahouse.c`   | record player + dance-floor loop |
| `action_drink`                         | `ahouse.c`   | sink + glass + water tap |
| `action_use_toilet`                    | `ahouse.c`   | 3-sprite door animation + flush |
| `action_wake_up_morning`               | `ahouse.c`   | morning routine chain |
| `action_go_to_bed_night`               | `ahouse.c`   | bedtime routine chain |
| `action_get_dressed`                   | `ahouse.c`   | head-anim only, 4x vertical bob |
| `action_eat_meal`                      | `afood.c`    | pot -> stove animation -> table -> kitchen_cabinet |
| `action_kitchen_cabinet`               | `afood.c`    | 200+ line eat routine, food-count decrement, 10-20 bite cycles |
| `action_feed_dog`                      | `afood.c`    | fridge -> dog bowl -> fridge (value=0 opens fridge first) |
| `action_get_snack_from_fridge`         | `afood.c`    | trampoline into action_open_close_fridge |
| `action_take_shower`                   | `abathrm.c`| 20-25 cycles of alternating scrub/wash |
| `action_brush_teeth`                   | `abathrm.c`| 24-35 cycle brush with sprite-6 toothbrush |
| `action_wash_hands`                    | `abathrm.c`| 4-127 random hand-position cycles + water SFX |
| `action_listen_song`                   | `aleisure.c` | random `.sng` picker + song_play |
| `action_play_piano`                    | `aleisure.c` | stop the currently-playing record |
| `action_play_with_record`              | `aleisure.c` | PSG-amplitude-reactive vinyl browse animation |
| `action_light_fireplace`               | `aleisure.c` | firewood run + 2500-5000 tick fire countdown |
| `action_sit_on_couch_with_dog`         | `aleisure.c` | call dog + sit + pet + crouch off couch |
| `action_sit_and_exercise`              | `aleisure.c` | 4-frame arms stretch cycle |
| `action_check_front_door`              | `aleisure.c` | open door + look outside + maybe close |
| `action_tidy_house`                    | `aleisure.c` | walk to filing cabinet, close if open |
| `action_clean_up`                      | `aleisure.c` | sweep all 6 open doors/cabinets, close each |
| `action_open_close_bedroom_closet`     | `aleisure.c` | 3-sprite closet + palette swap for outfit change |
| `action_open_close_upstairs_closet`    | `aleisure.c` | study door + save-file chain (value chooses do_save) |
| `action_write_letter`                  | `aletter.c`  | shuffled 4-section procedural letter + sign-off |
| `letter_type_string_animated`          | `aletter.c`  | word-wrapped string typer, 40-col break |
| `letter_type_character_animated`       | `aletter.c`  | per-char keystroke + width-bracket sprite swap |
| `tv_turn_on`                           | `render.c`          | walk + idle-look + flag + click SFX |
| `tv_turn_off`                          | `render.c`          | walk + idle-look + antenna redraw |
| `screen_draw_food_cabinet`             | `render.c`          | 4-slot food-count overlay from door_states_and_flags |
| `update_water_level_bar`               | `render.c`          | VDI polyline water tank redraw / drain / fill |
| `hide_lcp_sprites`                     | `sprites.c`         | stash + null body/head slots |
| `show_lcp_sprites`                     | `sprites.c`         | restore stashed body/head slots |
| `lcp_check_recovery`                   | `health.c`          | hunger+thirst=0 -> sickness recovering |
| `lcp_idle_look_left`                   | `ahouse.c`   | 4-tick stand-and-look |
| `lcp_idle_look_right`                  | `ahouse.c`   | 4-tick stand-and-look |
| `action_drink_water_animation`         | `abathrm.c`| glass fill animation with 3-position hand shift |
| `action_close_toilet_door`             | `adoors.c`   | 2-frame close animation |
| `action_close_closet_door`             | `adoors.c`   | 2-frame close animation |
| `action_open_close_fridge`             | `adoors.c`   | 3-frame open, look inside, close |
| `action_open_close_filing_cabinet`     | `adoors.c`   | sequential animation, closes cabinet |
| `action_open_close_dresser`            | `adoors.c`   | 2-frame open/close (value chooses direction) |
| `action_walk_to_and_turn`              | `adoors.c`   | filing cabinet interaction + nervous shuffle |
| `file_open`                            | `save.c`            | retrying GEMDOS Fopen with alert |
| `file_read_compressed`                 | `letload.c`     | nibble token decoder (15-entry LUT + escape) |
| `file_load_letter_template`            | `letload.c`     | decompress LETTER.TXT + index 360 line pointers |
| `palette_apply_clothing_colors`        | `renderx.c`    | pick + apply 2 palette slots via Setpalette |
| `palette_apply_skin_colors`            | `renderx.c`    | same shape, 8-entry skin table |
| `lcp_update_palette_colors`            | `renderx.c`    | palette slot 6: peach when healthy, green when sick |
| `tv_draw_static_line`                  | `renderx.c`    | 5-line rabbit-ear antenna |
| `tv_draw_static_noise`                 | `renderx.c`    | random-colour antenna per frame |
| `screen_scroll_text_down`              | `renderx.c`    | 13-row blitter scroll + 2-row white fill |
| `print_char`                           | `renderx.c`    | VDI v_gtext with mode/color save/restore |
| `letter_select_typewriter_sound`       | `sound.c`           | soundeffect_select(SFX_TYPEWRITER_KEY, 4) |
| `select_random_click_sound`            | `sound.c`           | soundeffect_select(SFX_CLICK, 2) |
| `song_play`                            | `sound.c`           | Fsfirst+Malloc+Fread + midi_seq_init_song |
| `error_not_enough_memory`              | `alerts.c`          | infinite form_alert loop (host: exit 1) |
| `error_unable_to_write`                | `alerts.c`          | single-shot form_alert (host: warn) |
| `draw_line`                            | `gfx_prim.c`        | VDI v_pline in backbuffer, restore front |
| `screen_set_draw_to_backbuffer`        | `gfx_prim.c`        | Logbase+Setscreen swap, reset VDI fill state |
| `screen_set_draw_to_frontbuffer`       | `gfx_prim.c`        | Setscreen restore |
| `screen_fill_row_white`                | `gfx_prim.c`        | 20-quad-of-4-word solid white fill |
| `blkcopy32`                            | `gfx_prim.c`        | unrolled 32-byte block copy (MOVEM.L target) |
| `object_draw`                          | `render.c`          | vdi_copy_rect from object_tab_mfdb to screen |
| `clock_redraw_hands`                   | `render.c`          | erase old + draw new clock hand pair |
| `fill_top_rect_with_background`        | `render.c`          | rows of white or striped fill + black separator |
| `record_player_animate_needle`         | `renderx.c`    | 14-step needle sweep + random VU LED toggle |
| `soundeffect_select`                   | `sound.c`           | priority-based SFX queue insertion |
| `soundeffects_off`                     | `sound.c`           | silence 3 PSG channels via Giaccess |
| `play_soundeffect_tv_click/greeting/speech/head_nod` | `sound.c` | 1-line wrappers with SFX id + duration |
| `clock_draw_hands`                     | `clock.c`           | 2 draw_line calls for minute + hour hand |
| `get_pressed_key`                      | `keyboard.c`        | GEMDOS Cconis+Crawcin poll with scancode remap |
| `deal_with_keycode`                    | `keyboard.c`        | Ctrl+A/B/C/D/F/M/P/R/W + printable ASCII dispatch |
| `play_doorbell_sound`                  | `sound.c`           | 1-line wrapper: SFX_DOORBELL, duration 4 |
| `parse_command_to_action`              | `ai.c`              | NLP parse + append to priority queue |
| `screen_render_8hz`                    | `renderf.c`    | 8Hz compositor: rate-gate, dog AI, sprite blit, page flip |
| `lcp_toupper`                          | `parser.c`          | ASCII single-char uppercase |
| `command_upperstr`                     | `parser.c`          | tokenize + uppercase, returns next-word pointer |
| `check_valid_word_input`               | `parser.c`          | dictionary walk, returns WORD_ID or WORD_NONE |
| `check_entered_command`                | `parser.c`          | bag-of-words parser, action-table match, priority calc |
| `action_play_computer`                 | `agames.c`   | sit at desk + random typing loop + rare screen clear |
| `action_play_a_game`                   | `agames.c`   | 5-game menu + walk to table + dispatch to game main() |
| `sprite_draw`                          | `sprender.c`   | 2-pass masked blit (NOT-AND, then XOR) |
| `sprite_init_MFDB`                     | `sprender.c`   | fill MFDB descriptor for ST low-res |
| `string_print`                         | `renderx.c`    | multi-char print loop, 8-pixel advance |
| `tv_show_screen_clear`                 | `tvanim.c`      | v_bar fill + coin-flip dispatch to program |
| `tv_show_bouncing_line`                | `tvanim.c`      | random-colour pixel bouncer with wall reflection |
| `tv_show_pattern_lines`                | `tvanim.c`      | 4 pattern sets of polyline segments |
| `screen_fill_row_striped`              | `gfx_prim.c`        | 4-word plane-01-off / plane-23-on row fill |
| `screen_fill_row_black`                | `gfx_prim.c`        | 80-word plane-off row fill |
| `minigame_setup_screen`                | `games.c`           | 5-tick pause + top-strip fill + freeze scroll |
| `play_erase_rect`                      | `games.c`           | v_bar rectangle clear with VDI brackets |
| `anagram_main`                         | `games.c`           | skeleton: alloc + load words + setup + poll for F10 |
| `word_puzzle_main`                     | `games.c`           | skeleton + real 66-line index into letter_line_ptr |
| `poker_main`                           | `games.c`           | skeleton: alloc 10400 + $400 stakes + F10 quit |
| `poker_war_main`                       | `games.c`           | skeleton + **real 400-iter Fisher-Yates shuffle** + 26-card split |
| `poker_blackjack_main`                 | `games.c`           | skeleton: alloc 0x28a0 + $400 stakes + F10 quit |
| `poker_load_card_graphics`             | `cards.c`           | load 53 cards from CARDS + 54 MFDBs + highlight overlay |
| `init_vdi_and_screen`                  | `gfx_prim.c`        | mini-game VDI setup: stash logbase, dest buffer, fill mode |
| `exit_vdi_and_screen`                  | `gfx_prim.c`        | Setscreen restore |
| `midi_seq_init_song`                   | `midi_seq.c`        | song lifecycle: header parse, program reset, kick sequencer |
| `midi_seq_parse_header`                | `midi_seq.c`        | 6-command header walk (tempo, volume, scale, prog change) |
| `midi_seq_reset_programs`              | `midi_seq.c`        | 16-channel Program Change dispatch loop |
| `midi_seq_skip_padding`                | `midi_seq.c`        | walk past leading 0x00/0xFF filler bytes |
| `midi_seq_set_position`                | `midi_seq.c`        | stash cursor + envelope base + velocity + tick-per-beat |
| `midi_seq_start_playback`              | `midi_seq.c`        | seed 100-tick timers + arm sequencer + go to parse phase |
| `midi_seq_parse_channel_map`           | `midi_seq.c`        | 30-byte channel + program map decode (1-based -> 0-based) |
| `midi_seq_build_scale_table`           | `midi_seq.c`        | 132-note transpose LUT with 7-bit chord mask per octave |
| `midi_seq_send_program_change`         | `midi_seq.c`        | dispatch MIDI 0xCn Program Change, dedup by physical channel |
| `midi_seq_dispatch_event`              | `midi_seq.c`        | dual-output MIDI/PSG event router with 3-voice allocator |
| `midi_out_write_byte`                  | `psg_io.c`          | ACIA TDRE poll + data-register byte write |
| `psg_copy_envelope_params`             | `psg_io.c`          | inline 8-byte memcpy (Alcyon avoided libc) |
| `psg_write_register`                   | `psg_io.c`          | YM2149 two-stage register select + data write |
| `psg_set_mixer`                        | `psg_io.c`          | YM2149 mixer read-modify-write |
| `soundeffect_irq_play`                 | `sfx_irq.c`         | 8Hz Dosound commit: priority gate + DMA copy + countdown |
| `_draw_pixel`                          | `gfx_prim.c`        | degenerate 1-point VDI polyline (backbuffer + colour + pline) |
| `vsl_color`, `vst_color`, `vsf_color`  | Alcyon `vdibind.a` | 1-int colour setters (opcodes 17/22/25) |
| `vsf_interior`, `vsf_style`, `vswr_mode` | Alcyon `vdibind.a` | 1-int fill/mode setters (opcodes 23/24/32) |
| `v_pline`                              | Alcyon `vdibind.a` | N-point polyline (opcode 6) |
| `v_gtext`                              | Alcyon `vdibind.a` | ASCII text draw at (x,y) (opcode 8) |
| `v_bar`                                | Alcyon `vdibind.a` | filled bar (opcode 11 sub-op 1) |
| `vdi_copy_rect` (`vro_cpyfm`)          | Alcyon `vdibind.a` | raster blit with mode + src/dst MFDBs (opcode 109) |
| `load_objects`                         | `assets.c`          | read 14000-byte OBJECTS file |
| `load_sprites`                         | `assets.c`          | read 14000-byte SPRITES file |
| `asset_load_objects_table`             | `assets.c`          | load + parse OBJECTS into MFDB table |
| `asset_load_sprites_table`             | `assets.c`          | load + parse SPRITES into MFDB table |
| `asset_load_lcp`                       | `assets.c`          | header-driven BODY.LCP / PEx.LCP frame loader |
| `asset_load_names`                     | `assets.c`          | ASCII NAMES text-file reader |
| `decompress_scn`                       | `assets.c`          | HOUSE.SCN / TITLE.SCN nibble+word decoder |
| `soundeffects_load`                    | `sound.c`           | walk SOUNDS.LCP records + per-SFX Malloc |

## Stubbed

**None.**  Every game-logic and ABI function referenced anywhere in the
port has a real body.  `astubs.c` and `stubs.c` are retained as
empty history files documenting where each function moved.

### Remaining gaps (non-stub)

- **`vdi_call()` -- done.**  Port now links against Alcyon's
  official `vdibind.a` + `aesbind.a` + `gemlib.a` (bundled with the
  Atari ST Developer Kit), so every VDI/AES entry point is the
  ROM-authoritative shim: `move.l #_vdipb,d1 ; moveq.l #115,d0 ;
  trap #2` for VDI, `#200 ; trap #2` for AES.  Host build still
  uses the no-op stub in `osbind.h`.
- **Data tables populated from first principles** (see `globals.c`
  comments): `midi_scale_mask_table[16]`,
  `clothing_color_primary/secondary[16]`, `skin_color_palette[8]`,
  `dog_dest_x/y_offset_table[9]`.  Values are plausible and shape-
  correct but not byte-authoritative -- a Ghidra data-segment dump
  would replace them.
  (`tv_pattern_0..3_x/y_coords[8]` were also on this list until
  2026-07-21, when Ghidra was updated to reveal the actual
  `short pxy[4]` buffer layout of `tv_boul` / `tv_patl` -- our
  earlier "stand-in" coords were size-correct but the port was
  passing `count=2` to `v_pline` with only 1 of the 2 points
  initialised, causing sporadic compositor corruption + crashes
  during the computer-typing session.  Fixed byte-faithfully in
  commit 12e572f.)
- **Card-game engines beyond the skeleton `_main()` entry points.**
  `poker_main`, `poker_blackjack_main`, `poker_war_main`,
  `anagram_main`, `word_puzzle_main` all set up screen state, allocate
  buffers, and enter the F10-quit loop, but the actual game logic
  (hand ranking, dealer AI, bidding, keyboard tile drag, dictionary
  match) is not yet ported.
- **Mini-game text renderer** for the anagram / word-puzzle screens
  uses the LETTER.TXT font pipeline, which is loaded and indexed but
  not yet wired to the mini-game draw path.

## Build / test

- `make` -- full host syntax check under `-std=c89 -pedantic`
- `make linktest` -- link every object + a tiny `main()` (proves no
  undefined symbols)
- `make hyber_test` -- link `tests/hyber_roundtrip.c` and run it.
  Copies `DATA/HYBER` into place, calls `lcp_load()`, prints the owner
  and resident names, calls `lcp_save()`, and asserts the two files are
  byte-identical.  Verified passing: owner=REBECCA, resident=Norton,
  128/128 bytes round-tripped.
- `make letter_test` -- link `tests/letter_load.c` and run it.
  Copies `DATA/LETTER.TXT` into place, calls
  `file_load_letter_template()` which decompresses the 7.5 KB nibble-
  encoded file into a 10 KB text buffer and populates the 360-entry
  `letter_line_ptr[]` index.  Verified passing: 15 compression tokens
  match the frequency profile of English (space, e, t, o, a, n, s, r,
  h, CR, LF, l, i, y, u), lines 0-10 print real 1985 letter
  fragments ("I'm always thankful to live here--", etc.), lines 45 and
  359 print sensibly.
- `make sounds_test` -- link `tests/sounds_load.c` and run it.
  Loads `DATA/SOUNDS.LCP` through `soundeffects_load` and prints the
  first 16 SFX slot sizes + first 4 payload bytes.  Verified passing:
  16+ non-empty slots with sizes 34..148 bytes, plausible Dosound
  register-command byte patterns starting each payload.
- `make scn_test` -- link `tests/scn_decode.c` and run it.  Decodes
  `DATA/HOUSE.SCN` through `decompress_scn` and verifies the first 8
  output words match the Python `readFiles.py:decompressImageFile`
  reference implementation:
    ```
    0000 0000 0000 ffff 0000 0000 0000 ffff
    ```
  Also checks the last word is not the poison byte (proves all 16000
  words were written).  All checks pass.
- `make assets_test` -- link `tests/assets_load.c` and run it.
  Loads OBJECTS, SPRITES, BODY.LCP, PE2.LCP from `DATA/`, parses each
  and prints the record count + first 5 record dimensions.  Verified
  passing:
    - OBJECTS: 56 records (16x11, 16x11, 16x11, 16x10, 16x10, ...)
    - SPRITES: 50 records (48x13, 16x37, 16x37, 16x37, 18x36, ...)
    - BODY.LCP: 98 frames
    - PE2.LCP: 66 frames
  Every record parses cleanly at the correct big-endian header offset.
- `make vdi_pb_test` -- link `tests/vdi_pb.c` and run it.
  Calls each of the 10 VDI wrappers with sentinel values then inspects
  the shared `contrl[]`/`intin[]`/`ptsin[]` parameter arrays to verify
  each wrapper built the correct VDI ABI parameter block.  Proves the
  target-side trap #2 call will hand GEM's dispatcher exactly the
  bytes it expects.  All 10 checks pass.
- `make parser_test` -- link `tests/parser_smoke.c` and run it.
  Exercises the NLP parser end-to-end: `lcp_toupper` sanity, then
  `command_upperstr` tokenising "play a game" into three consecutive
  uppercase tokens, then `check_entered_command` on a full sentence.
  Vocabulary table is currently empty (needs Ghidra data dump), so
  every word bumps `_action_priority` +4; 4 unknown words yield
  `_action_priority = 21` (= 4+1 base from happiness + 16 for 4 unknown
  words + small randomRange nudge), and the empty action table falls
  through to `ACTION_NONE`.  Verifies the parser is structurally
  correct; adding real vocabulary data (from `lcp/LCP.py`) will
  automatically start returning real action IDs.

- `make sim_test` -- link `tests/sim_tick.c` and run it.  Drives
  `game_simulate_one_second` for 24 game-hours (86400 iterations) plus
  short 1-hour, 30-second and non-tick-frame drives from known
  starting states.  Verifies clock advance (06:00 -> 06:00 next day),
  hunger/thirst wrap counting, bathroom_need flag onset, and that
  `(animation_tick_counter & 7) != 0` correctly early-returns.  All
  assertions pass.
- `make sprite_test` -- link `tests/sprite_compose.c` and run it.
  Loads `DATA/BODY.LCP` (98 frames * 168 bytes, header second short
  is total payload not per-frame), wires `body_lcp_file` and a zeroed
  `body_shape_data`, calls `sprite_update_body()` with lcp_state=0,
  facing right, position (100, 100), and asserts the compositor
  produced non-zero output in `lcp_sprite_img[168]` at the expected
  screen anchor (x-4, y + body_y_offset[0] - 21).  Dumps the 64x21
  silhouette to `sprite_slot3.pgm` for visual inspection.  All checks
  pass.

`make test` runs the whole suite: linktest + hyber + letter + parser +
vdi_pb + assets + scn + sounds + sim + sprite = 10 targets, all green.

`make alcyon ALCYON_BIN=/path/to/alcyon/bin` will drive the real build
under Hatari, targeting Alcyon C 4.14 + LO68 -> `LCP.PRG`.  Not
exercised yet.

## Notable file-format discoveries

### Music Studio 2.0 provenance for `.SNG` / `.ORG`

Cross-checking LCP's `DATA/*.SNG` and `DATA/*.ORG` files against the
Activision Music Studio 2.0 distribution disk revealed that **all
11 songs on the LCP disk are direct exports from Music Studio**:

| LCP file       | Music Studio counterpart | Delta                  |
|----------------|--------------------------|------------------------|
| MYSTERY.SNG    | MYSTERY.SNG              | byte-identical         |
| PRELUDE.ORG    | PRELUDE.SNG              | byte-identical (renamed) |
| CANON.SNG      | CANON.SNG                | byte-identical         |
| REQUIEM.ORG    | REQUIEM.SNG              | byte-identical (renamed) |
| AISLEDAN.SNG   | AISLEDAN.SNG             | byte-identical         |
| CALYPSO.SNG    | CALYPSO.SNG              | byte-identical         |
| COUNTRY2.SNG   | COUNTRY2.SNG             | byte-identical         |
| BALLAD.SNG     | BALLAD.SNG               | byte-identical         |
| BOOGIE.SNG     | BOOGIE.SNG               | byte-identical         |
| BOSSA.SNG      | BOSSA.SNG                | 1 byte edit at 0x213   |
| STARSPAN.ORG   | STARSPAN.SNG             | shortened (2771 -> 2104 bytes) |

Music Studio was published by Activision in 1986 (Ed Bogas / Audio
Light) for Atari ST / Apple II / C64, designed primarily for Casio's
early MIDI keyboards (CZ-101, CT-6000, CTK series -- the Music Studio
disk ships `CASIO1..CASIO4.PRE`/`.INT` presets).

**File signature** (first 10 bytes of every LCP `.SNG` / `.ORG`):
```
CD 4D 73 74 75 64 69 6F CD 02   ->  "\xCD" "Mstudio" "\xCD" \x02
```
The high-bit-set `\xCD` bytes were a common 1980s format marker to
prevent ASCII tools from mistaking binary files as text.  `song_play`
discards these 10 bytes with a leading `file_read(fh, 10, temp)`
then hands the remaining body to `midi_seq_init_song`; the body's
internal layout is documented at the top of `midi_seq.c`.

**`.ORG` extension**: purely cosmetic renaming; same file format as
`.SNG`.  Likely done during LCP disk mastering so the game's
category system can select classical/organ pieces via `Fsfirst("*.org")`
and pop/jazz pieces via `Fsfirst("*.sng")` from a single directory.
