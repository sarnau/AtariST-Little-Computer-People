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

short           bj_key;         /* pk_bjMn's key variable (a global in STX) */
char            psg_ovol;       /* psg_upEn's clamped output volume */
unsigned short  g_wkadj;        /* read once, in lcp_path's dead store */
unsigned short  ani_cnt;    /* STX: the & 7 test zero-extends */
short   g_secs;

short   t_min;
short   t_hour;
short   date_day;
short   dt_mon;
short   dt_year;

PLAYER  lcp;

BOOL16  ph_ans     = NO;
BOOL16  ph_call  = NO;
BOOL16  introSeq;

BOOL16  lunT_trg      = NO;
BOOL16  dinT_trg     = NO;
BOOL16  wkT_trg  = NO;
BOOL16  bedT_trg         = NO;

BOOL16  in_evrt;

short   lastAct;
short   g_trac                  = ACTION_NONE;

/* Ghidra's gameLoop always sets these via
   hs_posXY() during boot.  Ghidra keeps both in BSS;
   the cutscene sets them.  Port matches by leaving them at 0 -- the
   cutscene stub in init.c writes (300, 190) before gameLoop
   runs. */
short   lcp_x;
short   lcp_y;
BOOL16  g_lcldd;
long    cprot_r;      /* STX tests it with tst.l */
short   g_spdc;

BOOL16  alarm_p;
short   lcp_watr;

short   g_aliss;
short   g_aqueu[10];
short   g_apriq[10];

/* Ghidra head_anim_target_state @ 0x29b98 = 8, head_anim_current @ 0x29b96 = 8,
   head_anim_mode @ 0x29b9a = -1 (HEAD_ANIM_DISABLED). */
short   g_hatas                         = 8;
short   g_hacur                         = 8;
short   g_hamod                         = HEAD_ANIM_DISABLED;
short   g_hsfra;
long    g_sfret;
BOOL16  g_actif;
BOOL16  dg_petok               = NO;
short   g_wtx;
short   g_wty;
/* Ghidra triggered_event_list @ 0x2b6da: 10-short scratch buffer used
   by action handlers (bathroom, food, house, leisure, idle, simple)
   to cache a small set of state values indexed by variable expressions
   like `i & 3`.  Port previously declared [4], which was one byte
   short of a real out-of-bounds write via `pst_arr[4]` writes in the
   bathroom/food/house paths -- the fifth slot overlapped lcp_frdO. */
short   pst_arr[10];

short   lcp_frdO;
short   studyDrO;
short   lcp_clsO;
short   lcp_cabO;
short   lcp_drsO;
short   lcp_toiO;
short   lcp_flcO;
short   lcp_bwlS;
short   lcp_food;
short   lcp_recP              = 0;
short   lcp_tv                       = 0;

/* Openable-object frame-id slots, exactly in ROM data order
   (0x11758-0x1177e): study/front/cabinet/medicine/toilet doors,
   stove-off, the 3-slot stove-on table (g_obisa), then the fridge.
   od_draw sites read these slots, never enum constants. */
short   g_obisa[3]    = { 23, 24, 25 };  /* stove-on frame slots */


/* STX declares this a byte flag (tst.b at its use sites). */
char    mi_play;
short   dg_bwlch;
short   g_sfplf;
short   g_sfpli;

BOOL16  g_rbact          = NO;
char *  mi_sbuf;
/* Ghidra sng/org song file counts, set at boot by cntSong().
   BSS-zero to match Ghidra; port previously had org_cnt=8
   as a guess. */
short   sng_cnt;
short   org_cnt;
/* Six bytes of zero-initialized, completely unreferenced data the ROM
   carries between org_cnt and the PEx filename (data 0x11792). */
short   g_unus1[3]          = { 0, 0, 0 };
/* scn_cmn -- 30-byte scene common-data header shared between
   house.scn and title.scn.  Ghidra `scene_common_data` @ 0x4cf7c.
   The port's unScn helper (assets.c) fills this via fr_read. */
char    scn_cmn[30];
/* PEx.LCP filename.  Ghidra pex_name @ 0x2a0f8 points to "pex.lcp"
   at 0x2a330 and main() mutates index 2 to select the character.
   Port stores the string as a mutable static char array. */
