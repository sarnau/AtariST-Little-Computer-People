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
#include "sprites.h"
#include "tables.h"
#include "tick_tables.h"
#include "vocab.h"
#include "calendar.h"
#include "events.h"
#include "sprload.h"
#include "psgfreq.h"

short           bj_key;         /* pk_bjMn's key variable (a global, not a local) */
char            psg_ovol;       /* psg_upEn's clamped output volume */
unsigned short  g_wkadj;        /* read once, in lcp_path's dead store */
unsigned short  ani_cnt;    /* unsigned: the & 7 test zero-extends */
short   t_sec;         /* game seconds 0..59; gameSim1 steps it every 8th frame */

/* The game clock and calendar, set from the guestbook by st_titl and
   advanced by gameSim1. */
short   t_min;          /* minute 0..59 */
short   t_hour;         /* hour 0..23 */
short   t_day;       /* day of the month, 0-based */
short   t_mon;         /* month 0..11 */
short   t_year;        /* year - 1900 */

PLAYER  lcp;            /* the resident's record, saved to and loaded from "hyber" */
BOOL16  introSeq;       /* YES while the move-in cutscene runs; holds off keys, phone and events */

BOOL16  in_evrt;        /* YES while execEv runs a deferred event, so lcp_wkD is not interrupted */

short   lastAct;        /* the action doAct last ran; the random picker avoids repeating it */

/* Left at 0 in BSS; the move-in cutscene sets them before gameLoop
   runs. */
short   lcp_x;        /* resident's screen position in pixels */
short   lcp_y;
BOOL16  g_lcldd;      /* lc_load's result: 1 = a saved resident was read, 0 = new game */
long    cprot_r;      /* long: tested as a 32-bit value */
short   g_spdc;       /* walk speed, 5 from gameLoop; read only in lcp_path's dead store */

BOOL16  alarm_p;        /* YES while the alarm clock rings (Ctrl-A, a_wakum); a_wakfa clears it */
short   lcp_watr;       /* water tank level, 0 (empty) .. WATER_MAX; Ctrl-W refills, drinking drains */

/* Typed-command queue: prsCmd appends the action chk_encm found and
   its priority, chk_actT consumes from the front. */
short   g_aliss;        /* number of queued commands, 0..10 */
short   g_aqueu[10];    /* queued action ids */
short   g_apriq[10];    /* their priorities: < 4 dropped, 4..7 aged by one per round, >= 8 obeyed */
short   g_hsfra;        /* head sprite frame; sp_lcha derives it, actions override it to nod or talk */
long    g_sfret;        /* game ticks until the current sound effect is stopped (sc_ren8); 0 = none */
BOOL16  g_actif;        /* YES while the resident is busy in an activity, so lcp_wkD walks uninterrupted */
/* Walk target in screen pixels for lcp_wkD / lcp_path; both 0 = arrived. */
short   g_wtx;
short   g_wty;
/* A 10-short scratch buffer used by action handlers (bathroom, food,
   house, leisure, idle, simple) to cache a small set of state values
   indexed by variable expressions like `i & 3`.  The bathroom/food/house
   paths write pst_arr[4], so it must hold more than four. */
short   pst_arr[10];

/* Open (YES) / closed (NO) state of the house's doors and cupboards,
   unpacked from lcp.door_states_and_flags by lc_load and packed back
   by lcp_std; main draws each one accordingly at boot. */
short   lcp_frdO;       /* DSF_FRONT_DOOR, opened and closed by a_opcfd */
short   studyDrO;       /* DSF_STUDY_DOOR */
short   lcp_clsO;       /* DSF_CLOSET_DOOR */
short   lcp_cabO;       /* DSF_KITCHEN_CABINET */
short   lcp_drsO;       /* DSF_DRESSER */
short   lcp_toiO;       /* DSF_TOILET_DOOR */
short   lcp_flcO;       /* DSF_FILING_CABINET */
short   lcp_bwlS;       /* dog bowl fill, BOWL_EMPTY..BOWL_FULL */
short   lcp_food;       /* working copy of lcp.food_supply */


