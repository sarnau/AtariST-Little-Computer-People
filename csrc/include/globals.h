/*
 * globals.h -- extern declarations for global game state.
 *
 * Definitions live in globals.c.  This header exposes only the globals
 * currently referenced by ported modules; new externs get added as
 * additional subsystems come online.  Names preserved from Ghidra so
 * decompiled-source cross-reference works one-to-one.
 *
 * addr: individual globals by their Ghidra symbol names.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
#include "structs.h"

/* ---- Timing ------------------------------------------------------------ */
extern short    animation_tick_counter;         /* 8Hz frame counter */
extern short    game_seconds_counter;           /* 0..59 game-seconds  */

/* ---- Clock ------------------------------------------------------------- */
extern short    time_minutes;
extern short    time_hours;
extern short    date_day;
extern short    date_month;
extern short    date_year;

/* ---- Character --------------------------------------------------------- */
extern PLAYER   lcp;                            /* the resident LCP */

/* ---- Event queue / flags ---------------------------------------------- */
extern BOOL16   phone_answered_flag;
extern BOOL16   phone_call_active_flag;
extern BOOL16   intro_sequence_active;

/* ---- Once-per-day action triggers (cleared by daily_reset_action_flags) */
extern BOOL16   lunch_meal_triggered_today;
extern BOOL16   dinner_meal_triggered_today;
extern BOOL16   morning_wakeup_triggered_today;
extern BOOL16   bedtime_triggered_today;

/* ---- Calendar table (defined in calendar.c) --------------------------- */
extern short    days_per_month[];

/* ---- Deferred event queue (defined in events.c) ----------------------- */
extern short    triggered_event_list[];
extern BOOL16   in_execute_event_routine_flag;

/* ---- Command / AI state ----------------------------------------------- */
extern short    last_action;
extern short    trigger_action;

/* ---- LCP position and world state ------------------------------------- */
extern short    lcp_x;
extern short    lcp_y;
extern short    lcp_loaded;
extern short    copyprot_check_return;
extern short    game_speed_counter;

/* ---- Alarm / water state --------------------------------------------- */
extern BOOL16   ctrl_a_alarm_pressed_flag;
extern short    lcp_water_level;

/* ---- Command queue (populated by keyboard input) --------------------- */
extern short    _action_list_size;
extern short    _action_queue[];
extern short    _action_priority_queue[];

/* ---- Head animation / sound cross-cutting ---------------------------- */
extern short    head_anim_target_state;
extern short    head_anim_current;
extern short    head_anim_mode;
extern short    head_sprite_frame;
extern long     soundeffect_remaining_ticks;
extern BOOL16   action_interruptible_flag;
extern BOOL16   dog_pettable_flag;
extern short    walk_target_x;
extern short    walk_target_y;
extern short    PLAYER_STATE_ARRAY[];

/* ---- Time-of-day globals used by check_for_any_action_triggers ------- */
/* (these are aliased into the PLAYER struct: lcp.lunch_hour etc.
   Nothing to declare here.) */

/* ---- Cross-file helpers ---------------------------------------------- */
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern void     action_get_dressed();

/* ---- Door / furniture runtime unpacked flags (mirror of the packed
   bit-field in lcp.door_states_and_flags; unpacked at load time and
   repacked at save time). --------------------------------------------- */
extern short    lcp_front_door_open;
extern short    lcp_study_door_open;
extern short    lcp_closet_door_open;
extern short    lcp_cabinet_open;
extern short    lcp_dresser_open;
extern short    lcp_toilet_door_open;
extern short    lcp_filing_cabinet_open;
extern short    lcp_dog_bowl_status;
extern short    lcp_food_count;
extern short    lcp_record_playing;
extern short    lcp_tv_on;

