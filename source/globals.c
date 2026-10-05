/*
 * globals.c -- storage for game global variables.
 *
 * Definitions of every extern declared in globals.h.  Alcyon C places
 * zero-initialised globals in BSS automatically; explicit initialisers
 * here are only for values that matter at boot time before loadSavedGame()
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

short           bjKey;         /* playBlackjack's key variable (a global, not a local) */
char            envOutVol;       /* stepEnvelopes's clamped output volume */
unsigned short  walkAdjust;        /* read once, in walkStep's dead store */
unsigned short  frameCount;    /* unsigned: the & 7 test zero-extends */
short   t_sec;         /* game seconds 0..59; simStep steps it every 8th frame */

/* The game clock and calendar, set from the guestbook by titleScreen and
   advanced by simStep. */
short   t_min;          /* minute 0..59 */
short   t_hour;         /* hour 0..23 */
short   t_day;       /* day of the month, 0-based */
short   t_mon;         /* month 0..11 */
short   t_year;        /* year - 1900 */

PLAYER  resident;            /* the resident's record, saved to and loaded from "hyber" */
/* YES while the move-in cutscene runs; holds off keys, phone and events. */
BOOL16  movingIn;

/* YES while runEvent runs a deferred event, so walkToTarget is not
   interrupted. */
BOOL16  inEvent;

/* The action runAction last ran; the random picker avoids repeating it. */
short   lastAction;

/* Left at 0 in BSS; the move-in cutscene sets them before gameLoop
   runs. */
short   resX;        /* resident's screen position in pixels */
short   resY;
/* loadSavedGame's result: 1 = a saved resident was read, 0 = new game. */
BOOL16  loadedSave;
long    copyProtResult;      /* long: tested as a 32-bit value */
short   walkSpeed;   /* walk speed, 5 from gameLoop; read only in walkStep's dead store */

/* YES while the alarm clock rings (Ctrl-A, morningRoutine); wakeFromAlarm
   clears it. */
BOOL16  alarmRinging;
BOOL16  alarmSounding;       /* the alarm's ring sound has started (gameTick) */
short   ringCountdown;       /* gameTick's phone-ring countdown */
/* Water tank level, 0 (empty) .. WATER_MAX; Ctrl-W refills, drinking
   drains. */
short   waterLevel;

/* Typed-command queue: submitCommand appends the action matchCommand found and
   its priority, chooseAction consumes from the front. */
short   queueCount;        /* number of queued commands, 0..10 */
short   queueActions[10];    /* queued action ids */
/* Their priorities: < 4 dropped, 4..7 aged by one per round, >= 8 obeyed. */
short   queuePriority[10];
/* Head sprite frame; stepHead derives it, actions override it to nod or
   talk. */
short   headFrame;
/* Game ticks until the current sound effect is stopped (renderFrame); 0 =
   none. */
long    sfxTicksLeft;
/* YES while the resident is busy in an activity, so walkToTarget walks
   uninterrupted. */
BOOL16  noPreempt;
/* Walk target in screen pixels for walkToTarget / walkStep; both 0 = arrived. */
short   walkXTarget;
short   walkYTarget;
/* A 10-short scratch buffer used by action handlers (bathroom, food,
   house, leisure, idle, simple) to cache a small set of state values
   indexed by variable expressions like `i & 3`.  The bathroom/food/house
   paths write scratchArr[4], so it must hold more than four. */
short   scratchArr[10];

/* Open (YES) / closed (NO) state of the house's doors and cupboards,
   unpacked from resident.doorStatesAndFlags by loadSavedGame and packed back
   by studyVisit; main draws each one accordingly at boot. */
short   frontDoorOpen;       /* DSF_FRONT_DOOR, opened and closed by openFrontDoor */
short   studyDoorOpen;       /* DSF_STUDY_DOOR */
short   bedClosetOpen;       /* DSF_CLOSET_DOOR */
short   kitchenCabOpen;       /* DSF_KITCHEN_CABINET */
short   dresserOpen;       /* DSF_DRESSER */
short   toiletDoorOpen;       /* DSF_TOILET_DOOR */
short   filingCabOpen;       /* DSF_FILING_CABINET */
short   bowlLevel;       /* dog bowl fill, BOWL_EMPTY..BOWL_FULL */
short   foodSupply;       /* working copy of resident.foodSupply */

