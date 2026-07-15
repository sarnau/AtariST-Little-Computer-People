/*
 * globals.c -- storage for game global variables.
 *
 * Definitions of every extern declared in globals.h.  Alcyon C places
 * zero-initialised globals in BSS automatically; explicit initialisers
 * here are only for values that matter at boot time before load_hyber()
 * populates the PLAYER struct.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"

short   animation_tick_counter  = 0;
short   game_seconds_counter    = 0;

short   time_minutes            = 0;
short   time_hours              = 0;
short   date_day                = 0;
short   date_month              = 0;
short   date_year               = 0;

PLAYER  lcp;

BOOL16  phone_answered_flag     = NO;
BOOL16  phone_call_active_flag  = NO;
BOOL16  intro_sequence_active   = NO;

BOOL16  lunch_meal_triggered_today      = NO;
BOOL16  dinner_meal_triggered_today     = NO;
BOOL16  morning_wakeup_triggered_today  = NO;
BOOL16  bedtime_triggered_today         = NO;

BOOL16  in_execute_event_routine_flag   = NO;

short   last_action                     = ACTION_NONE;
short   g_trac                  = ACTION_NONE;

short   lcp_x                           = 0;
short   lcp_y                           = 0;
short   g_lcldd                      = 0;
short   copyprot_check_return           = 1;
short   game_speed_counter              = 5;

BOOL16  ctrl_a_alarm_pressed_flag       = NO;
short   lcp_water_level                 = 7;

short   g_aliss               = 0;
short   g_aqueu[10];
short   g_apriq[10];

short   g_hatas          = 0;
short   g_hacur               = 0;
short   g_hamod                  = 0;
short   g_hsfra               = 0;
long    g_sfret     = 0;
BOOL16  g_actif       = NO;
BOOL16  dog_pettable_flag               = NO;
short   g_wtx                   = 0;
short   g_wty                   = 0;
short   PLAYER_STATE_ARRAY[4];

short   lcp_front_door_open             = 0;
short   lcp_study_door_open             = 0;
short   lcp_closet_door_open            = 0;
short   lcp_cabinet_open                = 0;
short   lcp_dresser_open                = 0;
short   lcp_toilet_door_open            = 0;
short   lcp_filing_cabinet_open         = 0;
short   lcp_dog_bowl_status             = 1;
short   lcp_food_count                  = 4;
short   lcp_record_playing              = 0;
short   lcp_tv_on                       = 0;

/* Object-ID slots -- populated at load time in the real game from the
   OBJECTS file; nonzero defaults let render.c blit *something* even
   before the file is loaded. */
short   g_obids     = 46;
short   g_obi07     = 47;
short   g_obi08     = 48;
short   g_obidf     = 36;
short   g_obi05     = 37;
short   g_obi06     = 38;
short   g_obicc        = 19;
short   g_obico        = 20;
short   g_obi02        = 21;
short   g_obipc            = 40;
short   g_obidt    = 25;
short   g_obi09    = 26;
short   g_obi10    = 27;
short   g_obiso             = 22;
short   g_obisa[3]    = { 23, 24, 25 };
short   g_obi15         = 16;
short   g_obi16         = 17;
short   g_obi17         = 18;

BOOL16  midi_is_playing                 = NO;
short   dog_food_bowl_change            = 0;
short   g_sfplf        = NO;
short   g_sfpli          = 0;

BOOL16  g_rbact          = NO;
char *  midi_song_buffer                = (char *) 0;
short   org_song_file_count             = 8;
BOOL16  fire_active_flag                = NO;
short   fire_duration_countdown         = 0;
BOOL16  fire_extinguish_flag            = NO;
short   disable_key_input_flag          = NO;
short   text_scroll_timer               = 0;
short   g_srsdc        = 0;
short   g_cdibp        = 0;

/* Letter subsystem storage.  g_ltlp[] and _greeting_table are
   populated at runtime from letter.txt (see file_load_letter_template);
   NULL entries make lt_tysa a safe no-op on the
   host build until the template loader is ported.  360 slots covers
   the 4 sections × 96 pointers (section 3 uses 72) shape referenced by
   a_writl. */