/* ---- Object IDs for door art (indexes into _object_images[]) --------- */
extern short    object_id_door_study_closed;
extern short    object_id_door_study_open_1;
extern short    object_id_door_study_open_2;
extern short    object_id_door_front_closed;
extern short    object_id_door_front_open_1;
extern short    object_id_door_front_open_2;
extern short    object_id_cabinet_closed;
extern short    object_id_cabinet_open_1;
extern short    object_id_cabinet_open_2;
extern short    object_id_phone_call;
extern short    object_id_door_toilet_closed;
extern short    object_id_door_toilet_open_1;
extern short    object_id_door_toilet_open_2;
extern short    object_id_stove_off;
extern short    object_id_stove_animation[];
extern short    object_id_fridge_closed;
extern short    object_id_fridge_open_1;
extern short    object_id_fridge_open_2;

extern BOOL16   midi_is_playing;
extern short    dog_food_bowl_change;
extern short    soundeffect_playing_flag;
extern short    soundeffect_playing_id;

/* ---- Leisure / music / fire globals ---------------------------------- */
extern BOOL16   record_browsing_active;
extern char *   midi_song_buffer;
extern short    org_song_file_count;
extern BOOL16   fire_active_flag;
extern short    fire_duration_countdown;
extern BOOL16   fire_extinguish_flag;
extern short    disable_key_input_flag;
extern short    text_scroll_timer;
extern short    screen_scroll_down_count;
extern short    command_input_buffer_pos;

/* ---- Letter subsystem ------------------------------------------------- */
extern char *   letter_txt_content;
extern char *   letter_line_ptr[];
extern char *   letter_greeting_table[];
extern char *   month_name_table[];
extern short    letter_char_width_table[];
extern char     letter_scratch_buffer[];
extern unsigned char compression_tokens[];

/* ---- Object IDs for closet + fireplace art --------------------------- */
extern short    object_id_door_closet_closed;
extern short    object_id_door_closet_open_1;
extern short    object_id_door_closet_open_2;
extern short    object_id_fireplace_off;
extern short    object_id_fireplace_animation[];
extern short    object_id_filing_cabinet_closed;
extern short    object_id_filing_cabinet_open_1;
extern short    object_id_filing_cabinet_open_2;
extern short    object_id_dresser_closed;
extern short    object_id_dresser_open_1;
extern short    object_id_dresser_open_2;
extern short    object_id_blue_green;

/* ---- Saved LCP body/head pointers for hide/show ---------------------- */
extern short *  saved_body_sprite_ptr;
extern short *  saved_head_sprite_ptr;

/* ---- VDI handle + color table (populated at graphics init) ----------- */
extern short    vdihandle;
extern short    _vdi_color_table[];

/* ---- VDI parameter block ------------------------------------------- */
/* Shared per-call scratch arrays that every VDI wrapper stuffs before
   calling the trap #2 dispatcher.  Sized to the GEM VDI ABI maxima. */
extern short    contrl[];
extern short    intin[];
extern short    ptsin[];
extern short    intout[];
extern short    ptsout[];
extern short *  vdipb[];

/* ---- Screen buffer pointers ------------------------------------------ */
extern void *   dest_screenbase_ptr;

/* ---- Palette state --------------------------------------------------- */
extern short    main_colorpalette[];
extern short    clothing_color_primary[];
extern short    clothing_color_secondary[];
extern short    skin_color_palette[];

/* ---- MIDI sequencer state -------------------------------------------- */
extern BOOL16   midi_song_loop_flag;
extern BOOL16   midi_var_r;
extern short    midi_seq_phase;
extern unsigned char *  midi_data_base_ptr;

/* ---- MIDI sequencer state ------------------------------------------- */
extern unsigned char *  midi_seq_position;
extern long             midi_seq_max_position;
extern long             midi_envelope_data_base;
extern short            midi_velocity;
extern short            midi_default_velocity;
extern short            psg_current_volume;
extern short            psg_default_volume;
extern short            midi_note_event_index;
extern short            midi_note_event_count;
extern short            midi_ticks_per_beat;
extern short            midi_tempo;
extern short            aes_int_out[];

