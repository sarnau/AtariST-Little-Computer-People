# Ghidra long-name -> port short-name mapping (globals)

The port has to use short (<=8-char external) names because Alcyon C 4.14
truncates external symbols to 8 characters.  Ghidra's LCP.PRG database
uses long descriptive names.  This file lists the correspondence for
cross-referencing decompiler output against port source.

## Status: unable to auto-rename Ghidra globals

The GhidraMCP surface exposes no endpoint to rename data (global-variable)
symbols by address.  Available rename endpoints are:

- `mcp__ghidra__rename_function_by_address` -- functions only
- `mcp__ghidra__rename_variables` -- **locals only**; verified against
  `midi_tick_counter` inside `mq_tick`: the call reported
  `variables_renamed: 0, variables_failed: 0` because it only enumerated
  the function's local variables, not global references in the
  decompilation.

Ghidra scripting (`save_ghidra_script` writes to
`~/ghidra_scripts/`) has no matching *execute* endpoint over MCP, so a
Java `SymbolTable`-walking rename script cannot be triggered from the
port session.

Consequence: bulk renaming Ghidra's globals to the port's short names
requires either
  (a) a new MCP endpoint (`rename_data_by_address`), or
  (b) running the saved script `list_data_symbols.java` from Ghidra's
      Script Manager manually, then feeding a rename script the same
      way.

Until then the table below serves as the cross-reference.

## Address mismatch caveat

`csrc/tools/find_syms.py` addresses cover the **port's rebuilt PRG**
(different .o layout, different link order) -- they do NOT map to the
Ghidra project's addresses (which loaded the **original 1985 LCP.PRG**
at base 0x0).  Matching between the two is by *role* and *access
pattern* in decompiled code, not by address.

## Confirmed mappings

Derived from decompiling: `mq_tick`, `mq_advs`, `psg_upEn`, `psg_wr`,
`fl_ltpl`, `a_watat`, `chk_actT`, `chk_timA`, `sf_irqp`, `gameSim1`,
`cl_drwH`, `sp_draw`, `ag_intr`.

### Time / calendar / animation

| Ghidra                            | Port         |
|-----------------------------------|--------------|
| `animation_tick_counter`          | `ani_cnt`    |
| `game_seconds_counter`            | `g_secs`     |
| `time_minutes`                    | `t_min`      |
| `time_hours`                      | `t_hour`     |
| `date_day`                        | `date_day`   |
| `date_month`                      | `dt_mon`     |
| `date_year`                       | `dt_year`    |

### Player / AI state

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `lcp` (PLAYER struct)              | `lcp`        |
| `lcp_state`                        | `lcp_st`     |
| `lcp_facing_direction`             | `lcp_face`   |
| `lcp_filing_cabinet_open`          | `lcp_flcO`   |
| `lcp_water_level`                  | `lcp_watr`   |
| `last_action`                      | `lastAct`    |
| `trigger_action`                   | `g_trac`     |
| `intro_sequence_active`            | `introSeq`   |
| `phone_answered_flag`              | `ph_ans`     |
| `phone_call_active_flag`           | `ph_call`    |
| `ctrl_a_alarm_pressed_flag`        | `alarm_p`    |
| `lunch_meal_triggered_today`       | `lunT_trg`   |
| `dinner_meal_triggered_today`      | `dinT_trg`   |
| `morning_wakeup_triggered_today`   | `wkT_trg`    |
| `bedtime_triggered_today`          | `bedT_trg`   |
| `_action_queue[]`                  | `g_aqueu[]`  |
| `_action_priority_queue[]`         | `g_apriq[]`  |
| `_action_list_size`                | `g_aliss`    |

### Action tables (tick_tables)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `action_table_active[]`            | `g_obala[]`  |
| `action_table_moderate[]`          | `g_obcla[]`  |
| `action_table_relaxed[]`           | `g_obpha[]`  |
| `activity_schedule_table[]`        | `g_cotbl[]`  |
| `triggered_event_list[]`           | `pst_arr[]`  |