char *  pex_name                     = "PE0.LCP";
BOOL16  fire_act                = NO;
short   fire_dur;
BOOL16  fire_ext;
short   no_keyin          = NO;
short   tx_sctm;
short   g_srsdc;
short   g_cdibp;

/* Letter subsystem storage.  g_ltlp[] and _greeting_table are
   populated at runtime from letter.txt (see fl_ltpl);
   NULL entries make lt_tysa a safe no-op on the
   host build until the template loader is ported.  360 slots covers
   the 4 sections × 96 pointers (section 3 uses 72) shape referenced by
   a_writl. */
char *  g_lttx;
char *  g_ltlp[512];
/* g_ltg[4]: letter sign-off pointers.  In THIS ROM they are four
   NULL pointers in DATA (0x11fb2) -- the sign-off strings do not
   exist in the binary, and a_writl's pick hits lt_tysa's NULL guard,
   so letters simply end without one.  (The static "Sincerely," set
   belongs to the other game revision.) */
char *  g_ltg[4]        = {
        (char *) 0, (char *) 0, (char *) 0, (char *) 0
};
/* 16 more zero bytes of unreferenced ROM data (0x11fc2) -- likely
   four more never-used sign-off slots. */
char *  g_unus2[4]      = {
        (char *) 0, (char *) 0, (char *) 0, (char *) 0
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

/* Second ROM frame-id block (data 0x1200a-0x12026): closet door,
   fire-off, filing cabinet, dresser, and the sc_drfc food marker. */
char    g_ltscb[64];
char    in_str[256];
/* comp_tok[15]: the 15 most common byte values in the
   compressed stream.  Populated at load-time by fr_reac
   from the 15-byte header immediately following the size word. */
/* scn_dic[15]: the 15-entry word dictionary at the head of a .SCN
   file, and the size/buffer main uses while decoding one.  LCP_STX
   keeps all three as globals -- the .SCN file handling is inlined in
   main and only the nibble decoder is a function. */
short           scn_dic[15];
unsigned char   comp_tok[15];
short           scn_siz;
char *          scn_buf;

short * sv_bodyP;
short * sv_headP;

/* VDI init happens in graphics setup; on the host we default to a
   sentinel handle that the VDI stubs ignore. */
short   vdihnd;
short   vdi_hnd;    /* physical from graf_handle */
/* LCP_STX's aes_init has an empty frame: graf_handle writes its four
   cell/box metrics into globals, not into locals. */
short   gr_hwchar;
short   gr_hhchar;
short   gr_hwbox;
short   gr_hhbox;
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

/* The ROM's VDI parameter block (data 0x12054): the game-local
   arrays used by vdiown.c's bindings and vdi_go. */
short * vdipb[5];

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

void *  g_dscp;

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

/* Ghidra mi_varR @ 0x29af2 = 1 (byte).  Port previously had NO.
   LCP_STX declares both as char -- sgPlay writes them with moveb. */
char    g_molof             = NO;
char    mi_varR                      = YES;
char    g_mspha;   /* STX: byte */
unsigned char * mi_dbase;

/* ---- MIDI sequencer state ------------------------------------------- */
unsigned char * mi_sqpos;
long            g_msmap;
long            mi_env;
/* MIDI/PSG defaults.  Ghidra stores these as BYTES (not shorts) at their
   addresses; the code accesses them via move.b / cmp.b instructions.
   Values verified via disassembly at 0x101f4 / 0x10420 / 0x112a8 etc.
     mi_vel            @ 0x29a22 = 0x7F (127) -- max MIDI velocity
     mi_dvel    @ 0x29a24 = 0x7F (127)
     psg_dvol       @ 0x29a26 = 0x0F (15)  -- max PSG volume
   Port previously had mi_vel/default at 100 (guess). */
char            mi_vel           = 127; /* STX: byte */
/* STX declares these two as char (byte compares/stores; Alcyon
   word-aligns them, hence the 2-byte spacing). */
char            mi_dvel   = 127;
char            psg_cvol;     /* STX: byte */
char            psg_dvol      = 15;
/* mi_evi / mi_evcn live HERE in the ROM's data (0x120fa/0x120fc),
   with mi_evcn initialized to 9. */
short           mi_evi;
short           mi_evcn;
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

long            g_mtcou;
short           g_mtdiv;
/* mi_nlp0 (ROM data 0x1210e, initialized 100 like its neighbours);
   mq_stap resets it at song start. */
short           mi_nlp0;
long            mi_nxTk;       /* STX: long tick counters */
long            mi_lpTk;
/* g_msmsa is a BYTE and lives in the text segment behind mq_tick --
   see source/mq_tick.s. */
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

/* The remaining sequencer/PSG working state below belongs to the
   Timer-A music engine. */

/* Timer-A interrupt state.
   mi_rlock -- reentrancy guard so the tick handler doesn't recurse
                        into the sequencer if a game-code path (e.g. a UI
                        response) triggers another timer event before the
                        first handler completes.
   mi_svtv  -- previous Timer-A vector, saved so cs_mvIn's shutdown
                        path can restore it (currently we install for the
                        lifetime of the process, but the slot is here
                        for future symmetry with the ROM's teardown). */
long            mi_svtv;

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

unsigned char * mi_seqE;
unsigned char * mi_dptr;
char            mi_evTf;
char            mi_nnOn;
char            mi_lasT;
char            mi_nnOf;
char            mi_ccha;
char            mi_cnot;
char            mi_nmof;
char            mi_nlpA;
char            mi_slop         = NO;   /* STX: byte */

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

/* Loop stack -- {return_addr, remaining_count} pairs.  Max 24
   nested loops (48 entries + 2 slack). */
long            mi_lstk[50];

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


/* ---- PSG channel state ---------------------------------------------- */

/* Referenced by the ROM text (bss 0x3df96 / 0x4549a / 0x1dc7c /
   0x48bce). */
char            g_mcpro[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
/* LCP_STX's static content is 0..99 then 110..127, with the last 14
   entries zero -- the row 100..109 is simply missing from the 1985
   table.  Harmless: mq_bust rewrites all 132 entries (`g_mstr[i] = i`
   for i < 0x84) before anything reads them. */
unsigned char   g_mstr[132] = {
          0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
         12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,
         24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,
         36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
         48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
         60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,
         72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
         84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
         96,  97,  98,  99, 110, 111, 112, 113, 114, 115, 116, 117,
        118, 119, 120, 121, 122, 123, 124, 125, 126, 127
};
unsigned char   mi_chmap[16] = { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char            mi_pgmapb[16];
char *          mi_pgmap = mi_pgmapb;   /* STX: byte pointer */

BOOL16          psg_out              = YES;
/* psg_ntAc is a BYTE in the text segment behind mq_tick -- see
   source/mq_tick.s. */
short           env_val            = 5;    /* octave-5 baseline */
char            g_mnlol      = 0x17; /* A#0 */
char           g_mnhil       = 0x7f; /* MIDI max         */
short           g_mccha    = 1;
unsigned char   psg_chNt[3];           /* current MIDI note per PSG channel A/B/C */
PSG_ENVELOPE    psg_envelope[3];

/* psg_freq[132] -- populated in psgfreq.c from first
   principles (YM2149 formula: period = 2000000 / (16 * midi_freq)).
   Definition lives in its own TU so the ~1KB of table data doesn't
   clutter globals.c. */


/* ---- SFX / Dosound state -------------------------------------------- */
char            g_sfcup;
short           g_sfddh;
short           g_sfddl;
long            g_sfHz2;
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

void *  g_srlgb;
void *  sv_lgb;
void *  g_srptr;
/* dsb_stor: dedicated 32 KB offscreen buffer where the
   letter-typing status strip composites, kept separate from the
   main house buffer.  fillTopR(27) writes rows
   0..26 here so that the striped-white letter background is ready
   for the typewriter animation; screen_render_8hz blkcp32's the
   content into the compositor screen when the letter overlay is
   active.
   Sized from what fillTopR can actually write: its largest caller is
   mg_stp's fillTopR(0x4d), 77 rows of 160 bytes = 12320, and the
   align-up `(base + 512) & ~511` moves the start by at most 512 --
   so 12832 bytes, 6416 shorts.  (LCP_STX's own gap here is 12836.)
   g_dsb points at the raw base; stpScrB re-points it to the ALIGNED
   start at run time. */
short   dsb_stor[6416];
short * g_dsb = dsb_stor;

/* scr_scal (Ghidra 0x47ED0) -- always 1 (REZ_ST_MEDIUM).
   Multiplier for the 320x200 low-res screen dimensions in
   sprite_init_MFDB, matching the shape of the 1985 code even though
   the value is a constant. */
short   scr_scal;

/* LCP_STX's vdi_init opens the workstation through GLOBAL work
   arrays, not locals (its frame is only -6). */
short   work_in[11];
short   wk_out[57];

/* MFDB_A (Ghidra 0x2C82A) -- source MFDB for VDI raster copies.
   fd_addr = NULL is the VDI convention for "device screen", so
   vro_cpyfm(...) copies from the visible physbase into a memory
   buffer instead of another off-screen bitmap. */
MFDB    MFDB_A;

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

   Each holds ONE aligned screen: scrbufA the sprite compositor (also
   sc_ren8's alternate page-flip target -- there is no second screen
   at +0x8000, see parts/sc_ren8.c), scrbufB the decompressed
   house.scn background.

   Sized the way an ST program allocates a screen: declare a round
   32768 and align the base up inside it, leaving 32768 - slack >=
   32000 usable.  (The hardware needs 256-byte alignment; this code
   aligns to 512 -- `(base + 0x200) & ~0x1FF`.)

   scrbufB cannot be 32768: LCP_STX puts pk_quit 32600 bytes after it
   (25 relocation sites agree on that address), so the round
   allocation does not fit there.  32512 is 32000 + the 512 the
   align-up can shift, and leaves 88 bytes for globals the port does
   not have.  scrbufA has 33045 bytes of room, so the round 32768
   fits it with 277 to spare. */
unsigned char   scrbufA[32768];
unsigned char   scrbufB[32512];

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

BOOL16  g_sfacf;
short   g_sfcur;
short   g_sfdur;
short   g_sfdos;
short   g_sfdoc;
/* sf_pri (Ghidra 0x2b44c, 32-byte array indexed
   by SOUND_EFFECT_ID).  Lower value = higher priority (a new SFX
   preempts the current if the new one's priority <= the current's).
   Notable: SFX 12/13 (DOORBELL, DOORBELL_ECHO) at priority 0 beat
   everything; footsteps 0..5 at 30 lose to everything.
   Dumped verbatim from the data segment -- previous port had guessed
   values (0/5/3/8/etc) that gave wrong preemption. */
char    sf_pri[32] = {          /* STX: one byte per entry */
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
/* od_draw reads the object-MFDB table through this pointer, exactly
   as the ROM does (initialized data 0x121b6 -> table in BSS). */
MFDB *  g_obtmp = g_obtmt;

short   g_obtaw[64];
short   g_obtah[64];
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

BOOL16  g_inpmd;
char    g_cdinb[64];
BOOL16  food_dlv;
short   g_ptanf;

unsigned short  last_hz;   /* STX: clr.w zero-extension at every use */
long    last_vbc;
/* sv_phb: TOS's original Physbase, captured once at boot by
   aes_init via Physbase().  BSS-zero to match Ghidra's
   binary (the port previously initialised it to 0x28000L which put it
   in .data with a bogus fallback -- aes_init runs early so the
   fallback was never read, but matching Ghidra's memory layout keeps
   any future .data / BSS-boundary bug from being silently absorbed). */
void *  sv_phb;

/* g_srmfd / mf_scrp: the compositing target and the current
   physical screen descriptor.  Populated by the graphics init routine. */
MFDB    g_srmfd;
MFDB    mf_scrp;
MFDB *  cur_mf;

/* 200 Hz clock hi/lo halves.  The ST reads this atomically via a
   supervisor-mode long read at 0x4BA; we split the halves here so a
   short-sized access still compiles cleanly on the host. */
short   g_hzhi                      = 0;
short   g_hzlo                      = 0;
long    _vbclock                        = 0;

BOOL16  dg_vis;
short   dg_idlcd;
BOOL16  dg_nrbwl;
BOOL16  g_deact;
short   g_decou;
short   dg_ltgtI;
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

char *  cmd_inp;
short   g_aprio;

/* Per-slot MFDB arrays for the masked-blit sprite pipeline. */
MFDB    g_semfi[SPRITE_HW_SLOTS];
MFDB    g_semfm[SPRITE_HW_SLOTS];

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

char            g_ewb[10];
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
char            bm_lo[9] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00
};

/* ---- Mini-game storage ----------------------------------------------- */
char *          g_agwb;
char *          g_wpdb;
short *         crd_dat;

short           g_wpci;
short           g_agclc;
short           g_aggun;
short           g_agacu          = 0;
short           ag_clue;
short           g_agwol;
char            g_aginb[12];
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
BOOL16          mg_tofl;
short           sv_vqta[10];

short           pk_round;
BOOL16          pk_quit;
short           g_pcbet;
short           g_ppbet;
short           g_pcmon;
short           g_ppmon;
short           g_ppppa;
/* anagram_original_word: pointer into g_agwb dictionary (11-byte rows)
   set by ag_ssw when a word is picked.  Ghidra treats it as char *.
   The ROM places it between g_ppppa and pk_phase (data 0x124aa). */
char *          g_agorw;
short           pk_phase;
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
short           g_pchc;  /* poker_computer_hand_cards */

BOOL16          moff_f = 1;

/* Poker (5-card draw) working state.  Every field is per-hand: reset
   at the start of each round in pk_ante / pk_evhs / pk_show.
   In the ROM pk_ch / pk_ph are the WAR hands: 26 shorts each. */
short           pk_ch[5];           /* computer_hand -- CARD_TYPE 0..51 */
short           pk_ph[5];           /* player_hand */
short           pk_hrf[5];          /* hand_rank_flags   -- which cards
                                       form computer's pair/trip/etc */
short           pk_hsf[5];          /* hand_suit_flags   -- sorted copy
                                       of computer hand (used as kicker
                                       scratch by pk_show) */
short           pk_phrf[5];         /* player_hand_rank_flags */
short           pk_phsf[5];         /* player_hand_suit_flags */
short           pk_chrk;     /* computer_hand_rank
                                       0=high,1=pair,2=two-pair,3=trips,
                                       4=straight,5=flush,6=full,7=four,
                                       8=straight-flush,9=royal */
short           pk_phrk;     /* player_hand_rank */
short           pk_dslot;     /* winner (0=comp, 1=player) */
short           pk_sel[5];          /* card_selected -- 1 = discard */
short           pk_disc;     /* discard_count */
short           pk_dpile[52];       /* discard_pile of already-seen cards */
short           pk_dpos;     /* deck_position -- reused as
                                       raise amount / draw counter */
short           pk_phv;     /* player_hand_value -- saved bet */
short           pk_bet;     /* current bet accumulator (shared) */
BOOL16          pk_bluff;    /* computer intends to bluff */
BOOL16          pk_pass;    /* computer passed on the bet loop */

/* Editable poker prompts.  pk_bm / pk_rm have single-space digit
   slots at fixed offsets; pk_tcm's card count digit + trailing
   period/'s.' get patched in by pk_cdrw.  Buffer widths sized so
   the biggest overwrite (a 2-digit prefix like "20") still fits. */
char *          pk_bm     = "I'll bet 00.  ";
char *          pk_rm     = "I'll raise 00.";
char *          pk_tcm    = "I'll take 0 cards.";

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
short           pk_pcc; /* player_card_count       */
short           pk_ccc; /* computer_card_count     */
short           pk_pscc; /* player_split_card_count */
short           pk_wpr; /* saved bet across split  */
BOOL16          pk_wrf;
BOOL16          pk_wcs;
BOOL16          pk_c1bj;
BOOL16          pk_c2bj;
BOOL16          pk_bs1;
BOOL16          pk_bs2;
short           pk_cscore;
short           pk_pscore;

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
short           wp_blk;

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
MFDB            crd_mfdb[54];
MFDB            mf_scb_c;

BOOL16  g_dvdog;
BOOL16  ph_hu;
BOOL16  g_ptdoa              = NO;


/* (gameTick animation tables + frame-state globals live
   in tick_tables.c -- Alcyon C168's symbol-table overflows if they
   are added here.) */
