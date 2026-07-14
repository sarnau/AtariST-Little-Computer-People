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
extern short    g_trel[];
extern BOOL16   in_execute_event_routine_flag;

/* ---- Command / AI state ----------------------------------------------- */
extern short    last_action;
extern short    g_trac;

/* ---- LCP position and world state ------------------------------------- */
extern short    lcp_x;
extern short    lcp_y;
extern short    g_lcldd;
extern short    copyprot_check_return;
extern short    game_speed_counter;

/* ---- Alarm / water state --------------------------------------------- */
extern BOOL16   ctrl_a_alarm_pressed_flag;
extern short    lcp_water_level;

/* ---- Command queue (populated by keyboard input) --------------------- */
extern short    g_aliss;
extern short    g_aqueu[];
extern short    g_apriq[];

/* ---- Head animation / sound cross-cutting ---------------------------- */
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;
extern short    g_hsfra;
extern long     g_sfret;
extern BOOL16   g_actif;
extern BOOL16   dog_pettable_flag;
extern short    g_wtx;
extern short    g_wty;
extern short    PLAYER_STATE_ARRAY[];

/* ---- Time-of-day globals used by check_for_any_action_triggers ------- */
/* (these are aliased into the PLAYER struct: lcp.lunch_hour etc.
   Nothing to declare here.) */

/* ---- Cross-file helpers ---------------------------------------------- */
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern void     a_getd();

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
extern short    g_obids;
extern short    g_obi07;
extern short    g_obi08;
extern short    g_obidf;
extern short    g_obi05;
extern short    g_obi06;
extern short    g_obicc;
extern short    g_obico;
extern short    g_obi02;
extern short    g_obipc;
extern short    g_obidt;
extern short    g_obi09;
extern short    g_obi10;
extern short    g_obiso;
extern short    g_obisa[];
extern short    g_obi15;
extern short    g_obi16;
extern short    g_obi17;

extern BOOL16   midi_is_playing;
extern short    dog_food_bowl_change;
extern short    g_sfplf;
extern short    g_sfpli;

/* ---- Leisure / music / fire globals ---------------------------------- */
extern BOOL16   g_rbact;
extern char *   midi_song_buffer;
extern short    org_song_file_count;
extern BOOL16   fire_active_flag;
extern short    fire_duration_countdown;
extern BOOL16   fire_extinguish_flag;
extern short    disable_key_input_flag;
extern short    text_scroll_timer;
extern short    g_srsdc;
extern short    g_cdibp;

/* ---- Letter subsystem ------------------------------------------------- */
extern char *   g_lttx;
extern char *   g_ltlp[];
extern char *   g_ltg[];
extern char *   month_name_table[];
extern short    g_ltcwt[];
extern char     g_ltscb[];
extern unsigned char compression_tokens[];

/* ---- Object IDs for closet + fireplace art --------------------------- */
extern short    g_obidc;
extern short    g_obi03;
extern short    g_obi04;
extern short    g_obifo;
extern short    g_obifa[];
extern short    g_obifc;
extern short    g_obi13;
extern short    g_obi14;
extern short    g_obi11;
extern short    g_obido;
extern short    g_obi12;
extern short    g_obibg;

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
extern void *   g_dscp;

/* ---- Palette state --------------------------------------------------- */
extern short    main_colorpalette[];
extern short    g_clcop[];
extern short    g_clcos[];
extern short    skin_color_palette[];

/* ---- MIDI sequencer state -------------------------------------------- */
extern BOOL16   g_molof;
extern BOOL16   midi_var_r;
extern short    g_mspha;
extern unsigned char *  midi_data_base_ptr;

/* ---- MIDI sequencer state ------------------------------------------- */
extern unsigned char *  midi_seq_position;
extern long             g_msmap;
extern long             midi_envelope_data_base;
extern short            midi_velocity;
extern short            midi_default_velocity;
extern short            psg_current_volume;
extern short            psg_default_volume;
extern short            g_mnevi;
extern short            g_mnevc;
extern short            g_mtspb;
extern short            midi_tempo;
extern short            aes_int_out[];

extern long             g_mtcou;
extern short            midi_direct_write_mode;
extern short            g_mtdiv;
extern short            g_mtpre;
extern short            g_medu;
extern short            midi_next_event_tick;
extern short            midi_last_processed_tick;
extern BOOL16           g_msmsa;

extern unsigned char    midi_channel_map[];
extern short            g_mcpro[];
extern short            midi_program_map[];
extern unsigned char    g_mstr[];
extern unsigned char    g_msmk[];
extern BOOL16           g_moen;
extern unsigned char    g_meve[];
extern long             g_momap;