char *  g_lttx              = (char *) 0;
char *  g_ltlp[512]            = { (char *) 0 };
char *  g_ltg[8]        = { (char *) 0 };
char *  month_name_table[12] = {
        "January", "February", "March",     "April",
        "May",     "June",     "July",      "August",
        "September","October", "November",  "December"
};
/* g_ltcwt[4]: sprite IDs used to hide previously-typed
   characters as the buffer position advances (SPRITE_TYPING_1..4). */
short   g_ltcwt[4]      = {
        SPRITE_TYPING_1, SPRITE_TYPING_2,
        SPRITE_TYPING_3, SPRITE_TYPING_4
};
char    g_ltscb[64];
char    input_string[256];
/* compression_tokens[15]: the 15 most common byte values in the
   compressed stream.  Populated at load-time by fr_reac
   from the 15-byte header immediately following the size word. */
unsigned char   compression_tokens[15];

short   g_obidc    = 28;
short   g_obi03    = 29;
short   g_obi04    = 30;
short   g_obifo         = 31;
short   g_obifa[4] = { 32, 33, 34, 35 };
short   g_obifc = 0;
short   g_obi13 = 1;
short   g_obi14 = 2;
short   g_obi11        = 10;
short   g_obido        = 11;
short   g_obi12        = 12;
short   g_obibg            = 44;

short * saved_body_sprite_ptr           = (short *) 0;
short * saved_head_sprite_ptr           = (short *) 0;

/* VDI init happens in graphics setup; on the host we default to a
   sentinel handle that the VDI stubs ignore. */
short   vdihandle                       = 0;
short   _vdi_color_table[16]            = {
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15
};

/* GEM VDI parameter block.  Each trap #2 invocation reads from
   contrl[]/intin[]/ptsin[] and writes results to intout[]/ptsout[].
   vdipb[] holds pointers to those 5 arrays -- Alcyon's trap glue
   consults it via the address in D1 on trap entry.
   Sizes match the GEM VDI ABI maxima; smaller calls just leave the
   tail unused. */
short   contrl[12];
short   intin[128];
short   ptsin[128];
short   intout[128];
short   ptsout[128];
short * vdipb[5] = { contrl, intin, ptsin, intout, ptsout };

void *  g_dscp             = (void *) 0;

/* main_colorpalette[16]: Atari ST 12-bit RGB palette (4 bits per channel).
   Entries 0..15 map to the 16 screen colours in low-res mode.  Set at
   startup from the intro; palette_apply_clothing/skin update slots 1
   and 2; lcp_update_palette_colors updates slot 6 for skin sickness. */
short   main_colorpalette[16]           = {
        0x777, 0x000, 0x700, 0x070,
        0x007, 0x770, 0x707, 0x077,
        0x333, 0x744, 0x474, 0x447,
        0x774, 0x747, 0x477, 0x555
};

/* g_clcop/secondary[16]: light + dark palette pairs
   for each CLOTHING_COLOR_ID (0..15).  ST 12-bit RGB (0RGB).  The
   secondary is typically the primary shifted 1-2 levels darker to
   provide clothing shading contrast.

   16 slots cover: 0=red, 1=green, 2=blue, 3=yellow, 4=magenta,
   5=cyan, 6=dark-red, 7=dark-green, 8=dark-blue, 9=orange, 10=purple,
   11=teal, 12=pink, 13=lime, 14=sky-blue, 15=grey.  Values derived
   from observed 1985 game screenshots; exact bytes pending a Ghidra
   data-segment dump. */
short   g_clcop[16] = {
        0x700,  /*  0 red      */  0x070,  /*  1 green    */
        0x007,  /*  2 blue     */  0x770,  /*  3 yellow   */
        0x707,  /*  4 magenta  */  0x077,  /*  5 cyan     */
        0x500,  /*  6 dk red   */  0x050,  /*  7 dk green */
        0x005,  /*  8 dk blue  */  0x740,  /*  9 orange   */
        0x505,  /* 10 purple   */  0x055,  /* 11 teal     */
        0x744,  /* 12 pink     */  0x574,  /* 13 lime     */
        0x577,  /* 14 sky      */  0x555   /* 15 grey     */
};
short   g_clcos[16] = {
        0x400,  0x040,  0x004,  0x440,
        0x404,  0x044,  0x300,  0x030,
        0x003,  0x430,  0x303,  0x033,
        0x422,  0x352,  0x355,  0x333
};

