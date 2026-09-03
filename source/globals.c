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
BOOL16  introSeq;

BOOL16  in_evrt;

short   lastAct;

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
short   g_hsfra;
long    g_sfret;
BOOL16  g_actif;
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


/* STX declares this a byte flag (tst.b at its use sites). */
char    mi_play;
short   dg_bwlch;
short   g_sfplf;
short   g_sfpli;
char *  mi_sbuf;
/* Ghidra sng/org song file counts, set at boot by cntSong().
   BSS-zero to match Ghidra; port previously had org_cnt=8
   as a guess. */
short   sng_cnt;
short   org_cnt;
/* scn_cmn -- 30-byte scene common-data header shared between
   house.scn and title.scn.  Ghidra `scene_common_data` @ 0x4cf7c.
   The port's unScn helper (assets.c) fills this via fr_read. */
char    scn_cmn[30];
short   fire_dur;
BOOL16  fire_ext;
short   tx_sctm;
short   g_srsdc;
short   g_cdibp;

/* Letter subsystem storage.  g_ltlp[] and _greeting_table are
   populated at runtime from letter.txt (see fl_ltpl);
   NULL entries make lt_tysa a safe no-op on the
   host build until the template loader is ported.  360 slots: that is
   fl_ltpl's literal `for (linecount = 0; linecount < 360; ...)`, it is
   the 4 sections x 96 pointers (section 3 uses 72) shape a_writl
   indexes, LETTER.TXT decodes to 361 line segments, and LCP_STX's own
   gap here is 1440 bytes = 360 pointers. */
char *  g_lttx;
char *  g_ltlp[360];

/* Second ROM frame-id block (data 0x1200a-0x12026): closet door,
   fire-off, filing cabinet, dresser, and the sc_drfc food marker. */
char    g_ltscb[64];
char    in_str[80];             /* LCP_STX gap; a screen line */
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
/* mi_pgmapb must come BEFORE mi_pgmap: both truncate to the same
   8-char linkage name (_mi_pgma), which is the point -- the pointer
   is stored over the array's first 4 bytes -- but as68 only accepts
   the .comm ahead of the .data definition. */
char            mi_pgmapb[16];
char    g_mspha;   /* STX: byte */
unsigned char * mi_dbase;

/* ---- MIDI sequencer state ------------------------------------------- */
unsigned char * mi_sqpos;
long            g_msmap;
long            mi_env;
char            psg_cvol;     /* STX: byte */
/* mi_evi / mi_evcn live HERE in the ROM's data (0x120fa/0x120fc),
   with mi_evcn initialized to 9. */
short           mi_evi;
short           mi_evcn;
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
unsigned char   g_meve[4];

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

/* Event queue -- 3 shorts per active note: {duration, note|flags,
   physical MIDI channel byte}.  Max 60 slots -> 20 concurrent
   notes. */
short           mi_evq[60];

/* Loop stack -- {return_addr, remaining_count} pairs.  Max 24
   nested loops (48 entries + 2 slack). */
long            mi_lstk[50];

/* mi_nOS was a second name for mi_noSt below -- one is written by the
   note-on/off handler, the other read by mq_stop, and both map to the
   SAME LCP_STX address (bss 0x27738).  Merged into mi_noSt. */

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

/* mi_noSt (Ghidra midi_noteon_state @ 0x53df8): 128-entry table tracking
   which MIDI notes are currently sounding and on which logical channel.
   Value 0 = note not sounding.  Non-zero = the mi_chmap[] index (low
   nibble used) that owns the note, so mq_stop can emit a matching
   note-off through the correct MIDI channel on shutdown. */
unsigned char   mi_noSt[128];
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
/* 25 slots: SOUNDS.LCP holds 23 blocks before the size-0 sentinel
   that ends sf_sl's `index < 500` loop, and LCP_STX's gap here is 100
   bytes = 25 pointers.  The 500 is a loop limit, not the size. */
unsigned char * mi_ntLp[25];
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

   Sized as screen + alignment slack, not as a round power of two.
   The ST hardware only needs a 256-byte-aligned base, which would
   make 32000 + 255 enough -- but all three align-up sites in this
   program mask to 512, verified in the binary:

       sprites.c  0x15110  push scrbufA+0x1ff ; andi.l #-512,(sp)
       stpScrB    0x65ae   addl #512,d0 ; andl #-512,d0
       fillTopR   0x6880   addl #512,d0 ; andl #-512,d0

   so the base can move up by 511 (the +0x1ff form) or 512 (the +512
   forms) and the buffer needs 32000 + 512 = 32512.

   Both fit LCP_STX's room for them -- scrbufA has 33045 bytes before
   mi_dptr, scrbufB has 32600 before pk_quit -- with 533 and 88 bytes
   over for globals the port does not have. */
