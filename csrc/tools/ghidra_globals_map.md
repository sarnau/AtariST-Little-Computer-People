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
| `soundeffect_playing_flag`          | `g_sfacf`    |
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
| `screen_mfdb` (backbuffer)         | `mf_scrp`    |

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

## How to extend this table

1. Pick a port function `foo()` whose Ghidra counterpart still exists
   (functions were renamed in a prior session, so use the port's
   name).
2. Run `mcp__ghidra__decompile_function(name="foo")`.
3. Every long identifier that is not a local (no declaration inside
   the function) is a global.  Match it to the port global that
   `foo()` in `csrc/*.c` accesses at the same position.
4. Add a row here.
