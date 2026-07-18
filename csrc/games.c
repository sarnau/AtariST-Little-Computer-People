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

/* Anagram-subsystem globals + shared mini-game plumbing. */
extern char *   g_agorw;
extern char     g_agscw[];
extern char     g_aginb[];
extern char *   g_agwgm[];
extern char *   g_aggpr[];
extern short    g_agclc;
extern short    g_aggun;
extern short    g_agacu;
extern short    ag_clue;
extern short    g_agwol;
extern BOOL16   mg_tofl;
extern PLAYER   lcp;
extern short    lcp_x;
extern short    lcp_y;
extern short    lcp_st;
extern short    lcp_face;
extern short    g_hatas;
extern BOOL16   g_actif;
extern BOOL16   g_lcyof;
extern short    g_wtx;
extern short    g_wty;
extern short    g_wtwl;                                 /* lcp_water_level */
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_selaf[];
extern short    g_seslm[];
extern void     hs_posXY();
extern short    lcp_wkD();
extern void     lcp_hwt();
extern void     sp_upds();
extern void     sp_sprs();
extern void     sp_ssco();
extern void     sp_ss02();
extern void     a_wakfa();
extern void     a_uset();
extern void     a_drink();
extern short    getEv();
extern void     execEv();
extern void     deal_kc();
extern void     prCh();
extern void     vst_h20();
extern void     rst_vsth();
extern BOOL16   alarm_p;                                /* ctrl_a_alarm_pressed_flag */

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
                Mfree(buffer);
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

/* ---- Shared mini-game plumbing -------------------------------------- */

/* lcp_lgt: leave the game table for an interrupt event (alarm,
   bathroom, thirst, delivery).  Walks the resident to the kitchen
   sink area, tucks away the game-box + table-setting sprites, and
   re-attaches the game-box in the "carried-behind" slot so subsequent
   actions render the resident holding the box.
   addr: lcp_leave_game_table() */

void
lcp_lgt()
{
        short   save_x;
        short   save_y;

        no_keyin = YES;
        g_actif  = YES;
        g_lcyof  = NO;
        lcp_y    = lcp_y - 8;
        lcp_x    = lcp_x - 6;
        lcp_st   = STATE_STAND_SIDE_VIEW;
        gameTick(0);
        hs_posXY(POS_BTM_TABLE_RIGHT, &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_KITCHEN_SINK, &g_wtx, &g_wty);
        g_wty = g_wty + 5;
        lcp_wkD();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        sp_upds();

        save_x = g_sepex[g_seslm[4]];
        save_y = g_sepey[g_seslm[4]];
        g_selaf[4] = SPRITE_HIDDEN;
        sp_upds();
        sp_ssco(SPRITE_GAME_BOX);
        g_lcyof = NO;
        g_sepex[g_seslm[4]] = save_x;
        g_sepey[g_seslm[4]] = save_y;
}

/* lcp_rgt: reverse of lcp_lgt -- walk resident back to the kitchen
   table, put the game-box down, replace the table-setting sprite,
   and restore the seated STATE_EAT_BITE pose with the +8y/+6x
   offset expected by the mini-game overlays.
   addr: lcp_return_to_game_table() */

void
lcp_rgt()
{
        short   save_x;
        short   save_y;

        g_actif = YES;
        hs_posXY(POS_BTM_KITCHEN_SINK, &g_wtx, &g_wty);
        g_wtx = g_wtx + 6;
        g_wty = g_wty + 2;
        lcp_wkD();

        save_x = g_sepex[g_seslm[4]];
        save_y = g_sepey[g_seslm[4]];
        g_selaf[4] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[4] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_GAME_BOX);
        g_sepex[g_seslm[4]] = save_x;
        g_sepey[g_seslm[4]] = save_y;

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TABLE_SETTING);
        g_sepex[g_seslm[SPRITE_TABLE_SETTING]] = 103;
        g_sepey[g_seslm[SPRITE_TABLE_SETTING]] = 180;

        hs_posXY(POS_BTM_TABLE_RIGHT, &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_TABLE_LEFT, &g_wtx, &g_wty);
        lcp_wkD();

        lcp_st   = STATE_STAND_SIDE_VIEW;
        lcp_face = FACING_RIGHT;
        g_hatas  = 8;
        lcp_hwt();

        lcp_st = STATE_EAT_BITE;
        lcp_y  = lcp_y + 8;
        lcp_x  = lcp_x + 6;
        gameTick(0);
        no_keyin = NO;
        g_actif  = NO;
}