/* A byte flag, not BOOL16: every use tests it as a byte. */
char    songPlaying;
short   bowlChange;   /* bowl change for tick.c: -1 one step emptier, +1 fuller, 0 none */
short   sfxPlaying;        /* YES while a sound effect is playing */
short   sfxCurId;        /* id of the effect playing, tested to stop or chain it */
char *  songBuf;        /* Malloc'd copy of the loaded .SNG/.ORG file; NULL when none */
/* Song file counts (.SNG / .ORG), set at boot by countSongs(). */
short   songCount;
short   organCount;
/* Frames until the lit fire burns out (lightFire sets 2500..5000). */
short   fireTimeLeft;
BOOL16  fireDouse;       /* request for tick.c to put the fire out and redraw the grate */
/* Text-strip timer: > 0 frames until the typed line expires, < 0 a
   minigame owns the strip, 0 idle; picks renderFrame's copy. */
short   textTimer;
short   stripScroll;        /* frames left of the text strip's scroll-up after Return */
short   typedCursor;        /* cursor position in typedLine, 0..38 */

/* Letter subsystem storage.  letterLines[] is populated at runtime from
   LETTER.TXT (see loadLetterText).  360 slots: that is loadLetterText's literal
   `for (linecount = 0; linecount < 360; ...)`, it is the 4 sections x
   96 pointers (section 3 uses 72) shape writeLetter indexes, and
   LETTER.TXT decodes to 361 line segments. */
char *  letterText;
char *  letterLines[LETTER_LINES];

/* FORTY bytes, not 64: that is the room the original leaves for it. */
char    letterWord[40];
char    inputLine[80];             /* a screen line */
/* nibbleBytes[15]: the 15 most common byte values in the
   compressed stream.  Populated at load-time by unpackFile
   from the 15-byte header immediately following the size word. */
/* scnDict[15]: the 15-entry word dictionary at the head of a .SCN
   file, and the size/buffer main uses while decoding one.  All three
   are globals because the .SCN file handling is written out in main
   and only the nibble decoder is a function. */
short           scnDict[15];
unsigned char   nibbleBytes[15];
short           scnSize;
char *          scnBuffer;

/* The resident's body and head sprite images, saved by hideResident while
   it blanks them and put back by showResident. */
short * savedBodyImg;
short * savedHeadImg;

/* VDI init happens in graphics setup; on the host we default to a
   sentinel handle that the VDI stubs ignore. */
/* Virtual workstation handle from v_opnvwk, passed to every VDI call. */
short   vdiHandle;
short   physHandle;    /* physical from graf_handle */
/* graf_handle writes its four cell/box metrics into these globals,
   not into locals of initAes. */
short   charWidth;
short   charHeight;
short   boxWidth;
short   boxHeight;

/* The VDI parameter block: points at the game-local arrays used by
   the bindings in vdistx.c and their trap dispatcher, gsx1. */
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

/* 512-aligned start of stripStore (fillPanel); the text strip is drawn
   there. */
void *  stripBuf;
char    seqPhase;   /* a byte: the sequencer's SEQ_PHASE_* state, stepped by seqAdvance */
unsigned char * songEvents;   /* the loaded song's event stream, just past its header */

/* ---- MIDI sequencer state ------------------------------------------- */
unsigned char * songPos;   /* read position in the event stream, walked by parseEvents */
long            songAdsr;   /* address of the song's ADSR block, 8 bytes per channel */
char            noteVolume;     /* a byte: PSG volume for the note being parsed */
/* initSongState sets loopTop to 9, the loop stack's empty mark. */
short           queueLen;         /* number of shorts in use in noteQueue */
short           loopTop;        /* loop-stack index into loopStack */

/* Ticks per beat, published to the Timer-A handler.  ONE short: every
   access goes to a single cell, and the next cell is two bytes later.
   It sits exactly 14 bytes past AESBIND's int_out, which makes
   "int_out[7]" tempting -- but int_out is only 14 bytes, so declaring
   it that way collides with the next global. */
short           beatTicks;
long            timerTicks;   /* master Timer-A tick counter, counted up by timerAIsr */
/* Divider: timerAIsr runs stepEnvelopes each time it wraps. */
short           envDivider;
/* armSequencer resets ticksToNext at song start. */
short           ticksToNext;

/* The duration parseEvents computes for the event it is about to queue.
   A SECOND cell: parseEvents writes it and only queueNote reads it, while
   peekNoteDur's identical expression goes to ticksToNext, which drives the
   tick counters. */