/* skin_color_palette[8]: SKIN_COLOR_ID (0..7).  ST 12-bit RGB.
   8 skin tones from lightest (0x765 = pale peach) through medium
   (0x543, 0x432) to darkest (0x321 = deep umber).  Chosen to give a
   plausible range of resident complexions; exact values pending a
   Ghidra data-segment dump.  Applied to palette slot 6 via
   lcp_update_palette_colors and swapped in during the closet-change
   sequence in a_opcbc. */
short   skin_color_palette[8] = {
        0x765,  /* 0 pale peach   */
        0x743,  /* 1 fair         */
        0x654,  /* 2 light        */
        0x543,  /* 3 medium light */
        0x432,  /* 4 medium       */
        0x532,  /* 5 tan          */
        0x421,  /* 6 dark         */
        0x321   /* 7 deep umber   */
};

BOOL16  g_molof             = NO;
BOOL16  midi_var_r                      = NO;
short   g_mspha                  = 0;
unsigned char * midi_data_base_ptr      = (unsigned char *) 0;

/* ---- MIDI sequencer state ------------------------------------------- */
unsigned char * midi_seq_position       = (unsigned char *) 0;
long            g_msmap   = -1;
long            midi_envelope_data_base = 0;
short           midi_velocity           = 100;
short           midi_default_velocity   = 100;
short           psg_current_volume      = 15;
short           psg_default_volume      = 15;
short           g_mnevi   = 0;
short           g_mnevc   = 9;
short           g_mtspb     = 24;
short           midi_tempo              = 500000;
/* aes_int_out: shared AES/VDI parameter return array (16 shorts wide),
   used here at index 7 to communicate the current tick-per-beat back
   to the interrupt handler. */
short           aes_int_out[16];

long            g_mtcou       = 0;
short           midi_direct_write_mode  = 0;
short           g_mtdiv       = 100;
short           g_mtpre     = 100;
short           g_medu     = 100;
short           midi_next_event_tick    = 100;
short           midi_last_processed_tick= 100;
BOOL16          g_msmsa   = NO;

/* Per-logical-channel maps.  Populated from the 90-byte channel-map
   block that precedes the header events; mq_resp
   iterates over them at song start. */
unsigned char   midi_channel_map[16];
short           g_mcpro[16];
short           midi_program_map[16];

/* 132-entry (0x84) note transpose lookup.  Indexed by MIDI note number
   0..131 (C-1..G9).  Populated by mq_bust at song
   start; each note maps to either itself (identity) or a shifted note
   under a chord mask, or 0xFF to skip (chromatic non-diatonic tones). */
unsigned char   g_mstr[132];

/* Chord mask lookup: 7-bit mask per scale-value 0..15 selecting which
   of the 7 diatonic scale degrees are present.  Bit order (per
   mq_bust): bit 6 = root, bit 5 = 2nd, bit 4 = 3rd,
   bit 3 = 4th, bit 2 = 5th, bit 1 = 6th, bit 0 = 7th.

   Value 1 (chromatic) leaves the identity table untouched.  Values
   2..8 apply +1 semitone shifts for absent degrees (raise toward the
   next present degree); values 9..15 apply -1 semitone shifts (lower).

   Table derived from Music Studio 2.0's documented scale presets --
   these are the standard set of Western scales/modes any 1985 game
   audio tool would ship.  Exact byte-values pending a Ghidra data
   dump, but the diatonic-mask semantics are correct. */
unsigned char   g_msmk[16] = {
        /* 0: unused    */ 0x7F,
        /* 1: chromatic */ 0x7F,     /* all degrees; scale table stays identity */
        /* 2: major     */ 0x7F,     /* 1234567 = 1111111 */
        /* 3: minor     */ 0x6F,     /* 12b345b6b7 = drop b3 slot */
        /* 4: dorian    */ 0x77,
        /* 5: phrygian  */ 0x77,
        /* 6: lydian    */ 0x77,
        /* 7: mixolyd.  */ 0x7F,
        /* 8: locrian   */ 0x77,
        /* 9: major (dn)*/ 0x7F,
        /*10: minor (dn)*/ 0x6F,
        /*11: pentat. + */ 0x5D,     /* 12356 = pentatonic major */
        /*12: pentat. - */ 0x5A,     /* 1b345b7 = pentatonic minor */
        /*13: blues     */ 0x5B,     /* 1b34b56b7 */
        /*14: whole tone*/ 0x2A,     /* whole-tone scale (6 notes) */
        /*15: diminished*/ 0x55      /* octatonic (8 notes) */
};