unsigned char   scrbufA[32512];
unsigned char   scrbufB[32512];

BOOL16  g_sfacf;
short   g_sfcur;
short   g_sfdur;
short   g_sfdos;
short   g_sfdoc;

/* Raw file buffers -- populated at startup by asset_load_all().
   OBJECTS and SPRITES both size at 14000 bytes per Ghidra
   ldObj / ldSpr decompiles. */
unsigned char   obj_file[14000];
unsigned char   spr_file[14000];

/* Per-record MFDB tables + dimensions.  56 entries: main's OBJECTS
   walk is a fixed `for (i = 0; i < 56; i++)`, and LCP_STX's gaps for
   all three of these agree (1120 / 112 / 112). */
MFDB    g_obtmt[56];

short   g_obtaw[56];
short   g_obtah[56];

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

BOOL16  dg_vis;
short   dg_idlcd;
BOOL16  dg_nrbwl;
BOOL16  g_deact;
short   g_decou;
short   dg_ltgtI;

char *  cmd_inp;
short   g_aprio;

/* Per-slot MFDB arrays for the masked-blit sprite pipeline. */
MFDB    g_semfi[SPRITE_HW_SLOTS];
MFDB    g_semfm[SPRITE_HW_SLOTS];

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

/* ---- Mini-game storage ----------------------------------------------- */
char *          g_agwb;
char *          g_wpdb;
short *         crd_dat;

short           g_wpci;
short           g_agclc;
short           g_aggun;
short           ag_clue;
short           g_agwol;
char            g_aginb[12];
char            g_agscw[12];

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
short           pk_dpile[13];       /* discard_pile of already-seen cards:
                                       at most 5 + 5 discards plus the
                                       explicit pk_dpile[10] write, and
                                       LCP_STX's gap here is 26 bytes */
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
/* Five blanks, not ten: the decoded WORDPZ.TXT has at most five '@'
   markers on a line, and LCP_STX's gap here is 60 bytes. */
char            wp_ans[5][12];
short           wp_blk;

/* 54-entry MFDB table covering 52 card faces + 1 back + 1 highlight
   overlay.  All share crd_dat as their bitmap backing. */
MFDB            crd_mfdb[54];
MFDB            mf_scb_c;

BOOL16  g_dvdog;
BOOL16  ph_hu;


/* (gameTick animation tables + frame-state globals live
   in tick_tables.c -- Alcyon C168's symbol-table overflows if they
   are added here.) */

/* ==== initialized data ================================================
   EVERY initialized global in the program, in LCP_STX DATA order.

   The 1985 source kept all of this in ONE object: its data segment
   interleaves what the port had split across globals.c, sprglobs.c,
   tables.c, tick_tables.c, vocab.c, psgfreq.c, sprload.c, calendar.c
   and events.c -- and data from separate objects cannot interleave.

   The order comes from relocation pairing (see CLAUDE.md, "DATA and BSS
   layout"), not from taste.  DO NOT reorder by hand; re-derive it.
   ==================================================================== */


/* psg_register_offset_table @0x2985c.  Amp registers 8/9/10 with
   the PSG "write" bit (0x80) pre-set.  psg_upEn subtracts 0x80
   before calling psg_wr to recover the raw register number. */
unsigned char   psg_rot[3]  = { 0x88, 0x89, 0x8a };

/* Three pointers at psg_env[0..2], one per PSG channel.  NOTHING in
   the program references this table -- no text relocation targets it,
   in either binary -- but LCP_STX carries it in DATA immediately
   behind psg_rot (its three pointers are the data relocations at
   0x184/0x188/0x18c, 14 bytes apart = sizeof(PSG_ENVELOPE)), and
   Alcyon emits an unreferenced initialized global just the same. */
PSG_ENVELOPE *  psg_epp[3] = { &psg_envelope[0], &psg_envelope[1],
                               &psg_envelope[2] };


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


/* midi_envelope_sustain_table @0x298cc.  Reload for phase_timer
   during the sustain->release transition. */
short           mi_evst[16] = {
             0,    1,    2,    4,    8,   18,   24,   40,
            45,   60,   72,   90,  120,  180,  360, 30000
};