short           noteDur;
long            nextEvTick;   /* long tick counters: timerTicks value of the next event */
long            lastExpTick;       /* timerTicks when queued notes were last expired */
unsigned char   midiMsg[4];     /* MIDI message being built for sendMidiEvent */

/* The remaining sequencer/PSG working state below belongs to the
   Timer-A music engine. */

/* Timer-A interrupt state.
   envBusy (defined in mq_tick.s) -- reentrancy guard so the tick
                        handler doesn't recurse into the sequencer if a
                        game-code path triggers another timer event
                        before the first handler completes.
   oldTimerAVec  -- previous Timer-A vector, saved so it can be restored. */
void            (*oldTimerAVec)();

/* ---- MIDI sequencer parse state -----------------------------------
   The sequencer walks a 3-byte-per-event compact stream inside
   songPos..songEndPtr.  Per-event scratch (event-type flag, note-on
   trigger, current note/channel, note-length params) is unpacked
   into a set of byte / short globals below, then handed to
   queue-note-event / send-note-off / send-program-change to reach
   the sendMidiEvent dispatcher.

   durTable (in the initialized data below) is the duration lookup
   indexed by bits 0..4 of each note event's first byte. */

unsigned char * songEndPtr;        /* end of the stream; -1 = no limit */
unsigned char * loopTarget;        /* loop-back address popped by popLoop */
char            noteDecoded;        /* set once a note was decoded in this pass */
char            noteToQueue;        /* non-zero: queue the note (bit 4 of byte 0 clear) */
char            noteIsOff;   /* non-zero: the event is a note-off (bit 5 of byte 0) */
char            noteChan;        /* logical channel, low nibble of byte 0 */
char            noteNum;        /* note to play: noteMap-mapped, or literal (+-1) */
char            noteMode;        /* note-mode bits of byte 1: absolute, or one up/down */
char            noteAccent;        /* accent bit: full velocity and full PSG volume */

/* Event queue -- 3 shorts per active note: {duration, note|flags,
   physical MIDI channel byte}.  Max 60 slots -> 20 concurrent
   notes. */
short           noteQueue[60];

/* Loop stack -- {return position, remaining count} pairs.  initSongState starts
   the index at 9 (which also means "empty") and pushLoop only pushes
   while it is below 49, so entries 9..48 hold at most 20 nested loops;
   0..8 and 49 are never touched. */
long            loopStack[50];

/* ---- PSG envelope processor state -----------------------------------
   Bresenham-style integer ramp accumulator + delta, per channel.
   Every stepEnvelopes tick, accum += delta; whenever accum > 360 (0x168),
   currentVolume steps by rampDirection and accum -= 360.  This
   fractional accumulation lets the 50 Hz envelope produce
   sub-tick-precision volume ramps without floating point.

   All 4 envelope tables (rate/time/sustain/release) are 16 shorts
   each, addressed by the low nibble of the ADSR bytes.
   ampRegs is the {0x88, 0x89, 0x8a} amp-register-with-write-bit
   for the 3 PSG channels; the assembly subtracts 0x80 back off
   before the actual psgWrite call. */
short           rampDelta[3];
short           rampAccum[3];

/* noteOwner: 128-entry table tracking
   which MIDI notes are currently sounding and on which logical channel.
   Value 0 = note not sounding.  Non-zero = the chanMap[] index (low
   nibble used) that owns the note, so stopSequencer can emit a matching
   note-off through the correct MIDI channel on shutdown. */
unsigned char   noteOwner[128];
unsigned char   psgChanNote[3];           /* current MIDI note per PSG channel A/B/C */
PSG_ENVELOPE    psgEnvelope[3];

/* ---- SFX / Dosound state -------------------------------------------- */
/* sfxPriority priority of the playing effect, for preemption. */
char            sfxCurPrio;
/* Effect duration from its last 4 bytes, high word (200 Hz). */
short           sfxDurHi;
short           sfxDurLo;        /* ... and low word */
/* 200 Hz clock when the effect started; written, never read. */
long            sfxStartHz;
/* Per-SFX Dosound sequence pointers.  Each entry points to a 2-byte
   size header followed by a Dosound register-command stream ending in
   a 4-byte terminator.  Populated at startup from SOUNDS.LCP.
   25 slots: SOUNDS.LCP holds 23 blocks before the size-0 sentinel that
   ends loadSounds's `index < 500` loop, and the original leaves room for 25
   pointers.  The 500 is a loop limit, not the size. */