(action_table_* assignment to `g_ob{ala,cla,pha}` is by role;
`g_obfia[]` and `g_obdea[]` may match fire/dead tables not yet
sampled -- verify when decompiling `chk_encm` or fire/emergency
handlers.)

### MIDI sequencer

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `midi_is_playing`                  | `mi_play`    |
| `midi_tick_counter`                | `g_mtcou`    |
| `midi_tick_prescaler`              | `g_mtspb`    |
| `midi_tick_divider`                | `g_mtdiv`    |
| `midi_direct_write_mode`           | `mi_dwrm`    |
| `midi_reentrant_lock`              | `mi_rlock`   |
| `midi_sequencer_active`            | `g_msmsa`    |
| `midi_seq_phase`                   | `g_mspha`    |
| `midi_event_duration`              | `g_medu`     |
| `midi_next_event_tick`             | `mi_nxTk`    |
| `midi_last_processed_tick`         | `mi_lpTk`    |
| `midi_note_event_index`            | `g_mnevi`    |
| `midi_note_length_params[]`        | `mi_ntLp[]`  |
| `aes_int_out[]`                    | `aes_intO[]` |

### PSG / envelope

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `psg_notes_active`                 | `psg_ntAc`   |
| `psg_envelope[]`                   | `psg_envelope[]` |
| `psg_channel_ramp_accum[]`         | `psg_rmpA[]` |
| `psg_channel_ramp_delta[]`         | `psg_rmpD[]` |
| `psg_register_offset_table[]`      | `psg_rot[]`  |
| `psg_output_volume` (working reg)  | `psg_cvol`   |

### Sound effects (`sf_irqp`)

| Ghidra                              | Port         |
|-------------------------------------|--------------|
| `soundeffect_active_flag`           | `g_sfacf`    |
| `soundeffect_playing_flag`          | `g_sfplf`    |
| `soundeffect_current`               | `g_sfcur`    |
| `soundeffect_current_priority`      | `g_sfcup`    |
| `soundeffect_playing_id`            | `g_sfpli`    |
| `soundeffect_default_duration_hi`   | `g_sfddh`    |
| `soundeffect_default_duration_lo`   | `g_sfddl`    |
| `soundeffect_Hz200`                 | `g_sfHz2`    |
| `soundeffect_remaining_ticks`       | `g_sfret`    |
| `soundeffect_duration`              | `g_sfdur`    |
| `soundeffect_DoSound_Buffer[]`      | `g_sfDoB[]`  |
| `_soundeffect_priority_table[]`     | `sf_pri[]`   |

### Sprite render

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `sprite_mfdb_image[]`              | `g_semfi[]`  |
| `sprite_mfdb_mask[]`               | `g_semfm[]`  |
| `sprite_active_image[]`            | `g_seaim[]`  |
| `sprite_active_mask[]`             | `g_seams[]`  |
| `sprite_active_width[]`            | `g_seacw[]`  |
| `sprite_active_height[]`           | `g_seach[]`  |

### Letter / clock

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `letter_txt_content`               | `g_lttx`     |
| `letter_line_ptr[]`                | `g_ltlp[]`   |
| `clock_minute_position[]`          | `g_cmmip[]`  |
| `clock_hour_position[]`            | `g_chhop[]`  |

### VDI plumbing

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `vdihandle`                        | `vdihnd`     |
| `screen_mfdb` (compositing target) | `g_srmfd`    |
| `MFDB_screen_ptr` (source screen)  | `mf_scrp`    |
| `screen_scale_factor`              | `scr_scal`   |