/* A byte flag, not BOOL16: every use tests it as a byte. */
char    mi_play;
short   dg_bwlch;       /* bowl change for tick.c: -1 one step emptier, +1 fuller, 0 none */
short   g_sfplf;        /* YES while a sound effect is playing */
short   g_sfpli;        /* id of the effect playing, tested to stop or chain it */
char *  mi_sbuf;        /* Malloc'd copy of the loaded .SNG/.ORG file; NULL when none */
/* Song file counts (.SNG / .ORG), set at boot by cntSong(). */
short   sng_cnt;
short   org_cnt;
short   fire_dur;       /* frames until the lit fire burns out (a_lighf sets 2500..5000) */
BOOL16  fire_ext;       /* request for tick.c to put the fire out and redraw the grate */
short   tx_sctm;        /* text-strip timer: > 0 frames until the typed line expires,
                           < 0 a minigame owns the strip, 0 idle; picks sc_ren8's copy */
short   g_srsdc;        /* frames left of the text strip's scroll-up after Return */
short   g_cdibp;        /* cursor position in g_cdinb, 0..38 */

/* Letter subsystem storage.  g_ltlp[] is populated at runtime from
   LETTER.TXT (see fl_ltpl).  360 slots: that is fl_ltpl's literal
   `for (linecount = 0; linecount < 360; ...)`, it is the 4 sections x
   96 pointers (section 3 uses 72) shape a_writl indexes, and
   LETTER.TXT decodes to 361 line segments. */
char *  g_lttx;
char *  g_ltlp[360];

/* FORTY bytes, not 64: that is the room the original leaves for it. */
char    g_ltscb[40];
char    in_str[80];             /* a screen line */
/* comp_tok[15]: the 15 most common byte values in the
   compressed stream.  Populated at load-time by fr_reac
   from the 15-byte header immediately following the size word. */
/* scn_dic[15]: the 15-entry word dictionary at the head of a .SCN
   file, and the size/buffer main uses while decoding one.  All three
   are globals because the .SCN file handling is written out in main
   and only the nibble decoder is a function. */
short           scn_dic[15];
unsigned char   comp_tok[15];
short           scn_siz;
char *          scn_buf;

/* The resident's body and head sprite images, saved by hideLcp while
   it blanks them and put back by showLcp. */
short * sv_bodyP;
short * sv_headP;

/* VDI init happens in graphics setup; on the host we default to a
   sentinel handle that the VDI stubs ignore. */
short   vdihnd;     /* virtual workstation handle from v_opnvwk, passed to every VDI call */
short   vdi_hnd;    /* physical from graf_handle */
/* graf_handle writes its four cell/box metrics into these globals,
   not into locals of aes_init. */
short   gr_hwchar;
short   gr_hhchar;
short   gr_hwbox;
short   gr_hhbox;

/* The VDI parameter block: points at the game-local arrays used by
   vdiown.c's bindings and vdi_go. */
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

void *  g_dscp;     /* 512-aligned start of dsb_stor (fillTopR); the text strip is drawn there */
char    g_mspha;   /* a byte: the sequencer's SEQ_PHASE_* state, stepped by mq_advs */
unsigned char * mi_dbase;   /* the loaded song's event stream, just past its header */

/* ---- MIDI sequencer state ------------------------------------------- */
unsigned char * mi_sqpos;       /* read position in the event stream, walked by mq_pars */
long            mi_env;         /* address of the song's ADSR block, 8 bytes per channel */
char            psg_cvol;     /* a byte: PSG volume for the note being parsed */
/* mq_setp sets mi_evcn to 9, the loop stack's empty mark. */
short           mi_evi;         /* number of shorts in use in mi_evq */
short           mi_evcn;        /* loop-stack index into mi_lstk */

/* Ticks per beat, published to the Timer-A handler.  ONE short: every
   access goes to a single cell, and the next cell is two bytes later.
   It sits exactly 14 bytes past AESBIND's int_out, which makes
   "int_out[7]" tempting -- but int_out is only 14 bytes, so declaring
   it that way collides with the next global. */
short           mi_tpb;
long            g_mtcou;        /* master Timer-A tick counter, counted up by mq_tick */
short           g_mtdiv;        /* divider: mq_tick runs psg_upEn each time it wraps */
/* mq_stap resets mi_nlp0 at song start. */
short           mi_nlp0;

/* The duration mq_pars computes for the event it is about to queue.
   A SECOND cell: mq_pars writes it and only mq_qnne reads it, while
   mq_rdur's identical expression goes to mi_nlp0, which drives the
   tick counters. */
short           mi_ndur;
long            mi_nxTk;       /* long tick counters: g_mtcou value of the next event */
long            mi_lpTk;       /* g_mtcou when queued notes were last expired */
unsigned char   g_meve[4];     /* MIDI message being built for mq_dise */