/* ---- PSG channel state ---------------------------------------------- */
extern BOOL16           psg_output_enabled;
extern BOOL16           psg_notes_active;
extern unsigned char    psg_channel_notes[];
extern PSG_ENVELOPE     psg_envelope[];
extern unsigned short   psg_freq_table[];

extern short            envelope_val;                   /* transpose base */
extern char             g_mnlol;
extern char             g_mnhil;
extern short            g_mccha;

/* ---- SFX / Dosound state -------------------------------------------- */
extern short            g_sfcup;
extern short            g_sfddh;
extern short            g_sfddl;
extern long             g_sfHz2;
extern unsigned char *  midi_note_length_params[];
extern char             g_sfDoB[];

/* ---- Screen buffer state -------------------------------------------- */
extern void *   g_srlgb;
extern void *   save_logbase;
extern void *   g_srptr;
extern short *  g_dsb;

/* ---- Clock display ---------------------------------------------------- */
extern short    g_cmmin;
extern short    g_chhou;

/* ---- Sound-effect queue --------------------------------------------- */
extern BOOL16   g_sfacf;
extern short    g_sfcur;
extern short    g_sfdur;
extern short    g_sfdos;
extern short    g_sfdoc;
extern short    _soundeffect_priority_table[];

/* ---- Object / sprite backing storage (populated by asset loaders) --- */
extern unsigned char    objects_file[];
extern unsigned char    sprites_files[];
extern MFDB     g_obtmt[];
extern MFDB     g_setmt[];
extern short    g_obtaw[];
extern short    g_obtah[];
extern short    g_setaw[];
extern short    g_setah[];

/* Legacy pointer form retained for od_draw / render.c callers that
   walk the mfdb table by index arithmetic. */
extern void *   g_otmfd;
/* MFDB_screen_ptr now declared with the frame-timing MFDBs below. */

/* ---- Record player / letter needle state (packed inside letter subsys) */
extern short    g_ltlic;
extern short    g_ltpac;
extern unsigned short   _record_led_mask_table[];

/* ---- Clock hand endpoint tables ------------------------------------- */
extern short    g_cmmip[];
extern short    g_chhop[];

/* ---- Keyboard / command input --------------------------------------- */
extern BOOL16   game_input_mode_flag;
extern char     g_cdinb[];
extern BOOL16   food_delivery_available;
extern short    g_ptanf;

/* ---- Frame-timing counters ------------------------------------------ */
extern short    last_hz200;
extern long     last_vbclock;
extern void *   save_physbase;

/* ---- Screen MFDB descriptors ---------------------------------------- */
extern MFDB     g_srmfd;
extern MFDB     MFDB_screen_ptr;        /* alias with older name */
extern MFDB *   current_screen_mfdb;

/* ---- 200 Hz + VBL clock (host-side we roll these ourselves) --------- */
extern short    g_hzhi;
extern short    g_hzlo;
extern long     _vbclock;

/* ---- Dog wander behaviour ------------------------------------------- */
extern BOOL16   dog_visible;
extern short    dog_idle_countdown;
extern BOOL16   dog_near_food_bowl;
extern BOOL16   g_deact;
extern short    g_decou;
extern short    dog_last_target_index;
extern short    g_dseat[];
extern short    g_ddipt[];
extern short    g_ddxot[];
extern short    g_ddyot[];

/* ---- Command parser state ------------------------------------------- */
extern char *   _command_input_ptr;
extern short    g_aprio;

/* ---- Sprite MFDB arrays (one per hardware slot) --------------------- */
extern MFDB     g_semfi[];
extern MFDB     g_semfm[];

/* ---- TV animation coord tables ------------------------------------- */
extern short    g_tp0xc[];
extern short    g_tp0yc[];
extern short    g_tp1xc[];
extern short    g_tp1yc[];
extern short    g_tp2xc[];
extern short    g_tp2yc[];
extern short    g_tp3xc[];
extern short    g_tp3yc[];
extern short    g_tpcoi[];

/* ---- NLP parser tables (populated at runtime from vocabulary data)  */
extern unsigned char    g_ewb[];
extern char             _user_input_buffer[];
extern short            _happiness_to_priority[];
extern char *           valid_word_table[];
extern short            word__entered_to_position[];
extern short            g_ew2b[];
extern unsigned char    _bitmask_1_2_4_8_10_20_40_80_0[];
extern WORD_TO_ACTION   g_ew2a[];