### Dog AI

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `dog_pettable_flag`                | `dg_petok`   |
| `dog_idle_countdown`               | `dg_idlcd`   |
| `dog_food_bowl_change`             | `dg_bwlch`   |
| `dog_near_food_bowl`               | `dg_nrbwl`   |
| `dog_on_stairs_flag`               | `dg_stair`   |
| `dog_visible`                      | `dg_vis`     |
| `dog_initialized`                  | `dg_init`    |
| `dog_last_target_index`            | `dg_ltgtI`   |
| `dog_initial_target_index`         | `g_dgitx`    |
| `dog_initial_y_offset`             | `g_dgiyo`    |
| `dog_destination_position_table`   | `g_ddipt`    |
| `dog_dest_x_offset_table`          | `g_ddxot`    |
| `dog_dest_y_offset_table`          | `g_ddyot`    |
| `dog_eating_active`                | `g_deact`    |
| `dog_eating_countdown`             | `g_decou`    |
| `dog_flip_image_buffer`            | `g_dfimb`    |
| `dog_flip_mask_buffer`             | `g_dfmab`    |
| `dog_sprite_eating_anim_tab`       | `g_dseat`    |
| `dog_sprite_id`                    | `g_dsid`     |
| `dog_target_x`                     | `g_dtx`      |
| `dog_target_y`                     | `g_dty`      |
| `dog_walk_anim_cycle`              | `g_dwanc`    |
| `dog_walk_anim_frames`             | `g_dwanf`    |
| `delivery_is_for_dog`              | `g_dvdog`    |

### Head / body / stair

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `head_sprite_buffer`               | `g_hsbuf`    |
| `head_sprite_frame`                | `g_hsfra`    |
| `head_sprite_mask`                 | `g_hsmas`    |
| `head_sprite_mirror_flag`          | `g_hsmif`    |
| `head_anim_current`                | `g_hacur`    |
| `head_anim_target_state`           | `g_hatas`    |
| `head_anim_mode`                   | `g_hamod`    |
| `head_anim_delay_countdown`        | `g_hadec`    |
| `head_anim_state_last`             | `g_hastl`    |
| `head_height_per_state`            | `hd_hgt`     |
| `head_x_offset_per_state`          | `hd_xoff`    |
| `head_default_angle_per_state`     | `hd_dang`    |
| `head_movement_delta_table`        | `hd_mvd`     |
| `head_tilt_frame_offset`           | `hd_tilt`    |
| `head_shape_data`                  | `hd_shp`     |
| `body_shape_data`                  | `body_shp`   |
| `body_sprite_frame_table`          | `body_frT`   |
| `body_y_offset_per_state`          | `body_yof`   |
| `happiness_head_frame_offset`      | `mood_hfo`   |
| `staircase_waypoint_coords`        | `stair_wp`   |
| `stair_top_y_threshold`            | `stair_ty`   |
| `stair_bottom_y_threshold`         | `stair_by`   |

### Anagram game

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `anagram_all_clues_used`           | `g_agacu`    |
| `anagram_clue_count`               | `g_agclc`    |
| `anagram_guess_prompt_strings`     | `g_aggpr`    |
| `anagram_guess_number`             | `g_aggun`    |
| `anagram_input_buffer`             | `g_aginb`    |
| `anagram_original_word`            | `g_agorw`    |
| `anagram_scrambled_word`           | `g_agscw`    |
| `anagram_words_buffer`             | `g_agwb`     |
| `anagram_wrong_guess_messages`     | `g_agwgm`    |
| `anagram_word_length`              | `g_agwol`    |
| `anagram_clue_used_this_round`     | `ag_clue`    |

### Word puzzle

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `word_puzzle_blank_count`          | `wp_blk`     |
| `word_puzzle_failure_messages`     | `wp_fail`    |
| `word_puzzle_prompt_messages`      | `wp_prm`     |
| `word_puzzle_success_messages`     | `wp_succ`    |
| `word_puzzle_current_index`        | `g_wpci`     |
| `word_puzzle_data_buffer`          | `g_wpdb`     |