/* The remaining sequencer/PSG working state below belongs to the
   Timer-A music engine. */

/* Timer-A interrupt state.
   mi_rlock (defined in mq_tick.s) -- reentrancy guard so the tick
                        handler doesn't recurse into the sequencer if a
                        game-code path triggers another timer event
                        before the first handler completes.
   mi_svtv  -- previous Timer-A vector, saved so it can be restored. */
long            mi_svtv;

/* ---- MIDI sequencer parse state -----------------------------------
   The sequencer walks a 3-byte-per-event compact stream inside
   mi_sqpos..mi_seqE.  Per-event scratch (event-type flag, note-on
   trigger, current note/channel, note-length params) is unpacked
   into a set of byte / short globals below, then handed to
   queue-note-event / send-note-off / send-program-change to reach
   the mq_dise dispatcher.

   mi_ndt (in the initialized data below) is the duration lookup
   indexed by bits 0..4 of each note event's first byte. */

unsigned char * mi_seqE;        /* end of the stream; -1 = no limit */
unsigned char * mi_dptr;        /* loop-back address popped by mq_popl */
char            mi_evTf;        /* set once a note was decoded in this pass */
char            mi_nnOn;        /* non-zero: queue the note (bit 4 of byte 0 clear) */
char            mi_nnOf;        /* non-zero: the event is a note-off (bit 5 of byte 0) */
char            mi_ccha;        /* logical channel, low nibble of byte 0 */
char            mi_cnot;        /* note to play: g_mstr-mapped, or literal (+-1) */
char            mi_nmof;        /* note-mode bits of byte 1: absolute, or one up/down */
char            mi_nlpA;        /* accent bit: full velocity and full PSG volume */

/* Event queue -- 3 shorts per active note: {duration, note|flags,
   physical MIDI channel byte}.  Max 60 slots -> 20 concurrent
   notes. */
short           mi_evq[60];

/* Loop stack -- {return_addr, remaining_count} pairs.  mq_setp starts
   the index at 9 (which also means "empty") and mq_pshl only pushes
   while it is below 49, so entries 9..48 hold at most 20 nested loops;
   0..8 and 49 are never touched. */
long            mi_lstk[50];

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
short           psg_rdel[3];      /* ramp_delta   */
short           psg_racc[3];      /* ramp_accum   */

/* mi_noSt: 128-entry table tracking
   which MIDI notes are currently sounding and on which logical channel.
   Value 0 = note not sounding.  Non-zero = the mi_chmap[] index (low
   nibble used) that owns the note, so mq_stop can emit a matching
   note-off through the correct MIDI channel on shutdown. */
unsigned char   mi_noSt[128];
unsigned char   psg_chNt[3];           /* current MIDI note per PSG channel A/B/C */
PSG_ENVELOPE    psg_envelope[3];


/* ---- SFX / Dosound state -------------------------------------------- */
char            g_sfcup;        /* sf_pri priority of the playing effect, for preemption */
short           g_sfddh;        /* effect duration from its last 4 bytes, high word (200 Hz) */
short           g_sfddl;        /* ... and low word */
long            g_sfHz2;        /* 200 Hz clock when the effect started; written, never read */
/* Per-SFX Dosound sequence pointers.  Each entry points to a 2-byte
   size header followed by a Dosound register-command stream ending in
   a 4-byte terminator.  Populated at startup from SOUNDS.LCP.
   25 slots: SOUNDS.LCP holds 23 blocks before the size-0 sentinel that
   ends sf_sl's `index < 500` loop, and the original leaves room for 25
   pointers.  The 500 is a loop limit, not the size. */