/* ---- Mini-game state -------------------------------------------------- */
extern char *   g_agwb;
extern char *   g_wpdb;
extern short *  cards_data;

extern short    g_wpci;
extern short    g_agclc;
extern short    g_aggun;
extern short    g_agacu;
extern short    _anagram_clue_used_this_round;
extern short    g_agwol;
extern char     g_aginb[];
extern char     g_agorw[];
extern char     g_agscw[];
extern char *   g_agwgm[];

extern short    _poker_round_count;
extern BOOL16   poker_quit_flag;
extern short    g_pcmon;
extern short    g_ppmon;
extern short    g_ppppa;
extern short    g_pcbet;
extern short    g_ppbet;
extern short    poker_game_phase;
extern short    poker_draw_discard_flags[];
extern short    g_pcdrp[];
extern short    g_ppdrp[];

/* Card graphics: 54 MFDB descriptors covering 52 card faces + 1 shared
   back + 1 highlight overlay pattern, all sharing cards_data as their
   pixel storage.  MFDB_dest_screenbase_cards is a screen-buffer MFDB
   sized to the mini-game display area (320x77). */
extern MFDB     cards_MFDB_blocks[];
extern MFDB     MFDB_dest_screenbase_cards;

/* ---- Delivery / phone / petting flags -------------------------------- */
extern BOOL16   g_dvdog;
extern BOOL16   phone_hangup_flag;
extern BOOL16   g_ptdoa;

/* ---- Sprite head pipeline (defined in sprite_globals.c) --------------- */
extern short    g_hsbuf[];
extern short    g_hsmas[];
extern short    g_hsmif;
extern short *  pex_lcp_file;                   /* source head sheet */
extern short *  head_shape_data;                /* source head masks */
extern short    happiness_head_frame_offset[];
extern short    head_x_offset_per_state[];
extern short    head_height_per_state[];
extern short    head_default_angle_per_state[];
extern short    head_movement_delta_table[];
extern short    head_tilt_frame_offset[];
extern short    g_hadec;

/* ---- Bit-reverse LUT used by sp_lcpf / sp_flih - */
extern unsigned short   revert_table[];

/* ---- Walk-pathfinding state ------------------------------------------ */
extern short    g_wyx;
extern short    g_wyy;
extern short    lcp_on_stairs_flag;
extern BOOL16   footstep_trigger_flag;
extern short    g_hastl;
extern short    stair_top_y_threshold;
extern short    stair_bottom_y_threshold;

/* ---- Utility functions (implemented in movement.c etc) ---------------- */
extern void     house_get_position_xy();
extern short    get_floor_number_from_y();
extern short    calc_weekday();

/* ---- LCP animation state (defined in globals.c) ----------------------- */
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_lcyof;
extern short    g_lcieo;
extern short    g_lssh;
extern short    debug_hide_lcp_offscreen;

/* ---- Dog state -------------------------------------------------------- */
extern short    dog_x;
extern short    dog_y;
extern short    g_dtx;
extern short    g_dty;
extern short    g_dyx;
extern short    g_dyy;
extern short    g_dwanc;
extern short    g_dsid;
extern short    dog_on_stairs_flag;
extern short    dog_initialized;

/* ---- Hardware sprite double-buffer (8 slots) -------------------------- */
extern short    g_sepef[];
extern short *  g_sepim[];
extern short *  g_sepms[];
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_sepeh[];
extern short    g_sepew[];
extern short *  g_seaim[];
extern short *  g_seams[];
extern short    g_seacx[];
extern short    g_seacy[];
extern short    g_seach[];
extern short    g_seacw[];

/* ---- Sprite definition arrays (indexed by SPRITE_ID, 60 slots) -------- */
extern short *  g_sedim[];
extern short *  g_sedms[];
extern short    g_sedeh[];
extern short    g_sedew[];
extern short    g_selaf[];
extern short    g_seslm[];

/* ---- Body / carry frame tables (indexed by PLAYER_STATE) -------------- */
extern short    body_sprite_frame_table[];
extern short    carry_body_frame_table[];
extern short    body_y_offset_per_state[];

/* ---- LCP body / head buffers and file pointers ------------------------ */
extern short *  body_lcp_file;
extern short *  body_shape_data;
extern short    g_lsimg[];
extern short    g_lsmas[];

/* ---- Dog sprite tables ----------------------------------------------- */
extern short    g_dwanf[];
extern short *  dog_sprite_pointers[];
extern short *  dog_mask_pointers[];
extern short    g_dfimb[];
extern short    g_dfmab[];

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
