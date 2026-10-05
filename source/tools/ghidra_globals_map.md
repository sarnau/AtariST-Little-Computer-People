# Ghidra long-name -> port short-name mapping (globals)

The port has to use short (<=8-char external) names because Alcyon C 4.14
truncates external symbols to 8 characters.  Ghidra's LCP.PRG database
uses long descriptive names.  This file lists the correspondence for
cross-referencing decompiler output against port source.

## Coverage

**Currently mapped: all but THIRTEEN port globals** (measured
2026-09-06).  The previous note here said "~366/397, remaining ~93"
and was badly wrong: it compared lcp_sym.68k's 8-char TRUNCATED
linkage names against this file's full names, so `waterLevel`,
`movingIn` and dozens more counted as unmapped when they
are not.  Expand the truncations first -- and exclude DRI libc and the
AES library's own `gl_apid`, which are not port globals.

The thirteen that remain are listed at the end of this file with what
each one does.  None of them has a 1985 descriptive name left to
recover, so the way to "extend coverage" is NOT to invent names for
them; it is to decompile more Ghidra FUNCTIONS and record pairs the
original analysis actually produced.  Push new pairs with
`source/tools/sync_ghidra_names.sh`.

## Status: auto-rename pipeline

Use **`source/tools/sync_ghidra_names.sh`** (`verify` for a read-only
pass).  It drives `analyzeHeadless` with
`tools/ghidra/LcpSyncNames.java`: no server, no GUI, and it works on a
closed project.  **Ghidra must be CLOSED** -- it holds an exclusive
lock, and clearing a live lock is how the database gets corrupted; the
script refuses rather than fight over it.

Both lists are under version control now (2026-09-07); neither is
hand-maintained in `$HOME` any more:

  * **What to push** is `tools/ghidra/lcp_sync.tsv`, CURATED by hand.
    It stays curated deliberately -- a blanket push of every port name
    would overwrite the descriptive names Ghidra's own analysis carries
    for the globals below.
  * **What to check** is generated fresh on every `verify` run by
    `tools/gen_ghidra_verify.py`, from `lcp_sym.68k` and
    `tools/stx_bss_layout.tsv`.  It expands the truncated linkage names
    and omits the library symbols the repo has no authority over, so a
    mismatch it reports is real.  Its first run found four names an
    earlier sync had pushed truncated (`body_sh`, `body_pt`, `evnt_ti`,
    `form_al`) and five addresses Ghidra had no label at; the
    hand-written list it replaced had noticed none of them.  All fixed
    except altScreen, whose base is inferred -- **ok=693, mismatched=0**.

The older `apply_ghidra_renames.sh` is DEAD and this file used to
point at it.  It POSTs to a Ghidra HTTP server on :8089 and needs
`RenameLcpGlobals.java`, `list_data_symbols.java` and a fresh
`/tmp/ghidra_syms.txt` -- none of which are installed -- and it POSTed
to `/run_script`, which this plugin does not serve.

The plugin ITSELF is alive on :8089 while Ghidra is open, and it does
have a data-symbol rename -- `POST /rename_data` with
`{"address": ..., "newName": ...}`, and `POST /analyze_data_region` to
see what is there first.  Use that for a one-symbol correction instead
of closing Ghidra for the headless script.  Full recipe and the
parameter-name trap are in CLAUDE.md.

Three things to get right when building the list: Ghidra address =
link address + 0x10000; for BSS take the address from
`tools/stx_bss_layout.tsv` (lcp_sym.68k carries lo68's, not the
reference's); and expand lcp_sym.68k's 8-char TRUNCATED linkage names
against the `extern` declarations in `include/*.h` first, or Ghidra
ends up with `lcp_pat` for `walkStep`.

Rename BY ADDRESS, not by name, wherever the port's own name has
changed -- and always where two names were SWAPPED.  Going by name
chases a symbol that has moved, or collides with the name the other
cell still holds.

## Address mismatch caveat

`source/tools/find_syms.py` addresses cover the **port's rebuilt PRG**
(different .o layout, different link order) -- they do NOT map to the
Ghidra project's addresses (which loaded the **original 1985 LCP.PRG**
at base 0x0).  Matching between the two is by *role* and *access
pattern* in decompiled code, not by address.

## Confirmed mappings

Derived from decompiling: `timerAIsr`, `seqAdvance`, `stepEnvelopes`, `psgWrite`,
`loadLetterText`, `rummageCabinet`, `chooseAction`, `pickIdleAction`, `startSfx`, `simStep`,
`drawHands`, `drawSlot`, `anaIntroText`.

### Time / calendar / animation

| Ghidra                            | Port         |
|-----------------------------------|--------------|
| `animation_tick_counter`          | `frameCount`    |
| `game_seconds_counter`            | `t_sec`      |
| `time_minutes`                    | `t_min`      |
| `time_hours`                      | `t_hour`     |
| `date_day`                        | `t_day`      |
| `date_month`                      | `t_mon`      |
| `date_year`                       | `t_year`     |

### Player / AI state

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `lcp` (PLAYER struct)              | `lcp`        |
| `lcp_state`                        | `animState`     |
| `lcp_facing_direction`             | `resFacing`   |
| `lcp_filing_cabinet_open`          | `filingCabOpen`   |
| `lcp_water_level`                  | `waterLevel`   |
| `last_action`                      | `lastAction`    |
| `trigger_action`                   | `nextAction`     |
| `intro_sequence_active`            | `movingIn`   |
| `phone_answered_flag`              | `phoneAnswered`     |
| `phone_call_active_flag`           | `phoneRinging`    |
| `ctrl_a_alarm_pressed_flag`        | `alarmRinging`    |
| `lunch_meal_triggered_today`       | `lunchDone`   |
| `dinner_meal_triggered_today`      | `dinnerDone`   |
| `morning_wakeup_triggered_today`   | `wakeupDone`    |
| `bedtime_triggered_today`          | `bedtimeDone`   |
| `_action_queue[]`                  | `queueActions[]`  |
| `_action_priority_queue[]`         | `queuePriority[]`  |
| `_action_list_size`                | `queueCount`    |

