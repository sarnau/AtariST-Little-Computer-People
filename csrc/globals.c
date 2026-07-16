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

/* Ghidra's endless_game_loop always sets these via
   house_get_position_xy() during boot.  We seed them to Ghidra's
   post-cutscene "resident is inside, past the doorbell" state --
   cutscene_new_lcp_move_in sets (300, 190) right after the door
   opens, then walks the resident toward the screen edge.  Using
   that value lets the AI dispatcher pick up from a valid on-floor
   position without needing the full cutscene ported. */
short   lcp_x                           = 300;
short   lcp_y                           = 190;
short   g_lcldd                      = 0;
short   copyprot_check_return           = 0;      /* Ghidra: set by copyprot_main_check() during boot */
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
   populated at runtime from letter.txt (see fl_ltpl);
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
/* _vdi_color_table (Ghidra vdi_color_table @ 0x29b64): color_enum ->
   VDI palette-index permutation.  color_enum 0 (black) -> VDI slot 0,
   color_enum 14 (white) -> VDI slot 13, etc.  Not identity: VDI's
   default 16-entry palette-index-to-hardware-color assignment differs
   from the game's color_enum numbering, so text/lines call
   vst_color/vsl_color/vsf_color through this permutation to end up
   at the same on-screen hue as Ghidra. */