unsigned char * sfxData[25];
/* Working buffer for the currently-playing Dosound sequence, copied
   from sfxData[sfxReqId] each time a new effect starts.  FIFTY-SIX
   bytes: that is the room the original leaves before the next cell it
   uses.  startSfx copies `size` bytes here straight from SOUNDS.LCP, and
   the file has longer effects -- blocks 8 (SFX_HEAD_NOD) and 17
   (SFX_TOILET_REFILL) are 148 bytes -- so ON THIS MODEL the original
   overruns the buffer by up to 92 bytes.  Reproduced as written, not
   papered over with a bigger one.

   That 56 is an INFERENCE: a declared array size never reaches the
   compiled code.  sfxBuffer..drawLogbase is exactly 400 bytes, so the
   original may instead have had ONE 400-byte struct with sfxDosStat/
   sfxDosCtl as fields at +56/+58, and no overrun at all.  A 400-byte
   array plus two separate shorts is ruled out -- .comm packs densely,
   so sfxDosStat would land at +400.  The two readings behave identically;
   see docs/history.md.

   Where the overrun LANDS depends on the BSS layout:

     * Shipped build.  sfxBuffer is followed by sfxDosStat (+56) and sfxDosCtl
       (+58) -- both WRITE-ONLY, set by stopSfx() and read nowhere -- and
       then 342 bytes that no symbol claims.  The overrun dies in that
       hole, which is why 1985 shipped it.

     * Gated test build.  The linker's own .comm packing applies, and it
       puts objHeights -- the 56-entry object HEIGHT table -- at exactly
       +56.  The first head-nod or toilet refill overwrites it; the next
       drawObject() passes objHeights[i] - 1 to vro_cpyfm as a raster
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
char            sfxBuffer[400];   /* Dosound buffer, padded (test builds) */
#else
char            sfxBuffer[56];    /* Dosound buffer handed to Dosound by startSfx */
#endif

void *  drawLogbase;    /* logical screen saved by beginDraw, restored by endDraw */
void *  panelLogbase;     /* logical screen saved by panelBegin, restored by panelEnd */
void *  housePtr;    /* 512-aligned start of houseBuf: the house picture VDI draws into */
/* stripStore: offscreen buffer where the letter-typing status strip
   composites, kept separate from the main house buffer.
   fillPanel(PANEL_ROWS_TEXT) writes rows 0..26 here so that the striped-white letter
   background is ready for the typewriter animation; renderFrame
   copyBlocks32's the content into the compositor screen when the letter
   overlay is active.
   Sized from what fillPanel can actually write: its largest caller is
   mgSetup's fillPanel(PANEL_ROWS_GAME), 77 rows of 160 bytes = 12320, and the
   align-up `(base + 512) & ~511` moves the start by at most 512 --
   so 12832 bytes, 6416 shorts.
   fillPanel points stripBuf at the ALIGNED start at run time. */
short   stripStore[6416];

/* screenScale -- always 1 (REZ_ST_MEDIUM).
   Multiplier for the 320x200 low-res screen dimensions in initMfdb,
   kept even though the value is a constant. */
short   screenScale;

/* vdiInit opens the workstation through these GLOBAL work arrays,
   not through locals. */
short   work_in[11];
short   wk_out[57];

/* screenMfdb -- source MFDB for VDI raster copies.  fd_addr = NULL is the
   VDI convention for "device screen", so vro_cpyfm(...) copies from
   the visible physbase into a memory buffer instead of another
   off-screen bitmap.

   A SHORT ARRAY, not an MFDB: initHouseBuf clears screenMfdb[0] and screenMfdb[1]
   -- the two halves of fd_addr -- and copyScreen passes the array itself,
   both at the same address. */
short   screenMfdb[10];

/* altScreen / houseBuf -- BSS scratch for the two double-buffer
   compositing screens.

   Every screen-pointer site (initHouseBuf, fillPanel, initMfdb, renderFrame's
   alternate) aligns the base up to 512 bytes:
        aligned = (base + 0x200) & ~0x1FF
   (the sprite path writes it as base + 0x1FF, masked the same way).

   Each holds ONE aligned screen: altScreen the sprite compositor (also
   renderFrame's alternate page-flip target -- there is no second screen
   at +0x8000, see parts/renderFrame.c), houseBuf the decompressed
   house.scn background.

   Sized as screen + alignment slack, not as a round power of two.
   The ST hardware only needs a 256-byte-aligned base, which would
   make 32000 + 255 enough -- but this program masks to 512, so the
   base can move up by as much as 512 and the buffer needs
   32000 + 512 = 32512. */