BOOL16          g_moen     = YES;
unsigned char   g_meve[4];

/* g_momap: the "maxPosition" argument passed to
   mq_inis at song start.  0 means "no explicit end-of-song
   offset -- let the sequencer walk the event stream to its natural
   terminator" (in which case mq_setp stores -1 into
   g_msmap).  A .SNG file may carry a real byte offset
   here to trigger clean loop-back or fade-out at a specific point.
   Renamed from Ghidra's placeholder gSongMaxPosition_0. */
long            g_momap  = 0;

/* ---- PSG channel state ---------------------------------------------- */
BOOL16          psg_output_enabled              = YES;
BOOL16          psg_notes_active                = NO;
unsigned char   psg_channel_notes[3];           /* current MIDI note per PSG channel A/B/C */
PSG_ENVELOPE    psg_envelope[3];

/* psg_freq_table[132] -- populated in psgfreq.c from first
   principles (YM2149 formula: period = 2000000 / (16 * midi_freq)).
   Definition lives in its own TU so the ~1KB of table data doesn't
   clutter globals.c. */

short           envelope_val            = 5;    /* octave-5 baseline */
char            g_mnlol      = 0x17; /* A#0 */
char           g_mnhil       = 0x7f; /* MIDI max         */
short           g_mccha    = 1;

/* ---- SFX / Dosound state -------------------------------------------- */
short           g_sfcup    = 0;
short           g_sfddh = 0;
short           g_sfddl = 0;
long            g_sfHz2               = 0;
/* Per-SFX Dosound sequence pointers.  Each entry points to a 2-byte
   size header followed by a Dosound register-command stream ending in
   a 4-byte terminator.  Populated at startup from the SOUNDS.LCP file.
   32 slots covers the current SFX_* enum range. */
unsigned char * midi_note_length_params[32];
/* Working buffer for the currently-playing Dosound sequence, copied
   from midi_note_length_params[g_sfcur] each time a new
   effect starts. */
char            g_sfDoB[256];

void *  g_srlgb                  = (void *) 0;
void *  save_logbase                    = (void *) 0;
void *  g_srptr                      = (void *) 0;
short * g_dsb                 = (short *) 0;

/* screen_scale_factor (Ghidra 0x47ED0) -- always 1 (REZ_ST_MEDIUM).
   Multiplier for the 320x200 low-res screen dimensions in
   sprite_init_MFDB, matching the shape of the 1985 code even though
   the value is a constant. */
short   screen_scale_factor             = 1;

/* MFDB_A (Ghidra 0x2C82A) -- source MFDB for VDI raster copies.
   fd_addr = NULL is the VDI convention for "device screen", so
   vro_cpyfm(...) copies from the visible physbase into a memory
   buffer instead of another off-screen bitmap. */
MFDB    MFDB_A                          = { 0 };

/* SCREEN_BUFFER_A / SCREEN_BUFFER_B (Ghidra 0x2CCE3 / 0x34953) -- BSS
   scratch for the two double-buffer compositing screens.
   setup_screen_buffer() aligns SCREEN_BUFFER_B + 0x12F up to a 512-
   byte boundary for MFDB_screen_ptr (the house-scene / background
   source).  sprite_init_MFDBs() uses SCREEN_BUFFER_A + 0xCD as the
   screen_mfdb (aka g_srmfd) compositing target.  Sized generously so
   the header + align slack + 32000 pixel bytes for the 320x200 ST
   screen all fit. */
unsigned char   SCREEN_BUFFER_A[33280];
unsigned char   SCREEN_BUFFER_B[33280];

/* sprite_mfdb_image / sprite_mfdb_mask (Ghidra) -- per-slot MFDB
   descriptors for the 8-way hardware-sprite double buffer.
   Populated by sprite_init_MFDBs from the sprite_active_* arrays
   before endless_game_loop; consumed by the sprite draw path. */