extern long             midi_tick_counter;
extern short            midi_direct_write_mode;
extern short            midi_tick_divider;
extern short            midi_tick_prescaler;
extern short            midi_event_duration;
extern short            midi_next_event_tick;
extern short            midi_last_processed_tick;
extern BOOL16           midi_sequencer_active;

extern unsigned char    midi_channel_map[];
extern short            midi_current_program[];
extern short            midi_program_map[];
extern unsigned char    midi_scale_transpose_table[];
extern unsigned char    midi_scale_mask_table[];
extern BOOL16           midi_output_enabled;
extern unsigned char    midi_event[];
extern long             midi_song_max_position;

/* ---- PSG channel state ---------------------------------------------- */
extern BOOL16           psg_output_enabled;
extern BOOL16           psg_notes_active;
extern unsigned char    psg_channel_notes[];
extern PSG_ENVELOPE     psg_envelope[];
extern unsigned short   psg_freq_table[];

extern short            envelope_val;                   /* transpose base */
extern char             midi_note_lo_limit;
extern char             midi_note_hi_limit;
extern short            midi_current_channel;

/* ---- SFX / Dosound state -------------------------------------------- */
extern short            soundeffect_current_priority;
extern short            soundeffect_default_duration_hi;
extern short            soundeffect_default_duration_lo;
extern long             soundeffect_Hz200;
extern unsigned char *  midi_note_length_params[];
extern char             soundeffect_DoSound_Buffer[];

/* ---- Screen buffer state -------------------------------------------- */
extern void *   screen_logbase;
extern void *   save_logbase;
extern void *   screen_ptr;
extern short *  dest_scr_buffer;

/* ---- Clock display ---------------------------------------------------- */
extern short    clock_minute;
extern short    clock_hour;

/* ---- Sound-effect queue --------------------------------------------- */
extern BOOL16   soundeffect_active_flag;
extern short    soundeffect_current;
extern short    soundeffect_duration;
extern short    soundeffect_dosound_status;
extern short    soundeffect_dosound_control;
extern short    _soundeffect_priority_table[];

/* ---- Object / sprite backing storage (populated by asset loaders) --- */
extern unsigned char    objects_file[];
extern unsigned char    sprites_files[];
extern MFDB     object_tab_mfdb_table[];
extern MFDB     sprite_tab_mfdb_table[];
extern short    object_tab_width[];
extern short    object_tab_height[];
extern short    sprite_tab_width[];
extern short    sprite_tab_height[];

/* Legacy pointer form retained for object_draw / render.c callers that
   walk the mfdb table by index arithmetic. */
extern void *   object_tab_mfdb;
/* MFDB_screen_ptr now declared with the frame-timing MFDBs below. */

/* ---- Record player / letter needle state (packed inside letter subsys) */
extern short    letter_line_count;
extern short    letter_paragraph_count;
extern unsigned short   _record_led_mask_table[];

/* ---- Clock hand endpoint tables ------------------------------------- */
extern short    clock_minute_position[];
extern short    clock_hour_position[];

/* ---- Keyboard / command input --------------------------------------- */
extern BOOL16   game_input_mode_flag;
extern char     command_input_buffer[];
extern BOOL16   food_delivery_available;
extern short    petting_anim_frame;

/* ---- Frame-timing counters ------------------------------------------ */
extern short    last_hz200;
extern long     last_vbclock;
extern void *   save_physbase;

/* ---- Screen MFDB descriptors ---------------------------------------- */
extern MFDB     screen_mfdb;
extern MFDB     MFDB_screen_ptr;        /* alias with older name */
extern MFDB *   current_screen_mfdb;

/* ---- 200 Hz + VBL clock (host-side we roll these ourselves) --------- */
extern short    _hz_200_hi;
extern short    _hz_200_lo;
extern long     _vbclock;