### Action tables (tick_tables)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `action_table_active[]`            | `activeActions[]`  |
| `action_table_moderate[]`          | `moderateActions[]`  |
| `action_table_relaxed[]`           | `relaxedActions[]`  |
| `activity_schedule_table[]`        | `scheduleTiers[]`  |
| `triggered_event_list[]`           | `eventQueue[]`   |

**Four of these rows were wrong until 2026-09-06**, and the section
even flagged the first three as unverified ("assignment ... is by
role").  Verified now, and the port side was on the wrong symbols:

  * The action tables are `activeActions` / `moderateActions` / `relaxedActions`, each
    `[16]` and indexed `rndRng(0, 15)` in airandom.c to pick an action
    for the resident's activity level.  They used to be paired with
    `alarmFrames` / `clockFrames` / `phoneFrames`, which are OBJECT ANIMATION
    frame lists fed to `drawObject` -- and which this file ALSO pairs,
    correctly, with `object_alarm_animation` / `object_clock_animation`
    / `object_phone_animation` further down.  `alarmFrames` appeared twice
    with contradictory meanings.  Confirmed by address: alarmFrames is
    Ghidra 0x2b92a and dat_u3a.c's own comment reads
    "alarm_animation @ 0x2B92A".
  * `triggered_event_list` is **eventQueue**, not `scratchArr`.  eventQueue is
    Ghidra 0x2b6da -- exactly the address globals.c cites for that
    name, in a comment that sits above scratchArr's declaration by
    mistake.  eventQueue is the event FIFO queueEvent appends to and everything
    tests as `eventQueue[0] != ACTION_NONE`; scratchArr is a 10-short scratch
    buffer the action handlers cache player STATES in (useComputer fills
    it with STATE_HANDS_DOWN and friends), and it has no descriptive
    Ghidra name.

The Ghidra spellings of `action_table_*` are kept because the ROLE is
now proven, but note they were role-inferred by the original analysis
and the address-keyed sync found a stale port label (`alarmFrames`) rather
than a descriptive one sitting at activeActions's address -- so treat the
left column here as a description, not as a string to search Ghidra
for.  `fireFrames[]` and `bowlFrames[]` remain unsampled.

### MIDI sequencer

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `midi_is_playing`                  | `songPlaying`    |
| `midi_tick_counter`                | `timerTicks`    |
| `midi_tick_prescaler`              | `seqCountdown`    |
| `midi_ticks_per_beat`              | `ticksPerBeat`    |
| `midi_tick_divider`                | `envDivider`    |
| `midi_direct_write_mode`           | `seqBusy`    |
| `midi_reentrant_lock`              | `envBusy`   |
| `midi_sequencer_active`            | `songActive`    |
| `midi_seq_phase`                   | `seqPhase`    |
| `midi_event_duration`              | `ticksToNext`    |
| `midi_next_event_tick`             | `nextEvTick`    |
| `midi_last_processed_tick`         | `lastExpTick`    |
| `midi_note_event_index`            | `queueLen`     |
| `midi_note_length_params[]`        | `sfxData[]`  |
| `aes_int_out[]`                    | `beatTicks[]` |

### PSG / envelope

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `psg_notes_active`                 | `psgActive`   |
| `psg_envelope[]`                   | `psgEnvelope[]` |
| `psg_channel_ramp_accum[]`         | `rampAccum[]` |
| `psg_channel_ramp_delta[]`         | `rampDelta[]` |
| `psg_register_offset_table[]`      | `ampRegs[]`  |
| `psg_output_volume` (working reg)  | `noteVolume`   |

### Sound effects (`startSfx`)

| Ghidra                              | Port         |
|-------------------------------------|--------------|
| `soundeffect_active_flag`           | `sfxPending`    |
| `soundeffect_playing_flag`          | `sfxPlaying`    |
| `soundeffect_current`               | `sfxReqId`    |
| `soundeffect_current_priority`      | `sfxCurPrio`    |
| `soundeffect_playing_id`            | `sfxCurId`    |
| `soundeffect_default_duration_hi`   | `sfxDurHi`    |
| `soundeffect_default_duration_lo`   | `sfxDurLo`    |
| `soundeffect_Hz200`                 | `sfxStartHz`    |
| `soundeffect_remaining_ticks`       | `sfxTicksLeft`    |
| `soundeffect_duration`              | `sfxReqDur`    |
| `soundeffect_DoSound_Buffer[]`      | `sfxBuffer[]`  |
| `_soundeffect_priority_table[]`     | `sfxPriority[]`   |

### Sprite render

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `sprite_mfdb_image[]`              | `slotImgMfdb[]`  |
| `sprite_mfdb_mask[]`               | `slotMaskMfdb[]`  |
| `sprite_active_image[]`            | `drawnImage[]`  |
| `sprite_active_mask[]`             | `drawnMask[]`  |
| `sprite_active_width[]`            | `drawnWidth[]`  |
| `sprite_active_height[]`           | `drawnHeight[]`  |

### Letter / clock

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `letter_txt_content`               | `letterText`     |
| `letter_line_ptr[]`                | `letterLines[]`   |
| `clock_minute_position[]`          | `minuteHandXY[]`  |
| `clock_hour_position[]`            | `hourHandXY[]`  |

### VDI plumbing

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `vdihandle`                        | `vdiHandle`     |
| `screen_mfdb` (compositing target) | `frameMfdb`    |
| `MFDB_screen_ptr` (source screen)  | `houseMfdb`    |
| `screen_scale_factor`              | `screenScale`   |

### Dog AI

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `dog_pettable_flag` (Ghidra WRONG)  | `patAllowed`     |
| `dog_idle_countdown`               | `dogIdleCount`   |
| `dog_food_bowl_change`             | `bowlChange`   |
| `dog_near_food_bowl`               | `dogMayEat`   |
| `dog_on_stairs_flag`               | `dogOnStairs`   |
| `dog_visible`                      | `dogNoTopFlr`     |
| `dog_initialized`                  | `dogHidden`    |
| `dog_last_target_index`            | `dogLastPick`   |
| `dog_initial_target_index`         | `dogStartPos`    |
| `dog_initial_y_offset`             | `dogYStartNudge`    |
| `dog_destination_position_table`   | `dogRoamSpots`    |
| `dog_dest_x_offset_table`          | `dogXNudge`    |
| `dog_dest_y_offset_table`          | `dogYNudge`    |
| `dog_eating_active`                | `dogEating`    |
| `dog_eating_countdown`             | `dogEatCount`    |
| `dog_flip_image_buffer`            | `dogMirImage`    |
| `dog_flip_mask_buffer`             | `dogMirMask`    |
| `dog_sprite_eating_anim_tab`       | `dogEatFrames`    |
| `dog_sprite_id`                    | `dogSpriteId`     |
| `dog_target_x`                     | `dogXTarget`      |
| `dog_target_y`                     | `dogYTarget`      |
| `dog_walk_anim_cycle`              | `dogStepIdx`    |
| `dog_walk_anim_frames`             | `dogWalkSprites`    |
| `delivery_is_for_dog`              | `isDogDelivery`    |

### Head / body / stair

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `head_sprite_buffer`               | `headImage`    |
| `sprite_buffer`                    | `genMaskBuf`    |
| `head_sprite_frame`                | `headFrame`    |
| `head_sprite_mask`                 | `headMask`    |
| `head_sprite_mirror_flag`          | `headMirror`    |
| `head_anim_current`                | `headPose`    |
| `head_anim_target_state`           | `headTarget`    |
| `head_anim_mode`                   | `headMode`    |
| `head_anim_delay_countdown`        | `headDelay`    |
| `head_anim_state_last`             | `headLastWalk`    |
| `head_height_per_state`            | `headYOffset`     |
| `head_x_offset_per_state`          | `headXOffset`    |
| `head_default_angle_per_state`     | `headRestDir`    |
| `head_movement_delta_table`        | `headTurnStep`     |
| `head_tilt_frame_offset`           | `headTiltFrame`    |
| `head_shape_data`                  | `headShapes`     |
| `body_shape_data`                  | `bodyShapes`   |
| `body_sprite_frame_table`          | `bodyIndex`   |
| `body_y_offset_per_state`          | `bodyYOffset`   |
| `happiness_head_frame_offset`      | `moodHeadBase`   |
| `staircase_waypoint_coords`        | `stairWaypts`   |
| `stair_top_y_threshold`            | `xLanding`   |
| `stair_bottom_y_threshold`         | `yLanding`   |

### Anagram game

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `anagram_all_clues_used`           | `anaExtraGuess`    |
| `anagram_clue_count`               | `anaNumClues`    |
| `anagram_guess_prompt_strings`     | `anaPrompts`    |
| `anagram_guess_number`             | `anaGuessNum`    |
| `anagram_input_buffer`             | `anaInput`    |
| `anagram_original_word`            | `anaAnswer`    |
| `anagram_scrambled_word`           | `anaScrambled`    |
| `anagram_words_buffer`             | `anaDict`     |
| `anagram_wrong_guess_messages`     | `anaWrongMsgs`    |
| `anagram_word_length`              | `anaWordLen`    |
| `anagram_clue_used_this_round`     | `anaClueUsed`    |

### Word puzzle

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `word_puzzle_blank_count`          | `wpzBlanks`     |
| `word_puzzle_failure_messages`     | `wpzWrongMsgs`    |
| `word_puzzle_prompt_messages`      | `wpzPrompts`     |
| `word_puzzle_success_messages`     | `wpzRightMsgs`    |
| `word_puzzle_current_index`        | `wpzIndex`     |
| `word_puzzle_data_buffer`          | `wpzText`     |

### Poker / War

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `poker_bet_amount`                 | `pkrBet`     |
| `poker_computer_bluff_flag`        | `pkrBluffing`   |
| `poker_computer_card_count`        | `bjHitsDealer`     |
| `poker_computer_hand`              | `compHand`      |
| `poker_computer_hand_rank`         | `compRank`    |
| `poker_war_computer_score`         | `bjDealerScore`  |
| `poker_computer_war_cards`         | `compWarCards`     |
| `poker_discard_count`              | `pkrNumDisc`    |
| `poker_discard_pile`               | `pkrDiscPile`   |
| `poker_deck_position`              | `pkrRaiseAmt`    |
| `poker_card_display_slot`          | `pkrWinner`   |
| `poker_hand_rank_flags`            | `compScoring`     |
| `poker_hand_suit_flags`            | `compSorted`     |
| `poker_computer_passed`            | `pkrPassed`    |
| `poker_player_card_count`          | `bjHitsMain`     |
| `poker_player_hand`                | `plyrHand`      |
| `poker_game_phase`                 | `bjDidSplit`   |
| `poker_player_hand_rank_flags`     | `plyrScoring`    |
| `poker_player_hand_suit_flags`     | `plyrSorted`    |
| `poker_player_hand_value`          | `pkrLastBet`     |
| `poker_player_split_card_count`    | `bjHitsSplit`    |
| `poker_war_player_score`           | `bjPlyrScore`  |
| `poker_player_split_hand`          | `bjSplitHand`     |
| `poker_player_war_cards`           | `plyrWarCards`     |
| `poker_quit_flag`                  | `cardQuit`    |
| `poker_raise_message`              | `pkrMsgRaise`      |
| `poker_war_round`                  | `pkrRound`   |
| `poker_card_selected`              | `pkrSelected`     |
| `poker_take_cards_message`         | `pkrMsgTake`     |
| `poker_computer_hand_cards`        | `warDepth`     |
| `poker_computer_bet`               | `bjBetMain`    |
| `poker_computer_draw_pile`         | `compPile`    |
| `poker_computer_money`             | `compChips`    |
| `poker_player_bet`                 | `bjBetSplit`    |
| `poker_player_draw_pile`           | `plyrPile`    |
| `poker_player_money`               | `plyrChips`    |

### Cards

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `cards_data`                       | `cardImages`    |
| `cards_MFDB_blocks`                | `cardMfdb`   |
| `cards_x_pos_a`                    | `cardXComp`     |
| `cards_x_pos_b`                    | `cardXPlyr`     |
| `cards_y_pos_a`                    | `cardYComp`     |
| `cards_y_pos_b`                    | `cardYPlyr`     |

### Letter (extended)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `letter_line_count`                | `needlePos`    |
| `letter_paragraph_count`           | `vuLeds`    |
| `letter_char_width_table`          | `typingSprites`    |
| `letter_greeting_table`            | `letterSignoffs`      |

### Save / load

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `save_physbase`                    | `tosPhysbase`     |
| `save_logbase`                     | `panelLogbase`     |
| `saved_body_sprite_ptr`            | `savedBodyImg`   |
| `saved_head_sprite_ptr`            | `savedHeadImg`   |
| `saved_vqt_attr`                   | `savedTextAttr`    |
| `lcp_loaded`                       | `loadedSave`    |

### Fire event

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `fire_active_flag`                 | `fireBurning`   |
| `fire_duration_countdown`          | `fireTimeLeft`   |
| `fire_extinguish_flag`             | `fireDouse`   |

### TV patterns

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `tv_pattern_0_x_coords`            | `tvBar0X`    |
| `tv_pattern_0_y_coords`            | `tvBar0Y`    |
| `tv_pattern_1_x_coords`            | `tvBar1X`    |
| `tv_pattern_1_y_coords`            | `tvBar1Y`    |
| `tv_pattern_2_x_coords`            | `tvBar2X`    |
| `tv_pattern_2_y_coords`            | `tvBar2Y`    |
| `tv_pattern_3_x_coords`            | `tvBar3X`    |
| `tv_pattern_3_y_coords`            | `tvBar3Y`    |
| `tv_pattern_color_indices`         | `tvBarColor`    |

### Sprite engine (extended)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `sprite_active_x`                  | `drawnX`    |
| `sprite_active_y`                  | `drawnY`    |
| `sprite_pending_flag`              | `pendReady`    |
| `sprite_pending_width`             | `pendWidth`    |
| `sprite_pending_x`                 | `pendX`    |
| `sprite_pending_y`                 | `pendY`    |
| `sprite_pending_image`             | `pendImage`    |
| `sprite_pending_mask`              | `pendMask`    |
| `sprite_pending_height`            | `pendHeight`    |
| `sprite_slot_map`                  | `spriteSlot`    |
| `sprite_layer_flags`               | `spriteLayer`    |

### Object tables

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `object_tab_mfdb`                  | `objMfdbs`    |
| `object_tab_width`                 | `objWidths`    |
| `object_tab_height`                | `objHeights`    |
| `objects_file`                     | `objFileBuf`   |
| `action_interruptible_flag`        | `noPreempt`    |

### Clock / phone / misc

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `clock_minute`                     | `clockMinute`    |
| `clock_hour`                       | `clockHour`    |
| `phone_ring_countdown`             | `ringCountdown`     |
| `phone_hangup_flag`                | `phoneHangUp`      |
| `record_browsing_active`           | `organPlaying`    |
| `food_delivery_available`          | `pantryFull`   |

### Init / palette / parser / debug

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `copyprot_check_return`            | `copyProtResult`    |
| `game_speed_counter`               | `walkSpeed`     |
| `midi_noteon_state`                | `noteOwner`    |

### (subsystem line placeholder — do not remove)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `main_colorpalette`                | `mainPalette`   |
| `skin_color_palette`               | `skinColors`   |
| `month_name_table`                 | `monthNames`   |
| `pex_lcp_ptr`                      | `pexFrames`    |
| `pex_lcp_file`                     | `pexFrames`    |
| `sng_song_file_count`              | `songCount`    |
| `org_song_file_count`              | `organCount`    |
| `input_string`                     | `inputLine`     |
| `command_input_buffer`             | `typedLine`    |
| `command_input_ptr`                | `parsedLine`    |
| `debug_hide_lcp_offscreen`         | `debugHideLcp`   |
| `text_scroll_timer`                | `textTimer`    |
| `last_hz200`                       | `lasthz`    |
| `last_vbclock`                     | `lastFrameVbl`   |

## Ghidra names seen but role not yet mapped to a port global

`giselect`, `giwrite` -- PSG hardware registers (0xFF8800/0x8802), not
port globals (they are direct memory-mapped I/O in `psg_io.c`).
`isra` -- MFP interrupt-service register byte; not a port global.

## Port globals not yet paired with a Ghidra long name

Every port global not in the tables above; still to be paired by
sampling more decompilations.  Priority modules to sample next:
`source/dog.c` (dog AI, `dogXTarget`/`dogStepIdx`/`dogSpriteId`), `source/save.c`
(`savedBodyImg`, `savedHeadImg`, `tosPhysbase`, `panelLogbase`), `source/games.c`
(poker `pk_*` block, anagram `ag_*` block), `source/parser.c`
(`nibbleBytes`, `inputLine`, `parsedLine`), `source/letload.c`
(`typingSprites`, `letterWord`), `source/render.c` (compositor `textTimer`,
`screenScale`, `screenMfdb`, `altScreen`/`houseBuf`).

### Batch 3 additions

Derived from decompiling `renderFrame`, `updateBody`, `updateHead`, `drawSlot`,
`fillPanel`, `drawObject`, `dogNextWaypt`, `moveDog`, `parseSongHeader`, `psgWrite`,
`psgMixer`, `copyEnvelope`, `stepEnvelopes`, `aciaWrite`.

### Screen buffers / render targets

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `SCREEN_BUFFER_A`                  | `altScreen`    |
| `SCREEN_BUFFER_B`                  | `houseBuf`    |
| `screen_ptr`                       | `housePtr`    |
| `screen_logbase`                   | `drawLogbase`    |
| `screen_scroll_down_count`         | `stripScroll`    |
| `dest_screenbase_ptr`              | `stripBuf`     |
| `current_screen_mfdb`              | `flipMfdb`     |
| `MFDB_dest_screenbase_cards`       | `cardTableMfdb`   |

### LCP sprite render (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `body_lcp_file`                    | `bodyFrames`   |
| `lcp_sprite_img`                   | `bodyImage`    |
| `lcp_sprite_mask`                  | `bodyMask`    |
| `lcp_carrying_object_flag`         | `isCarrying`    |
| `lcp_sprites_hidden`               | `lcpHidden`     |
| `lcp_dog_bowl_status`              | `bowlLevel`   |
| `carry_body_frame_table`           | `carryFrames`     |

### Dog waypoint / floor (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `dog_waypoint_x`                   | `dogXWaypt`      |
| `dog_waypoint_y`                   | `dogYWaypt`      |
| `floor_bottom_y_coords`            | `floorBottomY`     |
| `floor_center_y_coords`            | `floorWalkY`     |

### MIDI sequencer (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `midi_channel_count`               | `songKey`     |
| `midi_current_channel`             | `noteChan`    |
| `midi_current_program`             | `sentProgram`    |
| `midi_current_note`                | `noteNum`    |
| `midi_data_ptr`                    | `loopTarget`    |
| `midi_data_base_ptr`               | `songEvents`   |
| `midi_default_velocity`            | `defVelocity`    |
| `midi_velocity`                    | `noteVel`     |
| `midi_tempo`                       | `songTempo`    |
| `midi_note_duration_table`         | `durTable`     |
| `midi_var_r`                       | `fixedChan`    |
| `midi_saved_timer_vector`          | `oldTimerAVec`    |
| `midi_song_buffer`                 | `songBuf`    |
| `midi_song_loop_flag`              | `useSongChan`    |
| `midi_seq_position`                | `songPos`   |
| `midi_note_event_queue`            | `noteQueue`     |
| `midi_note_event_count`            | `loopTop`    |
| `midi_note_on_flag`                | `noteToQueue`    |
| `midi_note_off_flag`               | `noteIsOff`    |
| `midi_note_mode_flags`             | `noteMode`    |
| `midi_note_hi_limit`               | `noteHigh`    |
| `midi_note_lo_limit`               | `noteLow`    |
| `midi_output_enabled`              | `midiOutOn`     |
| `midi_program_map`                 | `progMap`   |
| `midi_channel_map`                 | `chanMap`   |
| `midi_scale_mask_table`            | `keyScaleMask`     |
| `midi_scale_transpose_table`       | `noteMap`     |
| `midi_event_type_flag`             | `noteDecoded`    |
| `midi_loop_stack`                  | `loopStack`    |
| `midi_event`                       | `midiMsg`     |

### PSG (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `psg_channel_notes`                | `psgChanNote`   |
| `psg_default_volume`               | `defPsgVol`   |
| `psg_output_enabled`               | `psgOutOn`    |

### Sound effects (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `soundeffect_dosound_status`       | `sfxDosStat`    |
| `soundeffect_dosound_control`      | `sfxDosCtl`    |

### Poker (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `poker_bet_message`                | `pkrMsgBet`      |

## Batch 3 conflicts / ambiguities noted

- `MFDB_screen_ptr` (Ghidra) appears to be the port's `houseMfdb`, based
  on `copyBlocks32` argument order in `renderFrame` vs `renderf.c`. Existing
  row `screen_mfdb -> houseMfdb` looks reversed: my read is
  `screen_mfdb -> frameMfdb` and `MFDB_screen_ptr -> houseMfdb`.
  Left existing row alone per task instructions.
- `soundeffect_active_flag` (0x54010) is referenced by `renderFrame` for
  the post-render play flag reset; port uses `sfxPending` there.
  Existing row maps `soundeffect_playing_flag -> sfxPending`, which
  looks like the wrong pairing (`playing_flag` is at 0x5a2ca and
  probably matches port `sfxPlaying`). Not touched.
- Poker `bjBustMain / bjBustSplit / bjNatMain / bjNatSplit / bjDblSplit / bjMatchBet /
  bjDblMain / warDeck / plyrRank / potChips` still unresolved -- Ghidra
  has no matching-shaped long names in symbol dump; needs a
  decompile of the poker/blackjack game function (function names
  are not preserved in Ghidra either -- no `poker_main`/
  `poker_blackjack_main`/`wp_intr` symbols were found).
- Word puzzle: `wp_*` shorts (`wpzBlanks/prm/succ/fail`) are already
  mapped; `word_puzzle_player_answers` and
  `word_puzzle_current_index / _data_buffer / _blank_count` are
  mapped; no additional port shorts remain.
- Ghidra `midi_ticks_per_beat`, `midi_seq_max_position`,
  `midi_envelope_data_base`, `midi_duration_scale`,
  `midi_noteon_state`, `midi_dma_start_lo`, `midi_channel_volume`
  seen but port shorts (`seqCountdown`, `queueLen`, `songAdsr`, `envRelTab`,
  `envRateTab`, `envSusTab`, `envTimeTab`, `loopTop`, `mi_lasT`, `mi_nOS`,
  `ticksToNext`, `noteAccent`, `songEndPtr`) not confidently pairable from
  name alone -- needs decompile of `seqAdvance` / `timerAIsr` internals.
- Ghidra `poker_computer_hand_value_lo/hi`, `poker_card_deck_index`,
  `poker_pot_amount`, `poker_display_x_offset`, `poker_round_count`,
  `poker_card_back_mfdb`, `poker_draw_discard_flags` seen but port
  shorts uncertain.

### Batch 4 additions

Derived from decompiling `seqAdvance`, `parseEvents`, `expireNotes`, `pushLoop`,
`dispPot`, `playWar`, `warRound`, plus targeted grep of the port
against the Ghidra symbol dump.

#### MIDI envelope tables and sequencer

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `midi_seq_max_position`            | `songEndPtr`    |
| `midi_envelope_data_base`          | `songAdsr`     |
| `midi_envelope_rate_table`         | `envRateTab`    |
| `midi_envelope_time_table`         | `envTimeTab`    |
| `midi_envelope_sustain_table`      | `envSusTab`    |
| `midi_envelope_release_table`      | `envRelTab`    |

#### Poker / minigame

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `poker_pot_amount`                 | `potChips`    |
| `poker_draw_discard_flags`         | `warDeck`     |
| `disable_key_input_flag`           | `keysBlocked`   |
| `minigame_timeout_flag`            | `mgTimedOut`    |

## Batch 4 conflicts / ambiguities noted

- Ghidra's `g_mnevi` / `g_mnevc` are used in `seqAdvance`, `parseEvents`,
  `expireNotes`, `pushLoop` where the port uses `queueLen` / `loopTop`,
  yet the port also has separately-named globals `g_mnevi` /
  `g_mnevc` (used only in `stopSequencer`). The existing map rows
  `midi_note_event_index -> g_mnevi` and
  `midi_note_event_count -> g_mnevc` may therefore be pointing at
  the wrong port short: the addresses referenced by the sequencer
  hot path in Ghidra correspond to port `queueLen` / `loopTop`.
  Not touched -- needs an address-based audit before deciding
  which port short is the "real" counterpart.
- Similarly, `midi_event_duration` (Ghidra `g_medu`) is written
  from `aes_intO[7]` and used as "ticks until next event" in
  `seqAdvance`; the port assigns `ticksToNext = -1` and
  `ticksToNext = (short)nextEvTick - (short)timerTicks` in the same place,
  while the port's `g_medu` is only referenced by `stopSequencer`.
  So `midi_event_duration -> ticksToNext` looks like the correct
  pairing, but a duplicate row was not added.
- `midi_tick_prescaler` (Ghidra) is set from `aes_intO[7]` in
  `seqAdvance`; the port assigns to `seqCountdown` there. Existing map
  row is `midi_tick_prescaler -> ticksPerBeat`, but `ticksPerBeat` is used
  in `parseEvents` where Ghidra uses `midi_ticks_per_beat`. So the
  correct pairs appear to be `midi_tick_prescaler -> seqCountdown`
  and `midi_ticks_per_beat -> ticksPerBeat`. Not touched.
- `poker_blackjack_flag` (Ghidra 0x3d114) is a single BOOL; port
  has two (`bjNatMain`, `bjNatSplit`) for the two blackjack hands.
  Cannot pair without decompiling the blackjack routine (not
  found by name in this pass).
- `poker_computer_hand_value_lo`/`_hi`, `poker_card_deck_index`,
  `poker_display_x_offset`, `poker_round_count`, `poker_card_back_mfdb`
  still not paired to port shorts.
- Sprite descriptor arrays (`spriteBitmap`, `spriteMask`, `spriteHeight`,
  `spriteWidth`) live at Ghidra addresses that only have raw
  `PTR_ARRAY_xxxxxx` / `SHORT_ARRAY_xxxxxx` symbols -- no
  meaningful long name to pair against.

### Port shorts still unpaired (candidates for future decompile passes)

- Music Studio / MIDI: `phraseBits`, `g_molof`, `g_msmap`, `mi_nOS`,
  `ticksToNext`, `noteAccent`, `mi_lasT`, `queueLen`, `loopTop`,
  `moodPriority`, `mouseHidden`.  (Batch 4 paired `songEndPtr`, `songAdsr`,
  `envRelTab`, `envRateTab`, `envSusTab`, `envTimeTab`, `mgTimedOut`.)
- Screen/render extras: `bshdbuf`, `hshdbuf`, `hs_size`, `g_dsb`
  (may be dead; comment says former alias of `housePtr - 254`),
  `walkSpeed`, `spriteHeight`, `spriteWidth`, `spriteBitmap`, `spriteMask`, `g_setmt`,
  `g_setah`, `g_setaw`, `frameMfdb` (see conflict note above).
- Letter/clock: `letterWord`, `shirtPrimary`, `shirtSecondary`, `typedCursor`,
  `typedLine`, `patFrame`, `patActive`, `patSprites`, `patLastSprite`.
- Poker war: `bjDblSplit`, `bjMatchBet`, `bjDblMain`, `bjBustMain`, `bjBustSplit`,
  `bjNatMain`, `bjNatSplit`, `plyrRank`.  (Batch 4 paired `warDeck`,
  `potChips`.)
- Misc: `env_val`, `inEvent`, `vuLedMasks`, `studyDoorOpen`,
  `tickCount`, `copyProtResult`, `stripStore`, `cmdWord`, `footstepDue`, `bitMask8`,
  `cmdPriority`, `alarmSounding`, `stoveFrames`, `objMfdbs`, `carriedSprite`,
  `onStairs`, `recordPlaying`, `tvRunning`.

### Batch 5 additions

Derived from targeted grep of `/tmp/ghidra_syms.txt` against remaining
port shorts, cross-checked by usage patterns in the port
(`aleisure.c`, `keyboard.c`, `parser.c`, `assets.c`, `tick.c`,
`gfx_prim.c`, `walk.c`, `sprites.c`, `render.c`).

#### LCP appliance / state flags

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `lcp_record_playing`               | `recordPlaying`   |
| `lcp_tv_on`                        | `tvRunning`     |
| `lcp_study_door_open`              | `studyDoorOpen`   |
| `lcp_cabinet_open`                 | `kitchenCabOpen`   |
| `lcp_toilet_door_open`             | `toiletDoorOpen`   |
| `lcp_food_count`                   | `foodSupply`   |
| `lcp_closet_door_open`             | `bedClosetOpen`   |
| `lcp_dresser_open`                 | `dresserOpen`   |
| `lcp_front_door_open`              | `frontDoorOpen`   |
| `lcp_on_stairs_flag`               | `onStairs`    |
| `lcp_carried_object`               | `carriedSprite`    |

#### Petting / input / parser

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `game_input_mode_flag`             | `typingOff`    |
| `compression_tokens`               | `nibbleBytes`   |
| `clothing_color_primary`           | `shirtPrimary`    |
| `clothing_color_secondary`         | `shirtSecondary`    |
| `petting_dog_active`               | `patActive`    |
| `petting_anim_frame`               | `patFrame`    |
| `petting_last_sprite_slot`         | `patLastSprite`    |
| `command_input_buffer_pos`         | `typedCursor`    |
| `user_input_buffer`                | `cmdWord`    |

#### Scene / VDI

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `scene_common_data`                | *(deleted -- LCP_ORG-only; the 30 bytes are `scnDict`)* |
| `vdi_color_table`                  | `colorPens`   |
| `vdi_handle`                       | `physHandle`    |

#### Object animation tables (extras)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `object_fire_animation`            | `fireFrames`    |
| `object_dog_eating_animation`      | `bowlFrames`    |

#### Misc

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `sub_animation_frame_counter`      | `tickCount`    |
| `alarm_sound_started`              | `alarmSounding`    |
| `action_priority`                  | `cmdPriority`    |
| `mouse_off_flag`                   | `mouseHidden`     |

## Batch 5 conflicts / ambiguities noted

- Existing row `command_input_buffer -> parsedLine` is suspect:
  Ghidra `command_input_buffer` at 0x4d28a is the 64-byte char array,
  which the port declares as `char typedLine[64]`. The port's `parsedLine`
  is a `char *` (declared `char * parsedLine`) and most likely
  corresponds to Ghidra `command_input_ptr` at 0x2c6fc. Not touched
  per task rules -- a new `command_input_ptr -> parsedLine` row would
  create two Ghidra symbols renaming to `parsedLine`, which the rename
  script rejects.
- Ghidra `poker_blackjack_flag` (single BOOL @ 0x3d114) still cannot
  disambiguate port's `bjNatMain` / `bjNatSplit` (two flags). Also
  unresolved: `bjDblSplit`, `bjMatchBet`, `bjDblMain`, `bjBustMain`, `bjBustSplit`,
  `plyrRank` -- Ghidra symbol dump has no matching shapes.
- Ghidra `poker_computer_hand_value_lo/_hi`, `poker_card_deck_index`,
  `poker_display_x_offset`, `poker_round_count`, `poker_card_back_mfdb`
  still not paired to port shorts.
- Music-Studio-only MIDI shorts (`phraseBits`, `g_molof`, `g_msmap`,
  `songMaxPos`, `g_mccha`, `mi_nOS`, `ticksToNext`, `noteAccent`, `mi_lasT`,
  `queueLen`, `loopTop`) still unresolved -- Ghidra dump has no
  matching long-name shapes visible without decompiling the
  Music-Studio front-end (function names not preserved).
- Sprite descriptor arrays `spriteHeight`, `spriteWidth`, `spriteBitmap`, `spriteMask`,
  `g_setmt`, `g_setah`, `g_setaw` -- Ghidra addresses only carry
  raw `PTR_ARRAY_xxxxxx` / `SHORT_ARRAY_xxxxxx` labels, no long
  name to pair against.
- `stripStore`, `g_dsb`, `bshdbuf`, `hshdbuf`, `bitMask8`, `env_val`,
  `inEvent`, `vuLedMasks`, `footstepDue`, `g_hzhi`, `g_hzlo`, `sprFileBuf`,
  `letterWord`, `patSprites`, `stoveFrames`, `objMfdbs`, `moodPriority`,
  `copyProtResult` -- either port-only helpers or no matching-shape
  Ghidra long name in the dump.

Identity-name port shorts (already Ghidra-named the same; skipped
by the rename TSV generator but noted here for coverage):
`contrl`, `intin`, `intout`, `ptsin`, `ptsout`, `workin`,
`work_out`, `resX`, `resY`, `dogX`, `dogY`, `_vbclock`.

## OOB-audit log (2026-07-20)

Systematic sweep for `port_bytes < ROM_slot_bytes` AND code writes past
the port's declared end.  Covered `globals.c`, `sprglobs.c`,
`tick_tables.c`, `vocab.c`, then extended to `tables.c`, `sprload.c`,
`assets.c` plus BSS/partial arrays across other modules.

Real OOBs found and fixed in prior commits (all shape: port array
declared smaller than ROM slot):

- `scratchArr[4]` → `[10]` — action handlers wrote `scratchArr[4]`
- `cmdWord[32]` → `[42]` — `nextWord` copies up to 38 chars
- `compPile[26]` → `[52]` — `popCard` shift loop writes 50 bytes past end
- `plyrPile[26]` → `[52]` — same shift loop

Verified in this pass (safe, ROM has trailing pad or `sizeof`-cap):

- `genMaskBuf[14000]` — exact ROM match (`sprite_buffer` slot 0x36B0)
- `body_buf` bumped 20000 → 20160 to match ROM `body_lcp_file` slot
  exactly.  On-disk `BODY.LCP` is 16468 B so there was no truncation
  risk, but sizes now match Ghidra.
- `pex_buf` shrunk 12000 → 11088 to match ROM `pex_lcp_file` slot
  (0x4d2da → 0x4fe2a).
- `bodyImage[168]`, `bodyMask[168]`, `headImage[168]`, `headMask[168]` —
  `expandFrame` inner loop writes exactly 168 shorts; ROM 512/516 B pad
- `pexName[8]` — template filename; only 8 B ever touched
- `nibbleBytes[15]` — `letload.c` reads exactly 15 B
- `bitSet32[32]`, `bitClear32[32]`, `mirrorTable[256]` — all mask/index-bounded

Signature exhausted across all inspected modules; 0 remaining
CONFIRMED bugs in this class.

## How to extend this table

1. Pick a port function `foo()` whose Ghidra counterpart still exists
   (functions were renamed in a prior session, so use the port's
   name).
2. Run `mcp__ghidra__decompile_function(name="foo")`.
3. Every long identifier that is not a local (no declaration inside
   the function) is a global.  Match it to the port global that
   `foo()` in `source/*.c` accesses at the same position.
4. Add a row here.

## Recovered 2026-09-05 (address-keyed sync)

Pairs the map never carried.  Each was confirmed by ADDRESS -- the
port symbol and the Ghidra label occupy the same cell -- not by
guessing from the name.  Addresses come from byte-identical DATA/TEXT
and, for BSS, from `stx_bss_layout.tsv`.

**Co-location is not agreement.**  A row here says the two names are
the same cell; it does NOT say Ghidra's descriptive name is right.
Two in this batch are demonstrably wrong and are marked `(Ghidra
WRONG)` below -- the port name is the correct one and Ghidra has
already been renamed to it, so this table is the only place the bad
name survives:

  * `midi_channel_volume` -> **defProgMap** is the default PROGRAM map,
    not a volume table.  sendProgChange builds `midiMsg[0] = (chan) | 0xc0`,
    and 0xC0 is the MIDI Program Change status byte, so the following
    `midiMsg[1] = progMap[index]` is a program NUMBER.  unpackChanMap
    confirms the shape: it loads two parallel 15-entry tables out of
    the song header, the channel map from bytes 0..14 and the program
    map from 15..29.  Volume never enters it.
  * `midi_dma_start_lo` -> **bjPlyrScore** is the poker/blackjack
    PLAYER SCORE, compared against bjDealerScore all through games.c's
    hand comparison.  Nothing MIDI touches it.

Both are the same failure: a 1985 analyst naming an unlabelled cell
from its neighbourhood rather than its use.  Treat any Ghidra name in
this table as a lead, and check the use sites before adopting one.

| Ghidra long name                   | port short   |
| ---------------------------------- | ------------ |
| `PLAYER_STATE_ARRAY`                  | `scratchArr`      |
| `aes_addr_in`                         | `addr_in`      |
| `aes_addr_out`                        | `addr_ou`      |
| `aes_control`                         | `control`      |
| `aes_global`                          | `global`       |
| `aes_intO`                            | `int_out`      |
| `aes_int_in`                          | `int_in`       |
| `aes_params_ptr`                      | `ad_c`         |
| `bitmask_1_2_4_8_10_20_40_80`         | `mirrorDstBit`       |
| `bitmask_1_2_4_8_10_20_40_80_0`       | `bitMask8`        |
| `bitmask_32bit_and`                   | `bitClear32`      |
| `bitmask_32bit_or`                    | `bitSet32`       |
| `bitmask_80_40_20_10_8_4_2_1`         | `mirrorSrcBit`       |
| `body_ptr`                            | `body_pt`      |
| `body_shp`                            | `body_sh`      |
| `card_deck`                           | `bjKey`       |
| `ctrl_cnts`                           | `ctrl_cn`      |
| `days_in_month`                       | `daysPerMonth`     |
| `dest_scr_buffer`                     | `stripStore`     |
| `entered_word_bytes`                  | `phraseBits`        |
| `enteredword_to_action`               | `phraseTable`       |
| `enteredword_to_bit`                  | `wordBit`       |
| `footstep_trigger_flag`               | `footstepDue`       |
| `gSongMaxPosition_0`                  | `songMaxPos`      |
| `happiniess_to_priority`              | `moodPriority`     |
| `house_scene_size`                    | `scnSize`      |
| `in_execute_event_routine_flag`       | `inEvent`      |
| `mi_ntLp[25]`                         | `noteDur`      |
| `mi_ntLp[25]+2`                       | `noteAccent`      |
| `midi_channel_volume` (Ghidra WRONG)  | `defProgMap`     |
| `midi_dma_start_lo` (Ghidra WRONG)    | `bjPlyrScore`    |
| `object_alarm_animation`              | `alarmFrames`      |
| `object_clock_animation`              | `clockFrames`      |
| `object_phone_animation`              | `phoneFrames`      |
| `pblock`                              | `vdipb`        |
| `pex_ptr`                             | `pexName`     |
| `poker_card_back_mfdb`                | `plyrRank`      |
| `poker_card_deck_index`               | `bjNatSplit`      |
| `poker_computer_hand_value_hi`        | `bjBustSplit`       |
| `poker_computer_hand_value_lo`        | `bjBustMain`       |
| `poker_display_x_offset`              | `bjDealerScore`    |
| `poker_round_count`                   | `pkrRound`     |
| `psg_current_volume`                  | `noteVolume`     |
| `psg_freq_table`                      | `psgPeriod`     |
| `revert_table`                        | `mirrorTable`      |
| `room_position_x_table`               | `posXHalf`       |
| `scene_data_ptr`                      | `scnBuffer`      |
| `scn_cmn`                             | `scnDict`      |
| `screen_buffer_2`                     | `houseBuf`      |
| `soundfile_header`                    | `studioSig`       |
| `sprite_file_index_table`             | `spriteFileId`      |
| `sprites_files`                       | `sprFileBuf`     |
| `valid_word_table`                    | `vocabulary`      |
| `walk_target_x`                       | `walkXTarget`        |
| `walk_target_y`                       | `walkYTarget`        |
| `walk_waypoint_x`                     | `xWaypoint`        |
| `walk_waypoint_y`                     | `walkAdjust`      |
| `walk_waypoint_y`                     | `yWaypoint`        |
| `word_puzzle_player_answers`          | `wpzAnswers`       |
| `word_﻿entered_to_position`           | `wordByte`       |
| `work_out`                            | `wk_out`       |
| `workin`                              | `work_in`      |

## The globals with NO descriptive Ghidra name (2026-09-06)

Coverage was measured properly on 2026-09-06 and the old "~93
remaining" note was badly wrong -- it had been comparing lcp_sym.68k's
8-char TRUNCATED linkage names against this file's full names, so
`waterLevel`, `movingIn` and dozens like them counted as
unmapped when they are not.  Expanding the truncations first (and
dropping DRI libc and the AES library's own `gl_apid`) leaves
**thirteen** port globals with no descriptive counterpart:

| port      | what it is                                             |
|-----------|--------------------------------------------------------|
| `g_atact[16]` | action table, resident on HIGH activity -- see above |
| `g_atmod[16]` | action table, moderate activity                    |
| `g_atrel[16]` | action table, relaxed activity                     |
| `g_trel[10]`  | the triggered-event FIFO -- see above              |
| `g_rphs[48]`  | Y offset from the floor baseline per HOUSE_POS; the companion to `posXHalf`, and posToXY's `floor_y - posYOffset[i]` |
| `gr_hwchar`   | graf_handle's character cell width                 |
| `gr_hhchar`   | ...cell height                                     |
| `gr_hwbox`    | ...box width                                       |
| `gr_hhbox`    | ...box height.  LCP_STX's initAes has an empty frame because graf_handle writes all four straight to globals |
| `psg_epp[3]`  | pointers to the three PSG_ENVELOPE structs         |
| `psg_ovol`    | stepEnvelopes's clamped output volume                   |
| `psg_vrg[8]`  | PSG volume-register list `{8,0,9,0,10,0,-1,0}` -- registers 8/9/10 are the three channel volumes, -1 ends it |
| `g_unus3`     | `= -1`, referenced by NOTHING.  Same class as `studioSig` and `parseNumber`: a 1985 declaration that still costs its bytes |

These are not gaps to be filled by guessing.  Ghidra either shows a
placeholder (`PTR_ARRAY_xxx` / `SHORT_ARRAY_xxx`) at these addresses or
now carries the port's own name from the address-keyed sync, so there
is no 1985 descriptive name left to recover for them -- inventing one
and recording it here would manufacture provenance that does not
exist.  The `what it is` column is derived from use sites, which is
what this table should have been keyed on all along.