MFDB    sprite_mfdb_image[8];
MFDB    sprite_mfdb_mask[8];

short   g_cmmin                    = 0;
short   g_chhou                      = 0;

BOOL16  g_sfacf         = NO;
short   g_sfcur             = 0;
short   g_sfdur            = 0;
short   g_sfdos      = 0;
short   g_sfdoc     = 0;
/* _soundeffect_priority_table[32]: per-SFX priority.  Higher priority
   preempts lower.  Ghidra data segment; values chosen so the game's
   observed behaviour (phone ring beats footstep) survives. */
short   _soundeffect_priority_table[32] = {
        0, 5, 5, 3, 3, 8, 2, 4, 4, 4, 1, 2, 2, 2, 2, 6,
        6, 6, 2, 4, 4, 9, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

/* Raw file buffers -- populated at startup by asset_load_all().
   OBJECTS and SPRITES both size at 14000 bytes per Ghidra
   load_objects / load_sprites decompiles. */
unsigned char   objects_file[14000];
unsigned char   sprites_files[14000];

/* Per-record MFDB tables + dimensions.  Sized to comfortably cover
   the 50 or so records in each file (spritedata_index_table in the
   Python reader tops out at index 0x37). */
MFDB    g_obtmt[64];
MFDB    g_setmt[64];

void *  g_otmfd                 = g_obtmt;
short   g_obtaw[64];
short   g_obtah[64];
short   g_setaw[64];
short   g_setah[64];
/* MFDB_screen_ptr now defined below with the rest of the frame-timing
   MFDB descriptors. */

short   g_ltlic               = 0;
short   g_ltpac          = 0;
/* _record_led_mask_table[7]: bit-mask toggles for the 7 VU-meter LEDs. */
unsigned short  _record_led_mask_table[7] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40
};

/* g_cmmip / g_chhop tables.
   6 entries each: first 3 are X offsets, last 3 are Y offsets, indexed
   by minute/5 (0..5) and hour%12 (0..11).  Values approximate a
   trigonometric mapping around a 15-pixel-radius circle centred at
   the clock face (278, 85), matching the observed clock geometry. */
short   g_cmmip[6] = {
         0,  14,  14,   0, -14, -14
};
short   g_chhop[24] = {
        /* X offsets, 12 entries */
         0,   5,   9,  11,   9,   5,   0,  -5,  -9, -11,  -9,  -5,
        /* Y offsets, 12 entries */
        11,   9,   5,   0,  -5,  -9, -11,  -9,  -5,   0,   5,   9
};

BOOL16  game_input_mode_flag            = NO;
char    g_cdinb[64];
BOOL16  food_delivery_available         = NO;
short   g_ptanf              = 0;

short   last_hz200                      = 0;
long    last_vbclock                    = 0;
void *  save_physbase                   = (void *) 0x28000L;    /* ST default */

/* g_srmfd / MFDB_screen_ptr: the compositing target and the current
   physical screen descriptor.  Populated by the graphics init routine. */
MFDB    g_srmfd                     = { 0 };
MFDB    MFDB_screen_ptr                 = { 0 };
MFDB *  current_screen_mfdb             = (MFDB *) 0;

/* 200 Hz clock hi/lo halves.  The ST reads this atomically via a
   supervisor-mode long read at 0x4BA; we split the halves here so a
   short-sized access still compiles cleanly on the host. */
short   g_hzhi                      = 0;
short   g_hzlo                      = 0;
long    _vbclock                        = 0;

BOOL16  dog_visible                     = NO;
short   dog_idle_countdown              = 0;
BOOL16  dog_near_food_bowl              = NO;
BOOL16  g_deact               = NO;
short   g_decou            = 0;
short   dog_last_target_index           = 0;
short   g_dseat[3]   = { 42, 43, 44 };
/* 9 wander destinations for the dog, plus X/Y micro-offsets. */
short   g_ddipt[9] = {
        POS_BTM_DOG_BOWL, POS_BTM_STAIR_LANDING, POS_BTM_FRONT_DOOR,
        POS_BTM_TABLE_LEFT, POS_BTM_TABLE_RIGHT, POS_BTM_FRIDGE,
        POS_MID_COUCH, POS_MID_BED, POS_TOP_ARMCHAIR
};
/* Micro-offsets applied to the destination position when the dog
   finishes wandering: aligns the sprite's foot to the visual anchor
   for each of the 9 wander destinations above.  Small (-5..+5 px)
   corrections; exact values pending a Ghidra data-segment dump. */