### Poker / War

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `poker_bet_amount`                 | `pk_bet`     |
| `poker_computer_bluff_flag`        | `pk_bluff`   |
| `poker_computer_card_count`        | `pk_ccc`     |
| `poker_computer_hand`              | `pk_ch`      |
| `poker_computer_hand_rank`         | `pk_chrk`    |
| `poker_war_computer_score`         | `pk_cscore`  |
| `poker_computer_war_cards`         | `pk_cwc`     |
| `poker_discard_count`              | `pk_disc`    |
| `poker_discard_pile`               | `pk_dpile`   |
| `poker_deck_position`              | `pk_dpos`    |
| `poker_card_display_slot`          | `pk_dslot`   |
| `poker_hand_rank_flags`            | `pk_hrf`     |
| `poker_hand_suit_flags`            | `pk_hsf`     |
| `poker_computer_passed`            | `pk_pass`    |
| `poker_player_card_count`          | `pk_pcc`     |
| `poker_player_hand`                | `pk_ph`      |
| `poker_game_phase`                 | `pk_phase`   |
| `poker_player_hand_rank_flags`     | `pk_phrf`    |
| `poker_player_hand_suit_flags`     | `pk_phsf`    |
| `poker_player_hand_value`          | `pk_phv`     |
| `poker_player_split_card_count`    | `pk_pscc`    |
| `poker_war_player_score`           | `pk_pscore`  |
| `poker_player_split_hand`          | `pk_psh`     |
| `poker_player_war_cards`           | `pk_pwc`     |
| `poker_quit_flag`                  | `pk_quit`    |
| `poker_raise_message`              | `pk_rm`      |
| `poker_war_round`                  | `pk_round`   |
| `poker_card_selected`              | `pk_sel`     |
| `poker_take_cards_message`         | `pk_tcm`     |
| `poker_computer_hand_cards`        | `g_pchc`     |
| `poker_computer_bet`               | `g_pcbet`    |
| `poker_computer_draw_pile`         | `g_pcdrp`    |
| `poker_computer_money`             | `g_pcmon`    |
| `poker_player_bet`                 | `g_ppbet`    |
| `poker_player_draw_pile`           | `g_ppdrp`    |
| `poker_player_money`               | `g_ppmon`    |

### Cards

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `cards_data`                       | `crd_dat`    |
| `cards_MFDB_blocks`                | `crd_mfdb`   |
| `cards_x_pos_a`                    | `crd_xa`     |
| `cards_x_pos_b`                    | `crd_xb`     |
| `cards_y_pos_a`                    | `crd_ya`     |
| `cards_y_pos_b`                    | `crd_yb`     |

### Letter (extended)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `letter_line_count`                | `g_ltlic`    |
| `letter_paragraph_count`           | `g_ltpac`    |
| `letter_char_width_table`          | `g_ltcwt`    |
| `letter_greeting_table`            | `g_ltg`      |

### Save / load

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `save_physbase`                    | `sv_phb`     |
| `save_logbase`                     | `sv_lgb`     |
| `saved_body_sprite_ptr`            | `sv_bodyP`   |
| `saved_head_sprite_ptr`            | `sv_headP`   |
| `saved_vqt_attr`                   | `sv_vqta`    |
| `lcp_loaded`                       | `g_lcldd`    |

### Fire event

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `fire_active_flag`                 | `fire_act`   |
| `fire_duration_countdown`          | `fire_dur`   |
| `fire_extinguish_flag`             | `fire_ext`   |

### TV patterns

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `tv_pattern_0_x_coords`            | `g_tp0xc`    |
| `tv_pattern_0_y_coords`            | `g_tp0yc`    |
| `tv_pattern_1_x_coords`            | `g_tp1xc`    |
| `tv_pattern_1_y_coords`            | `g_tp1yc`    |
| `tv_pattern_2_x_coords`            | `g_tp2xc`    |
| `tv_pattern_2_y_coords`            | `g_tp2yc`    |
| `tv_pattern_3_x_coords`            | `g_tp3xc`    |
| `tv_pattern_3_y_coords`            | `g_tp3yc`    |
| `tv_pattern_color_indices`         | `g_tpcoi`    |