unsigned char   altScreen[32512];
unsigned char   houseBuf[32512];

/* Sound-effect request from sfxSelect, started by renderFrame via startSfx. */
BOOL16  sfxPending;        /* YES: a request is pending */
short   sfxReqId;        /* requested effect id (index into sfxData / sfxPriority) */
short   sfxReqDur;        /* requested duration in game ticks; -1 = the effect's own */
short   sfxDosStat;        /* set by stopSfx, read nowhere */
short   sfxDosCtl;        /* set by stopSfx, read nowhere */

/* Raw file buffers, filled at startup by loadObjects / loadSprites.  OBJECTS
   and SPRITES are each read into 14000 bytes. */
unsigned char   objFileBuf[14000];
unsigned char   sprFileBuf[14000];

/* Per-record MFDB tables + dimensions.  56 entries: main's OBJECTS
   walk is a fixed `for (i = 0; i < 56; i++)`. */
MFDB    objMfdbs[56];

short   objWidths[56];        /* each object's width in pixels, for drawObject */
short   objHeights[56];        /* each object's height in pixels */

/* YES while leaveGameTable has the resident away from a minigame: only
   the Ctrl hot keys work, typing is ignored. */
BOOL16  typingOff;
/* The command line being typed, NUL-terminated, up to 38 chars. */
char    typedLine[64];
/* Set when Ctrl-F is refused because the cabinet is full; handleKey only. */
BOOL16  pantryFull;
short   patFrame;   /* frame of the Ctrl-P patting-hand animation, stepped by tick.c */

union LASTHZ    lasthz;    /* last_hz / mi_lasT -- see globals.h */
long    lastFrameVbl;       /* VBL count at renderFrame's last frame, to pace it */
/* tosPhysbase: TOS's original Physbase, captured once at boot by
   initAes via Physbase().  Deliberately uninitialised: an explicit
   initialiser would move it from BSS into .data. */
void *  tosPhysbase;

/* frameMfdb / houseMfdb: the compositing target and the current
   physical screen descriptor.  Populated by the graphics init routine. */
MFDB    frameMfdb;
MFDB    houseMfdb;
MFDB *  flipMfdb;     /* renderFrame's page-flip MFDB */

/* Dog wander and eating state, run by renderFrame. */
BOOL16  dogNoTopFlr;   /* YES during a minigame: wander targets limited to entries 3..8 */
short   dogIdleCount;       /* frames to idle before picking the next target, 20..200 */
/* Set when the stair-landing target is picked; lets the dog eat at its
   bowl. */
BOOL16  dogMayEat;
BOOL16  dogEating;        /* YES while the dog is eating */
/* Frames of eating left; the bowl drops a step at 60, 30, 4 and 0. */
short   dogEatCount;
short   dogLastPick;       /* last wander target picked, never picked twice in a row */

/* The line submitCommand hands to matchCommand (always typedLine). */
char *  parsedLine;
/* Priority matchCommand gives the typed command: mood, chance, unknown
   words. */
short   cmdPriority;

/* Per-slot MFDB arrays for the masked-blit sprite pipeline. */
MFDB    slotImgMfdb[SPRITE_HW_SLOTS];
MFDB    slotMaskMfdb[SPRITE_HW_SLOTS];

/* ---- NLP parser state ------------------------------------------------
   phraseBits accumulates the bit masks of the recognised words; matchCommand
   then matches it against the rule table. */

char            phraseBits[10];
/* 42 bytes: nextWord() walks input from typedLine (bounded < 38 chars)
   and writes one byte per alphabetic char to cmdWord. */
char            cmdWord[42];

/* ---- Mini-game storage ----------------------------------------------- */
char *          anaDict;   /* Malloc'd "words" dictionary for Anagrams, 11-byte rows */
char *          wpzText;         /* Malloc'd Word Puzzles text */
short *         cardImages;        /* Malloc'd card images for War, Poker and Blackjack */