unsigned char * mi_ntLp[25];
/* Working buffer for the currently-playing Dosound sequence, copied
   from mi_ntLp[g_sfcur] each time a new effect starts.  FIFTY-SIX
   bytes: that is the room the original leaves before the next cell it
   uses.  sf_irqp copies `size` bytes here straight from SOUNDS.LCP, and
   the file has longer effects -- blocks 8 (SFX_HEAD_NOD) and 17
   (SFX_TOILET_REFILL) are 148 bytes -- so ON THIS MODEL the original
   overruns the buffer by up to 92 bytes.  Reproduced as written, not
   papered over with a bigger one.

   That 56 is an INFERENCE: a declared array size never reaches the
   compiled code.  g_sfDoB..g_srlgb is exactly 400 bytes, so the
   original may instead have had ONE 400-byte struct with g_sfdos/
   g_sfdoc as fields at +56/+58, and no overrun at all.  A 400-byte
   array plus two separate shorts is ruled out -- .comm packs densely,
   so g_sfdos would land at +400.  The two readings behave identically;
   see CLAUDE.md.

   Where the overrun LANDS depends on the BSS layout:

     * Shipped build.  g_sfDoB is followed by g_sfdos (+56) and g_sfdoc
       (+58) -- both WRITE-ONLY, set by sf_so() and read nowhere -- and
       then 342 bytes that no symbol claims.  The overrun dies in that
       hole, which is why 1985 shipped it.

     * Gated test build.  The linker's own .comm packing applies, and it
       puts g_obtah -- the 56-entry object HEIGHT table -- at exactly
       +56.  The first head-nod or toilet refill overwrites it; the next
       od_draw() passes g_obtah[i] - 1 to vro_cpyfm as a raster
       coordinate, and TOS bus errors inside the VDI.

   So the buffer is padded to 400 IN TEST BUILDS ONLY, reproducing the
   original's gap so a long run stays representative.  The shipped
   configuration keeps 56 -- widening it there would change the BSS
   size in the header and break byte identity. */
/* cp68 has no defined(), so the gates are collected one at a time. */
#ifdef SKIP_COPYPROT
#define LCP_SFDOB_PAD   1
#endif
#ifdef SKIP_TITLE
#define LCP_SFDOB_PAD   1
#endif
#ifdef SKIP_MIDI
#define LCP_SFDOB_PAD   1
#endif

#ifdef LCP_SFDOB_PAD
char            g_sfDoB[400];   /* Dosound buffer, padded (test builds) */
#else
char            g_sfDoB[56];    /* Dosound buffer handed to Dosound by sf_irqp */
#endif

void *  g_srlgb;    /* logical screen saved by sc_sdtb, restored by sc_sdtf */
void *  sv_lgb;     /* logical screen saved by initVdi, restored by exitVdi */
void *  g_srptr;    /* 512-aligned start of scrbufB: the house picture VDI draws into */
/* dsb_stor: offscreen buffer where the letter-typing status strip
   composites, kept separate from the main house buffer.
   fillTopR(27) writes rows 0..26 here so that the striped-white letter
   background is ready for the typewriter animation; sc_ren8
   blkcp32's the content into the compositor screen when the letter
   overlay is active.
   Sized from what fillTopR can actually write: its largest caller is
   mg_stp's fillTopR(0x4d), 77 rows of 160 bytes = 12320, and the
   align-up `(base + 512) & ~511` moves the start by at most 512 --
   so 12832 bytes, 6416 shorts.
   fillTopR points g_dscp at the ALIGNED start at run time. */
short   dsb_stor[6416];

/* scr_scal -- always 1 (REZ_ST_MEDIUM).
   Multiplier for the 320x200 low-res screen dimensions in sp_iniM,
   kept even though the value is a constant. */
short   scr_scal;

/* vdi_init opens the workstation through these GLOBAL work arrays,
   not through locals. */
short   work_in[11];
short   wk_out[57];

/* MFDB_A -- source MFDB for VDI raster copies.  fd_addr = NULL is the
   VDI convention for "device screen", so vro_cpyfm(...) copies from
   the visible physbase into a memory buffer instead of another
   off-screen bitmap.

   A SHORT ARRAY, not an MFDB: stpScrB clears MFDB_A[0] and MFDB_A[1]
   -- the two halves of fd_addr -- and cpyScr passes the array itself,
   both at the same address. */
short   MFDB_A[10];

/* scrbufA / scrbufB -- BSS scratch for the two double-buffer
   compositing screens.

   Every screen-pointer site (stpScrB, fillTopR, sp_iniM, sc_ren8's
   alternate) aligns the base up to 512 bytes:
        aligned = (base + 0x200) & ~0x1FF
   (the sprite path writes it as base + 0x1FF, masked the same way).

   Each holds ONE aligned screen: scrbufA the sprite compositor (also
   sc_ren8's alternate page-flip target -- there is no second screen
   at +0x8000, see parts/sc_ren8.c), scrbufB the decompressed
   house.scn background.

   Sized as screen + alignment slack, not as a round power of two.
   The ST hardware only needs a 256-byte-aligned base, which would
   make 32000 + 255 enough -- but this program masks to 512, so the
   base can move up by as much as 512 and the buffer needs
   32000 + 512 = 32512. */