### Sprite engine (extended)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `sprite_active_x`                  | `g_seacx`    |
| `sprite_active_y`                  | `g_seacy`    |
| `sprite_pending_flag`              | `g_sepef`    |
| `sprite_pending_width`             | `g_sepew`    |
| `sprite_pending_x`                 | `g_sepex`    |
| `sprite_pending_y`                 | `g_sepey`    |
| `sprite_pending_image`             | `g_sepim`    |
| `sprite_pending_mask`              | `g_sepms`    |
| `sprite_pending_height`            | `g_sepeh`    |
| `sprite_slot_map`                  | `g_seslm`    |
| `sprite_layer_flags`               | `g_selaf`    |

### Object tables

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `object_tab_mfdb`                  | `g_otmfd`    |
| `object_tab_width`                 | `g_obtaw`    |
| `object_tab_height`                | `g_obtah`    |
| `objects_file`                     | `obj_file`   |
| `action_interruptible_flag`        | `g_actif`    |

### Clock / phone / misc

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `clock_minute`                     | `g_cmmin`    |
| `clock_hour`                       | `g_chhou`    |
| `phone_ring_countdown`             | `g_phrc`     |
| `phone_hangup_flag`                | `ph_hu`      |
| `record_browsing_active`           | `g_rbact`    |
| `food_delivery_available`          | `food_dlv`   |

### Init / palette / parser / debug

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `copyprot_check_return`            | `cprot_r`    |
| `game_speed_counter`               | `g_spdc`     |
| `midi_noteon_state`                | `mi_noSt`    |

### (subsystem line placeholder — do not remove)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `main_colorpalette`                | `main_pal`   |
| `skin_color_palette`               | `skin_pal`   |
| `month_name_table`                 | `mo_names`   |
| `pex_lcp_ptr`                      | `pex_ptr`    |
| `pex_lcp_file`                     | `pex_name`   |
| `sng_song_file_count`              | `sng_cnt`    |
| `org_song_file_count`              | `org_cnt`    |
| `input_string`                     | `in_str`     |
| `command_input_buffer`             | `cmd_inp`    |
| `debug_hide_lcp_offscreen`         | `dbg_hide`   |
| `text_scroll_timer`                | `tx_sctm`    |
| `last_hz200`                       | `last_hz`    |
| `last_vbclock`                     | `last_vbc`   |

## Ghidra names seen but role not yet mapped to a port global

`giselect`, `giwrite` -- PSG hardware registers (0xFF8800/0x8802), not
port globals (they are direct memory-mapped I/O in `psg_io.c`).
`isra` -- MFP interrupt-service register byte; not a port global.

## Port globals not yet paired with a Ghidra long name

Every port global not in the tables above; still to be paired by
sampling more decompilations.  Priority modules to sample next:
`csrc/dog.c` (dog AI, `g_dtx`/`g_dwanc`/`g_dsid`), `csrc/save.c`
(`sv_bodyP`, `sv_headP`, `sv_phb`, `sv_lgb`), `csrc/games.c`
(poker `pk_*` block, anagram `ag_*` block), `csrc/parser.c`
(`comp_tok`, `in_str`, `cmd_inp`), `csrc/letload.c`
(`g_ltcwt`, `g_ltscb`), `csrc/render.c` (compositor `tx_sctm`,
`scr_scal`, `MFDB_A`, `scrbufA`/`scrbufB`).

### Batch 3 additions

Derived from decompiling `sc_ren8`, `sp_updb`, `sp_lchu`, `sp_draw`,
`fillTopR`, `od_draw`, `dg_wkPth`, `dg_mvAni`, `mq_parh`, `psg_wr`,
`psg_mix`, `psg_cpE`, `psg_upEn`, `mowrit`.

### Screen buffers / render targets

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `SCREEN_BUFFER_A`                  | `scrbufA`    |
| `SCREEN_BUFFER_B`                  | `scrbufB`    |
| `screen_ptr`                       | `g_srptr`    |
| `screen_logbase`                   | `g_srlgb`    |
| `screen_scroll_down_count`         | `g_srsdc`    |
| `dest_screenbase_ptr`              | `g_dscp`     |
| `current_screen_mfdb`              | `cur_mf`     |
| `MFDB_dest_screenbase_cards`       | `mf_scb_c`   |