short           wpzIndex;         /* current Word Puzzle, 0..32, wrapping on F1/F2 */
/* Anagrams clues taken for this word; written, never read. */
short           anaNumClues;
short           anaGuessNum;        /* Anagrams guess number, 1..9 */
short           anaClueUsed;        /* 1 once F1 has given a clue for this guess */
short           anaWordLen;        /* length of the Anagrams word */
char            anaInput[12];    /* the Anagrams guess being typed, 10 chars + NUL */
/* TEN bytes, the room the original leaves for it: the scrambled word
   buffer holds a 9-letter word and its NUL. */
char            anaScrambled[10];

/* Mini-game shared state.
   mgTimedOut: set YES by mgWaitKey when the 7200-frame (~15 min) idle
            timeout fires; games check it to distinguish "user pressed
            F10" from "we auto-quit due to inactivity".
   savedTextAttr: 10-short buffer holding the pre-mini-game VDI text
            attributes so textNormal can restore them after temporarily
            switching to 20-pixel height for the title/answer render. */
BOOL16          mgTimedOut;
short           savedTextAttr[10];

short           pkrRound;       /* 0 at Poker start, 1 after pkrShowdown; read nowhere */
/* YES ends the card game (F10, timeout, or a side out of chips). */
BOOL16          cardQuit;
short           bjBetMain;        /* Blackjack: chips bet on the player's first hand */
short           bjBetSplit;        /* Blackjack: chips bet on the split hand */
/* The resident's chips (Poker/Blackjack, 400) or cards (War, 26). */
short           compChips;
short           plyrChips;        /* the player's chips or cards, likewise */
short           potChips;        /* chips in the pot */
/* anaAnswer: pointer into anaDict dictionary (11-byte rows)
   set by anaPickWord when a word is picked. */
char *          anaAnswer;
short           bjDidSplit;       /* Blackjack: 1 once the player has split */
short           warDeck[52];   /* War: the shuffled deck dealt into the two draw piles */
/* Computer's and player's draw piles, 52 shorts each: popCard's
   unconditional
     for (i = 0; i < 51; i = i + 1) pile[i] = pile[i + 1];
   shifts the whole pile. */
short           compPile[52];
short           plyrPile[52];

/* War/Blackjack per-round face-down "war" cards.  Sized 52 so the
   deepest possible recursion (all cards ending up here) still fits.
   CARD_NONE sentinel terminates.  warDepth counts the number of prior
   war rounds this hand (indexes further into the arrays). */
short           plyrWarCards[52];             /* player's war cards */
short           compWarCards[52];             /* computer's war cards */
short           warDepth;

/* Poker (5-card draw) working state.  Every field is per-hand: reset
   at the start of each round in pkrAnte / pkrDealHands / pkrShowdown.
   compHand / plyrHand also serve as the War hands. */
short           compHand[5];           /* the resident's hand, CARD_TYPE 0..51 */
short           plyrHand[5];           /* the player's hand */
short           compScoring[5];        /* 1 for each of the resident's cards
                                          that forms his pair, trips, ... */
short           compSorted[5];         /* sorted copy of the resident's hand
                                          (kicker scratch for pkrShowdown) */
short           plyrScoring[5];        /* the same two for the player */
short           plyrSorted[5];
short           compRank;              /* HAND_* rank of the resident's hand */
short           plyrRank;              /* HAND_* rank of the player's hand */
short           pkrWinner;             /* winner (0=comp, 1=player) */
short           pkrSelected[5];        /* 1 = card marked for discard */
short           pkrNumDisc;            /* cards in pkrDiscPile */
short           pkrDiscPile[13];       /* cards already seen: at most 5 + 5
                                          discards plus the explicit
                                          pkrDiscPile[10] write */
short           pkrRaiseAmt;           /* raise amount, also a draw counter */
short           pkrLastBet;            /* the player's previous bet */
short           pkrBet;     /* current bet accumulator (shared) */
BOOL16          pkrBluffing;    /* computer intends to bluff */
BOOL16          pkrPassed;    /* computer passed on the bet loop */