unsigned char   scrbufA[32512];
unsigned char   scrbufB[32512];

/* Sound-effect request from sf_sele, started by sc_ren8 via sf_irqp. */
BOOL16  g_sfacf;        /* YES: a request is pending */
short   g_sfcur;        /* requested effect id (index into mi_ntLp / sf_pri) */
short   g_sfdur;        /* requested duration in game ticks; -1 = the effect's own */
short   g_sfdos;        /* set by sf_so, read nowhere */
short   g_sfdoc;        /* set by sf_so, read nowhere */

/* Raw file buffers, filled at startup by ldObj / ldSpr.  OBJECTS
   and SPRITES are each read into 14000 bytes. */
unsigned char   obj_file[14000];
unsigned char   spr_file[14000];

/* Per-record MFDB tables + dimensions.  56 entries: main's OBJECTS
   walk is a fixed `for (i = 0; i < 56; i++)`. */
MFDB    g_obtmt[56];

short   g_obtaw[56];        /* each object's width in pixels, for od_draw */
short   g_obtah[56];        /* each object's height in pixels */

BOOL16  g_inpmd;        /* YES while lcp_lgt has the resident away from a minigame:
                           only the Ctrl hot keys work, typing is ignored */
char    g_cdinb[64];    /* the command line being typed, NUL-terminated, up to 38 chars */
BOOL16  food_dlv;       /* set when Ctrl-F is refused because the cabinet is full; deal_kc only */
short   g_ptanf;        /* frame of the Ctrl-P patting-hand animation, stepped by tick.c */

union LASTHZ    lasthz;    /* last_hz / mi_lasT -- see globals.h */
long    last_vbc;       /* VBL count at sc_ren8's last frame, to pace it */
/* sv_phb: TOS's original Physbase, captured once at boot by
   aes_init via Physbase().  Deliberately uninitialised: an explicit
   initialiser would move it from BSS into .data. */
void *  sv_phb;

/* g_srmfd / mf_scrp: the compositing target and the current
   physical screen descriptor.  Populated by the graphics init routine. */
MFDB    g_srmfd;
MFDB    mf_scrp;
MFDB *  cur_mf;     /* sc_ren8's page-flip MFDB */

/* Dog wander and eating state, run by sc_ren8. */
BOOL16  dg_vis;         /* YES during a minigame: wander targets limited to entries 3..8 */
short   dg_idlcd;       /* frames to idle before picking the next target, 20..200 */
BOOL16  dg_nrbwl;       /* set when the stair-landing target is picked; lets the dog eat at its bowl */
BOOL16  g_deact;        /* YES while the dog is eating */
short   g_decou;        /* frames of eating left; the bowl drops a step at 60, 30, 4 and 0 */
short   dg_ltgtI;       /* last wander target picked, never picked twice in a row */

char *  cmd_inp;        /* the line prsCmd hands to chk_encm (always g_cdinb) */
short   g_aprio;        /* priority chk_encm gives the typed command: mood, chance, unknown words */

/* Per-slot MFDB arrays for the masked-blit sprite pipeline. */
MFDB    g_semfi[SPRITE_HW_SLOTS];
MFDB    g_semfm[SPRITE_HW_SLOTS];

/* ---- NLP parser state ------------------------------------------------
   g_ewb accumulates the bit masks of the recognised words; chk_encm
   then matches it against the rule table. */

char            g_ewb[10];
/* 42 bytes: cmd_upp() walks input from g_cdinb (bounded < 38 chars)
   and writes one byte per alphabetic char to usr_buf. */
char            usr_buf[42];

/* ---- Mini-game storage ----------------------------------------------- */
char *          g_agwb;         /* Malloc'd "words" dictionary for Anagrams, 11-byte rows */
char *          g_wpdb;         /* Malloc'd Word Puzzles text */
short *         crd_dat;        /* Malloc'd card images for War, Poker and Blackjack */

short           g_wpci;         /* current Word Puzzle, 0..32, wrapping on F1/F2 */
short           g_agclc;        /* Anagrams clues taken for this word; written, never read */
short           g_aggun;        /* Anagrams guess number, 1..9 */
short           ag_clue;        /* 1 once F1 has given a clue for this guess */
short           g_agwol;        /* length of the Anagrams word */
char            g_aginb[12];    /* the Anagrams guess being typed, 10 chars + NUL */
/* TEN bytes, the room the original leaves for it: the scrambled word
   buffer holds a 9-letter word and its NUL. */