### LCP sprite render (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `body_lcp_file`                    | `body_ptr`   |
| `lcp_sprite_img`                   | `g_lsimg`    |
| `lcp_sprite_mask`                  | `g_lsmas`    |
| `lcp_carrying_object_flag`         | `g_lcyof`    |
| `lcp_sprites_hidden`               | `g_lssh`     |
| `lcp_dog_bowl_status`              | `lcp_bwlS`   |
| `carry_body_frame_table`           | `cy_frT`     |

### Dog waypoint / floor (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `dog_waypoint_x`                   | `g_dyx`      |
| `dog_waypoint_y`                   | `g_dyy`      |
| `floor_bottom_y_coords`            | `flr_by`     |
| `floor_center_y_coords`            | `flr_cy`     |

### MIDI sequencer (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `midi_channel_count`               | `g_mchcn`    |
| `midi_current_channel`             | `mi_ccha`    |
| `midi_current_program`             | `g_mcpro`    |
| `midi_current_note`                | `mi_cnot`    |
| `midi_data_ptr`                    | `mi_dptr`    |
| `midi_data_base_ptr`               | `mi_dbase`   |
| `midi_default_velocity`            | `mi_dvel`    |
| `midi_velocity`                    | `mi_vel`     |
| `midi_tempo`                       | `mi_temp`    |
| `midi_note_duration_table`         | `mi_ndt`     |
| `midi_var_r`                       | `mi_varR`    |
| `midi_saved_timer_vector`          | `mi_svtv`    |
| `midi_song_buffer`                 | `mi_sbuf`    |
| `midi_song_loop_flag`              | `mi_slop`    |
| `midi_seq_position`                | `mi_sqpos`   |
| `midi_note_event_queue`            | `mi_evq`     |
| `midi_note_event_count`            | `g_mnevc`    |
| `midi_note_on_flag`                | `mi_nnOn`    |
| `midi_note_off_flag`               | `mi_nnOf`    |
| `midi_note_mode_flags`             | `mi_nmof`    |
| `midi_note_hi_limit`               | `g_mnhil`    |
| `midi_note_lo_limit`               | `g_mnlol`    |
| `midi_output_enabled`              | `g_moen`     |
| `midi_program_map`                 | `mi_pgmap`   |
| `midi_channel_map`                 | `mi_chmap`   |
| `midi_scale_mask_table`            | `g_msmk`     |
| `midi_scale_transpose_table`       | `g_mstr`     |
| `midi_event_type_flag`             | `mi_evTf`    |
| `midi_loop_stack`                  | `mi_lstk`    |
| `midi_event`                       | `g_meve`     |

### PSG (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `psg_channel_notes`                | `psg_chNt`   |
| `psg_default_volume`               | `psg_dvol`   |
| `psg_output_enabled`               | `psg_out`    |

### Sound effects (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `soundeffect_dosound_status`       | `g_sfdos`    |
| `soundeffect_dosound_control`      | `g_sfdoc`    |

### Poker (extra)

| Ghidra                             | Port         |
|------------------------------------|--------------|
| `poker_bet_message`                | `pk_bm`      |

## Batch 3 conflicts / ambiguities noted

- `MFDB_screen_ptr` (Ghidra) appears to be the port's `mf_scrp`, based
  on `blkcp32` argument order in `sc_ren8` vs `renderf.c`. Existing
  row `screen_mfdb -> mf_scrp` looks reversed: my read is
  `screen_mfdb -> g_srmfd` and `MFDB_screen_ptr -> mf_scrp`.
  Left existing row alone per task instructions.
- `soundeffect_active_flag` (0x54010) is referenced by `sc_ren8` for
  the post-render play flag reset; port uses `g_sfacf` there.
  Existing row maps `soundeffect_playing_flag -> g_sfacf`, which
  looks like the wrong pairing (`playing_flag` is at 0x5a2ca and
  probably matches port `g_sfplf`). Not touched.