/* ---- Dog wander behaviour ------------------------------------------- */
extern BOOL16   dog_visible;
extern short    dog_idle_countdown;
extern BOOL16   dog_near_food_bowl;
extern BOOL16   dog_eating_active;
extern short    dog_eating_countdown;
extern short    dog_last_target_index;
extern short    dog_sprite_eating_anim_tab[];
extern short    dog_destination_position_table[];
extern short    dog_dest_x_offset_table[];
extern short    dog_dest_y_offset_table[];

/* ---- Command parser state ------------------------------------------- */
extern char *   _command_input_ptr;
extern short    _action_priority;

/* ---- Sprite MFDB arrays (one per hardware slot) --------------------- */
extern MFDB     sprite_mfdb_image[];
extern MFDB     sprite_mfdb_mask[];

/* ---- TV animation coord tables ------------------------------------- */
extern short    tv_pattern_0_x_coords[];
extern short    tv_pattern_0_y_coords[];
extern short    tv_pattern_1_x_coords[];
extern short    tv_pattern_1_y_coords[];
extern short    tv_pattern_2_x_coords[];
extern short    tv_pattern_2_y_coords[];
extern short    tv_pattern_3_x_coords[];
extern short    tv_pattern_3_y_coords[];
extern short    tv_pattern_color_indices[];

/* ---- NLP parser tables (populated at runtime from vocabulary data)  */
extern unsigned char    _entered_word_bytes[];
extern char             _user_input_buffer[];
extern short            _happiness_to_priority[];
extern char *           valid_word_table[];
extern short            word__entered_to_position[];
extern short            _enteredword_to_bit[];
extern unsigned char    _bitmask_1_2_4_8_10_20_40_80_0[];
extern WORD_TO_ACTION   _enteredword_to_action[];

/* ---- Mini-game state -------------------------------------------------- */
extern char *   anagram_words_buffer;
extern char *   word_puzzle_data_buffer;
extern short *  cards_data;

extern short    word_puzzle_current_index;
extern short    anagram_clue_count;
extern short    anagram_guess_number;
extern short    anagram_all_clues_used;
extern short    _anagram_clue_used_this_round;
extern short    anagram_word_length;
extern char     anagram_input_buffer[];
extern char     anagram_original_word[];
extern char     anagram_scrambled_word[];
extern char *   anagram_wrong_guess_messages[];

extern short    _poker_round_count;
extern BOOL16   poker_quit_flag;
extern short    poker_computer_money;
extern short    poker_player_money;
extern short    poker_pot_amount;
extern short    poker_computer_bet;
extern short    poker_player_bet;
extern short    poker_game_phase;
extern short    poker_draw_discard_flags[];
extern short    poker_computer_draw_pile[];
extern short    poker_player_draw_pile[];

/* Card graphics: 54 MFDB descriptors covering 52 card faces + 1 shared
   back + 1 highlight overlay pattern, all sharing cards_data as their
   pixel storage.  MFDB_dest_screenbase_cards is a screen-buffer MFDB
   sized to the mini-game display area (320x77). */
extern MFDB     cards_MFDB_blocks[];
extern MFDB     MFDB_dest_screenbase_cards;

/* ---- Delivery / phone / petting flags -------------------------------- */
extern BOOL16   delivery_is_for_dog;
extern BOOL16   phone_hangup_flag;
extern BOOL16   petting_dog_active;

/* ---- Sprite head pipeline (defined in sprite_globals.c) --------------- */
extern short    head_sprite_buffer[];
extern short    head_sprite_mask[];
extern short    head_sprite_mirror_flag;
extern short *  pex_lcp_file;                   /* source head sheet */
extern short *  head_shape_data;                /* source head masks */
extern short    happiness_head_frame_offset[];
extern short    head_x_offset_per_state[];
extern short    head_height_per_state[];
extern short    head_default_angle_per_state[];
extern short    head_movement_delta_table[];
extern short    head_tilt_frame_offset[];
extern short    head_anim_delay_countdown;