char            g_agscw[10];

/* Mini-game shared state.
   mg_tofl: set YES by mg_wkev when the 7200-frame (~15 min) idle
            timeout fires; games check it to distinguish "user pressed
            F10" from "we auto-quit due to inactivity".
   sv_vqta: 10-short buffer holding the pre-mini-game VDI text
            attributes so rst_vsth can restore them after temporarily
            switching to 20-pixel height for the title/answer render. */
BOOL16          mg_tofl;
short           sv_vqta[10];

short           pk_round;       /* 0 at Poker start, 1 after pk_show; read nowhere */
BOOL16          pk_quit;        /* YES ends the card game (F10, timeout, or a side out of chips) */
short           g_pcbet;        /* Blackjack: chips bet on the player's first hand */
short           g_ppbet;        /* Blackjack: chips bet on the split hand */
short           g_pcmon;        /* the resident's chips (Poker/Blackjack, 400) or cards (War, 26) */
short           g_ppmon;        /* the player's chips or cards, likewise */
short           g_ppppa;        /* chips in the pot */
/* anagram_original_word: pointer into g_agwb dictionary (11-byte rows)
   set by ag_ssw when a word is picked. */
char *          g_agorw;
short           pk_phase;       /* Blackjack: 1 once the player has split */
short           pk_dsc[52];     /* War: the shuffled deck dealt into the two draw piles */
/* Computer's and player's draw piles, 52 shorts each: pk_rmch's
   unconditional
     for (i = 0; i < 51; i = i + 1) pile[i] = pile[i + 1];
   shifts the whole pile. */
short           g_pcdrp[52];
short           g_ppdrp[52];

/* War/Blackjack per-round face-down "war" cards.  Sized 52 so the
   deepest possible recursion (all cards ending up here) still fits.
   CARD_NONE sentinel terminates.  g_pchc counts the number of prior
   war rounds this hand (indexes further into the arrays). */
short           pk_pwc[52];             /* player's war cards */
short           pk_cwc[52];             /* computer's war cards */
short           g_pchc;

/* Poker (5-card draw) working state.  Every field is per-hand: reset
   at the start of each round in pk_ante / pk_evhs / pk_show.
   pk_ch / pk_ph also serve as the War hands. */
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
short           pk_dpile[13];       /* discard_pile of already-seen cards:
                                       at most 5 + 5 discards plus the
                                       explicit pk_dpile[10] write */
short           pk_dpos;     /* deck_position -- reused as
                                       raise amount / draw counter */
short           pk_phv;     /* player_hand_value -- saved bet */
short           pk_bet;     /* current bet accumulator (shared) */
BOOL16          pk_bluff;    /* computer intends to bluff */
BOOL16          pk_pass;    /* computer passed on the bet loop */

/* Blackjack per-hand state.
   pk_psh[]  -- 3rd hand slot used when the player elects to split
                two matching down-cards (post-deal, both aces or
                two of the same rank).  CARD_NONE-terminated.
   pk_pcc / pk_ccc / pk_pscc -- remaining-hits counters (start at
                CARD_BJ_MAX = 3 for standard "up to 5 cards" rule;
                each hit decrements by CARD_BJ_STEP = 1; stop at
                CARD_BJ_STOP = 0).
   pk_wpr    -- saved bet during split-hand bookkeeping.
   pk_wrf / pk_wcs  -- split-hand round-active flags.
   pk_c1bj / pk_c2bj -- first / second hand natural-blackjack
                achieved this round (used to skip the hit loop).
   pk_bs1 / pk_bs2  -- first / second hand busted flag.
   pk_cscore / pk_pscore -- computer-picked / player-picked score
                once the double-value-with-ace picker resolves.
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
   the player during solve_phase:
      wp_prm    9 entries (0..4 random first-word, 5..8 for word
                slots 2..5)
      wp_succ   6 entries, random on solve
      wp_fail   6 entries, random on wrong answer
   Five blanks, not ten: the decoded WORDPZ.TXT has at most five '@'
   markers on a line. */
char            wp_ans[5][12];
short           wp_blk;

/* 54-entry MFDB table covering 52 card faces + 1 back + 1 highlight
   overlay.  All share crd_dat as their bitmap backing. */
MFDB            crd_mfdb[54];
MFDB            mf_scb_c;       /* the 320x77 card-table area the cards are copied into */

