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
#include "globals.h"
#include "sprglobs.h"

short   ani_cnt  = 0;
short   g_secs    = 0;

short   t_min            = 0;
short   t_hour              = 0;
short   date_day                = 0;
short   dt_mon              = 0;
short   dt_year               = 0;

PLAYER  lcp;

BOOL16  ph_ans     = NO;
BOOL16  ph_call  = NO;
BOOL16  introSeq   = NO;

BOOL16  lunT_trg      = NO;
BOOL16  dinT_trg     = NO;
BOOL16  wkT_trg  = NO;
BOOL16  bedT_trg         = NO;

BOOL16  in_evrt   = NO;

short   lastAct                     = ACTION_NONE;
short   g_trac                  = ACTION_NONE;

/* Ghidra's gameLoop always sets these via
   hs_posXY() during boot.  Ghidra keeps both in BSS;
   the cutscene sets them.  Port matches by leaving them at 0 -- the
   cutscene stub in init.c writes (300, 190) before gameLoop
   runs. */
short   lcp_x                           = 0;
short   lcp_y                           = 0;
BOOL16  g_lcldd                      = 0;
short   cprot_r           = 0;      /* Ghidra: set by copyprot_main_check() during boot */
short   g_spdc              = 5;

BOOL16  alarm_p       = NO;
short   lcp_watr                 = 7;

short   g_aliss               = 0;
short   g_aqueu[10];
short   g_apriq[10];

/* Ghidra head_anim_target_state @ 0x29b98 = 8, head_anim_current @ 0x29b96 = 8,
   head_anim_mode @ 0x29b9a = -1 (HEAD_ANIM_DISABLED). */
short   g_hatas                         = 8;
short   g_hacur                         = 8;
short   g_hamod                         = HEAD_ANIM_DISABLED;
short   g_hsfra               = 0;
long    g_sfret     = 0;
BOOL16  g_actif       = NO;
BOOL16  dg_petok               = NO;
short   g_wtx                   = 0;
short   g_wty                   = 0;
/* Ghidra triggered_event_list @ 0x2b6da: 10-short scratch buffer used
   by action handlers (bathroom, food, house, leisure, idle, simple)
   to cache a small set of state values indexed by variable expressions
   like `i & 3`.  Port previously declared [4], which was one byte
   short of a real out-of-bounds write via `pst_arr[4]` writes in the
   bathroom/food/house paths -- the fifth slot overlapped lcp_frdO. */
short   pst_arr[10];

short   lcp_frdO             = 0;
short   studyDrO             = 0;
short   lcp_clsO            = 0;
short   lcp_cabO                = 0;
short   lcp_drsO                = 0;
short   lcp_toiO            = 0;
short   lcp_flcO         = 0;
short   lcp_bwlS             = 1;
short   lcp_food                  = 4;
short   lcp_recP              = 0;
short   lcp_tv                       = 0;

/* g_obisa: stove-on animation frame IDs, indexed at runtime by
   a_eatm's `pick = rndRng(0,2)` cooking loop.  Ghidra ROM has an
   array of three shorts at 0x2b4b6; port keeps the array shape for
   variable-indexed access.  All other object slots are now inlined
   as OBJ_* constants at their call sites. */
short   g_obisa[3]    = { OBJ_STOVE_ON_1, OBJ_STOVE_ON_2, OBJ_STOVE_ON_3 };

BOOL16  mi_play                 = NO;
short   dg_bwlch            = 0;
short   g_sfplf        = NO;
short   g_sfpli          = 0;

BOOL16  g_rbact          = NO;
char *  mi_sbuf                = (char *) 0;
/* Ghidra sng/org song file counts, set at boot by cntSong().
   BSS-zero to match Ghidra; port previously had org_cnt=8
   as a guess. */
short   sng_cnt             = 0;
short   org_cnt             = 0;
/* scn_cmn -- 30-byte scene common-data header shared between
   house.scn and title.scn.  Ghidra `scene_common_data` @ 0x4cf7c.
   The port's unScn helper (assets.c) fills this via fr_read. */
char    scn_cmn[30];
/* PEx.LCP filename.  Ghidra pex_name @ 0x2a0f8 points to "pex.lcp"
   at 0x2a330 and main() mutates index 2 to select the character.
   Port stores the string as a mutable static char array. */
char    pex_name[8]                  = "PE0.LCP";
BOOL16  fire_act                = NO;
short   fire_dur         = 0;
BOOL16  fire_ext            = NO;
short   no_keyin          = NO;
short   tx_sctm               = 0;
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
/* g_ltg[4]: the four letter sign-offs picked at random by
   a_writl (`letter_type_string_animated(g_ltg[rndRng(0,3)], -8)` at
   the greeting slot).  Byte-for-byte the ROM's DATA layout at 0x2b671
   (verified via /read_memory): "Sincerely,\0Cordially,\0Yours
   Truly,\0Love,\0".  Previously left NULL because a stub loader was
   assumed to populate it from LETTER.TXT -- but the ROM's
   file_load_letter_template only walks 360 template lines and the
   sign-offs are static C string literals compiled into DATA. */