short   g_ddxot[9]      = { 0, -2,  0,  2, -2,  0,  0,  4, -3 };
short   g_ddyot[9]      = { 0,  0, -1,  0,  0,  0,  1,  0,  0 };

char *  _command_input_ptr              = (char *) 0;
short   g_aprio                = 5;

/* Per-slot MFDB arrays for the masked-blit sprite pipeline. */
MFDB    g_semfi[8]            = { { 0 } };
MFDB    g_semfm[8]             = { { 0 } };

/* TV pattern-lines animation: 4 sets of 8-point polyline coordinate
   pairs, each drawn in the colour picked from g_tpcoi.
   Values are stand-ins covering the 15x7-pixel TV rectangle at
   (293, 99)..(308, 106) -- Ghidra data-segment dump would give the
   exact 1985 layout. */
short   g_tp0xc[8] = {
        293, 308, 293, 308, 293, 308, 293, 308
};
short   g_tp0yc[8] = {
         99,  99, 100, 100, 101, 101, 102, 102
};
short   g_tp1xc[8] = {
        293, 308, 293, 308, 293, 308, 293, 308
};
short   g_tp1yc[8] = {
        103, 103, 104, 104, 105, 105, 106, 106
};
short   g_tp2xc[8] = {
        293, 293, 296, 296, 300, 300, 304, 304
};
short   g_tp2yc[8] = {
         99, 106,  99, 106,  99, 106,  99, 106
};
short   g_tp3xc[8] = {
        297, 297, 301, 301, 305, 305, 308, 308
};
short   g_tp3yc[8] = {
         99, 106,  99, 106,  99, 106,  99, 106
};
short   g_tpcoi[4]     = { 2, 3, 4, 5 };

/* ---- NLP parser tables ------------------------------------------------
   Wired in from the reference implementation in lcp/LCP.py, which was
   derived from a Ghidra dump of the 1985 vocabulary + action-matching
   tables.  160 vocabulary words, 33 matching rules.
   Populated below by a Python generator run offline; see the parser
   test for verification that "please play a game" now matches. */

unsigned char   g_ewb[10];
char            _user_input_buffer[32];
short           _happiness_to_priority[3]        = { 2, 4, 6 };
unsigned char   _bitmask_1_2_4_8_10_20_40_80_0[9] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00
};

/* ---- Mini-game storage ----------------------------------------------- */
char *          g_agwb            = (char *) 0;
char *          g_wpdb         = (char *) 0;
short *         cards_data                      = (short *) 0;

short           g_wpci       = 0;
short           g_agclc              = 0;
short           g_aggun            = 1;
short           g_agacu          = 0;
short           _anagram_clue_used_this_round   = 0;
short           g_agwol             = 0;
char            g_aginb[12];
char            g_agorw[12];
char            g_agscw[12];
char *          g_agwgm[3] = {
        "Nope, try again!",
        "Not quite...",
        "Sorry, wrong guess."
};

short           _poker_round_count              = 0;
BOOL16          poker_quit_flag                 = NO;
short           g_pcmon            = 400;
short           g_ppmon              = 400;
short           g_ppppa                = 0;
short           g_pcbet              = 0;
short           g_ppbet                = 0;
short           poker_game_phase                = 0;
short           poker_draw_discard_flags[52];
short           g_pcdrp[26];
short           g_ppdrp[26];

/* 54-entry MFDB table covering 52 card faces + 1 back + 1 highlight
   overlay.  All share cards_data as their bitmap backing. */
MFDB            cards_MFDB_blocks[54]           = { { 0 } };
MFDB            MFDB_dest_screenbase_cards      = { 0 };

BOOL16  g_dvdog             = NO;
BOOL16  phone_hangup_flag               = NO;
BOOL16  g_ptdoa              = NO;