BOOL16  g_dvdog;        /* YES while er_dogf runs er_food for a dog-food delivery */
BOOL16  ph_hu;          /* request for gameTick to hang the phone up and stop its ring */


/* (gameTick animation tables + frame-state globals live
   in tick_tables.c -- Alcyon C168's symbol-table overflows if they
   are added here.) */

/* ==== initialized data ================================================
   EVERY initialized global in the program, in the original's DATA
   order.

   The 1985 source kept all of this in ONE object: its data segment
   interleaves tables whose code lives in sprglobs.c, tables.c,
   tick_tables.c, vocab.c, psgfreq.c, sprload.c, calendar.c, events.c
   and here -- and data from separate objects cannot interleave.

   The order is the original's, not taste (see CLAUDE.md, "DATA and
   BSS layout").  DO NOT reorder by hand.
   ==================================================================== */


/* PSG register offsets.  Amp registers 8/9/10 with
   the PSG "write" bit (0x80) pre-set.  psg_upEn subtracts 0x80
   before calling psg_wr to recover the raw register number. */
unsigned char   psg_rot[3]  = { 0x88, 0x89, 0x8a };

/* Three pointers at psg_env[0..2], one per PSG channel.  NOTHING in
   the program references this table, but the original carries it in
   DATA immediately behind psg_rot, and Alcyon emits an unreferenced
   initialized global just the same.  Do not delete. */
PSG_ENVELOPE *  psg_epp[3] = { &psg_envelope[0], &psg_envelope[1],
                               &psg_envelope[2] };


/* Envelope rate table.  32-byte table indexed by phase_timer
   (already loaded from an ADSR duration byte). */
short           mi_evrt[16] = {
             0,  360,  180,  120,   85,   72,   60,   45,
            30,   20,   15,   12,   10,    8,    6,    4
};


/* Envelope time table.  Reload value for phase_timer
   when transitioning between ADSR phases. */
short           mi_evtt[16] = {
             0,    1,    2,    3,    4,    5,    6,    8,
            12,   18,   24,   30,   36,   45,   60,   90
};


/* Envelope sustain table.  Reload for phase_timer
   during the sustain->release transition. */
short           mi_evst[16] = {
             0,    1,    2,    4,    8,   18,   24,   40,
            45,   60,   72,   90,  120,  180,  360, 30000
};


/* Envelope release table.  Applied to ramp_delta
   during the sustain->release transition. */
short           mi_evrl[16] = {
             0,  360,  180,   90,   45,   20,   15,    9,
             8,    6,    5,    4,    3,    2,    1,    0
};


BOOL16          g_moen     = YES;   /* send notes to MIDI OUT */


BOOL16          psg_out              = YES;     /* play notes on the YM2149 PSG */

/* MIDI channel count; mq_parh's channel-count case writes p[2]
   here. */
short           g_mchcn                 = 1;

short           mi_temp              = 120;     /* song tempo in beats per minute, from the song header */

/* Ticks per beat at the default tempo mi_temp = 120. */
short           g_mtspb     = 20;


/* 22 entries, not 32: that is the room the original leaves.  mq_pars
   indexes it with a 5-bit field (`mi_ndt[*mi_sqpos & 0x1f]`), so 22..31
   would read past the end -- the .SNG data never produces them. */
short           mi_ndt[22] = {
           0,    2,    2,    3,    4,    5,    6,    8,
           9,   12,   16,   18,   24,   32,   36,   48,
          64,   72,   96,  128,  144,    0
};


/* 128 entries, the values the original ships: YM2149 tone periods,
   period = 2000000 / (16 * f) for MIDI note n.  Entries below index 23
   are 0 -- flagged too-low by mq_dise. */
short           psg_freq[128] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0fc0,
    0x0ecb, 0x0df3, 0x0d32, 0x0c85, 0x0be8, 0x0b18, 0x0a9d, 0x09f7,
    0x0963, 0x08e0, 0x086b, 0x07e0, 0x0783, 0x0713, 0x06b0, 0x0642,
    0x05f4, 0x059c, 0x054e, 0x04fb, 0x04b1, 0x0470, 0x042c, 0x03f8,
    0x03ba, 0x0383, 0x0352, 0x0321, 0x02f5, 0x02ca, 0x02a3, 0x027d,
    0x0258, 0x0238, 0x0218, 0x01fa, 0x01dd, 0x01c3, 0x01a9, 0x0191,
    0x017a, 0x0166, 0x0151, 0x013e, 0x012d, 0x011c, 0x010c, 0x00fd,
    0x00ef, 0x00e1, 0x00d4, 0x00c8, 0x00bd, 0x00b3, 0x00a8, 0x009f,
    0x0096, 0x008e, 0x0086, 0x007e, 0x0077, 0x0070, 0x006a, 0x0064,
    0x005e, 0x0059, 0x0054, 0x004f, 0x004b, 0x0047, 0x0043, 0x003f,
    0x003b, 0x0038, 0x0035, 0x0032, 0x002f, 0x002c, 0x002a, 0x0027,
    0x0025, 0x0023, 0x0021, 0x001f, 0x001d, 0x001c, 0x001a, 0x0019,
    0x0017, 0x0016, 0x0015, 0x0013, 0x0012, 0x0011, 0x0010, 0x000f,
    0x000e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