/* Blackjack per-hand state.
   bjSplitHand[]  -- 3rd hand slot used when the player elects to split
                two matching down-cards (post-deal, both aces or
                two of the same rank).  CARD_NONE-terminated.
   bjHitsMain / bjHitsDealer / bjHitsSplit -- remaining-hits counters (start at
                CARD_BJ_MAX = 3 for standard "up to 5 cards" rule;
                each hit decrements by CARD_BJ_STEP = 1; stop at
                CARD_BJ_STOP = 0).
   bjMatchBet    -- saved bet during split-hand bookkeeping.
   bjDblMain / bjDblSplit  -- split-hand round-active flags.
   bjNatMain / bjNatSplit -- first / second hand natural-blackjack
                achieved this round (used to skip the hit loop).
   bjBustMain / bjBustSplit  -- first / second hand busted flag.
   bjDealerScore / bjPlyrScore -- computer-picked / player-picked score
                once the double-value-with-ace picker resolves.
*/
short           bjSplitHand[5];
short           bjHitsMain;
short           bjHitsDealer;
short           bjHitsSplit;
short           bjMatchBet;
BOOL16          bjDblMain;
BOOL16          bjDblSplit;
BOOL16          bjNatMain;
BOOL16          bjNatSplit;
BOOL16          bjBustMain;
BOOL16          bjBustSplit;
short           bjDealerScore;
short           bjPlyrScore;

/* Word Puzzle state.
   wpzAnswers[i][12]  -- player's typed answer for blank i.  Max 10
                     chars + terminator + 1 slack byte.
   wpzBlanks         -- count of blanks in the current puzzle (== rows
                     of wpzAnswers[] actually in use).
   The 3 flavor-text pointer arrays hold string literals shown to
   the player while solving:
      wpzPrompts    9 entries (0..4 random first-word, 5..8 for word
                slots 2..5)
      wpzRightMsgs   6 entries, random on solve
      wpzWrongMsgs   6 entries, random on wrong answer
   Five blanks, not ten: the decoded WORDPZ.TXT has at most five '@'
   markers on a line. */
char            wpzAnswers[5][12];
short           wpzBlanks;

/* 54-entry MFDB table covering 52 card faces + 1 back + 1 highlight
   overlay.  All share cardImages as their bitmap backing. */
MFDB            cardMfdb[54];
/* The 320x77 card-table area the cards are copied into. */
MFDB            cardTableMfdb;

/* YES while dogFoodDelivery runs foodDelivery for a dog-food delivery. */
BOOL16  isDogDelivery;
BOOL16  phoneHangUp;   /* request for gameTick to hang the phone up and stop its ring */

/* ==== initialized data ================================================
   EVERY initialized global in the program, in the original's DATA
   order.

   The 1985 source kept all of this in ONE object: its data segment
   interleaves tables used all over the game, and data from separate
   objects cannot interleave.

   The order is the original's, not taste (see docs/history.md, "DATA and
   BSS layout").  DO NOT reorder by hand.
   ==================================================================== */

/* PSG register offsets.  Amp registers 8/9/10 with
   the PSG "write" bit (0x80) pre-set.  stepEnvelopes subtracts 0x80
   before calling psgWrite to recover the raw register number. */
unsigned char   ampRegs[3] = { 0x88, 0x89, 0x8a };

/* Three pointers at psgEnvelope[0..2], one per PSG channel.  NOTHING in
   the program references this table, but the original carries it in
   DATA immediately behind ampRegs, and Alcyon emits an unreferenced
   initialized global just the same.  Do not delete. */
PSG_ENVELOPE *  envPtrs[3] = { &psgEnvelope[0], &psgEnvelope[1],
                               &psgEnvelope[2] };

/* Envelope rate table.  32-byte table indexed by phaseTimer
   (already loaded from an ADSR duration byte). */
short           envRateTab[16] = {
             0,  360,  180,  120,   85,   72,   60,   45,
            30,   20,   15,   12,   10,    8,    6,    4
};

/* Envelope time table.  Reload value for phaseTimer
   when transitioning between ADSR phases. */
short           envTimeTab[16] = {
             0,    1,    2,    3,    4,    5,    6,    8,
            12,   18,   24,   30,   36,   45,   60,   90
};

/* Envelope sustain table.  Reload for phaseTimer
   during the sustain->release transition. */
short           envSusTab[16] = {
             0,    1,    2,    4,    8,   18,   24,   40,
            45,   60,   72,   90,  120,  180,  360, 30000
};

/* Envelope release table.  Applied to rampDelta
   during the sustain->release transition. */
short           envRelTab[16] = {
             0,  360,  180,   90,   45,   20,   15,    9,
             8,    6,    5,    4,    3,    2,    1,    0
};

BOOL16          midiOutOn = YES;   /* send notes to MIDI OUT */

BOOL16          psgOutOn = YES;   /* play notes on the YM2149 PSG */

/* MIDI channel count; parseSongHeader's channel-count case writes p[2]
   here. */
short           songKey = 1;

/* song tempo in beats per minute, from the song header. */
short           songTempo = 120;

