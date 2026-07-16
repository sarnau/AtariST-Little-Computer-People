/*
 * games.c -- mini-game entry points + shared setup helpers.
 *
 * Fully-ported helpers:
 *   mg_stp -- 5-tick pause, top-strip clear, freeze the
 *                            text scroll pane, disable keyboard input.
 *   plEr       -- v_bar-based rectangular clear at (x1,y1)-(x2,y2)
 *                            with VDI init/exit brackets.
 *
 * Skeleton-ported game mains:
 *   Each of the 5 game mains has its *outer flow* ported for real:
 *     1. Allocate the game-specific data buffer via GEMDOS_Malloc
 *        (sizes verified from Ghidra: 10000 for anagram, 2000 for
 *        word puzzle, 10400 for poker/war, 0x28a0 = 10432 for
 *        blackjack).  On OOM, er_nomem (infinite alert
 *        loop on ST; exit(1) on host).
 *     2. Load the required data file with fr_reac
 *        ("words" for anagram, "wordpz.txt" for word puzzle).  Card
 *        games load their graphics via pk_ldCrd.
 *     3. Call mg_stp.
 *     4. Print the game title.
 *     5. Enter a key-poll loop that terminates on F10.
 *     6. Free the buffer via GEMDOS_Mfree, restore tx_sctm,
 *        clear no_keyin, return.
 *
 * The *inner* game logic (word scrambling, poker hand evaluation,
 * card-shuffle algorithms, ~50 subsystem helpers per game) is
 * intentionally deferred to per-game batches -- each of the 5 games
 * is a self-contained 100..600-line subsystem worth its own file.
 * With this skeleton, pressing F10 (or the outer game menu timing
 * out) cleanly returns to the house so the overall port continues
 * to link and run to completion.
 *
 * addr: mg_stp(), plEr(),
 *       ag_main(), wp_main(), pk_main(),
 *       pk_wrMn(), pk_bjMn()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    g_trel[];
extern void     gameTick();
extern short    no_keyin;
extern short    tx_sctm;
extern char *   g_ltlp[];
extern short    vdihnd;
extern char *   g_agwb;
extern char *   g_wpdb;
extern short *  crd_dat;
extern short    g_wpci;
extern short    pk_round;
extern BOOL16   pk_quit;
extern short    g_pcmon;
extern short    g_ppmon;
extern short    g_ppppa;
extern short    pk_dsc[];
extern short    g_pcdrp[];
extern short    g_ppdrp[];
extern short    rndRng();                  /* random.c */
#include <osbind.h>

extern short    getKey();
extern void     strPr();
extern void     fillTopR();
extern void     fr_reac();
extern void     er_nomem();
extern void     initVdi();
extern void     exitVdi();
extern void     v_bar();
extern void     pk_ldCrd();

/* KEY_F10 already defined in enums.h as 0x144 in our compact encoding. */

/* ---- Real helpers ---------------------------------------------------- */

/* mg_stp: prep the top status strip for the game menu.
   Fills 0x4d = 77 rows (the full text pane) with the house background,
   then freezes the text-scroll pane (`tx_sctm = -1`) and
   disables keyboard input from the game-command dispatcher so keys
   don't leak into the parser while a mini-game is running.
   addr: mg_stp() */

void
mg_stp()
{
        gameTick(5);
        fillTopR(0x4d);
        tx_sctm      = -1;
        no_keyin = YES;
}

/* plEr: clear a rectangle via VDI v_bar.  Bracketed by
   initVdi / exitVdi which are the mini-game-
   specific VDI setup helpers (deferred stubs for now).
   addr: plEr() */

void
plEr(x1, y1, x2, y2)
short   x1;
short   y1;
short   x2;
short   y2;
{
        short   rect[4];

        rect[0] = x1;
        rect[1] = y1;
        rect[2] = x2;
        rect[3] = y2;
        initVdi();
        v_bar(vdihnd, rect);
        exitVdi();
}

/* ---- Mini-game skeletons -------------------------------------------- */

/* Shared cleanup at exit from any game: restore text scroll, free the
   game's data buffer if allocated, re-enable keyboard input. */

static void
gameCln(buffer)
void *  buffer;
{
        tx_sctm      = 0;
        no_keyin = NO;
        if (buffer != (void *) 0)
                _gemdos(GEMDOS_Mfree, (long) buffer, 0L, 0L);
}

/* Poll-loop skeleton: tick + read key + return true if user quit. */

static short
gamePlWQ()
{
        short   key;
        for (;;) {
                key = getKey();
                gameTick(0);
                if (key == KEY_F10)
                        return 1;
                if (g_trel[0] != ACTION_NONE)
                        return 1;
        }
}

/* ag_main: outer flow verified; word-selection / scrambling /
   input-buffer inner loops are deferred.
   addr: ag_main() */