/* ---- Bit-reverse LUT used by sprite_lcp_flip / sprite_flip_horizontal - */
extern unsigned short   revert_table[];

/* ---- Walk-pathfinding state ------------------------------------------ */
extern short    walk_waypoint_x;
extern short    walk_waypoint_y;
extern short    lcp_on_stairs_flag;
extern BOOL16   footstep_trigger_flag;
extern short    head_anim_state_last;
extern short    stair_top_y_threshold;
extern short    stair_bottom_y_threshold;

/* ---- Utility functions (implemented in movement.c etc) ---------------- */
extern void     house_get_position_xy();
extern short    get_floor_number_from_y();
extern short    calc_weekday();

/* ---- LCP animation state (defined in globals.c) ----------------------- */
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    lcp_carrying_object_flag;
extern short    lcp_carried_object;
extern short    lcp_sprites_hidden;
extern short    debug_hide_lcp_offscreen;

/* ---- Dog state -------------------------------------------------------- */
extern short    dog_x;
extern short    dog_y;
extern short    dog_target_x;
extern short    dog_target_y;
extern short    dog_waypoint_x;
extern short    dog_waypoint_y;
extern short    dog_walk_anim_cycle;
extern short    dog_sprite_id;
extern short    dog_on_stairs_flag;
extern short    dog_initialized;

/* ---- Hardware sprite double-buffer (8 slots) -------------------------- */
extern short    sprite_pending_flag[];
extern short *  sprite_pending_image[];
extern short *  sprite_pending_mask[];
extern short    sprite_pending_x[];
extern short    sprite_pending_y[];
extern short    sprite_pending_height[];
extern short    sprite_pending_width[];
extern short *  sprite_active_image[];
extern short *  sprite_active_mask[];
extern short    sprite_active_x[];
extern short    sprite_active_y[];
extern short    sprite_active_height[];
extern short    sprite_active_width[];

/* ---- Sprite definition arrays (indexed by SPRITE_ID, 60 slots) -------- */
extern short *  sprite_def_image[];
extern short *  sprite_def_mask[];
extern short    sprite_def_height[];
extern short    sprite_def_width[];
extern short    sprite_layer_flags[];
extern short    sprite_slot_map[];

/* ---- Body / carry frame tables (indexed by PLAYER_STATE) -------------- */
extern short    body_sprite_frame_table[];
extern short    carry_body_frame_table[];
extern short    body_y_offset_per_state[];

/* ---- LCP body / head buffers and file pointers ------------------------ */
extern short *  body_lcp_file;
extern short *  body_shape_data;
extern short    lcp_sprite_img[];
extern short    lcp_sprite_mask[];

/* ---- Dog sprite tables ----------------------------------------------- */
extern short    dog_walk_anim_frames[];
extern short *  dog_sprite_pointers[];
extern short *  dog_mask_pointers[];
extern short    dog_flip_image_buffer[];
extern short    dog_flip_mask_buffer[];

/* ---- Floor geometry (used by pathfinding) ---------------------------- */
extern short    floor_bottom_y_coords[];
extern short    floor_center_y_coords[];
extern short    staircase_waypoint_coords[];

/* ---- Tick-loop counters ----------------------------------------------- */
extern short    sub_animation_frame_counter;
extern short    animation_tick_counter;

/* ---- Externals implemented in other TUs (subset used by sim.c) -------- */
extern short    randomRange();                  /* random.c */
extern void     lcp_become_sick();              /* health.c  */
extern void     lcp_update_palette_colors();    /* render.c  */
extern void     daily_reset_action_flags();     /* ai.c      */
extern short    days_in_month();                /* calendar.c*/
extern void     put_event_to_list();            /* ai.c      */
extern short    get_event_from_list();          /* events.c  */
extern void     execute_event();                /* ai.c      */
extern void     check_for_any_action_triggers();/* ai.c      */
extern void     do_action();                    /* actions.c */

#endif  /* GLOBALS_H */