- Poker `pk_bs1 / pk_bs2 / pk_c1bj / pk_c2bj / pk_wcs / pk_wpr /
  pk_wrf / pk_dsc / pk_phrk / g_ppppa` still unresolved -- Ghidra
  has no matching-shaped long names in symbol dump; needs a
  decompile of the poker/blackjack game function (function names
  are not preserved in Ghidra either -- no `poker_main`/
  `poker_blackjack_main`/`wp_intr` symbols were found).
- Word puzzle: `wp_*` shorts (`wp_blk/prm/succ/fail`) are already
  mapped; `word_puzzle_player_answers` and
  `word_puzzle_current_index / _data_buffer / _blank_count` are
  mapped; no additional port shorts remain.
- Ghidra `midi_ticks_per_beat`, `midi_seq_max_position`,
  `midi_envelope_data_base`, `midi_duration_scale`,
  `midi_noteon_state`, `midi_dma_start_lo`, `midi_channel_volume`
  seen but port shorts (`g_mtpre`, `mi_evi`, `mi_env`, `mi_evrl`,
  `mi_evrt`, `mi_evst`, `mi_evtt`, `mi_evcn`, `mi_lasT`, `mi_nOS`,
  `mi_nlp0`, `mi_nlpA`, `mi_seqE`) not confidently pairable from
  name alone -- needs decompile of `mq_advs` / `mq_tick` internals.
- Ghidra `poker_computer_hand_value_lo/hi`, `poker_card_deck_index`,
  `poker_pot_amount`, `poker_display_x_offset`, `poker_round_count`,
  `poker_card_back_mfdb`, `poker_draw_discard_flags` seen but port
  shorts uncertain.

### Port shorts still unpaired (candidates for future decompile passes)

- Music Studio / MIDI: `g_ewb`, `g_molof`, `g_msmap`, `mi_nOS`,
  `mi_nlp0`, `mi_nlpA`, `mi_lasT`, `mi_seqE`, `mi_evi`, `mi_evrl`,
  `mi_evrt`, `mi_evst`, `mi_evtt`, `mi_evcn`, `mi_env`, `mg_tofl`,
  `mood_pri`, `moff_f`.
- Screen/render extras: `bshdbuf`, `hshdbuf`, `hs_size`, `g_dsb`
  (may be dead; comment says former alias of `g_srptr - 254`),
  `g_spdc`, `g_sedeh`, `g_sedew`, `g_sedim`, `g_sedms`, `g_setmt`,
  `g_setah`, `g_setaw`, `g_srmfd` (see conflict note above).
- Letter/clock: `g_ltscb`, `g_clcop`, `g_clcos`, `g_cdibp`,
  `g_cdinb`, `g_ptanf`, `g_ptdoa`, `g_ptdsi`, `g_ptlss`.
- Poker war: `pk_wcs`, `pk_wpr`, `pk_wrf`, `pk_bs1`, `pk_bs2`,
  `pk_c1bj`, `pk_c2bj`, `pk_dsc`, `pk_phrk`, `g_ppppa`.
- Misc: `env_val`, `no_keyin`, `in_evrt`, `rec_ledt`, `studyDrO`,
  `subAniC`, `cprot_r`, `dsb_stor`, `usr_buf`, `fs_trg`, `bm_lo`,
  `g_aprio`, `g_alsts`, `g_obisa`, `g_obtmt`, `g_lcieo`,
  `lcp_stR`, `lcp_recP`, `lcp_tv`.

## How to extend this table

1. Pick a port function `foo()` whose Ghidra counterpart still exists
   (functions were renamed in a prior session, so use the port's
   name).
2. Run `mcp__ghidra__decompile_function(name="foo")`.
3. Every long identifier that is not a local (no declaration inside
   the function) is a global.  Match it to the port global that
   `foo()` in `csrc/*.c` accesses at the same position.
4. Add a row here.