/* MIDI/PSG defaults.  These are BYTES, not shorts: the code accesses
   them with byte moves and compares.
     mi_vel   = 127 -- max MIDI velocity
     mi_dvel  = 127
     psg_dvol = 15  -- max PSG volume */
char            mi_vel           = 127; /* a byte: velocity of the note being parsed */

/* These two are char (byte compares/stores; Alcyon word-aligns them,
   hence the 2-byte spacing). */
char            mi_dvel   = 127;   /* velocity of an unaccented note, from the song header */

char            psg_dvol      = 15;    /* PSG volume of an unaccented note, from the header velocity */

/* The note range the sequencer will play, HIGH first in the data:
   0x60 is the top of the range and 0x24 the bottom, and mq_dise
   rejects a note above the first and below the second. */
char            g_mnhi      = 0x60;    /* highest note played */

char           g_mnlo       = 0x24;    /* lowest note played */


/* 132-entry (0x84) note transpose lookup.  Indexed by MIDI note number
   0..131 (C-1..G9).  Populated by mq_bust at song
   start; each note maps to either itself (identity) or a shifted note
   under a chord mask, or 0xFF to skip (chromatic non-diatonic tones). */


/* ---- PSG channel state ---------------------------------------------- */

/* Program last sent on each MIDI channel; -1 = none, so the first
   mq_sepc always sends one. */
char            g_mcpro[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

/* Logical song channel -> physical MIDI channel (low nibble); mq_pacm
   fills it from the song header. */
unsigned char   mi_chmap[16] = { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The static content is 0..99 then 110..127, with the last 14
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

/* g_msmsa is a BYTE and lives in the text segment behind mq_tick --
   see source/mq_tick.s. */
/* g_msmk: 16-byte chord-mask lookup. */
unsigned char   g_msmk[16] = {
        0xFF, 0xFF, 0x77, 0x37, 0x33, 0x13, 0x11, 0x01,
        0x00, 0xFE, 0xEE, 0xEC, 0xCC, 0xC8, 0x88, 0x00
};


/* The default program map, sixteen bytes of INITIALIZED data right
   behind g_msmk; mi_pgmap points here.  Named mi_pgtab rather than
   mi_pgmapb because Alcyon truncates a linkage name to eight
   characters and _mi_pgmapb would collide with _mi_pgmap. */
unsigned char   mi_pgtab[16] = {
        0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x11
};


/* mi_slop and mi_varR are char: sgPlay writes them as bytes.
   mi_slop is written by sgPlay and read by mq_pars. */
char            mi_slop         = 1;

char    mi_varR                      = YES;    /* channel every note uses when mi_slop is NO */

char *          mi_pgmap = (char *) mi_pgtab;   /* a byte pointer: MIDI program per logical channel */

/* ---- the MIDI object ------------------------------------------------
   midi_seq.c is compiled AS PART OF THIS FILE, right here.  Alcyon
   emits a switch jump table into the .data of the object that holds
   the function, and the original has mq_parh's and mq_dise's tables
   sitting between mi_pgmap and main_pal.  Data from separate objects
   cannot interleave, so the globals and the MIDI code are ONE object,
   and the split point is exactly here.

   tools/stx_units.txt therefore names midi_seq.c as this file's
   constituent, and alcyon_link.sh links globals.o as the first game
   object.
   ---------------------------------------------------------------- */
#include "midi_seq.c"






/* No globals here for the 200 Hz clock or the VBL counter.  Both are
   ATARI ST SYSTEM VARIABLES in low memory -- _hz_200 at $04BA/$04BC
   and _vbclock at $0462 -- and sf_irqp and sc_ren8 read them there
   directly, under Super. */