/* mg_wkev: wait for a key while continuing to process urgent game
   events (alarm, bathroom, thirst, delivery/doorbell).  Forwards the
   Ctrl-modified event keycodes to the keyboard dispatcher so the AI
   queue sees them.  On 7200 idle frames (~15 min) sets mg_tofl=YES
   and returns KEY_F10 to force the game to quit.
   addr: minigame_wait_for_key_with_events() */

short
mg_wkev()
{
        short           key;
        short           event;
        unsigned short  idle;

        idle    = 0;
        mg_tofl = NO;

        /* Drain any keys the game accidentally left in the buffer. */
        do {
                key = getKey();
        } while (key != 0);

        for (;;) {
                key = getKey();
                if (key != 0) {
                        if (key == KEY_CTRL_A_ALARM  ||
                            key == KEY_CTRL_B_BOOK    ||
                            key == KEY_CTRL_C_CALL     ||
                            key == KEY_CTRL_D_DOGFOOD    ||
                            key == KEY_CTRL_F_FOOD  ||
                            key == KEY_CTRL_W_WATER)
                                deal_kc(key);
                        return key;
                }
                if (alarm_p != NO) {
                        lcp_lgt();
                        a_wakfa();
                        lcp_rgt();
                }
                if (lcp.bathroom_need != NO) {
                        lcp_lgt();
                        a_uset();
                        lcp_rgt();
                }
                if (lcp.thirst_level > 0 && lcp.water_level != 0) {
                        lcp_lgt();
                        a_drink();
                        lcp_rgt();
                }
                if (idle > 7200) break;
                if (g_trel[0] != ACTION_NONE) {
                        lcp_lgt();
                        event = getEv();
                        execEv(event);
                        lcp_rgt();
                }
                gameTick(0);
                idle = idle + 1;
        }
        mg_tofl = YES;
        return KEY_F10;
}

/* ---- Anagram helpers ------------------------------------------------- */

/* ag_csb: clear/redraw the bottom info bar (5,62)-(319,75).
   Used to wipe status prompts between guesses.
   addr: anagram_clear_status_bar() */

void
ag_csb()
{
        short   rect[4];
        rect[0] = 5;   rect[1] = 62;
        rect[2] = 319; rect[3] = 75;
        initVdi();
        v_bar(vdihnd, rect);
        exitVdi();
}

/* ag_cwda: clear the right-panel word display area (162,10)-(319,49).
   addr: anagram_clear_word_display_area() */

void
ag_cwda()
{
        short   rect[4];
        rect[0] = 162; rect[1] = 10;
        rect[2] = 319; rect[3] = 49;
        initVdi();
        v_bar(vdihnd, rect);
        exitVdi();
}

/* ag_cswa: clear the left-panel intro/instructions area
   (5,10)-(160,60).  Called by ag_intr's caller when moving between
   rounds.
   addr: anagram_clear_scrambled_word_area() */

void
ag_cswa()
{
        short   rect[4];
        rect[0] = 5;   rect[1] = 10;
        rect[2] = 160; rect[3] = 60;
        initVdi();
        v_bar(vdihnd, rect);
        exitVdi();
}

/* ag_cgpa: clear the middle separator bar (166,50)-(319,65) where
   the "Guess #N?" prompt is drawn.
   addr: anagram_clear_guess_prompt_area() */

void
ag_cgpa()
{
        short   rect[4];
        rect[0] = 166; rect[1] = 50;
        rect[2] = 319; rect[3] = 65;
        initVdi();
        v_bar(vdihnd, rect);
        exitVdi();
}

/* ag_sgp: draw "Guess #N?" for the current attempt.
   addr: anagram_show_guess_prompt() */

void
ag_sgp(guess)
short   guess;
{
        ag_cgpa();
        strPr(g_aggpr[guess - 1], 166, 57, COLOR_black);
}

/* ag_dwl: display a word in 20px-tall text in the right panel at
   (162, 37), with 12px spacing per character.
   addr: anagram_display_word_large() */

void
ag_dwl(word, text_color)
char *  word;
short   text_color;
{
        short   x;

        ag_cwda();
        vst_h20();
        x = 0;
        for (; *word != '\0'; word = word + 1) {
                prCh((short) *word, x + 162, 37, text_color);
                x = x + 12;
        }
        rst_vsth();
}