short   _vdi_color_table[16]            = {
        0,  2,  3,  6,  4,  7,  5,  8,
        9, 10, 11, 14, 12, 15, 13,  1
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
   Entries 0..15 map to the 16 screen colours in low-res mode.  Values
   dumped from the 1985 data segment at Ghidra 0x29B44 (via
   ghidra_scripts/DumpPalette.java).  aes_vdi_jnit loads this via
   _xbios(XBIOS_Setpalette,main_colorpalette) at boot -- there is no
   later runtime palette rewrite from this table; slot 0 is the
   background (black), slot 14 white, etc.  pa_cloc overwrites slots
   1 and 2 from the primary/secondary clothing tables; slot 6 is
   overwritten by lcp_update_palette_colors for the sickness skin. */
short   main_colorpalette[16]           = {
        0x000, 0x442, 0x265, 0x754,
        0x310, 0x040, 0x754, 0x760,
        0x247, 0x631, 0x700, 0x333,
        0x555, 0x007, 0x777, 0x410
};

/* g_clcop / g_clcos (Ghidra clothing_color_primary / _secondary):
   16 pairs of primary + secondary 12-bit RGB shirt colours indexed
   by CLOTHING_COLOR_ID.  Ported from Ghidra 0x2A2E4 / 0x2A2C4 --
   the earlier port sized these correctly but the values were
   observed-from-screenshots guesses (bright primaries + darker
   secondaries) that don't match the real palette.  The actual 1985
   values include several duplicate blue primaries (0x006 for slots
   0..4) so a random clothing pick usually gives the same blue shirt. */
short   g_clcop[16] = {
        0x006, 0x006, 0x006, 0x006,
        0x006, 0x676, 0x676, 0x500,
        0x500, 0x735, 0x140, 0x641,
        0x623, 0x036, 0x242, 0x442
};
short   g_clcos[16] = {
        0x060, 0x760, 0x606, 0x066,
        0x767, 0x007, 0x700, 0x030,
        0x767, 0x465, 0x314, 0x255,
        0x662, 0x406, 0x156, 0x514
};

/* skin_color_palette[8]: SKIN_COLOR_ID (0..7).  ST 12-bit RGB.
   8 skin tones from lightest (0x765 = pale peach) through medium
   (0x543, 0x432) to darkest (0x321 = deep umber).  Chosen to give a
   plausible range of resident complexions; exact values pending a
   Ghidra data-segment dump.  Applied to palette slot 6 via
   lcp_update_palette_colors and swapped in during the closet-change
   sequence in a_opcbc. */
short   skin_color_palette[8] = {
        0x512, 0x742, 0x567, 0x762,
        0x745, 0x145, 0x160, 0x565
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
/* Ghidra g_mchcn @ 0x298F0.  mh_chac (0x11246) writes the
   header byte here and passes it through midi_seq_build_scale_table. */
short           g_mchcn      = 0;
/* Ghidra midi_ticks_per_beat @ 0x298F4 = 20; midi_tempo @ 0x298F2 = 120. */
short           g_mtspb     = 20;
short           midi_tempo              = 120;
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
/* dest_scr_buffer_storage: dedicated 32 KB offscreen buffer where the
   letter-typing status strip composites, kept separate from the
   main house buffer.  fill_top_rect_with_background(27) writes rows
   0..26 here so that the striped-white letter background is ready
   for the typewriter animation, but this content is NEVER visible
   until the letter sequence composites it.  Sized 32000+256 to
   cover a full ST low-res screen plus the +0x7f (=254 byte) offset
   `g_dsb` is anchored at. */
short   dest_scr_buffer_storage[16256];
short * g_dsb = dest_scr_buffer_storage;

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

/* scrbufA / scrbufB (Ghidra SCREEN_BUFFER_A / _B) -- BSS scratch
   for the two double-buffer compositing screens.

   scrbufB (~33 KB): holds the decompressed house.scn background at
     an aligned offset (setup_screen_buffer rounds scrbufB + 0x12F
     UP to the next 512-byte boundary).

   scrbufA: PORT-SPECIFIC LAYOUT.  The 1985 code depends on
     SCREEN_BUFFER_A landing at an odd address so that:
       + 0xCD  (used by sprite_init_MFDBs)         -> aligned
       + 0x19A (used by renderf.c page-flip)       -> 256-aligned
     Neither is guaranteed under our linker.  Instead of trying to
     reproduce the original binary's linker luck, we size scrbufA
     large enough (64 KB) to fit TWO independent 256-aligned
     32 KB screens: one at scrbufA + 0x000 (compositor writes),
     one at scrbufA + 0x8000 (alt physbase for the page-flip).
     sprite_init_MFDBs and renderf.c compute both offsets by
     rounding scrbufA UP to the next 256 boundary. */
unsigned char   scrbufA[65536];
unsigned char   scrbufB[33280];

/* sprite_mfdb_image / sprite_mfdb_mask are Ghidra's names for the
   per-slot 8-way sprite MFDBs.  Our port already had them under the
   older names g_semfi / g_semfm (defined later in this file with
   { { 0 } } initializers) and referenced from sprender.c's sp_draw.
   sp_imfs writes through those existing arrays -- see sprites.c. */

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

/* g_cmmip (Ghidra clock_minute_position @ 0x2B566, 15 shorts):
   circle-position table for the minute hand.  Indexed by the current
   minute/5 mod 12 giving one of 12 positions on a small circle around
   the clock centre; three padding entries at the end.  Values dumped
   live from Ghidra. */
short   g_cmmip[15] = {
         0,   2,   3,   3,   3,   2,   0,  -2,
        -3,  -3,  -3,  -2,   0,   2,   3
};
/* g_chhop (Ghidra clock_hour_position @ 0x2B584, 15 shorts): same
   shape for the hour hand, smaller radius (2 vs 3 pixels). */
short   g_chhop[15] = {
         0,   1,   2,   2,   2,   1,   0,  -1,
        -2,  -2,  -2,  -1,   0,   1,   2
};

BOOL16  game_input_mode_flag            = NO;
char    g_cdinb[64];
BOOL16  food_delivery_available         = NO;
short   g_ptanf              = 0;

short   last_hz200                      = 0;
long    last_vbclock                    = 0;
/* save_physbase: TOS's original Physbase, captured once at boot by
   aes_vdi_jnit via _xbios(XBIOS_Physbase).  Both page-flip screens
   in sc_ren8 alternate between this address and an alt buffer; the
   hardcoded 0x28000 fallback (1MB-machine TOS physbase) is only
   used if aes_vdi_jnit hasn't run yet -- do NOT rely on it. */
void *  save_physbase                   = (void *) 0x28000L;

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
/* Ghidra dog_destination_position_table @ 0x2B8DE, 10 entries -- the
   HOUSE_POS index the dog picks as its next wander target.  Values
   {0, 5, 11, 19, 29, 32, 33, 41, 47, 47} correspond (per HOUSE_POS
   enum) roughly to bowl / stair landings / doors / table / fridge /
   couch / armchair.  Last two are duplicated 47 (POS_TOP_ARMCHAIR). */
short   g_ddipt[10] = { 0, 5, 11, 19, 29, 32, 33, 41, 47, 47 };
/* Ghidra dog_dest_x_offset_table @ 0x2B906, dog_dest_y_offset_table
   @ 0x2B8F2 (10 shorts each): per-destination pixel nudges applied
   after house_get_position_xy returns the anchor for the destination. */
short   g_ddxot[10]     = { 0, 0, 0, 0, 10, 0, 0, 0, 0, 0 };
short   g_ddyot[10]     = { 3, 9, 2, 10, 6, 0, 0, 11, 3, 3 };

char *  _command_input_ptr              = (char *) 0;
short   g_aprio                = 5;

/* Per-slot MFDB arrays for the masked-blit sprite pipeline. */
MFDB    g_semfi[8]            = { { 0 } };
MFDB    g_semfm[8]             = { { 0 } };

/* TV pattern animation (Ghidra tv_pattern_N_x_coords / _y_coords).
   Four vertical scanlines drawn inside the TV screen -- each is a
   constant-X, descending-Y run of 8 points.  Colours picked from
   tv_pattern_color_indices (10, 5, 7, 13 in the main palette). */
short   g_tp0xc[8] = { 293, 293, 293, 293, 293, 293, 293, 293 };
short   g_tp0yc[8] = { 106, 105, 104, 103, 102, 101, 100,  99 };
short   g_tp1xc[8] = { 297, 297, 297, 297, 297, 297, 297, 297 };
short   g_tp1yc[8] = { 106, 105, 104, 103, 102, 101, 100,  99 };
short   g_tp2xc[8] = { 301, 301, 301, 301, 301, 301, 301, 301 };
short   g_tp2yc[8] = { 106, 105, 104, 103, 102, 101, 100,  99 };
short   g_tp3xc[8] = { 305, 305, 305, 305, 305, 305, 305, 305 };
short   g_tp3yc[8] = { 106, 105, 104, 103, 102, 101, 100,  99 };
short   g_tpcoi[4] = { 10, 5, 7, 13 };

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

/* (game_tick_and_animate animation tables + frame-state globals live
   in tick_tables.c -- Alcyon C168's symbol-table overflows if they
   are added here.) */