/* midi_envelope_release_table @0x298ac.  Applied to ramp_delta
   during the sustain->release transition. */
short           mi_evrl[16] = {
             0,  360,  180,   90,   45,   20,   15,    9,
             8,    6,    5,    4,    3,    2,    1,    0
};


BOOL16          g_moen     = YES;


BOOL16          psg_out              = YES;

/* Ghidra midi_channel_count @ 0x298F0 = 1 (byte).  Ports mh_chac
   writes p[2] here and passes through mq_bust. */
short           g_mchcn                 = 1;

short           mi_temp              = 120;

/* Ghidra midi_ticks_per_beat @ 0x298F4 = 20; mi_temp @ 0x298F2 = 120. */
short           g_mtspb     = 20;


/* 22 entries, not 32: LCP_STX's gap here is 44 bytes.  mq_pars indexes
   it with a 5-bit field (`mi_ndt[*mi_sqpos & 0x1f]`), so 22..31 would
   read past the end -- the .SNG data never produces them. */
short           mi_ndt[22] = {
           0,    2,    2,    3,    4,    5,    6,    8,
           9,   12,   16,   18,   24,   32,   36,   48,
          64,   72,   96,  128,  144,    0
};


/* 128 entries, the values LCP_STX ships (its data gap here is 256
   bytes).  Entries below index 23 are 0 -- flagged too-low by
   mq_dise. */
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

char            psg_dvol      = 15;

char            g_mnlol      = 0x60;

char           g_mnhil       = 0x24;


/* 132-entry (0x84) note transpose lookup.  Indexed by MIDI note number
   0..131 (C-1..G9).  Populated by mq_bust at song
   start; each note maps to either itself (identity) or a shifted note
   under a chord mask, or 0xFF to skip (chromatic non-diatonic tones). */


/* ---- PSG channel state ---------------------------------------------- */

/* Referenced by the ROM text (bss 0x3df96 / 0x4549a / 0x1dc7c /
   0x48bce). */
char            g_mcpro[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

unsigned char   mi_chmap[16] = { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

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

/* g_msmsa is a BYTE and lives in the text segment behind mq_tick --
   see source/mq_tick.s. */
/* g_msmk (Ghidra midi_scale_mask_table @ 0x29ad0): 16-byte chord-mask
   lookup.  Dumped verbatim -- previous port had guessed the values
   from Music Studio 2.0 documentation but the real ones diverge
   significantly (e.g. slot 3 is 0x37 not 0x6F, slot 4 is 0x33 not 0x77). */
unsigned char   g_msmk[32] = {
        0xFF, 0xFF, 0x77, 0x37, 0x33, 0x13, 0x11, 0x01,
        0x00, 0xFE, 0xEE, 0xEC, 0xCC, 0xC8, 0x88, 0x00,
        0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x11
};


/* Ghidra mi_varR @ 0x29af2 = 1 (byte).  Port previously had NO.
   LCP_STX declares both as char -- sgPlay writes them with moveb. */


/* One byte, one variable: sgPlay writes it (the port called that
   write g_molof) and mq_pars reads it.  Both port names mapped to
   the same LCP_STX address, data 0x414. */
char            mi_slop         = 1;

char    mi_varR                      = YES;

char *          mi_pgmap = mi_pgmapb;   /* STX: byte pointer */

/* ---- the MIDI object ------------------------------------------------
   midi_seq.c is compiled AS PART OF THIS FILE, right here.  Alcyon
   emits a switch jump table into the .data of the object that holds
   the function, and LCP_STX has mq_parh's table (7 case longs + 7
   targets) and mq_dise's (5 targets) -- 76 bytes in all -- sitting
   between mi_pgmap at data 0x418 and main_pal at 0x468.  Data from
   separate objects cannot interleave, so the globals and the MIDI
   code are ONE object, and the split point is exactly here.

   tools/stx_units.txt therefore names midi_seq.c as this file's
   constituent, and alcyon_link.sh puts globals.o where midi_seq.o
   used to be (text 0x12a).
   ---------------------------------------------------------------- */
#include "midi_seq.c"






/* No globals here for the 200 Hz clock or the VBL counter.  Both are
   ATARI ST SYSTEM VARIABLES in low memory -- _hz_200 at $04BA/$04BC
   and _vbclock at $0462 -- and the code reads them there directly,
   under Super, in sf_irqp and sc_ren8.  The port used to define
   g_hzhi/g_hzlo/_vbclock as its own storage as well; nothing ever
   relocated against them, so they were eight dead bytes in the data
   segment. */