/* ag_intr: draw the 5-line intro text in the left panel.
   addr: anagram_show_intro_text() */

void
ag_intr()
{
        strPr("I am thinking of",  5, 17, COLOR_black);
        strPr("a word.  Here it",  5, 25, COLOR_black);
        strPr("is jumbled up...",  5, 33, COLOR_black);
        strPr("See if you can ",   5, 41, COLOR_black);
        strPr("guess what it is.", 5, 49, COLOR_black);
}

/* ag_matc: character-by-character equality test for two C strings.
   Preserves the 1985 shape (walks both strings even after finding a
   mismatch and only reports the final result) so the port stays
   byte-comparable.
   Returns 1 if the strings match, 0 otherwise.
   addr: anagram_match_result() */

short
ag_matc(a, b)
char *  a;
char *  b;
{
        BOOL16  mismatch;
        char    ca;
        char    cb;

        mismatch = NO;
        while (*a != '\0' && *b != '\0') {
                ca = *a;
                cb = *b;
                b = b + 1;
                a = a + 1;
                if (ca != cb)
                        mismatch = YES;
        }
        if (mismatch != NO || *a != '\0' || *b != '\0')
                return 0;
        return 1;
}

/* ag_ssw: pick a random word from the 150-entry dictionary (11 bytes
   per row), copy it into g_agscw, then scramble by swapping character
   pairs 10..20 times.  Re-scramble if the result equals the original.
   Also plants a '\0' terminator into g_agwb at the word's row-tail so
   subsequent code can read g_agorw as a plain C string.
   addr: anagram_select_and_scramble_word() */

void
ag_ssw()
{
        short   idx;
        short   pos;
        short   n;
        char *  wp;
        short   ia;
        short   ib;
        char    tmp;

        idx      = rndRng(0, 0x95);            /* 0..149 */
        pos      = 0;
        g_agorw  = g_agwb + (short)(idx * 11);
        for (wp = g_agorw; *wp > ' ' && *wp != '.'; wp = wp + 1) {
                g_agscw[pos] = *wp;
                pos          = pos + 1;
        }
        g_agscw[pos] = '\0';
        *wp          = '\0';
        g_agwol      = pos;

        for (;;) {
                if (ag_matc(g_agscw, g_agorw) == 0)
                        break;
                n = 0;
                for (;;) {
                        if (rndRng(10, 0x14) <= n)
                                break;
                        ia  = rndRng(0, g_agwol - 1);
                        ib  = rndRng(0, g_agwol - 1);
                        tmp = g_agscw[ib];
                        g_agscw[ib] = g_agscw[ia];
                        g_agscw[ia] = tmp;
                        n = n + 1;
                }
        }
        ag_dwl(g_agscw, COLOR_green);
}

/* ag_main: full anagram game loop.
   Structure: outer per-word loop, middle per-guess loop, inner
   per-keypress loop.  Uses labels (`new_word` / `validate`) to mirror
   the two `goto LAB_00018210` and `goto LAB_00018562` jumps in the
   1985 source -- flattening them into structured flow would change
   the shape of the port.
   addr: ag_main() (== anagram_main) */