/* Ticks per beat at the default tempo songTempo = 120. */
short           ticksPerBeat = 20;

/* 22 entries, not 32: that is the room the original leaves.  parseEvents
   indexes it with a 5-bit field (`durTable[*songPos & 0x1f]`), so 22..31
   would read past the end -- the .SNG data never produces them. */
short           durTable[22] = {
           0,    2,    2,    3,    4,    5,    6,    8,
           9,   12,   16,   18,   24,   32,   36,   48,
          64,   72,   96,  128,  144,    0
};

/* 128 entries, the values the original ships: YM2149 tone periods,
   period = 2000000 / (16 * f) for MIDI note n.  Entries below index 23
   are 0 -- flagged too-low by sendMidiEvent. */
short           psgPeriod[128] = {
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
     noteVel   = 127 -- max MIDI velocity
     defVelocity  = 127
     defPsgVol = 15  -- max PSG volume */
char            noteVel = 127;   /* a byte: velocity of the note being parsed */

/* These two are char (byte compares/stores; Alcyon word-aligns them,
   hence the 2-byte spacing). */
/* Velocity of an unaccented note, from the song header. */
char            defVelocity = 127;

/* PSG volume of an unaccented note, from the header velocity. */
char            defPsgVol = 15;

/* The note range the sequencer will play, HIGH first in the data:
   0x60 is the top of the range and 0x24 the bottom, and sendMidiEvent
   rejects a note above the first and below the second. */
char            noteHigh = 0x60;   /* highest note played */

char           noteLow = 0x24;   /* lowest note played */

/* 132-entry (0x84) note transpose lookup.  Indexed by MIDI note number
   0..131 (C-1..G9).  Populated by buildNoteMap at song
   start; each note maps to either itself (identity) or a shifted note
   under a chord mask, or 0xFF to skip (chromatic non-diatonic tones). */

/* ---- PSG channel state ---------------------------------------------- */

/* Program last sent on each MIDI channel; -1 = none, so the first
   sendProgChange always sends one. */
char            sentProgram[16] = {
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
};

/* Logical song channel -> physical MIDI channel (low nibble); unpackChanMap
   fills it from the song header. */
unsigned char   chanMap[16] = { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The static content is 0..99 then 110..127, with the last 14
   entries zero -- the row 100..109 is simply missing from the 1985
   table.  Harmless: buildNoteMap rewrites all 132 entries (`noteMap[i] = i`
   for i < 0x84) before anything reads them. */
unsigned char   noteMap[132] = {
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

/* songActive is a BYTE and lives in the text segment behind timerAIsr --
   see source/mq_tick.s. */
/* keyScaleMask: 16-byte chord-mask lookup. */
unsigned char   keyScaleMask[16] = {
        0xFF, 0xFF, 0x77, 0x37, 0x33, 0x13, 0x11, 0x01,
        0x00, 0xFE, 0xEE, 0xEC, 0xCC, 0xC8, 0x88, 0x00
};

/* The default program map, sixteen bytes of INITIALIZED data right
   behind keyScaleMask; progMap points here. */
unsigned char   defProgMap[16] = {
        0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x11
};

/* useSongChan and fixedChan are char: playSongFile writes them as bytes.
   useSongChan is written by playSongFile and read by parseEvents. */
char            useSongChan = 1;

char    fixedChan = YES;   /* channel every note uses when useSongChan is NO */

/* a byte pointer: MIDI program per logical channel. */
char *          progMap = (char *) defProgMap;

/* ---- the MIDI object ------------------------------------------------
   midi_seq.c is compiled AS PART OF THIS FILE, right here.  Alcyon
   emits a switch jump table into the .data of the object that holds
   the function, and the original has parseSongHeader's and sendMidiEvent's tables
   sitting between progMap and mainPalette.  Data from separate objects
   cannot interleave, so the globals and the MIDI code are ONE object,
   and the split point is exactly here.

   tools/stx_units.txt therefore names midi_seq.c as this file's
   constituent, and alcyon_link.sh links globals.o as the first game
   object.
   ---------------------------------------------------------------- */
#include "midi_seq.c"

/* No globals here for the 200 Hz clock or the VBL counter.  Both are
   ATARI ST SYSTEM VARIABLES in low memory -- _hz_200 at $04BA/$04BC
   and _vbclock at $0462 -- and startSfx and renderFrame read them there
   directly, under Super. */