char *  g_ltg[4]        = {
        "Sincerely,",
        "Cordially,",
        "Yours Truly,",
        "Love,"
};
char *  mo_names[12] = {
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
char    in_str[256];
/* comp_tok[15]: the 15 most common byte values in the
   compressed stream.  Populated at load-time by fr_reac
   from the 15-byte header immediately following the size word. */
unsigned char   comp_tok[15];

short * sv_bodyP           = (short *) 0;
short * sv_headP           = (short *) 0;

/* VDI init happens in graphics setup; on the host we default to a
   sentinel handle that the VDI stubs ignore. */
short   vdihnd                       = 0;
short   vdi_hnd                      = 0;    /* physical from graf_handle */
/* vdi_colt (Ghidra vdi_color_table @ 0x29b64): color_enum ->
   VDI-color permutation.  ROM data at 0x29b64 (verified via
   /read_memory) is {0,2,3,6,4,7,5,8,9,10,11,14,12,15,13,1} -- exactly
   TOS's default ST-low permutation from VDI-index to palette-slot.
   The game names its own colours by palette slot (see main_pal) and
   calls vsl_color(vdi_colt[color_enum]) so that after TOS's
   permutation the pen lands on palette slot `color_enum`.

   Byte-for-byte match to ROM.  With a properly-opened VDI workstation
   (LCP.PRG launched directly from the GEM desktop / Hatari --auto),
   TOS applies its default permutation and color_enum 13 (blue) ->
   vdi_colt[13] = 15 -> palette 13 = main_pal[13] = 0x007 blue.
   Launching via COMMAND.PRG leaves the workstation in a state that
   collapses vsl_color's colour arg into pen 15 (dark brown 0x410)
   regardless of index -- see the sc_sdtb comment. */
short   vdi_colt[16]            = {
        0,  2,  3,  6,  4,  7,  5,  8,
        9, 10, 11, 14, 12, 15, 13,  1
};

/* GEM VDI shared scratch arrays.  Gemlib source (alcyon/gemlib/vdi.c)
   defines these in vdi.o, but the pre-compiled Atari DK vdibind.a we
   link against does NOT pull vdi.o in with its contrl definitions in
   a way lo68 recognises for our wrapper callsites -- so the app has
   to supply the storage.  Every VDI wrapper in vdibind.a stuffs
   these arrays before firing trap #2. */
short   contrl[12];
short   intin[128];
short   ptsin[128];
short   intout[128];
short   ptsout[128];

/* v_opnvwk in/out arrays.  Ghidra: workin at 0x47ea8 (11 shorts),
   work_out at 0x4d218 (57 shorts).  Both are globals in the ROM, not
   stack locals -- vdi_init only allocates 6 bytes on the stack
   (link.w A6,-0x6 at 0x16680), enough for the loop counter only. */
short   workin[11];
short   work_out[57];

void *  g_dscp             = (void *) 0;

/* main_pal[16]: Atari ST 12-bit RGB palette (4 bits per channel).
   Entries 0..15 map to the 16 screen colours in low-res mode.  Values
   dumped from the 1985 data segment at Ghidra 0x29B44 (via
   ghidra_scripts/DumpPalette.java).  aes_init loads this via
   Setpalette(main_pal) at boot -- there is no
   later runtime palette rewrite from this table; slot 0 is the
   background (black), slot 14 white, etc.  pa_cloc overwrites slots
   1 and 2 from the primary/secondary clothing tables; slot 6 is
   overwritten by lcp_upal for the sickness skin. */
short   main_pal[16]           = {
        0x000, 0x442, 0x265, 0x754,
        0x310, 0x040, 0x754, 0x760,
        0x247, 0x631, 0x700, 0x333,
        0x555, 0x007, 0x777, 0x410
};

/* g_clcop / g_clcos (Ghidra clothing_color_primary @ 0x2A2E4 and
   clothing_color_secondary @ 0x2A2C4): 16 pairs of primary +
   secondary 12-bit RGB shirt colours indexed by CLOTHING_COLOR_ID.
   Values dumped verbatim from Ghidra -- note the five duplicate blue
   primaries (0x006 for slots 0..4) which bias random clothing picks
   toward the same blue shirt. */
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

/* skin_pal[8] (Ghidra @ 0x2A304): SKIN_COLOR_ID (0..7),
   ST 12-bit RGB.  Values dumped verbatim from the data segment.
   Applied to palette slot 6 via lcp_upal and
   swapped in during the closet-change sequence in a_opcbc. */
short   skin_pal[8] = {
        0x512, 0x742, 0x567, 0x762,
        0x745, 0x145, 0x160, 0x565
};

BOOL16  g_molof             = NO;
/* Ghidra mi_varR @ 0x29af2 = 1 (byte).  Port previously had NO. */
BOOL16  mi_varR                      = YES;
short   g_mspha                  = 0;
unsigned char * mi_dbase      = (unsigned char *) 0;

/* ---- MIDI sequencer state ------------------------------------------- */
unsigned char * mi_sqpos       = (unsigned char *) 0;
long            g_msmap   = -1;
long            mi_env = 0;
/* MIDI/PSG defaults.  Ghidra stores these as BYTES (not shorts) at their
   addresses; the code accesses them via move.b / cmp.b instructions.
   Values verified via disassembly at 0x101f4 / 0x10420 / 0x112a8 etc.
     mi_vel            @ 0x29a22 = 0x7F (127) -- max MIDI velocity
     mi_dvel    @ 0x29a24 = 0x7F (127)
     psg_dvol       @ 0x29a26 = 0x0F (15)  -- max PSG volume
   Port previously had mi_vel/default at 100 (guess). */
short           mi_vel           = 127;
short           mi_dvel   = 127;
short           psg_cvol      = 15;
short           psg_dvol      = 15;
/* mi_evi (midi_note_event_index @ 0x4b9ca) and mi_evcn
   (midi_note_event_count @ 0x4b9cc) are declared with the sequencer
   state block further down; mq_setp resets them at song start. */
/* Ghidra midi_channel_count @ 0x298F0 = 1 (byte).  Ports mh_chac
   writes p[2] here and passes through mq_bust. */
short           g_mchcn                 = 1;
/* Ghidra midi_ticks_per_beat @ 0x298F4 = 20; mi_temp @ 0x298F2 = 120. */
short           g_mtspb     = 20;
short           mi_temp              = 120;
/* aes_intO: shared AES/VDI parameter return array (16 shorts wide),
   used here at index 7 to communicate the current tick-per-beat back
   to the interrupt handler. */
short           aes_intO[16];

long            g_mtcou       = 0;
short           mi_dwrm  = 0;
short           g_mtdiv       = 100;
short           g_mtpre     = 100;
/* mi_nlp0 (midi_event_duration @ 0x4b7b0) is declared further down
   with the sequencer state block; mq_stap resets it at song start. */
short           mi_nxTk    = 100;
short           mi_lpTk= 100;
BOOL16          g_msmsa   = NO;

/* Timer-A interrupt state.
   mi_rlock -- reentrancy guard so the tick handler doesn't recurse
                        into the sequencer if a game-code path (e.g. a UI
                        response) triggers another timer event before the
                        first handler completes.
   mi_svtv  -- previous Timer-A vector, saved so cs_mvIn's shutdown
                        path can restore it (currently we install for the
                        lifetime of the process, but the slot is here
                        for future symmetry with the ROM's teardown). */
short           mi_rlock                = 0;
long            mi_svtv                    = 0;

/* ---- MIDI sequencer parse state -----------------------------------
   The sequencer walks a 3-byte-per-event compact stream inside
   mi_sqpos..mi_seqE.  Per-event scratch (event-type flag, note-on
   trigger, current note/channel, note-length params) is unpacked
   into a set of byte / short globals below, then handed to
   queue-note-event / send-note-off / send-program-change to reach
   the mq_dise dispatcher.

   mi_ndt is the 32-entry duration lookup indexed by byte1[0..4] of
   each note event; values pulled from ROM 0x298f6 (21 real
   entries, rest are zero). */

unsigned char * mi_seqE      = (unsigned char *) 0;
unsigned char * mi_dptr      = (unsigned char *) 0;
char            mi_evTf         = 0;
char            mi_nnOn        = 0;
char            mi_lasT         = 0;
char            mi_nnOf         = 0;
char            mi_ccha         = 0;
char            mi_cnot         = 0;
char            mi_nmof         = 0;
char            mi_nlpA         = 0;
short           mi_nlp0         = 0;
BOOL16          mi_slop         = NO;

short           mi_ndt[32] = {
           0,    2,    2,    3,    4,    5,    6,    8,
           9,   12,   16,   18,   24,   32,   36,   48,
          64,   72,   96,  128,  144,    0,    0,    0,
           0,    0,    0,    0,    0,    0,    0,    0
};

/* Event queue -- 3 shorts per active note: {duration, note|flags,
   physical MIDI channel byte}.  Max 60 slots -> 20 concurrent
   notes. */
short           mi_evq[60];
short           mi_evi          = 0;

/* Loop stack -- {return_addr, remaining_count} pairs.  Max 24
   nested loops (48 entries + 2 slack). */
long            mi_lstk[50];
short           mi_evcn         = 0;

/* Per-MIDI-note bookkeeping (128 possible notes -- one byte each). */
unsigned char   mi_nOS[128];

/* ---- PSG envelope processor state -----------------------------------
   Bresenham-style integer ramp accumulator + delta, per channel.
   Every psg_upEn tick, accum += delta; whenever accum > 360 (0x168),
   current_volume steps by ramp_direction and accum -= 360.  This
   fractional accumulation lets the 50 Hz envelope produce
   sub-tick-precision volume ramps without floating point.

   All 4 envelope tables (rate/time/sustain/release) are 16 shorts
   each, addressed by the low nibble of the ADSR bytes.
   psg_rot is the {0x88, 0x89, 0x8a} amp-register-with-write-bit
   for the 3 PSG channels; the assembly subtracts 0x80 back off
   before the actual psg_wr call. */
short           psg_rmpD[3];      /* ramp_delta   */
short           psg_rmpA[3];      /* ramp_accum   */

/* Ghidra midi_envelope_rate_table @0x2986c.  32-byte table indexed
   by phase_timer (already loaded from an ADSR duration byte). */
short           mi_evrt[16] = {
             0,  360,  180,  120,   85,   72,   60,   45,
            30,   20,   15,   12,   10,    8,    6,    4
};

/* midi_envelope_time_table @0x2988c.  Reload value for phase_timer
   when transitioning between ADSR phases. */
short           mi_evtt[16] = {
             0,    1,    2,    3,    4,    5,    6,    8,
            12,   18,   24,   30,   36,   45,   60,   90
};

/* midi_envelope_release_table @0x298ac.  Applied to ramp_delta
   during the sustain->release transition. */
short           mi_evrl[16] = {
             0,    1,    2,    4,    8,   18,   24,   40,
            45,   60,   72,   90,  120,  180,  360, 30000
};

/* midi_envelope_sustain_table @0x298cc.  Reload for phase_timer
   during the sustain->release transition. */
short           mi_evst[16] = {
             0,  360,  180,   90,   45,   20,   15,    9,
             8,    6,    5,    4,    3,    2,    1,    0
};

/* psg_register_offset_table @0x2985c.  Amp registers 8/9/10 with
   the PSG "write" bit (0x80) pre-set.  psg_upEn subtracts 0x80
   before calling psg_wr to recover the raw register number. */
unsigned char   psg_rot[3]  = { 0x88, 0x89, 0x8a };

/* Per-logical-channel maps.  Populated from the 90-byte channel-map
   block that precedes the header events; mq_resp
   iterates over them at song start. */
unsigned char   mi_chmap[16];
short           g_mcpro[16];
short           mi_pgmap[16];

/* mi_noSt (Ghidra midi_noteon_state @ 0x53df8): 128-entry table tracking
   which MIDI notes are currently sounding and on which logical channel.
   Value 0 = note not sounding.  Non-zero = the mi_chmap[] index (low
   nibble used) that owns the note, so mq_stop can emit a matching
   note-off through the correct MIDI channel on shutdown. */
unsigned char   mi_noSt[128];

/* 132-entry (0x84) note transpose lookup.  Indexed by MIDI note number
   0..131 (C-1..G9).  Populated by mq_bust at song
   start; each note maps to either itself (identity) or a shifted note
   under a chord mask, or 0xFF to skip (chromatic non-diatonic tones). */
unsigned char   g_mstr[132];

/* g_msmk (Ghidra midi_scale_mask_table @ 0x29ad0): 16-byte chord-mask
   lookup.  Dumped verbatim -- previous port had guessed the values
   from Music Studio 2.0 documentation but the real ones diverge
   significantly (e.g. slot 3 is 0x37 not 0x6F, slot 4 is 0x33 not 0x77). */
unsigned char   g_msmk[16] = {
        0xFF, 0xFF, 0x77, 0x37, 0x33, 0x13, 0x11, 0x01,
        0x00, 0xFE, 0xEE, 0xEC, 0xCC, 0xC8, 0x88, 0x00
};

BOOL16          g_moen     = YES;
unsigned char   g_meve[4];

/* g_momap: the "maxPos" argument passed to
   mq_inis at song start.  0 means "no explicit end-of-song
   offset -- let the sequencer walk the event stream to its natural
   terminator" (in which case mq_setp stores -1 into
   g_msmap).  A .SNG file may carry a real byte offset
   here to trigger clean loop-back or fade-out at a specific point.
   Renamed from Ghidra's placeholder gSongMaxPosition_0. */
long            g_momap  = 0;

/* ---- PSG channel state ---------------------------------------------- */
BOOL16          psg_out              = YES;
BOOL16          psg_ntAc                = NO;
unsigned char   psg_chNt[3];           /* current MIDI note per PSG channel A/B/C */
PSG_ENVELOPE    psg_envelope[3];

/* psg_freq[132] -- populated in psgfreq.c from first
   principles (YM2149 formula: period = 2000000 / (16 * midi_freq)).
   Definition lives in its own TU so the ~1KB of table data doesn't
   clutter globals.c. */

short           env_val            = 5;    /* octave-5 baseline */
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
/* Ghidra mi_ntLp @ 0x53f7a: 26 pointers (104 bytes to
   next symbol).  sf_sl loops up to 500 iterations breaking on size==0,
   so the array should be sized for the max number of entries in
   SOUNDS.LCP; 64 gives plenty of headroom. */
unsigned char * mi_ntLp[64];
/* Working buffer for the currently-playing Dosound sequence, copied
   from mi_ntLp[g_sfcur] each time a new
   effect starts. */
char            g_sfDoB[256];

void *  g_srlgb                  = (void *) 0;
void *  sv_lgb                    = (void *) 0;
void *  g_srptr                      = (void *) 0;
/* dsb_stor: dedicated 32 KB offscreen buffer where the
   letter-typing status strip composites, kept separate from the
   main house buffer.  fillTopR(27) writes rows
   0..26 here so that the striped-white letter background is ready
   for the typewriter animation; screen_render_8hz blkcp32's the
   content into the compositor screen when the letter overlay is
   active.
   Sized 32000 (one ST low-res screen) + 512 (worst-case align-up
   slack from `(base + 0x200) & ~0x1FF`, verified via raw disasm of
   fillTopR at 0x1686c) + margin.  g_dsb is
   set to the ALIGNED start in stpScrB -- do not
   initialise it here. */
short   dsb_stor[17408];
short * g_dsb = (short *) 0;

/* scr_scal (Ghidra 0x47ED0) -- always 1 (REZ_ST_MEDIUM).
   Multiplier for the 320x200 low-res screen dimensions in
   sprite_init_MFDB, matching the shape of the 1985 code even though
   the value is a constant. */
short   scr_scal             = 1;

/* MFDB_A (Ghidra 0x2C82A) -- source MFDB for VDI raster copies.
   fd_addr = NULL is the VDI convention for "device screen", so
   vro_cpyfm(...) copies from the visible physbase into a memory
   buffer instead of another off-screen bitmap. */
MFDB    MFDB_A                          = { 0 };

/* scrbufA / scrbufB (Ghidra SCREEN_BUFFER_A / _B) -- BSS scratch
   for the two double-buffer compositing screens.

   All four screen-pointer sites (stpScrB,
   fillTopR, sprite_init_MFDBs,
   screen_render_8hz alt) use the same align-up pattern:
        aligned = (base + 0x200) & ~0x1FF        (verified via raw
   disasm at 0x16576 / 0x1686c / 0x25116).  Ghidra's decompiler
   folds the compile-time-known base + 0x200 into bogus literal
   offsets ("+0x12F", "+0x7F", "+0xCD", "0x2CA00") which the port
   MUST NOT reproduce -- our BSS placement is different.

   scrbufB (~33 KB): holds the decompressed house.scn background.
     Uses one aligned screen at (scrbufB + 0x200) & ~0x1FF.
     Worst-case shift = 512 bytes, so size >= 32000 + 512 = 32512.

   scrbufA (64 KB): holds TWO 32 KB screens for the sprite
     compositor and the alt page-flip target.
       compositor = (scrbufA + 0x200) & ~0x1FF   (sp_imfs)
       alt        = compositor + 0x8000          (renderf.c)
     Worst-case footprint = 512 (align) + 0x8000 (alt offset)
     + 32000 (alt screen) = 65280 bytes; scrbufA[65536] fits. */
unsigned char   scrbufA[65536];
unsigned char   scrbufB[33280];

/* sprite_mfdb_image / sprite_mfdb_mask are Ghidra's names for the
   per-slot 8-way sprite MFDBs.  Our port already had them under the
   older names g_semfi / g_semfm (defined later in this file with
   { { 0 } } initializers) and referenced from sprender.c's sp_draw.
   sp_imfs writes through those existing arrays -- see sprites.c. */

/* Ghidra clock_minute @ 0x2B562 = 5, clock_hour @ 0x2B564 = 6.
   These are the "last-drawn" hand positions.  t_min/t_hour
   start at 0 (BSS), so the first cl_redrH call sees a mismatch
   and paints the initial 0:00 hands over the pre-drawn 5:06 default. */
short   g_cmmin                         = 5;
short   g_chhou                         = 6;

BOOL16  g_sfacf         = NO;
short   g_sfcur             = 0;
short   g_sfdur            = 0;
short   g_sfdos      = 0;
short   g_sfdoc     = 0;
/* sf_pri (Ghidra 0x2b44c, 32-byte array indexed
   by SOUND_EFFECT_ID).  Lower value = higher priority (a new SFX
   preempts the current if the new one's priority <= the current's).
   Notable: SFX 12/13 (DOORBELL, DOORBELL_ECHO) at priority 0 beat
   everything; footsteps 0..5 at 30 lose to everything.
   Dumped verbatim from the data segment -- previous port had guessed
   values (0/5/3/8/etc) that gave wrong preemption. */
short   sf_pri[32] = {
         30,  30,  30,  30,  30,  30,  15,  15,
         15,  15,  15,  15,   0,   0,  15,  15,
         15,  15,  15,  14,  16,   1,  15,   0,
          0,   0, 205,  77, 115, 116, 117, 100
};

/* Raw file buffers -- populated at startup by asset_load_all().
   OBJECTS and SPRITES both size at 14000 bytes per Ghidra
   ldObj / ldSpr decompiles. */
unsigned char   obj_file[14000];
unsigned char   spr_file[14000];

/* Per-record MFDB tables + dimensions.  Sized to comfortably cover
   the 50 or so records in each file (spritedata_index_table in the
   Python reader tops out at index 0x37). */
MFDB    g_obtmt[64];
MFDB    g_setmt[64];

short   g_obtaw[64];
short   g_obtah[64];
short   g_setaw[64];
short   g_setah[64];
/* mf_scrp now defined below with the rest of the frame-timing
   MFDB descriptors. */

/* Ghidra letter_line_count @ 0x2b5a2 = -1 (short).  First frame of
   rp_anim (record-player needle sweep) skips the draw when g_ltlic
   is < 0, then decrements to -3, then wraps to 13.  Port had 0. */
short   g_ltlic                         = -1;
short   g_ltpac          = 0;
/* rec_ledt[7]: bit-mask toggles for the 7 VU-meter LEDs. */
unsigned short  rec_ledt[7] = {
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

BOOL16  g_inpmd            = NO;
char    g_cdinb[64];
BOOL16  food_dlv         = NO;
short   g_ptanf              = 0;

short   last_hz                      = 0;
long    last_vbc                    = 0;
/* sv_phb: TOS's original Physbase, captured once at boot by
   aes_init via Physbase().  BSS-zero to match Ghidra's
   binary (the port previously initialised it to 0x28000L which put it
   in .data with a bogus fallback -- aes_init runs early so the
   fallback was never read, but matching Ghidra's memory layout keeps
   any future .data / BSS-boundary bug from being silently absorbed). */
void *  sv_phb                   = (void *) 0;

/* g_srmfd / mf_scrp: the compositing target and the current
   physical screen descriptor.  Populated by the graphics init routine. */
MFDB    g_srmfd                     = { 0 };
MFDB    mf_scrp                 = { 0 };
MFDB *  cur_mf             = (MFDB *) 0;

/* 200 Hz clock hi/lo halves.  The ST reads this atomically via a
   supervisor-mode long read at 0x4BA; we split the halves here so a
   short-sized access still compiles cleanly on the host. */
short   g_hzhi                      = 0;
short   g_hzlo                      = 0;
long    _vbclock                        = 0;

BOOL16  dg_vis                     = NO;
short   dg_idlcd              = 0;
BOOL16  dg_nrbwl              = NO;
BOOL16  g_deact               = NO;
short   g_decou            = 0;
short   dg_ltgtI           = 0;
/* Ghidra g_dgitx @ 0x2b8f0 = POS_BTM_SCREEN_EDGE.  Used by cutscene
   at startup to seed the dog's first wander target -- the dog walks
   in from the bottom-screen edge. */
short   g_dgitx        = POS_BTM_SCREEN_EDGE;
/* Ghidra g_dgiyo @ 0x2b904 = 3.  Y micro-nudge applied
   to the initial dog target position. */
short   g_dgiyo            = 3;
short   g_dseat[3]   = {
        SPRITE_DOG_EATING_1, SPRITE_DOG_EATING_2, SPRITE_DOG_EATING_3
};
/* Ghidra dog_destination_position_table @ 0x2B8DE, 10 HOUSE_POS
   entries the dog picks (via rndRng) as its next wander target.
   Last two duplicate POS_BTM_SCREEN_EDGE so it's picked with 2x
   probability -- the dog favours wandering off-screen. */
short   g_ddipt[10] = {
        POS_TOP_LIVING_ROOM,       POS_TOP_GAME_CHAIR_RIGHT,
        POS_TOP_FIREPLACE_RIGHT,   POS_MID_BEDROOM_WALK,
        POS_MID_COMPUTER_DESK,     POS_BTM_STAIR_LANDING,
        POS_BTM_DOG_BOWL,          POS_BTM_WATER_TAP,
        POS_BTM_SCREEN_EDGE,       POS_BTM_SCREEN_EDGE
};
/* Ghidra dog_dest_x_offset_table @ 0x2B906, dog_dest_y_offset_table
   @ 0x2B8F2 (10 shorts each): per-destination pixel nudges applied
   after hs_posXY returns the anchor for the destination. */
short   g_ddxot[10]     = { 0, 0, 0, 0, 10, 0, 0, 0, 0, 0 };
short   g_ddyot[10]     = { 3, 9, 2, 10, 6, 0, 0, 11, 3, 3 };

char *  cmd_inp              = (char *) 0;
short   g_aprio                = 5;

/* Per-slot MFDB arrays for the masked-blit sprite pipeline. */
MFDB    g_semfi[SPRITE_HW_SLOTS] = { { 0 } };
MFDB    g_semfm[SPRITE_HW_SLOTS] = { { 0 } };

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
/* Ghidra user_input_buffer @ 0x4b782: 42-byte ROM slot.  Port
   previously declared [32] which cmd_upp() could overflow: it walks
   input from g_cdinb (bounded < 38 chars) and writes one byte per
   alphabetic char to usr_buf, potentially 38+ bytes. */
char            usr_buf[42];
/* Ghidra happiniess_to_priority (sic) @ 0x2bf98: {3, 1, 0}.  Used as
   the base priority for parsed commands -- HAPPY (0) gives priority 3
   (accepts more), SAD (2) gives 0 (rejects most).  Port previously
   had guessed {2, 4, 6} which inverted the intended behavior. */
short           mood_pri[3]        = { 3, 1, 0 };
unsigned char   bm_lo[9] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00
};

/* ---- Mini-game storage ----------------------------------------------- */
char *          g_agwb            = (char *) 0;
char *          g_wpdb         = (char *) 0;
short *         crd_dat                      = (short *) 0;

short           g_wpci       = 0;
short           g_agclc              = 0;
short           g_aggun            = 1;
short           g_agacu          = 0;
short           ag_clue   = 0;
short           g_agwol             = 0;
char            g_aginb[12];
/* anagram_original_word: pointer into g_agwb dictionary (11-byte rows)
   set by ag_ssw when a word is picked.  Ghidra treats it as char *. */
char *          g_agorw           = (char *) 0;
char            g_agscw[12];
char *          g_agwgm[3] = {
        "Nope, try again!",
        "Not quite...",
        "Sorry, wrong guess."
};

/* anagram_guess_prompt_strings: shown per attempt (0..8 -> "Guess #1?"..
   "Guess #9?").  Rendered by ag_sgp at (166, 57). */
char *          g_aggpr[9] = {
        "Guess #1?",
        "Guess #2?",
        "Guess #3?",
        "Guess #4?",
        "Guess #5?",
        "Guess #6?",
        "Guess #7?",
        "Guess #8?",
        "Guess #9?"
};

/* Mini-game shared state.
   mg_tofl: set YES by mg_wkev when the 7200-frame (~15 min) idle
            timeout fires; games check it to distinguish "user pressed
            F10" from "we auto-quit due to inactivity".
   sv_vqta: 10-short buffer holding the pre-mini-game VDI text
            attributes so rst_vsth can restore them after temporarily
            switching to 20-pixel height for the title/answer render. */
BOOL16          mg_tofl                    = NO;
short           sv_vqta[10];

short           pk_round              = 0;
BOOL16          pk_quit                 = NO;
short           g_pcmon            = 400;
short           g_ppmon              = 400;
short           g_ppppa                = 0;
short           g_pcbet              = 0;
short           g_ppbet                = 0;
short           pk_phase                = 0;
short           pk_dsc[52];
/* Ghidra poker_computer_draw_pile @ 0x47e24 and poker_player_draw_pile
   @ 0x3f712: 52-short ROM slots (104 bytes each).  Port previously
   declared [26], which pk_rmch's unconditional
     for (i = 0; i < 51; i = i + 1) pile[i] = pile[i + 1];
   overflowed by 25 slots (50 bytes) per draw.  The sibling
   pk_dpile[52] was already correctly sized. */
short           g_pcdrp[52];
short           g_ppdrp[52];

/* War/Blackjack per-round face-down "war" cards.  Sized 52 so the
   deepest possible recursion (all cards ending up here) still fits.
   CARD_NONE sentinel terminates.  g_pchc counts the number of prior
   war rounds this hand (indexes further into the arrays). */
short           pk_pwc[52];             /* poker_player_war_cards */
short           pk_cwc[52];             /* poker_computer_war_cards */
short           g_pchc            = 0;  /* poker_computer_hand_cards */

BOOL16          moff_f              = NO;

/* Poker (5-card draw) working state.  Every field is per-hand: reset
   at the start of each round in pk_ante / pk_evhs / pk_show. */
short           pk_ch[5];           /* computer_hand -- CARD_TYPE 0..51 */
short           pk_ph[5];           /* player_hand */
short           pk_hrf[5];          /* hand_rank_flags   -- which cards
                                       form computer's pair/trip/etc */
short           pk_hsf[5];          /* hand_suit_flags   -- sorted copy
                                       of computer hand (used as kicker
                                       scratch by pk_show) */
short           pk_phrf[5];         /* player_hand_rank_flags */
short           pk_phsf[5];         /* player_hand_suit_flags */
short           pk_chrk    = 0;     /* computer_hand_rank
                                       0=high,1=pair,2=two-pair,3=trips,
                                       4=straight,5=flush,6=full,7=four,
                                       8=straight-flush,9=royal */
short           pk_phrk    = 0;     /* player_hand_rank */
short           pk_dslot   = 0;     /* winner (0=comp, 1=player) */
short           pk_sel[5];          /* card_selected -- 1 = discard */
short           pk_disc    = 0;     /* discard_count */
short           pk_dpile[52];       /* discard_pile of already-seen cards */
short           pk_dpos    = 0;     /* deck_position -- reused as
                                       raise amount / draw counter */
short           pk_phv     = 0;     /* player_hand_value -- saved bet */
short           pk_bet     = 0;     /* current bet accumulator (shared) */
BOOL16          pk_bluff   = NO;    /* computer intends to bluff */
BOOL16          pk_pass    = NO;    /* computer passed on the bet loop */

/* Editable poker prompts.  pk_bm / pk_rm have single-space digit
   slots at fixed offsets; pk_tcm's card count digit + trailing
   period/'s.' get patched in by pk_cdrw.  Buffer widths sized so
   the biggest overwrite (a 2-digit prefix like "20") still fits. */
char            pk_bm[]   = "I'll bet 00.  ";
char            pk_rm[]   = "I'll raise 00.";
char            pk_tcm[]  = "I'll take 0 cards.";

/* Blackjack per-hand state.
   pk_psh[]  -- 3rd hand slot used when the player elects to split
                two matching down-cards (post-deal, both aces or
                two of the same rank).  CARD_NONE-terminated.
   pk_pcc / pk_ccc / pk_pscc -- remaining-hits counters (start at
                CARD_BJ_MAX = 3 for standard "up to 5 cards" rule;
                each hit decrements by CARD_BJ_STEP = 1; stop at
                CARD_BJ_STOP = 0).
   pk_wpr    -- saved bet during split-hand bookkeeping (Ghidra
                calls it poker_war_player_score).
   pk_wrf / pk_wcs  -- split-hand round-active flags (Ghidra:
                _poker_war_round / _poker_war_computer_score).
   pk_c1bj / pk_c2bj -- first / second hand natural-blackjack
                achieved this round (used to skip the hit loop).
                Ghidra reused _poker_computer_card_count /
                _poker_card_deck_index for these.
   pk_bs1 / pk_bs2  -- first / second hand busted flag.  Ghidra
                reused _poker_computer_hand_value_lo / _hi.
   pk_cscore / pk_pscore -- computer-picked / player-picked score
                once the double-value-with-ace picker resolves.
                Ghidra reused poker_display_x_offset and
                midi_dma_start_lo.
*/
short           pk_psh[5];      /* player_split_hand */
short           pk_pcc     = 0; /* player_card_count       */
short           pk_ccc     = 0; /* computer_card_count     */
short           pk_pscc    = 0; /* player_split_card_count */
short           pk_wpr     = 0; /* saved bet across split  */
BOOL16          pk_wrf     = NO;
BOOL16          pk_wcs     = NO;
BOOL16          pk_c1bj    = NO;
BOOL16          pk_c2bj    = NO;
BOOL16          pk_bs1     = NO;
BOOL16          pk_bs2     = NO;
short           pk_cscore  = 0;
short           pk_pscore  = 0;

/* Word Puzzle state.
   wp_ans[i][12]  -- player's typed answer for blank i.  Max 10
                     chars + terminator + 1 slack byte.
   wp_blk         -- count of blanks in the current puzzle (== rows
                     of wp_ans[] actually in use).
   The 3 flavor-text pointer arrays hold string literals shown to
   the player during solve_phase, extracted verbatim from ROM via
   the Ghidra HTTP /read_memory endpoint:
      wp_prm  @ 0x2a46c   9 entries (0..4 random first-word, 5..8
                                                        for word slots 2..5)
      wp_succ @ 0x2a490   6 entries, random on solve
      wp_fail @ 0x2a4a8   6 entries, random on wrong answer  */
char            wp_ans[10][12];
short           wp_blk    = 0;

char *          wp_prm[9] = {
        "OK, what's the first word?",
        "Good luck! What's the first word?",
        "Alright. Type in the first word.",
        "This won't be easy! First word first.",
        "Here we go. What's the first word?",
        "What's the second word?",
        "What's the third word?",
        "What's the fourth word?",
        "What's the fifth word?"
};

char *          wp_succ[6] = {
        "You got it!!",
        "Good going. That's right!",
        "Congratulations. That's it!",
        "I don't believe it!! You're right!",
        "You're pretty good. That's right!",
        "You got that one. How about another?"
};

char *          wp_fail[6] = {
        "Too bad. You missed it.",
        "Better luck next time.",
        "Good try, but that's the wrong answer.",
        "That's not it. How about another try?",
        "Nope.",
        "Not quite."
};

/* Card display positions -- 5 slots per row, extracted from Ghidra
   memory at 0x2a4fe / 0x2a508 / 0x2a512 / 0x2a51c.  Row A = computer
   (y=11 top strip), Row B = player (y=37 middle strip).  X columns
   are spaced 28 pixels apart (15-px card + 13-px gutter). */
short           crd_xa[5]         = { 70, 98, 126, 154, 182 };
short           crd_ya[5]         = { 11, 11, 11, 11, 11 };
short           crd_xb[5]         = { 70, 98, 126, 154, 182 };
short           crd_yb[5]         = { 37, 37, 37, 37, 37 };

/* 54-entry MFDB table covering 52 card faces + 1 back + 1 highlight
   overlay.  All share crd_dat as their bitmap backing. */
MFDB            crd_mfdb[54]           = { { 0 } };
MFDB            mf_scb_c      = { 0 };

BOOL16  g_dvdog             = NO;
BOOL16  ph_hu               = NO;
BOOL16  g_ptdoa              = NO;

/* Openable-object frame ids.  The ROM keeps the whole set as
   initialized word globals (base-0 data 0x11758..0x1177e) and every
   od_draw of a door / appliance reads them -- it never pushes the
   enum constants.  Values verified against the original DATA
   segment; a_kitcc additionally takes &od_ph2 as an array base. */
short   od_stcl = 46;           /* 0x11758 OBJ_DOOR_STUDY_CLOSED   */
short   od_sto1 = 47;           /* 0x1175a OBJ_DOOR_STUDY_OPEN_1   */
short   od_sto2 = 48;           /* 0x1175c OBJ_DOOR_STUDY_OPEN_2   */
short   od_frcl = 36;           /* 0x1175e OBJ_DOOR_FRONT_CLOSED   */
short   od_fro1 = 37;           /* 0x11760 OBJ_DOOR_FRONT_OPEN_1   */
short   od_fro2 = 38;           /* 0x11762 OBJ_DOOR_FRONT_OPEN_2   */
short   od_cbcl = 19;           /* 0x11764 OBJ_CABINET_CLOSED      */
short   od_cbo1 = 20;           /* 0x11766 OBJ_CABINET_OPEN_1      */
short   od_cbo2 = 21;           /* 0x11768 OBJ_CABINET_OPEN_2      */
short   od_med1 = 40;           /* 0x1176a OBJ_MEDICINE_OPEN_1     */
short   od_tocl = 25;           /* 0x1176c OBJ_DOOR_TOILET_CLOSED  */
short   od_too1 = 26;           /* 0x1176e OBJ_DOOR_TOILET_OPEN_1  */
short   od_too2 = 27;           /* 0x11770 OBJ_DOOR_TOILET_OPEN_2  */
short   od_ph1  = 22;           /* 0x11772 OBJ_PHONE_1             */
short   od_ph2  = 23;           /* 0x11774 OBJ_PHONE_2             */
short   od_ph3  = 24;           /* 0x11776 OBJ_PHONE_3             */
short   od_tcl2 = 25;           /* 0x11778 (unreferenced twin)     */
short   od_fdcl = 16;           /* 0x1177a OBJ_FRIDGE_CLOSED       */
short   od_fdo1 = 17;           /* 0x1177c OBJ_FRIDGE_OPEN_1       */
short   od_fdo2 = 18;           /* 0x1177e OBJ_FRIDGE_OPEN_2       */

/* Second ROM frame-id block (base-0 data 0x1200a..0x12026). */
short   od_clcl = 28;           /* 0x1200a OBJ_DOOR_CLOSET_CLOSED  */
short   od_clo1 = 29;           /* 0x1200c OBJ_DOOR_CLOSET_OPEN_1  */
short   od_clo2 = 30;           /* 0x1200e OBJ_DOOR_CLOSET_OPEN_2  */
short   od_fir0 = 31;           /* 0x12010 OBJ_FIRE_OFF            */
short   od_ficl = 0;            /* 0x1201a OBJ_FILING_CABINET_CLOSED */
short   od_fio1 = 1;            /* 0x1201c OBJ_FILING_CAB_OPEN_1   */
short   od_fio2 = 2;            /* 0x1201e OBJ_FILING_CAB_OPEN_2   */
short   od_drcl = 10;           /* 0x12020 OBJ_DRESSER_CLOSED      */
short   od_dro1 = 11;           /* 0x12022 OBJ_DRESSER_OPEN_1      */
short   od_dro2 = 12;           /* 0x12024 OBJ_DRESSER_OPEN_2      */

/* (gameTick animation tables + frame-state globals live
   in tick_tables.c -- Alcyon C168's symbol-table overflows if they
   are added here.) */