void
ag_main()
{
        short   walk_result;
        short   index;
        short   guess_count;
        short   clue_count;
        short   key_pressed;
        char    typed_char;
        BOOL16  word_complete;

        g_agwb = (char *) Malloc(10000L);
        if (g_agwb == (char *) 0)
                er_nomem();
        fr_reac("words", (unsigned char *) g_agwb, 10000);
        mg_stp();
        strPr("***ANAGRAMS***", 5, 8, COLOR_black);
        ag_intr();

new_word:
        ag_csb();
        g_agclc = 0;
        g_aggun = 1;
        ag_ssw();

        do {
                g_agacu = 0;
                ag_clue = 0;
                strPr("F1 Clue, F10 Quit", 183, 8, COLOR_blue);
                ag_sgp(g_aggun);
                for (index = 0; index < 10; index = index + 1)
                        g_aginb[index] = ' ';
                g_aginb[10]   = '\0';
                gameTick(0);
                word_complete = NO;
                if (g_aggun > 8 &&
                    (g_aggun > 9 || g_agacu == 0))
                        return;
                index         = 0;
                key_pressed   = 0;
                do {
                        for (;;) {
                                for (;;) {
                                        if (key_pressed == KEY_CTRL_M)
                                                goto validate;
                                        strPr(g_aginb, 239, 57, COLOR_green);
                                        key_pressed = mg_wkev();
                                        if (key_pressed > 0x40 &&
                                            key_pressed < 0x5B)
                                                key_pressed = key_pressed + 0x20;
                                        if (key_pressed < 0x7B &&
                                            key_pressed > 0x60) {
                                                g_aginb[index] = (char) key_pressed;
                                                index = index + 1;
                                                if (index > 9) {
                                                        index = 9;
                                                        ag_sgp(g_aggun);
                                                }
                                        }
                                        if (key_pressed != KEY_CURSOR_LEFT)
                                                break;
                                        if (index == 0)
                                                g_aginb[0] = ' ';
                                        else if (index == 9 &&
                                                 g_aginb[9] != ' ')
                                                g_aginb[9] = ' ';
                                        else {
                                                index = index - 1;
                                                g_aginb[index] = ' ';
                                        }
                                        ag_sgp(g_aggun);
                                }
                                if (key_pressed == KEY_F10) {
                                        tx_sctm  = 0;
                                        no_keyin = NO;
                                        Mfree(g_agwb);
                                        return;
                                }
                                if (key_pressed == KEY_F1 &&
                                    ag_clue == 0) {
                                        walk_result = ag_matc(g_agorw, g_agscw);
                                        if (walk_result == 0)
                                                break;
                                }
                                if (word_complete != NO)
                                        goto validate;
                        }

                        /* Clue path: reveal one letter. */
                        g_agclc = g_agclc + 1;
                        g_aggun = g_aggun + 1;
                        ag_sgp(g_aggun);
                        if (g_aggun == 9)
                                g_agacu = 1;
                        ag_clue = 1;
                        plEr(182, 0, 319, 9);
                        strPr("         F10 Quit", 183, 8, COLOR_blue);
                        for (guess_count = 0;
                             guess_count < g_agwol &&
                             g_agorw[guess_count] == g_agscw[guess_count];
                             guess_count = guess_count + 1) ;
                        clue_count = g_agwol;
                        if (guess_count != g_agwol) {
                                do {
                                        clue_count = clue_count - 1;
                                } while (g_agorw[guess_count] !=
                                         g_agscw[clue_count]);
                                typed_char           = g_agscw[clue_count];
                                g_agscw[clue_count]  = g_agscw[guess_count];
                                g_agscw[guess_count] = typed_char;
                        }
                        ag_dwl(g_agscw, COLOR_green);
                        walk_result = ag_matc(g_agorw, g_agscw);
                        if (walk_result != 0) {
                                ag_csb();
                                strPr("You took too many clues!",
                                                     5, 69, COLOR_black);
                                ag_dwl(g_agorw, COLOR_black);
                                gameTick(0x14);
                                word_complete = YES;
                        }
                } while (word_complete == NO);

validate:
                if (word_complete != NO)
                        goto new_word;
                for (index = 0;
                     g_aginb[index] != ' ' &&
                     g_aginb[index] != '\0';
                     index = index + 1) ;
                if (g_aginb[index] == ' ')
                        g_aginb[index] = '\0';
                walk_result = ag_matc(g_aginb, g_agorw);
                if (walk_result != 0) {
                        strPr("YOU GOT IT!!!!!!",
                                             5, 69, COLOR_black);
                        ag_dwl(g_agorw, COLOR_black);
                        gameTick(0x1e);
                        ag_csb();
                        goto new_word;
                }
                if (g_aggun > 7)
                        break;
                g_aggun     = g_aggun + 1;
                walk_result = rndRng(0, 2);
                strPr(g_agwgm[walk_result], 5, 69, COLOR_black);
                gameTick(0x14);
                ag_csb();
        } while (1);

        /* Too many wrong guesses: show the answer, start a new word. */
        g_aggun = g_aggun + 1;
        strPr("Sorry, too many guesses!",
                             5, 69, COLOR_black);
        gameTick(0x14);
        ag_csb();
        strPr("Here is the word.",
                             5, 69, COLOR_black);
        ag_dwl(g_agorw, COLOR_black);
        gameTick(0x1e);
        ag_cwda();
        goto new_word;
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
                (char *) Malloc(2000L);
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
        crd_dat = (short *) Malloc(10400L);
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

        crd_dat = (short *) Malloc(10400L);
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
        crd_dat = (short *) Malloc(0x28a0L);
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