void
ag_main()
{
        g_agwb =
                (char *) _gemdos(GEMDOS_Malloc, 10000L, 0L, 0L);
        if (g_agwb == (char *) 0)
                er_nomem();
        fr_reac("words",
                             (unsigned char *) g_agwb,
                             10000);

        mg_stp();
        strPr("***ANAGRAMS***", 5, 8, COLOR_black);
        /* anagram_show_intro_text, anagram_select_and_scramble_word,
           the guess/clue loop -- deferred. */
        gamePlWQ();
        gameCln(g_agwb);
        g_agwb = (char *) 0;
}

/* wp_main: outer flow verified; per-puzzle parse + fill-in-
   the-blank dispatch is deferred.  Loads 66-line wordpz.txt into
   g_ltlp entries 0..0x41 via the same line-indexing pattern
   as fl_ltpl.
   addr: wp_main() */

void
wp_main()
{
        char *  parse_ptr;
        short   line_index;

        g_wpdb =
                (char *) _gemdos(GEMDOS_Malloc, 2000L, 0L, 0L);
        if (g_wpdb == (char *) 0)
                er_nomem();

        mg_stp();
        fr_reac("wordpz.txt",
                             (unsigned char *) g_wpdb,
                             1536);

        /* Index the 66 lines (33 puzzles * 2 lines each). */
        parse_ptr = g_wpdb;
        for (line_index = 0; line_index < 0x42;
             line_index = line_index + 1) {
                g_ltlp[line_index] = parse_ptr;
                do {
                        parse_ptr = parse_ptr + 1;
                } while ((unsigned char) *parse_ptr > 31);
                while ((unsigned char) *parse_ptr < ' ')
                        parse_ptr = parse_ptr + 1;
        }

        g_wpci = 0;
        strPr("**WORD PUZZLE #  **", 8, 8, COLOR_black);
        /* The per-puzzle "choose then solve" loop -- deferred. */
        gamePlWQ();
        gameCln(g_wpdb);
        g_wpdb = (char *) 0;
}

/* pk_main: outer flow verified; 5-card-draw round logic (ante, deal,
   draw, showdown, computer AI) is deferred.
   addr: pk_main() */

void
pk_main()
{
        crd_dat = (short *) _gemdos(GEMDOS_Malloc, 10400L, 0L, 0L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCrd();
        mg_stp();

        pk_round    = 0;
        pk_quit       = NO;
        g_pcmon  = 400;
        g_ppmon    = 400;
        g_ppppa      = 0;

        strPr("***POKER***", 5, 8, COLOR_black);
        /* Round loop with ante/deal/bet/draw/showdown phases -- deferred. */
        gamePlWQ();
        gameCln(crd_dat);
        crd_dat = (short *) 0;
}

/* pk_wrMn: outer flow verified; card-shuffling is real (52-card
   deck initialized 0..51, then 400 random-swap iterations, then split
   into two 26-card piles).  The head-to-head reveal + score-tracking
   loop is deferred.
   addr: pk_wrMn() */

void
pk_wrMn()
{
        short   input_key;
        short   card_index;
        short   temp;
        short   i;
        short   j;
        short   k;

        crd_dat = (short *) _gemdos(GEMDOS_Malloc, 10400L, 0L, 0L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCrd();
        mg_stp();

        g_pcmon = 26;
        g_ppmon   = 26;
        g_ppppa     = 0;

        /* Deck initialization: 52 cards indexed 0..51. */
        for (i = 0; i < 52; i = i + 1)
                pk_dsc[i] = i;

        /* Fisher-Yates-lite shuffle: 400 random-pair swaps.  Sufficient
           over a 52-element array to fully randomise the deck. */
        j = 400;
        while (j != 0) {
                input_key = rndRng(0, 51);
                do {
                        card_index = rndRng(0, 51);
                } while (input_key == card_index);
                temp = pk_dsc[card_index];
                pk_dsc[card_index] =
                        pk_dsc[input_key];
                pk_dsc[input_key] = temp;
                j = j - 1;
        }

        /* Split the shuffled deck into two 26-card piles. */
        k = 0;
        for (i = 0; i < 52; i = i + 2) {
                g_pcdrp[k] = pk_dsc[i];
                g_ppdrp[k]   = pk_dsc[i + 1];
                k = k + 1;
        }

        strPr("***WAR***", 5, 8, COLOR_black);
        /* Reveal/compare loop -- deferred. */
        gamePlWQ();
        gameCln(crd_dat);
        crd_dat = (short *) 0;
}

/* pk_bjMn: outer flow verified; hit/stand/double logic
   and dealer AI are deferred.
   addr: pk_bjMn() */

void
pk_bjMn()
{
        crd_dat = (short *) _gemdos(GEMDOS_Malloc, 0x28a0L, 0L, 0L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCrd();
        mg_stp();

        g_pcmon = 400;
        g_ppmon   = 400;

        strPr("***BLACKJACK***", 5, 8, COLOR_black);
        /* Round loop with bet/deal/hit/stand/dealer/settle -- deferred. */
        gamePlWQ();
        gameCln(crd_dat);
        crd_dat = (short *) 0;
}
