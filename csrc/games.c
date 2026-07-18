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
extern void     a_peeka();
extern short    g_hsfra;
extern short    g_hamod;
extern short    pk_pwc[];
extern short    pk_cwc[];
extern short    g_pchc;

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

/* Forward declarations: word-puzzle helpers live after the poker
   helper block and per-file extern additions further down. */
extern void     wp_shwm();
extern void     wp_rtmp();
extern void     wp_solv();
extern char     wp_ans[][12];
extern short    wp_blk;
extern char *   wp_prm[];
extern char *   wp_succ[];
extern char *   wp_fail[];
extern short    lcp_upp();
extern char     in_str[];

/* wp_main: WORD PUZZLE main loop.
   Loads wordpz.txt into a 2000-byte buffer, indexes 66 line
   pointers (33 puzzles x {template, solution}) via the same
   ordinal-scan pattern used by fl_ltpl for letter-writing.
   Displays the numeric puzzle selector; F1 next, F2 prev
   (wraps 0..0x20), F5 solve, F10 quit.  Per-puzzle: parses the
   template to count '@' blanks and seed wp_ans[i][0] with the
   character following each '@' (used by the render/scan logic
   below), renders, and waits for the next key.
   Preserves the two Ghidra gotos (LAB_000177ac = next-puzzle,
   LAB_0001797c = cleanup) verbatim.
   addr: wp_main() (== word_puzzle_main) */

void
wp_main()
{
        short   key;
        char *  parse_ptr;
        short   line_index;
        char    cur;

        g_wpdb = (char *) Malloc(2000L);
        if (g_wpdb == (char *) 0)
                er_nomem();
        mg_stp();
        fr_reac("wordpz.txt",
                             (unsigned char *) g_wpdb, 1536);

        /* Index the 66 lines. */
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

next_puzzle:
        for (;;) {
                strPr("Choose the puzzle",   8,  16, COLOR_black);
                strPr("you wish to solve.",  8,  24, COLOR_black);
                strPr("F1 Next, F5 Solve", 176,  8, COLOR_red);
                strPr("F2 Last, F10 Quit", 176, 16, COLOR_red);
                plEr(128,  0, 143,  8);
                plEr(  0, 50, 319, 69);

                /* Scan the template: count '@' blanks and seed the
                   player-answer buffer with the character following
                   each '@' (typically the punctuation the answer will
                   sit against, so the render pass has an anchor). */
                parse_ptr = g_ltlp[g_wpci + g_wpci];
                wp_blk = 0;
                for (;;) {
                        cur = *parse_ptr;
                        parse_ptr = parse_ptr + 1;
                        if (cur < ' ') break;
                        if (cur == '@') {
                                wp_ans[wp_blk][0] = *parse_ptr;
                                wp_ans[wp_blk][1] = '\0';
                                wp_blk = wp_blk + 1;
                        }
                }

                plEr(128, 0, 135, 8);
                sprintf(in_str, "%2d", g_wpci + 1);
                strPr(in_str, 128, 8, COLOR_black);
                wp_rtmp();

                for (;;) {
                        gameTick(0);
                        key = mg_wkev();
                        if (key == KEY_F1)
                                break;
                        if (key == KEY_F2) {
                                g_wpci = g_wpci - 1;
                                if (g_wpci < 0)
                                        g_wpci = 0x20;
                                goto next_puzzle;
                        }
                        if (key == KEY_F5) {
                                wp_solv();
                                if (mg_tofl != NO)
                                        goto cleanup;
                                gameTick(0x28);
                                goto next_puzzle;
                        }
                        if (key == KEY_F10)
                                goto cleanup;
                }
                g_wpci = g_wpci + 1;
                if (0x20 < g_wpci)
                        g_wpci = 0;
        }

cleanup:
        no_keyin = NO;
        tx_sctm  = 0;
        Mfree(g_wpdb);
        g_wpdb = (char *) 0;
        return;
}

/* Forward declaration for the showdown routine, used by pk_main. */
static void     pk_show();

/* pk_bjwr: nested war round.  Both players draw 3 face-down cards
   (or fewer if either is down to their last chip) then 1 face-up
   card.  On tie, recurses via the outer for loop with g_pchc++.
   Returns 0 (normal completion), -1 (computer out of cards / user
   quit), or -2 (player out of cards).
   addr: poker_blackjack_war_round() */

static short
pk_bjwr()
{
        short   drawn;
        short   res;
        short   idx;
        short   off;      /* (g_pchc << 2) + g_pchc + g_pchc == g_pchc*6 */

        g_pchc = 0;
        for (idx = 1; idx < 52; idx = idx + 1) {
                pk_cwc[idx] = CARD_NONE;
                pk_pwc[idx] = CARD_NONE;
        }
        for (;;) {
                pk_pmsg("... WAR!! ...");
                gameTick(10);
                if (g_pcmon == 0) return -1;
                if (g_ppmon == 0) return -2;

                idx = 1;
                while (idx < 4 && g_ppmon != 1 && g_pcmon != 1) {
                        off = g_pchc + g_pchc; off = off + off + g_pchc + g_pchc;
                        drawn = pk_rmch(g_ppdrp, &g_ppmon);
                        pk_pwc[idx + off] = drawn;
                        g_ppppa = g_ppppa + 1;
                        pk_drcs(CARD_BACK, idx, 1);
                        pk_dpot();
                        pk_dppm();
                        gameTick(3);
                        drawn = pk_rmch(g_pcdrp, &g_pcmon);
                        pk_cwc[idx + off] = drawn;
                        g_ppppa = g_ppppa + 1;
                        pk_drcs(CARD_BACK, idx, 0);
                        pk_dpot();
                        pk_awp();
                        gameTick(3);
                        idx = idx + 1;
                }

                /* Final face-up card each. */
                off = g_pchc + g_pchc; off = off + off + g_pchc + g_pchc;
                drawn = pk_rmch(g_ppdrp, &g_ppmon);
                pk_pwc[idx + off] = drawn;
                g_ppppa = g_ppppa + 1;
                pk_drcs(CARD_BACK, idx, 1);
                pk_dpot();
                pk_dppm();
                gameTick(3);
                drawn = pk_rmch(g_pcdrp, &g_pcmon);
                pk_cwc[idx + off] = drawn;
                g_ppppa = g_ppppa + 1;
                pk_drcs(drawn, idx, 0);
                pk_dpot();
                pk_awp();
                gameTick(3);

                pk_pmsg("Let's see what you've got...");
                strPr("F1 Show", 225, 18, COLOR_red);
                while ((res = pk_inph(KEY_F1, 0, 0)) != 1) {
                        if (mg_tofl != NO)
                                return -1;
                }
                pk_drcs(pk_pwc[idx + off], idx, 1);
                plEr(225, 10, 319, 60);
                gameTick(5);

                if ((short)((short) pk_cwc[idx + off] % 13) <
                    (short)((short) pk_pwc[idx + off] % 13)) {
                        /* Player wins the war round. */
                        pk_pmsg("You win the war!!!");
                        gameTick(8);
                        while (idx = idx - 1, idx != 0) {
                                pk_drcs(pk_cwc[idx + off], idx, 0);
                                gameTick(1);
                        }
                        gameTick(10);
                        res = g_ppppa;
                        pk_annr(1);
                        g_ppmon = g_ppmon - res;
                        for (idx = 0; pk_cwc[idx] != CARD_NONE; idx = idx + 1) {
                                pk_actd(g_ppdrp, &g_ppmon, pk_pwc[idx]);
                                pk_actd(g_ppdrp, &g_ppmon, pk_cwc[idx]);
                        }
                        return 0;
                }
                if ((short)((short) pk_pwc[idx + off] % 13) <
                    (short)((short) pk_cwc[idx + off] % 13))
                        break;
                g_pchc = g_pchc + 1;
        }

        /* Computer wins the war round. */
        pk_pmsg("I win the war!!!");
        gameTick(8);
        while (idx = idx - 1, idx != 0) {
                off = g_pchc + g_pchc; off = off + off + g_pchc + g_pchc;
                pk_drcs(pk_pwc[idx + off], idx, 1);
                gameTick(1);
        }
        gameTick(10);
        res = g_ppppa;
        pk_annr(0);
        g_pcmon = g_pcmon - res;
        for (idx = 0; pk_cwc[idx] != CARD_NONE; idx = idx + 1) {
                pk_actd(g_pcdrp, &g_pcmon, pk_pwc[idx]);
                pk_actd(g_pcdrp, &g_pcmon, pk_cwc[idx]);
        }
        return 0;
}

/* pk_wrMn: WAR mini-game main loop.  After the standard init
   (Malloc, load cards, mg_stp, 400-swap shuffle, split into 26-card
   piles, initial display), enters a per-round loop:
     - erase card/message areas
     - check both money=0 exits
     - each player reveals one card
     - compare ranks (mod 13)
     - player-higher: witty computer reaction, pk_annr(1), transfer
     - computer-higher: reaction by margin+rank, a_peeka animation,
                              pk_annr(0), transfer
     - tie: pk_bjwr() (nested war)
   Ghidra 385-instruction port.
   addr: pk_wrMn() (== poker_war_main) */

void
pk_wrMn()
{
        short   ikey;
        short   cidx;
        char *  sp;
        short   i;
        short   j;
        short   k;
        short   saved_head_frame;
        short   saved_head_mode;
        short   res;

        crd_dat = (short *) Malloc(10400L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCrd();
        mg_stp();

        g_pcmon = 26;
        g_ppmon = 26;
        g_ppppa = 0;

        /* Deck 0..51 then Fisher-Yates-lite 400-swap shuffle. */
        for (i = 0; i < 52; i = i + 1)
                pk_dsc[i] = i;
        j = 400;
        while (j != 0) {
                ikey = rndRng(0, 51);
                do {
                        cidx = rndRng(0, 51);
                } while (ikey == cidx);
                res = pk_dsc[cidx];
                pk_dsc[cidx] = pk_dsc[ikey];
                pk_dsc[ikey] = res;
                j = j - 1;
        }
        k = 0;
        for (i = 0; i < 52; i = i + 2) {
                g_pcdrp[k] = pk_dsc[i];
                g_ppdrp[k] = pk_dsc[i + 1];
                k = k + 1;
        }

        pk_awp();
        pk_dppm();
        pk_dpot();

        /* Per-round loop (Ghidra LAB_0001b29c). */
        for (;;) {
                pk_awp();
                pk_dpot();
                plEr(5, 63, 319, 75);
                plEr(225, 10, 319, 60);
                plEr(70, 10, 219, 62);

                if (g_pcmon == 0) {
                        pk_pmsg("I'm out of cards! You're too good!");
                        gameTick(0x14);
                        break;
                }
                if (g_ppmon == 0) {
                        pk_pmsg("No cards, huh? Better luck next time.");
                        gameTick(0x14);
                        break;
                }

                gameTick(5);
                pk_pwc[0] = pk_rmch(g_ppdrp, &g_ppmon);
                g_ppppa = g_ppppa + 1;
                pk_drcs(CARD_BACK, 0, 1);
                pk_dpot();
                pk_dppm();
                gameTick(3);
                pk_cwc[0] = pk_rmch(g_pcdrp, &g_pcmon);
                g_ppppa = g_ppppa + 1;
                pk_drcs(pk_cwc[0], 0, 0);
                pk_dpot();
                pk_awp();

                pk_pmsg("Show me your card, Ace.");
                strPr("F1  Show", 225, 18, COLOR_red);
                strPr("F10 Quit", 225, 26, COLOR_red);
                i = 0;
                while (i != 1 && i != 2)
                        i = pk_inph(KEY_F1, KEY_F10, 0);
                if (i == 2)
                        break;

                pk_drcs(pk_pwc[0], 0, 1);
                plEr(225, 10, 319, 60);
                gameTick(5);

                if ((short)((short) pk_cwc[0] % 13) <
                    (short)((short) pk_pwc[0] % 13)) {
                        /* Player wins. */
                        if ((short)((short) pk_pwc[0] % 13) == 12) {
                                sp = "Ace? I don't believe it!";
                        } else {
                                ikey = rndRng(1, 6);
                                switch (ikey) {
                                case 1: sp = "You're awfully lucky!";       break;
                                case 2: sp = "Arrghh!";                        break;
                                case 3: sp = "You're tough.";                 break;
                                case 4: sp = "I'll get you next time.";     break;
                                case 5: sp = "Dog-gone it.";                   break;
                                case 6: sp = "All right. Slow down.";       break;
                                default: sp = "";                                    break;
                                }
                        }
                        pk_pmsg(sp);
                        gameTick(8);
                        pk_annr(1);
                        plEr(70, 10, 219, 62);
                        g_ppmon = g_ppmon - 2;
                        pk_actd(g_ppdrp, &g_ppmon, pk_pwc[0]);
                        pk_actd(g_ppdrp, &g_ppmon, pk_cwc[0]);
                        continue;
                }

                ikey = (short) pk_cwc[0] % 13;
                cidx = (short) pk_pwc[0] % 13;

                if (cidx < ikey) {
                        /* Computer wins by margin (ikey - cidx). */
                        if (ikey == 12) {
                                sp = "Ace takes it!";
                        } else if ((short)(ikey - cidx) < 3) {
                                ikey = rndRng(0, 1);
                                sp = (ikey == 0)
                                    ? "Hmm... That's not too bad!"
                                    : "Whew! That was too close.";
                        } else if ((short)(ikey - cidx) < 7) {
                                if (cidx < 4) {
                                        ikey = rndRng(0, 1);
                                        sp = (ikey == 0)
                                            ? "Not a very high card, but I'll take it."
                                            : "That's an easy card to beat.";
                                } else if (cidx < 9) {
                                        ikey = rndRng(1, 3);
                                        if      (ikey == 1) sp = "Alright. I win!";
                                        else if (ikey == 2) sp = "Better luck next time.";
                                        else                sp = "Hey... look at that!";
                                } else {
                                        sp = "Great, a face card, and it's mine now!";
                                }
                        } else {
                                ikey = rndRng(1, 3);
                                if      (ikey == 1) sp = "No contest. You lose!";
                                else if (ikey == 2) sp = "Beat you by a mile.";
                                else                sp = "That was easy!";
                        }
                        pk_pmsg(sp);
                        saved_head_frame = g_hsfra;
                        saved_head_mode  = g_hamod;
                        a_peeka();
                        g_hamod = saved_head_mode;
                        gameTick(8);
                        pk_annr(0);
                        plEr(70, 10, 219, 62);
                        g_pcmon = g_pcmon - 2;
                        pk_actd(g_pcdrp, &g_pcmon, pk_pwc[0]);
                        pk_actd(g_pcdrp, &g_pcmon, pk_cwc[0]);
                        g_hsfra = saved_head_frame;
                        continue;
                }

                /* Tie -> war round. */
                res = pk_bjwr();
                if (mg_tofl != NO)
                        break;
                if (res == -1) {
                        pk_pmsg("I'm out of cards! You're too good!");
                        gameTick(0x14);
                        break;
                }
                if (res == -2) {
                        pk_pmsg("No cards, huh? Better luck next time.");
                        gameTick(0x14);
                        break;
                }
        }

        /* Common cleanup path (Ghidra LAB_0001b308). */
        tx_sctm  = 0;
        no_keyin = NO;
        Mfree(crd_dat);
        moff();
        crd_dat = (short *) 0;
}

/* pk_bjMn moved to the end of the file so it can reference every
   blackjack helper (pk_dbhi/chsc/dchd/cnbj/sbet/bjr) without needing
   forward declarations.  Entry point + calling signature preserved
   so agames.c compiles unchanged. */

/* ---- Poker/War/Blackjack shared helpers ---------------------------- */

extern short    g_pcmon;
extern short    g_ppmon;
extern short    g_ppppa;
extern short    g_pchc;
extern short    pk_pwc[];
extern short    pk_cwc[];
extern MFDB     crd_mfdb[];
extern MFDB     mf_scb_c;
extern short    crd_xa[];
extern short    crd_ya[];
extern short    crd_xb[];
extern short    crd_yb[];
extern short    pk_ch[];
extern short    pk_ph[];
extern short    pk_hrf[];
extern short    pk_hsf[];
extern short    pk_phrf[];
extern short    pk_phsf[];
extern short    pk_chrk;
extern short    pk_phrk;
extern short    pk_dslot;
extern short    pk_sel[];
extern short    pk_disc;
extern short    pk_dpile[];
extern short    pk_dpos;
extern short    pk_phv;
extern short    pk_bet;
extern BOOL16   pk_bluff;
extern BOOL16   pk_pass;
extern char     pk_bm[];
extern char     pk_rm[];
extern char     pk_tcm[];
extern void     vdi_cprt();
extern void     moff();
extern short    ph_ans;  /* dummy pull-in to satisfy per-file extern block */

/* pk_pmsg: print a green status message in the bottom info bar
   (5,71)..(319,75) after clearing the strip.
   addr: poker_print_message() */

void
pk_pmsg(str)
char *  str;
{
        plEr(5, 63, 319, 75);
        strPr(str, 5, 71, COLOR_green);
}

/* pk_awp: display computer money count in the top-left panel.
   Ghidra hand-formats a 3-digit decimal (space-padded on leading
   zeros) into a fixed 12-byte stack buffer, laying the digit bytes
   at positions 0/1/2 with '\0' at position 3 and using positions
   4/6/8 as scratch for the quotient/remainder pass.  Preserved
   verbatim so the port stays byte-comparable.
   addr: poker_award_pot() */

void
pk_awp()
{
        char    str[12];

        plEr(5, 10, 31, 20);
        str[3] = '\0';
        str[8] = (char)((int) g_pcmon / 100);
        str[0] = str[8] + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (char)((int)(g_pcmon % 100) / 10);
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = (char)((int)(g_pcmon % 100) % 10);
        str[2] = str[4] + '0';
        strPr(str, 5, 18, COLOR_black);
}

/* pk_dppm: display player money count in the bottom-left panel.
   Same three-digit-with-space-padding shape as pk_awp.
   addr: poker_display_player_money() */

void
pk_dppm()
{
        char    str[12];

        plEr(5, 50, 31, 60);
        str[3] = '\0';
        str[8] = (char)((int) g_ppmon / 100);
        str[0] = str[8] + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (char)((int)(g_ppmon % 100) / 10);
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = (char)((int)(g_ppmon % 100) % 10);
        str[2] = str[4] + '0';
        strPr(str, 5, 58, COLOR_black);
}

/* pk_dpot: display the pot amount in the middle panel.
   addr: poker_display_pot() */

void
pk_dpot()
{
        char    str[14];

        plEr(31, 30, 57, 40);
        str[3] = '\0';
        str[8] = (char)((int) g_ppppa / 100);
        str[0] = str[8] + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (char)((int)(g_ppppa % 100) / 10);
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = (char)((int)(g_ppppa % 100) % 10);
        str[2] = str[4] + '0';
        strPr(str, 31, 38, COLOR_black);
}

/* pk_rmch: pop a card off the top of `pile`; shift the remaining
   `*count-1` entries down one slot.  Returns CARD_NONE if the pile
   was already empty.  Used by war/blackjack to draw from each
   player's draw pile.
   addr: poker_remove_card_from_hand() */

short
pk_rmch(pile, count)
short * pile;
short * count;
{
        short   card;
        short   n;
        short   i;

        if (*count == 0)
                return CARD_NONE;
        card    = *pile;
        n       = *count;
        *count  = n - 1;
        if ((short)(n - 1) != 0) {
                for (i = 0; i < 51; i = i + 1)
                        pile[i] = pile[i + 1];
        }
        return card;
}

/* pk_actd: append `val` at position pile[*idx] and increment idx.
   Used to push captured cards back to the winner's draw pile after
   a round resolves.
   addr: poker_add_card_to_discard() */

void
pk_actd(pile, idx, val)
short * pile;
short * idx;
short   val;
{
        pile[*idx] = val;
        *idx = *idx + 1;
}

/* pk_annr: transfer the pot to the round winner one chip per tick
   (winner=0 -> computer, winner=1 -> player).  Zeros the pot when
   done.  Animated so the running total ticks up on screen.
   addr: poker_ante_and_new_round() */

void
pk_annr(winner)
short   winner;
{
        while (g_ppppa != 0) {
                if (winner == 0) {
                        g_pcmon = g_pcmon + 1;
                        g_ppppa = g_ppppa - 1;
                        pk_awp();
                        pk_dpot();
                        gameTick(0);
                } else {
                        g_ppmon = g_ppmon + 1;
                        g_ppppa = g_ppppa - 1;
                        pk_dppm();
                        pk_dpot();
                        gameTick(0);
                }
        }
        g_ppppa = 0;
}

/* pk_inph: wait for one of the caller-supplied F-keys (a/b/c),
   digit keys 1..5, or auto-timeout.  Returns codes 1..8 for
   a/b/c/1/2/3/4/5 respectively, or -1 on inactivity timeout.
   Handles the ambient event pump via mg_wkev between polls.
   addr: poker_input_handler() */

short
pk_inph(a, b, c)
short   a;
short   b;
short   c;
{
        short   ch;

        for (;;) {
                gameTick(0);
                ch = mg_wkev();
                if (ch == a) return 1;
                if (ch == b) return 2;
                if (ch == c) return 3;
                if (ch == 0x31) return 4;         /* '1' */
                if (ch == 0x32) return 5;         /* '2' */
                if (ch == 0x33) return 6;         /* '3' */
                if (ch == 0x34) return 7;         /* '4' */
                if (ch == 0x35) return 8;         /* '5' */
                if (mg_tofl != NO)
                        return -1;
        }
}

/* pk_drcs: blit one card sprite at slot `xi` of row `yi`.
   card=CARD_BACK selects the shared face-down back MFDB
   (crd_mfdb[52]); face cards 0..51 index directly.  15x23-pixel
   card artwork.
   addr: poker_draw_card_sprite() */

void
pk_drcs(card, xi, yi)
short   card;
short   xi;
short   yi;
{
        short   x;
        short   y;

        if (yi == 0) {
                x = crd_xa[xi];
                y = crd_ya[xi];
        } else {
                x = crd_xb[xi];
                y = crd_yb[xi];
        }
        vdi_cprt(vdihnd, S_ONLY, &crd_mfdb[card], &mf_scb_c,
                              0, 0, 15, 23,
                              x, y, x + 15, y + 23);
}

/* ---- Poker helpers ------------------------------------------------- */

/* pk_ante: opening prompt "Ante up to play." + F1 Ante / F10 Quit.
   On F1: both players contribute 1 chip to the pot (empty check first,
   with "Sorry, you're all out!!!" / "I'm all out!!!" exits).  On F10
   or timeout: sets pk_quit.
   addr: poker_ante_phase() */

static void
pk_ante()
{
        short   r;

        g_ppppa = 0;
        plEr(225, 10, 319, 60);
        strPr("F1  Ante", 225, 18, COLOR_red);
        strPr("F10 Quit", 225, 34, COLOR_red);
        pk_pmsg("Ante up to play.");
        pk_quit = NO;
        r = 0;
        while (r != 1 && r != 3 && r != -1)
                r = pk_inph(KEY_F1, 0, KEY_F10);
        if (r == 3 || r == -1) {
                pk_quit = YES;
        } else if (g_ppmon == 0) {
                pk_pmsg("Sorry, you're all out!!!");
                gameTick(0x1e);
                pk_quit = YES;
        } else if (g_pcmon == 0) {
                pk_pmsg("I'm all out!!!");
                gameTick(0x1e);
                pk_quit = YES;
        } else {
                plEr(5, 63, 319, 75);
                g_ppmon = g_ppmon - 1;
                pk_dppm();
                g_ppppa = g_ppppa + 1;
                pk_dpot();
                g_pcmon = g_pcmon - 1;
                pk_awp();
                g_ppppa = g_ppppa + 1;
                pk_dpot();
        }
}

/* pk_evh: evaluate a 5-card hand.  Writes hand rank into *hand_rank
   (0=high card ... 9=royal flush) and marks the cards that make up
   the winning combination in rank_flags[i]=1.  suit_flags receives
   a rank-sorted copy of the hand (used as kicker scratch by pk_show).
   Preserves the exact Ghidra shape including two goto exits so the
   port stays byte-comparable with the 1985 asm.
   addr: poker_evaluate_hand() */

static void
pk_evh(hand, rank_flags, suit_flags, hand_rank)
short * hand;
short * rank_flags;
short * suit_flags;
short * hand_rank;
{
        short    bp;
        short    hc;
        unsigned short  sc[5];      /* pair-slot flag scratch */
        short    rc[5];             /* rank_flags scratch */
        short    trips;
        short    i, j;
        BOOL16   flush;
        BOOL16   straight;
        short    tmp;

        *hand_rank = 0;
        for (i = 0; i < 5; i = i + 1)
                suit_flags[i] = hand[i];

        /* Bubble sort suit_flags[] by rank ascending. */
        straight = YES;
        while (straight) {
                straight = NO;
                for (i = 0; i < 4; i = i + 1) {
                        if (suit_flags[i + 1] % 13 <
                            suit_flags[i]     % 13) {
                                tmp = suit_flags[i + 1];
                                suit_flags[i + 1] = suit_flags[i];
                                suit_flags[i] = tmp;
                                straight = YES;
                        }
                }
        }

        /* Straight detection: 0..3 monotonic +1 in rank. */
        straight = YES;
        for (i = 0; i < 3; i = i + 1) {
                if ((short)(suit_flags[i] % 13) !=
                    (short)(suit_flags[i + 1] % 13 - 1))
                        straight = NO;
        }
        flush = NO;
        /* Wheel straight A-2-3-4-5 lives with Ace-high sorted last. */
        if (suit_flags[4] % 13 == 12 && straight != NO &&
            suit_flags[0] % 13 ==  0)
                flush = YES;
        if (flush == NO &&
            (short)(suit_flags[3] % 13) !=
            (short)(suit_flags[4] % 13 - 1))
                straight = NO;

        /* Flush: all same suit (card / 13). */
        flush = YES;
        for (i = 0; i < 4; i = i + 1) {
                if (suit_flags[i]     / 13 !=
                    suit_flags[i + 1] / 13)
                        flush = NO;
        }
        if (straight != NO) *hand_rank = 4;
        if (flush != NO)    *hand_rank = 5;
        if (straight != NO && flush != NO)
                *hand_rank = 8;
        /* Royal: T-J-Q-K-A of one suit -- rank[0] == 8 (ten). */
        if (*hand_rank == 8 && suit_flags[0] % 13 == 8)
                *hand_rank = 9;
        if (*hand_rank != 0)
                return;

        for (i = 0; i < 5; i = i + 1) {
                rank_flags[i] = 0;
                rc[i]         = 0;
                sc[i]         = 0;
        }

        hc = 0;                 /* first-hit rank (pair/trip/quad) */
        bp = 0;                 /* second-hit rank (two pair / full) */
        i = 0;
        while (i <= 12) {
                trips = 0;
                for (j = 0; j < 5; j = j + 1) {
                        if ((short) hand[j] % 13 == i) {
                                if (hc == 0)
                                        rc[j] = 1;
                                else if (bp == 0)
                                        sc[j] = 1;
                                trips = trips + 1;
                        }
                }
                if (trips == 4)
                        break;
                if (trips == 3) {
                        if (hc == 0) {
                                hc = 3;
                                for (j = 0; j < 5; j = j + 1)
                                        rank_flags[j] = rc[j];
                        } else if (bp == 0) {
                                bp = 3;
                                for (j = 0; j < 5; j = j + 1)
                                        rank_flags[j] = sc[j];
                                goto rank_from_hc_bp;
                        }
                }
                if (trips == 1) {
                        if (hc == 0) {
                                for (j = 0; j < 5; j = j + 1)
                                        rc[j] = 0;
                        } else if (bp == 0) {
                                for (j = 0; j < 5; j = j + 1)
                                        sc[j] = 0;
                        }
                }
                if (trips == 2) {
                        if (hc == 0) {
                                hc = 1;
                                for (j = 0; j < 5; j = j + 1)
                                        rank_flags[j] = rc[j];
                        } else if (bp == 0) {
                                if (hc == 1) {
                                        bp = 1;
                                        for (j = 0; j < 5; j = j + 1)
                                                rank_flags[j] = sc[j] | rank_flags[j];
                                } else if (hc == 3) {
                                        bp = 1;
                                }
                        }
                }
                i = i + 1;
        }
        /* Four of a kind: rank 7, flags = rc (the 4 matched cards). */
        hc = 7;
        for (i = 0; i < 5; i = i + 1)
                rank_flags[i] = rc[i];

rank_from_hc_bp:
        if ((short)(bp + hc) == 7) *hand_rank = 7;   /* 4-of-kind */
        if ((short)(bp + hc) == 3) *hand_rank = 3;   /* trips */
        if ((short)(bp + hc) == 4) *hand_rank = 6;   /* full house */
        if ((short)(bp + hc) == 2) *hand_rank = 2;   /* two pair */
        if ((short)(bp + hc) != 1) return;
        *hand_rank = 1;                                          /* one pair */
}

/* pk_evhs: deal a fresh 5-card hand to each player.  Zeros both
   hands with CARD_NONE, then repeatedly draws random cards
   (uniqueness check across both hands) until 10 unique picks land.
   Player hand shown face-up, computer hand face-down.
   addr: poker_evaluate_hands() */

static void
pk_evhs()
{
        BOOL16  dup;
        short   j;
        short   c;
        short   i;

        for (i = 0; i < 5; i = i + 1) {
                pk_ch[i] = CARD_NONE;
                pk_ph[i] = CARD_NONE;
        }
        for (i = 0; i < 5; i = i + 1) {
                dup = YES;
                while (dup != NO) {
                        c   = rndRng(0, 51);
                        dup = NO;
                        for (j = 0; j < 5; j = j + 1) {
                                if (pk_ch[j] == c || pk_ph[j] == c)
                                        dup = YES;
                        }
                }
                pk_ch[i] = c;
                dup = YES;
                while (dup != NO) {
                        c   = rndRng(0, 51);
                        dup = NO;
                        for (j = 0; j < 5; j = j + 1) {
                                if (pk_ch[j] == c || pk_ph[j] == c)
                                        dup = YES;
                        }
                }
                pk_ph[i] = c;
        }
        plEr(70, 10, 219, 62);
        for (i = 0; i < 5; i = i + 1) {
                pk_drcs(pk_ph[i], i, 1);
                gameTick(3);
                pk_drcs(CARD_BACK, i, 0);
                gameTick(3);
        }
}

/* pk_blf: 1/15 chance of bluff -- but only when the computer's hand
   is weak (rank < 2, i.e. high card or single pair).  Sets pk_bluff.
   addr: poker_computer_decide_bluff() */

static void
pk_blf()
{
        short   r;

        pk_bluff = NO;
        r = rndRng(0, 14);
        if (r == 0 && pk_chrk < 2)
                pk_bluff = YES;
}

/* pk_cace: should the computer open?  If bluffing, always yes (0).
   Otherwise looks for a card of rank >= King (rank 11 = Q, 12 = A).
   Returns rank of best card if it beats jacks (>= Q), else -1
   (pass) -- classic "Jacks or better to open".
   addr: poker_computer_check_ace() */

static short
pk_cace()
{
        short   ret;
        short   best;
        short   i;

        if (pk_bluff == NO && pk_chrk == 0) {
                best = 0;
                for (i = 0; i < 5; i = i + 1) {
                        if ((short) pk_ch[best] % 13 <
                            (short) pk_ch[i]    % 13)
                                best = i;
                }
                ret = (short) pk_ch[best] % 13;
                if (ret < 12)
                        ret = -1;
        } else {
                ret = 0;
        }
        return ret;
}

/* pk_dbet: quick "call vs raise" decision.  Returns 'c' (call) if
   out of money or weak hand, else 'r' (raise) with pk_dpos set to
   money/10 clamped to [1, 20].
   addr: poker_computer_decide_bet() */

static short
pk_dbet()
{
        short   ch;

        if (g_pcmon == 0)
                ch = 'c';
        else if (pk_bluff == NO && pk_chrk < 2)
                ch = 'c';
        else {
                pk_dpos = g_pcmon / 10;
                if (pk_dpos == 0)      pk_dpos = 1;
                else if (pk_dpos > 20) pk_dpos = 20;
                ch = 'r';
        }
        return ch;
}

/* pk_ddec: one chip animated transfer for the current player.
   `who`==0 -> computer contributes, 1 -> player contributes.
   `n` chips to move.  Caps pk_bet at 20 to prevent unbounded
   sessions.  Called every tick during pk_cbet's F1 hold-to-raise.
   addr: poker_computer_draw_decision() */

static void
pk_ddec(who, n)
short   who;
short   n;
{
        short   left;

        left = n;
        if (pk_bet != 20) {
                while (left != 0 &&
                       (who != 0 || g_pcmon != 0) &&
                       (who != 1 || g_ppmon != 0)) {
                        if (who == 0) {
                                g_pcmon = g_pcmon - 1;
                                pk_awp();
                                g_ppppa = g_ppppa + 1;
                                pk_dpot();
                                pk_bet = pk_bet + 1;
                        }
                        left = left - 1;
                        if (who == 1) {
                                g_ppmon = g_ppmon - 1;
                                pk_dppm();
                                g_ppppa = g_ppppa + 1;
                                pk_dpot();
                                pk_bet = pk_bet + 1;
                        }
                }
        }
}

/* pk_cbet: player betting UI.  Shows caller-provided prompt then
   F1 Bet / F3 Enter / F5 Pass-Clr keys.  F1 held: pk_ddec bumps one
   chip per tick.  F3: locks in current bet (or sets pk_pass if 0).
   F5: refunds current bet.  Returns 0 normally, -1 on timeout.
   addr: poker_computer_bet_decision() */

static short
pk_cbet(str)
char *  str;
{
        short   r;

        pk_bet  = 0;
        pk_pass = NO;
        pk_pmsg(str);
        plEr(225, 10, 319, 60);
        strPr("F1 Bet",       225, 18, COLOR_red);
        strPr("F3 Enter",     225, 26, COLOR_red);
        strPr("F5 Pass/Clr", 225, 34, COLOR_red);
        for (;;) {
                r = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                if (r == -1) return -1;
                if (r == 3)  break;
                if (r == 1) {
                        if (g_ppmon != 0) {
                                pk_ddec(1, 1);
                                for (;;) {
                                        r = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                                        if (r == -1) return -1;
                                        if (r == 2 && pk_bet != 0)
                                                return 0;
                                        if (r == 1)
                                                pk_ddec(1, 1);
                                        if (r == 3) {
                                                if (pk_bet == 0) {
                                                        pk_pass = YES;
                                                        return 0;
                                                }
                                                g_ppmon = g_ppmon + pk_bet;
                                                g_ppppa = g_ppppa - pk_bet;
                                                pk_bet  = 0;
                                                pk_dppm();
                                                pk_dpot();
                                        }
                                }
                        }
                        return -1;
                }
        }
        pk_pass = YES;
        return 0;
}

/* pk_cdrw: computer AI draw phase.  Evaluates hand, decides bluff,
   picks discard count by rank:
      rank 0 (high card)  -> discard 4, keep highest
      rank 1 (one pair)   -> discard 3, keep pair
      rank 2 (two pair)   -> discard 1, keep both pairs
      rank 3 (three)      -> discard 2, keep trips
      rank >=4            -> stay
   When bluffing, picks 0..2 discards from non-rank cards to fake it.
   Then re-draws unique replacements, patches pk_tcm with "N card"
   or "N cards", and animates the swap (blank slot flash -> new back).
   addr: poker_computer_draw_cards() */

static void
pk_cdrw()
{
        short   nc;                  /* card_in_use flag -> BOOL16 */
        short   dm;                  /* draw_message_count / scratch */
        short   dc;                  /* discard count seed / temp */
        short   card;
        short   n;                   /* new_card */
        short   i;
        short   r;

        for (i = 0; i < 5; i = i + 1)
                pk_sel[i] = 0;
        pk_evh(pk_ch, pk_hrf, pk_hsf, &pk_chrk);
        pk_blf();

        if (pk_bluff == NO) {
                if (pk_chrk < 4) {
                        if (pk_chrk == 3) {
                                card = 2;
                                for (i = 0; i < 5; i = i + 1)
                                        if (pk_hrf[i] == 0)
                                                pk_sel[i] = 1;
                        } else if (pk_chrk == 2) {
                                card = 1;
                                for (i = 0; i < 5; i = i + 1)
                                        if (pk_hrf[i] == 0)
                                                pk_sel[i] = 1;
                        } else if (pk_chrk == 1) {
                                card = 3;
                                for (i = 0; i < 5; i = i + 1)
                                        if (pk_hrf[i] == 0)
                                                pk_sel[i] = 1;
                        } else if (pk_chrk == 0) {
                                card = 4;
                                dc   = 0;
                                for (i = 0; i < 5; i = i + 1) {
                                        if ((short) pk_ch[dc] % 13 <
                                            (short) pk_ch[i]  % 13)
                                                dc = i;
                                }
                                for (i = 0; i < 5; i = i + 1)
                                        if (i != dc)
                                                pk_sel[i] = 1;
                        } else {
                                card = 0;
                        }
                } else {
                        card = 0;
                }
        } else {
                card = rndRng(0, 2);
                dc = 0;
                i  = card;
                while (dc < 5 && i != 0) {
                        if (pk_hrf[dc] == 0) {
                                pk_sel[dc] = 1;
                                i = i - 1;
                        }
                        dc = dc + 1;
                }
        }

        if (card == 0) {
                pk_pmsg("I'll stay!");
                gameTick(8);
                return;
        }

        pk_tcm[10] = (char) card + '0';
        if (card == 1) {
                pk_tcm[16] = '.';
                pk_tcm[17] = '\0';
        } else {
                pk_tcm[16] = 's';
                pk_tcm[17] = '.';
        }
        pk_pmsg(pk_tcm);
        gameTick(8);

        for (i = 0; i < 5; i = i + 1) {
                if (pk_sel[i] == 1) {
                        nc = YES;
                        while (nc != NO) {
                                n  = rndRng(0, 51);
                                nc = NO;
                                for (dm = 0; dm < 5; dm = dm + 1) {
                                        if (pk_ch[dm] == n) nc = YES;
                                        if (pk_ph[dm] == n) nc = YES;
                                }
                                dm = pk_disc;
                                while (r = dm - 1, dm != 0) {
                                        dm = r;
                                        if (pk_dpile[r] == n) nc = YES;
                                }
                                pk_dpile[pk_disc] = pk_ch[i];
                                pk_disc = pk_disc + 1;
                                pk_ch[i] = n;
                                pk_drcs(CARD_HIGHLIGHT, i, 0);
                                gameTick(3);
                        }
                }
        }
        for (i = 0; i < 5; i = i + 1) {
                if (pk_sel[i] == 1) {
                        pk_drcs(CARD_BACK, i, 0);
                        gameTick(1);
                }
        }
}

/* pk_show: showdown.  Reveals the computer's face-down hand, then
   evaluates both hands, then plays the poker hand-comparison ladder:
    rank tie -> kicker/high-card ladder for pair, two-pair, trips,
                       full house, four-of-kind, straight-flush;
    rank differs -> higher rank wins.
   Winner blinks (5x on/off) then pk_annr transfers the pot.
   sets pk_round to 1 as a "round completed" side-flag used by the
   caller.  Preserves Ghidra's exact per-rank tiebreak shape --
   including reusing pk_hsf as the sorted-hand scratch and the
   CARD_HEART_KING (value 0, rank 0) seeds -- for byte fidelity.
   addr: poker_showdown() */

static void
pk_show()
{
        short   ck;         /* computer_kicker */
        short   pk;         /* player_kicker */
        short   ch;         /* computer_high_card */
        short   ph;         /* player_high_card */
        unsigned short  br; /* bet_round / blink counter */
        short   i;
        short   j;

        /* Reveal computer hand, animated. */
        for (i = 0; i < 5; i = i + 1) {
                pk_drcs(pk_ch[i], i, 0);
                gameTick(2);
        }
        pk_evh(pk_ch, pk_hrf,  pk_hsf,  &pk_chrk);
        pk_evh(pk_ph, pk_phrf, pk_phsf, &pk_phrk);

        if (pk_phrk < pk_chrk) pk_dslot = 0;
        if (pk_chrk < pk_phrk) pk_dslot = 1;

        if (pk_chrk == pk_phrk) {
                pk_dslot = 1;

                /* Straight/flush/straight-flush tiebreak: compare
                   highest sorted-hand card rank. */
                if ((pk_chrk == 8 || pk_chrk == 5 || pk_chrk == 4) &&
                    pk_phsf[4] % 13 < pk_hsf[4] % 13)
                        pk_dslot = 0;

                /* Trips, full house, quads: compare the pair/trip
                   card's rank. */
                if (pk_chrk == 7 || pk_chrk == 6 || pk_chrk == 3) {
                        for (br = 0;
                             (short) br < 5 && pk_hrf[(short) br] != 1;
                             br = br + 1) ;
                        for (i = 0;
                             i < 5 && pk_phrf[i] != 1;
                             i = i + 1) ;
                        if ((short) pk_ph[i] % 13 <
                            (short) pk_ch[(short) br] % 13)
                                pk_dslot = 0;
                }

                /* Two pair tiebreak. */
                if (pk_chrk == 2) {
                        pk = 0; ck = 0; ph = 0; ch = 0;
                        for (i = 0; i < 5; i = i + 1) {
                                if (pk_hrf[i] != 0 &&
                                    (short) pk % 13 <
                                    (short) pk_ch[i] % 13)
                                        pk = pk_ch[i];
                                if (pk_hrf[i] != 0 &&
                                    (short) pk_ch[i] % 13 <
                                    (short) pk % 13)
                                        ck = pk_ch[i];
                                if (pk_phrf[i] != 0 &&
                                    (short) ph % 13 <
                                    (short) pk_ph[i] % 13)
                                        ph = pk_ph[i];
                                if (pk_phrf[i] != 0 &&
                                    (short) pk_ph[i] % 13 <
                                    (short) ph % 13)
                                        ch = pk_ph[i];
                        }
                        if ((short) ph % 13 < (short) pk % 13) {
                                pk_dslot = 0;
                        } else if ((short) pk % 13 == (short) ph % 13 &&
                                   (short) ch % 13 <
                                   (short) ck % 13) {
                                pk_dslot = 0;
                        } else if ((short) pk % 13 == (short) ph % 13 &&
                                   (short) ck % 13 == (short) ch % 13) {
                                for (i = 0;
                                     i < 5 && pk_hrf[i] != 0;
                                     i = i + 1) ;
                                for (br = 0;
                                     (short) br < 5 &&
                                     pk_phrf[(short) br] != 0;
                                     br = br + 1) ;
                                if ((short) pk_ph[(short) br] % 13 <
                                    (short) pk_ch[i] % 13)
                                        pk_dslot = 0;
                        }
                }

                /* One pair: compare pair rank, then kicker ladder. */
                if (pk_chrk == 1) {
                        pk = 0; ph = 0;
                        for (i = 0; i < 5; i = i + 1) {
                                if (pk_hrf[i]  != 0) pk = pk_ch[i];
                                if (pk_phrf[i] != 0) ph = pk_ph[i];
                        }
                        if ((short) ph % 13 < (short) pk % 13) {
                                pk_dslot = 0;
                        } else if ((short) pk % 13 == (short) ph % 13) {
                                for (i = 4; i >= 0; i = i - 1) {
                                        if (pk_phsf[i] % 13 <
                                            pk_hsf[i]  % 13) {
                                                pk_dslot = 0; break;
                                        }
                                        if (pk_hsf[i]  % 13 <
                                            pk_phsf[i] % 13) {
                                                pk_dslot = 1; break;
                                        }
                                }
                        }
                }

                /* High card: pure kicker ladder from top down. */
                if (pk_chrk == 0) {
                        for (i = 4; i >= 0; i = i - 1) {
                                if (pk_phsf[i] % 13 <
                                    pk_hsf[i]  % 13) {
                                        pk_dslot = 0; break;
                                }
                                if (pk_hsf[i]  % 13 <
                                    pk_phsf[i] % 13) {
                                        pk_dslot = 1; break;
                                }
                        }
                }
        }

        pk_round = 1;

        if (pk_dslot == 0) {
                pk_pmsg("I win!!!");
                for (br = 0; (short) br < 10; br = br + 1) {
                        gameTick(2);
                        if ((br & 1) == 0) {
                                for (j = 0; j < 5; j = j + 1)
                                        pk_drcs(pk_ch[j], j, 0);
                        } else {
                                for (j = 0; j < 5; j = j + 1)
                                        pk_drcs(CARD_HIGHLIGHT, j, 0);
                        }
                }
                for (j = 0; j < 5; j = j + 1)
                        pk_drcs(pk_ch[j], j, 0);
                pk_annr(0);
        }
        if (pk_dslot == 1) {
                pk_pmsg("You're so lucky!!!");
                for (br = 0; (short) br < 10; br = br + 1) {
                        gameTick(2);
                        if ((br & 1) == 0) {
                                for (j = 0; j < 5; j = j + 1)
                                        pk_drcs(pk_ph[j], j, 1);
                        } else {
                                for (j = 0; j < 5; j = j + 1)
                                        pk_drcs(CARD_HIGHLIGHT, j, 1);
                        }
                }
                for (j = 0; j < 5; j = j + 1)
                        pk_drcs(pk_ph[j], j, 1);
                pk_annr(1);
        }
}

/* pk_main_body: 5-card draw poker main loop.  Full 819-instruction
   port of poker_main.  Structure:
     - init (Malloc, load cards, mg_stp, money=400 each, initial
       display)
     - per-round loop:
        1. plEr; pk_ante()
        2. pk_evhs() -- deal
        3. pk_cbet("Do you feel lucky today?") -- first bet round
        4. On computer-pass: nothing (unless bluff triggers below).
           Otherwise: computer sees, drains player bet chip-by-chip.
        5. Player discard phase: F1 Draw (with 1..5 to toggle cards)
           / F3 Stay.  Highlighted card = pk_sel[i]=1, blank slot.
        6. pk_cdrw() -- computer AI draws
        7. pk_cbet("Want to make a bet?") -- final bet round
        8. On computer pass with weak hand: call and go to showdown.
           Otherwise: pk_cace() to decide if it opens; if it folds,
           award pot to player.  If it opens, use pk_dbet to pick
           call vs raise.  On raise: patch pk_rm, chip transfer,
           player F1 See / F3 Fold, then F1 Raise / F3 Enter / F5
           Call loop.
        9. pk_show() -- showdown reveals + hand comparison ladder
       10. tick(0x18), loop.
   All Ghidra gotos (LAB_00018da0 = cleanup, LAB_00018d72 = tick-
   before-next-round, LAB_00019082 = keep-polling-during-discard,
   LAB_00019514 / LAB_00019950 = final-bet raise loop) preserved
   verbatim as gotos to match the shape of the 1985 source.
   addr: pk_main() (== poker_main) */

void
pk_main()
{
        short   ikey;
        BOOL16  in_use;
        short   dcount;
        short   card;
        short   i;
        short   loc8;
        short   raise_amt;
        short   res;

        crd_dat = (short *) Malloc(10400L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCrd();
        mg_stp();

        pk_round = 0;
        pk_quit  = NO;
        g_pcmon  = 400;
        g_ppmon  = 400;
        g_ppppa  = 0;
        pk_awp();
        pk_dppm();
        pk_dpot();

        for (;;) {
                plEr(70, 10, 219, 62);
                pk_ante();
                if (pk_quit == YES)
                        goto cleanup;
                pk_evhs();

                ikey = pk_cbet("Do you feel lucky today?");
                if (ikey == -1) {
                        if (mg_tofl == NO) {
                                pk_pmsg("Sorry, you're all out!");
                                gameTick(10);
                        }
                        goto cleanup;
                }
                if (pk_pass == NO) {
                        pk_pmsg("I'll see your bet.");
                        while (ikey = pk_bet - 1,
                               in_use = (pk_bet != 0),
                               pk_bet = ikey, in_use != NO) {
                                if (g_pcmon == 0) {
                                        pk_pmsg("Sorry, I'm all out!");
                                        gameTick(10);
                                        goto cleanup;
                                }
                                g_pcmon = g_pcmon - 1;
                                pk_awp();
                                g_ppppa = g_ppppa + 1;
                                pk_dpot();
                                gameTick(0);
                        }
                } else {
                        pk_pmsg("That's all right with me.");
                        gameTick(10);
                }
                gameTick(0x10);

                pk_disc = 0;
                pk_pmsg("Do you want any cards?");
                plEr(225, 10, 319, 60);
                strPr("F1 Draw", 225, 18, COLOR_red);
                strPr("F3 Stay", 225, 26, COLOR_red);
                for (i = 0; i < 5; i = i + 1)
                        pk_sel[i] = 0;

discard_loop:
                do {
                        ikey = pk_inph(KEY_F1, KEY_F3, 0);
                        if (ikey != 2 && ikey != -1) {
                                for (i = 0;
                                     i < 5 && pk_sel[i] != 1;
                                     i = i + 1) ;
                                if (i == 5)
                                        strPr("F3 Stay", 225, 26, COLOR_red);
                                else
                                        strPr("F3 Stay", 225, 26, COLOR_lt_grey);
                                if (ikey != 1) {
                                        if (ikey > 3 && ikey < 9) {
                                                if (pk_sel[ikey - 4] == 0) {
                                                        pk_sel[ikey - 4] = 1;
                                                        pk_drcs(CARD_HIGHLIGHT, ikey - 4, 1);
                                                } else {
                                                        pk_sel[ikey - 4] = 0;
                                                        pk_drcs(pk_ph[ikey - 4], ikey - 4, 1);
                                                }
                                                for (i = 0;
                                                     i < 5 && pk_sel[i] != 1;
                                                     i = i + 1) ;
                                                if (i == 5)
                                                        strPr("F3 Stay", 225, 26, COLOR_red);
                                                else
                                                        strPr("F3 Stay", 225, 26, COLOR_lt_grey);
                                        }
                                        goto discard_loop;
                                }
                                for (i = 0;
                                     i < 5 && pk_sel[i] != 1;
                                     i = i + 1) ;
                                if (i == 5) goto discard_loop;
                        }
                        if (mg_tofl != NO) goto cleanup;
                        if (ikey == 1) {
                                for (i = 0; i < 5; i = i + 1) {
                                        if (pk_sel[i] != 1) continue;
                                        in_use = YES;
                                        while (in_use != NO) {
                                                card = rndRng(0, 51);
                                                in_use = NO;
                                                for (dcount = 0; dcount < 5; dcount = dcount + 1) {
                                                        if (pk_ch[dcount] == card)
                                                                in_use = YES;
                                                        if (pk_ph[dcount] == card)
                                                                in_use = YES;
                                                }
                                                dcount = pk_disc;
                                                while (res = dcount - 1,
                                                       dcount != 0) {
                                                        dcount = res;
                                                        if (pk_dpile[res] == card)
                                                                in_use = YES;
                                                }
                                        }
                                        pk_dpile[pk_disc] = pk_ph[i];
                                        pk_disc = pk_disc + 1;
                                        pk_ph[i] = card;
                                        pk_drcs(card, i, 1);
                                        gameTick(3);
                                }
                        }
                        if (ikey != 2) break;
                        for (i = 0;
                             i < 5 && pk_sel[i] != 1;
                             i = i + 1) ;
                } while (i != 5);

                pk_cdrw();
                ikey = pk_cbet("Want to make a bet?");
                if (ikey == -1) {
                        if (mg_tofl == NO) {
                                pk_pmsg("Sorry, you're all out!");
                                gameTick(10);
                        }
                        goto cleanup;
                }
                if (pk_pass == NO) {
                        ikey = pk_cace();
                        if (ikey == -1) {
                                pk_pmsg("I feel unlucky. I fold.");
                                gameTick(8);
                                pk_pmsg("Your pot.");
                                pk_annr(1);
                        } else {
                                if (g_pcmon < pk_bet) {
                                        pk_pmsg("Sorry, I'm all out!");
                                        gameTick(10);
                                        goto cleanup;
                                }
                                pk_pmsg("Ok. I'll see your bet.");
                                i = pk_bet;
                                pk_bet = 0;
                                while (i != 0) {
                                        pk_ddec(0, 1);
                                        gameTick(0);
                                        i = i - 1;
                                }
                                ikey = pk_dbet();
                                if (ikey == 'c') {
                                        pk_pmsg("I'll call.");
                                        gameTick(8);
                                        pk_show();
                                } else {
                                        pk_rm[11] = (char)((int) pk_dpos / 10) + '0';
                                        if (pk_rm[11] == '0')
                                                pk_rm[11] = ' ';
                                        pk_rm[12] = (char)((int) pk_dpos % 10) + '0';
                                        pk_pmsg(pk_rm);
                                        raise_amt = pk_dpos;
                                        pk_bet    = 0;
                                        while (raise_amt != 0) {
                                                pk_ddec(0, 1);
                                                gameTick(0);
                                                raise_amt = raise_amt - 1;
                                        }
                                        gameTick(8);
                                        pk_pmsg("You think I'm bluffin'?");
                                        pk_phv = pk_dpos;
                                        plEr(225, 10, 319, 60);
                                        strPr("F1 See",  225, 18, COLOR_red);
                                        strPr("F3 Fold", 225, 34, COLOR_red);
                                        ikey = pk_inph(KEY_F1, 0, KEY_F3);
                                        if (ikey == -1) goto cleanup;
                                        if (ikey == 3) {
                                                plEr(225, 10, 319, 60);
                                                pk_pmsg("My pot.");
                                                gameTick(8);
                                                pk_annr(0);
                                        } else {
                                                if (ikey != 1) return;
                                                loc8   = pk_phv;
                                                pk_bet = 0;
                                                while (loc8 != 0) {
                                                        pk_ddec(1, 1);
                                                        gameTick(0);
                                                        loc8 = loc8 - 1;
                                                }
                                                if (g_ppmon == 0) {
                                                        pk_pmsg("Sorry, you're all out!");
                                                        gameTick(10);
                                                        goto cleanup;
                                                }
                                                plEr(225, 10, 319, 60);
                                                plEr(5, 63, 319, 75);
                                                strPr("F1 Raise", 225, 18, COLOR_red);
                                                strPr("F3 Enter", 225, 26, COLOR_red);
                                                strPr("F5 Call",  225, 34, COLOR_red);
                                                do {
                                                        ikey = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                                                        if (ikey == -1) goto raiseloop2;
                                                        if (ikey == 3) {
                                                                pk_show();
                                                                goto endround;
                                                        }
                                                } while (ikey != 1 || g_ppmon == 0);
                                                pk_bet   = 0;
                                                pk_dpos  = 0;
                                                pk_ddec(1, 1);
                                                pk_dpos  = pk_dpos + 1;
raiseloop2:
                                                if (mg_tofl != NO) goto cleanup;
                                                while ((ikey = pk_inph(KEY_F1, KEY_F3, KEY_F5),
                                                        ikey != -1 && ikey != 2)) {
                                                        if (ikey == 1) {
                                                                pk_ddec(1, 1);
                                                                if (g_ppmon != 0)
                                                                        pk_dpos = pk_dpos + 1;
                                                        }
                                                }
                                                if (mg_tofl != NO) goto cleanup;
                                                if (g_pcmon < pk_dpos) {
                                                        pk_pmsg("Sorry, I'm all out.");
                                                        gameTick(10);
                                                        goto cleanup;
                                                }
                                                pk_pmsg("Ok. I'll see your bet.");
                                                loc8   = pk_dpos;
                                                pk_bet = 0;
                                                while (loc8 != 0) {
                                                        pk_ddec(0, 1);
                                                        gameTick(0);
                                                        loc8 = loc8 - 1;
                                                }
                                                gameTick(5);
                                                pk_pmsg("I'll call.");
                                                gameTick(8);
                                                pk_show();
                                        }
                                }
                        }
                } else if (pk_bluff == NO && pk_chrk == 0) {
                        pk_pmsg("Ok, I'll call.");
                        gameTick(10);
                        pk_show();
                } else {
                        i = rndRng(5, 15);
                        if (g_pcmon < i)
                                i = g_pcmon;
                        pk_bm[9] = (char)((int) i / 10) + '0';
                        if (pk_bm[9] == '0')
                                pk_bm[9] = ' ';
                        pk_bm[10] = (char)((int) i % 10) + '0';
                        pk_pmsg(pk_bm);
                        pk_bet = 0;
                        while (i != 0) {
                                pk_ddec(0, 1);
                                gameTick(0);
                                i = i - 1;
                        }
                        gameTick(10);
                        pk_pmsg("Will you see my bet?");
                        pk_phv = pk_bet;
                        plEr(225, 10, 319, 60);
                        strPr("F1 See",  225, 18, COLOR_red);
                        strPr("F3 Fold", 225, 34, COLOR_red);
                        ikey = pk_inph(KEY_F1, 0, KEY_F3);
                        if (ikey == -1) goto cleanup;
                        if (ikey == 3) {
                                plEr(225, 10, 319, 60);
                                pk_pmsg("My pot.");
                                gameTick(8);
                                pk_annr(0);
                        } else {
                                if (ikey != 1) return;
                                loc8   = pk_phv;
                                pk_bet = 0;
                                while (loc8 != 0) {
                                        pk_ddec(1, 1);
                                        gameTick(0);
                                        loc8 = loc8 - 1;
                                }
                                if (g_ppmon == 0) {
                                        pk_pmsg("Sorry, you're all out!");
                                        gameTick(10);
                                        goto cleanup;
                                }
                                plEr(225, 10, 319, 60);
                                plEr(5, 63, 319, 75);
                                strPr("F1 Raise", 225, 18, COLOR_red);
                                strPr("F3 Enter", 225, 26, COLOR_red);
                                strPr("F5 Call",  225, 34, COLOR_red);
                                pk_dpos = 0;
                                do {
                                        ikey = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                                        if (ikey == -1) goto raiseloop1;
                                        if (ikey == 3) {
                                                pk_show();
                                                goto endround;
                                        }
                                } while (ikey != 1 || g_ppmon == 0);
                                pk_bet   = 0;
                                pk_ddec(1, 1);
                                pk_dpos  = pk_dpos + 1;
raiseloop1:
                                if (mg_tofl != NO) goto cleanup;
                                while ((ikey = pk_inph(KEY_F1, KEY_F3, KEY_F5),
                                        ikey != -1 && ikey != 2)) {
                                        if (ikey == 1) {
                                                pk_ddec(1, 1);
                                                if (g_ppmon != 0)
                                                        pk_dpos = pk_dpos + 1;
                                        }
                                }
                                if (mg_tofl != NO) goto cleanup;
                                if (g_pcmon < pk_dpos) {
                                        pk_pmsg("Sorry, I,m all out.");
                                        gameTick(10);
                                        goto cleanup;
                                }
                                pk_pmsg("Ok. I'll see your bet.");
                                gameTick(8);
                                loc8   = pk_dpos;
                                pk_bet = 0;
                                while (loc8 != 0) {
                                        pk_ddec(0, 1);
                                        gameTick(0);
                                        loc8 = loc8 - 1;
                                }
                                gameTick(5);
                                pk_pmsg("I'll call.");
                                gameTick(8);
                                pk_show();
                        }
                }

endround:
                gameTick(0x18);
        }

cleanup:
        tx_sctm  = 0;
        no_keyin = NO;
        Mfree(crd_dat);
        moff();
}

/* ---- Blackjack helpers --------------------------------------------- */

extern short    pk_psh[];
extern short    pk_pcc;
extern short    pk_ccc;
extern short    pk_pscc;
extern short    pk_wpr;
extern BOOL16   pk_wrf;
extern BOOL16   pk_wcs;
extern BOOL16   pk_c1bj;
extern BOOL16   pk_c2bj;
extern BOOL16   pk_bs1;
extern BOOL16   pk_bs2;
extern short    pk_cscore;
extern short    pk_pscore;
extern short    g_pcbet;
extern short    g_ppbet;
extern short    pk_phase;

/* pk_dbhi: display bet with highlight.  Selector 1 -> render
   pk_bet_computer at (31, 51); anything else -> pk_bet_player.
   Same fixed-buffer 3-digit format as pk_awp/dppm/dpot.
   addr: poker_display_bet_with_highlight() */

static void
pk_dbhi(sel)
short   sel;
{
        short   val;
        char    str[12];

        val = (sel == 1) ? g_pcbet : g_ppbet;
        plEr(31, 43, 57, 53);
        str[3] = '\0';
        str[8] = (char)((int) val / 100);
        str[0] = str[8] + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (char)((int)(val % 100) / 10);
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = (char)((int)(val % 100) % 10);
        str[2] = str[4] + '0';
        strPr(str, 31, 51, COLOR_black);
}

/* pk_chsc: blackjack card value.  ace_mode=0 forces all aces to 1;
   ace_mode=1 counts one ace as 11 (soft hand), rest as 1.  8..Q
   (rank 6..11 pre-Ace) all score 10.  Rank 0..5 (2..7) score
   rank+2.  Rank 12 = Ace.  Called twice per turn with mode 0 then
   mode 1 to pick the better score without busting.
   addr: poker_calculate_hand_score() */

static short
pk_chsc(hand, ace_mode)
short * hand;
short   ace_mode;
{
        short   score;
        short   i;
        BOOL16  ace_high;

        ace_high = NO;
        score    = 0;
        for (i = 0; i < 5 && hand[i] != CARD_NONE; i = i + 1) {
                if ((short) hand[i] % 13 == 12) {
                        if (ace_mode == 0)
                                score = score + 1;
                        else if (ace_high != NO)
                                score = score + 1;
                        else {
                                score    = score + 11;
                                ace_high = YES;
                        }
                } else if ((short) hand[i] % 13 < 12 &&
                                    7 < (short) hand[i] % 13) {
                        score = score + 10;
                } else {
                        score = (short) hand[i] % 13 + 2 + score;
                }
        }
        return score;
}

/* pk_dchd: deal one card into `hand` at the next CARD_NONE slot.
   Rejects duplicates against BOTH poker hands AND the split hand.
   `face_down`=0 -> face-up, else face-down (CARD_BACK) sprite.
   Returns -1 if the hand is already full (all 5 slots used).
   addr: poker_deal_card_to_hand() */

static short
pk_dchd(hand, face_down)
short * hand;
short   face_down;
{
        BOOL16                  dup;
        unsigned short  row;
        short                   card;
        short                   j;
        short                   i;

        for (i = 0; i < 5 && hand[i] != CARD_NONE; i = i + 1) ;
        if (i == 5)
                return -1;

        dup = YES;
        while (dup != NO) {
                card = rndRng(0, 51);
                dup  = NO;
                for (j = 0; j < 5; j = j + 1) {
                        if (pk_ch[j]  == card) dup = YES;
                        if (pk_ph[j]  == card) dup = YES;
                        if (pk_psh[j] == card) dup = YES;
                }
        }
        hand[i] = card;
        row = (hand != pk_ch) ? 1 : 0;
        if (face_down == 0)
                pk_drcs(card, i, row);
        else
                pk_drcs(CARD_BACK, i, row);
        gameTick(3);
        return 0;
}

/* pk_cnbj: check for natural blackjack (an Ace + a Ten/J/Q/K in
   the initial two-card hand).  Returns 1 if hand[0]/hand[1] form
   a natural, 0 otherwise.
   addr: poker_check_natural_blackjack() */

static short
pk_cnbj(hand)
short * hand;
{
        short   r0;
        short   r1;

        r0 = (short) hand[0] % 13;
        r1 = (short) hand[1] % 13;
        if (r0 == 12 && r1 < 12 && r1 > 7)
                return 1;
        if (r1 == 12 && r0 < 12 && r0 > 7)
                return 1;
        return 0;
}

/* pk_sbet: settle a bet.  winner=0 -> computer keeps bet; winner=1
   -> player wins bet back.  `mode` (third arg, stack slot 0xa in
   Ghidra) tweaks payoff:
     mode == 0 -> normal (single transfer)
     mode == 1 -> player-side double-collect (natural blackjack
                     bonus: computer also pays out its side)
     mode == 2 -> split-hand -- suppress the second (player)
                     transfer
   Sets pk_quit if someone runs out mid-transfer.
   addr: poker_settle_bet() */

static void
pk_sbet(bet_ptr, winner, mode)
short * bet_ptr;
short   winner;
short   mode;
{
        short   loc8;
        short   orig;
        short   cur;

        if (winner == 0) {
                orig = *bet_ptr;
                while (cur = *bet_ptr,
                       *bet_ptr = *bet_ptr - 1,
                       cur != 0) {
                        g_pcmon = g_pcmon + 1;
                        pk_awp();
                        if (bet_ptr == &g_pcbet)
                                pk_dbhi(1);
                        else
                                pk_dbhi(2);
                        gameTick(0);
                }
                if (mode != 0) {
                        while (orig != 0) {
                                if (g_ppmon == 0) {
                                        pk_quit = YES;
                                        return;
                                }
                                g_ppmon = g_ppmon - 1;
                                pk_dppm();
                                g_pcmon = g_pcmon + 1;
                                pk_awp();
                                gameTick(0);
                                orig = orig - 1;
                        }
                }
        } else if (winner == 1) {
                orig = *bet_ptr;
                loc8 = *bet_ptr;
                while (cur = *bet_ptr,
                       *bet_ptr = *bet_ptr - 1,
                       cur != 0) {
                        g_ppmon = g_ppmon + 1;
                        pk_dppm();
                        if (bet_ptr == &g_pcbet)
                                pk_dbhi(1);
                        else
                                pk_dbhi(2);
                        gameTick(0);
                }
                if (mode != 2) {
                        while (loc8 != 0) {
                                if (g_pcmon == 0) {
                                        pk_quit = YES;
                                        break;
                                }
                                g_pcmon = g_pcmon - 1;
                                pk_awp();
                                g_ppmon = g_ppmon + 1;
                                pk_dppm();
                                gameTick(0);
                                loc8 = loc8 - 1;
                        }
                }
                if (mode == 1) {
                        while (orig != 0) {
                                if (g_pcmon == 0) {
                                        pk_quit = YES;
                                        return;
                                }
                                g_pcmon = g_pcmon - 1;
                                pk_awp();
                                g_ppmon = g_ppmon + 1;
                                pk_dppm();
                                gameTick(0);
                                orig = orig - 1;
                        }
                }
        }
}

/* pk_bjr: play one blackjack round for `hand` at `row`.  If the
   caller is playing under the double-down/split forced-single-hit
   modes (pk_wrf for the primary hand, pk_wcs for the split), auto-
   deals one final card and returns.  Otherwise prompts with the
   `prompt` string + F1 Hit / F3 Stand.  Loops F1 hits until either
   the player stands (F3), busts (>21), or exhausts the 3-hit
   allowance.  Returns 0 on stand, -1 on bust or timeout.
   addr: poker_blackjack_round() */

static short
pk_bjr(hand, row, prompt)
short * hand;
short   row;
char *  prompt;
{
        short   res;
        short * cnt_ptr;
        short   score;
        short   i;
        short   j;
        BOOL16  forced;

        cnt_ptr = &pk_pcc;
        if (hand == pk_ph)  cnt_ptr = &pk_pcc;
        if (hand == pk_psh) cnt_ptr = &pk_pscc;
        if (hand == pk_ch)  cnt_ptr = &pk_ccc;

        plEr(225, 10, 319, 60);
        forced = NO;
        if ((hand == pk_ph  && pk_wrf != NO) ||
            (hand == pk_psh && pk_wcs != NO))
                forced = YES;
        else {
                strPr("F1 Hit",   225, 18, COLOR_red);
                strPr("F3 Stand", 225, 26, COLOR_red);
        }
        for (i = 0; hand[i] != CARD_NONE; i = i + 1)
                pk_drcs(hand[i], i, row);

        if (forced != NO) {
                pk_pmsg("Here's your card.");
                gameTick(0x10);
                *cnt_ptr = *cnt_ptr - CARD_BJ_STEP;
                pk_dchd(hand, 0);
                score = 0;
                for (j = 0; j < 5 && hand[j] != CARD_NONE;
                     j = j + 1) {
                        if ((short) hand[j] % 13 == 12)
                                score = score + 1;
                        else if ((short) hand[j] % 13 < 12 &&
                                 (short) hand[j] % 13 > 7)
                                score = score + 10;
                        else
                                score = (short) hand[j] % 13 + 2 + score;
                }
                return (score < 22) ? 0 : -1;
        }

        pk_pmsg(prompt);
        do {
                res = 0;
                while (res != 1 && res != 2 && res != -1) {
                        gameTick(0);
                        res = pk_inph(KEY_F1, KEY_F3, 0);
                }
                if (mg_tofl != NO)
                        return -1;
                if (res == 2) {
                        pk_pmsg("OK, you stand.");
                        return 0;
                }
                if (res == 1) {
                        *cnt_ptr = *cnt_ptr - CARD_BJ_STEP;
                        pk_dchd(hand, 0);
                        score = 0;
                        for (j = 0; j < 5 && hand[j] != CARD_NONE;
                             j = j + 1) {
                                if ((short) hand[j] % 13 == 12)
                                        score = score + 1;
                                else if ((short) hand[j] % 13 < 12 &&
                                         (short) hand[j] % 13 > 7)
                                        score = score + 10;
                                else
                                        score = (short) hand[j] % 13
                                                            + 2 + score;
                        }
                        if (21 < score)
                                return -1;
                }
        } while (*cnt_ptr != CARD_BJ_STOP);
        pk_pmsg("You cannot take any more cards.");
        gameTick(0xf);
        return 0;
}

/* pk_bjMn: BLACKJACK main game loop.  Full 1270-instruction port of
   poker_blackjack_main.  Flow:
     1. Malloc 0x28a0 card buffer + pk_ldCrd + mg_stp + money 400 each
     2. Per-round loop (Ghidra LAB_0001bcc8):
        a. plEr card + bet displays; init bets = 0; F1 Bet / F10 Quit
        b. Bet-entry loop: F1 to add chip (up to 20 cap), F3 to enter,
           F5 to clear.  Player quit -> game over.
        c. Deal 2 cards each (player face-up, computer[1] face-down)
        d. Check pk_cnbj on each hand -> natural blackjack early payoff
        e. On matching first two cards, "Do you wish to split?"
           F1 -> split path: 2nd hand initialised, player must match
           first bet, second natural also possible
        f. Double-down prompt (F1) doubles bet on first (and split)
           hand.  Wraps pk_pcc/pscc to CARD_BJ_STEP so only one hit.
        g. pk_bjr for each hand -> hit/stand loop
        h. Reveal computer's face-down card, hit up to 3 times using
           two-value picker (score-with-no-Ace vs score-with-Ace-11)
        i. Compare final scores, pk_sbet transfers.
     3. Cleanup on quit (Ghidra LAB_0001bd9e): tx_sctm=0, no_keyin=NO,
        Mfree, moff().
   Preserves every Ghidra goto (LAB_0001bcbe, LAB_0001bd9e, LAB_0001beb6)
   verbatim so the port stays byte-comparable with the 1985 asm.

   Deliberate naming detail: this exports as pk_bjMn (the existing
   symbol) rather than a wrapper-plus-body pair, because Alcyon's
   8-char external-symbol truncation would map both `pk_bjMn` and
   `pk_bjMn_body` to the same `_pk_bjMn` link name.

   addr: pk_bjMn() (== poker_blackjack_main) */

void
pk_bjMn()
{
        short   res;
        short   rv;
        BOOL16  game_over;
        short   round_ctr;
        short   br;
        short   ikey;
        short   i;

        crd_dat = (short *) Malloc(0x28a0L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCrd();
        mg_stp();
        g_pcmon = 400;
        g_ppmon = 400;
        pk_awp();
        pk_dppm();

        for (;;) {
                plEr(70, 10, 219, 62);
                plEr(31, 43, 57, 53);
                g_pcbet = 0;
                g_ppbet = 0;
                pk_phase = 0;
                plEr(225, 10, 319, 60);
                strPr("F1  Bet",  225, 18, COLOR_red);
                strPr("F10 Quit", 225, 34, COLOR_red);
                pk_pmsg("What's your bet?");
                pk_quit = NO;
                ikey    = 0;
                while (ikey != 1 && ikey != 3)
                        ikey = pk_inph(KEY_F1, 0, KEY_F10);
                if (ikey == 3) goto cleanup;

                for (br = 0; br < 5; br = br + 1) {
                        pk_ch[br]  = CARD_NONE;
                        pk_ph[br]  = CARD_NONE;
                        pk_psh[br] = CARD_NONE;
                }
                plEr(70, 10, 219, 62);
                if (g_ppmon == 0) {
                        pk_pmsg("Game's over. I win.");
                        gameTick(0x14);
                        goto cleanup;
                }
                g_ppmon = g_ppmon - 1;
                pk_dppm();
                g_pcbet = g_pcbet + 1;
                pk_dbhi(1);
                pk_bet = 1;
                strPr("F3  Enter", 225, 26, COLOR_red);
                strPr("F10 Quit",  225, 34, COLOR_lt_grey);
                strPr("F5  Clear", 225, 34, COLOR_red);

bet_loop:
                do {
                        ikey = 0;
                        while (ikey != 1 && ikey != 2 &&
                                 ikey != 3 && ikey != -1) {
                                gameTick(0);
                                ikey = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                        }
                        if (mg_tofl != NO) goto cleanup;
                        if (ikey == 3) {
                                g_ppmon = g_ppmon + pk_bet;
                                pk_bet  = 0;
                                g_pcbet = 0;
                                pk_dbhi(1);
                                pk_dppm();
                                pk_dpile[10] = CARD_BJ_STEP;
                                break;
                        }
                        if (ikey == 1) {
                                if (g_ppmon == 0) {
                                        pk_pmsg("Game's over. I win.");
                                        pk_quit = YES;
                                        break;
                                }
                                if (pk_bet == 20) goto bet_loop;
                                g_ppmon = g_ppmon - 1;
                                pk_dppm();
                                g_pcbet = g_pcbet + 1;
                                pk_dbhi(1);
                                pk_bet  = pk_bet + 1;
                        }
                } while (ikey != 2);

                if (pk_dpile[10] == CARD_BJ_STOP) {
                        if (pk_quit != NO) {
                                gameTick(20);
                                goto cleanup;
                        }
                        pk_pmsg(" ");
                        plEr(225, 10, 319, 60);
                        pk_dchd(pk_ph, 0);
                        pk_dchd(pk_ch, 1);
                        pk_dchd(pk_ph, 0);
                        pk_dchd(pk_ch, 0);
                        gameTick(10);
                        res = pk_cnbj(pk_ph);
                        rv  = pk_cnbj(pk_ch);
                        if (res != 0 && rv != 0) {
                                pk_pmsg("You have BLACKJACK...but so do I !!");
                                pk_drcs(pk_ch[0], 0, 0);
                                gameTick(0x14);
                                pk_sbet(&g_pcbet, 1, 0);
                                if (pk_quit != NO) {
                                        pk_pmsg("Game's over. I win.");
                                        gameTick(0x14);
                                        goto cleanup;
                                }
                        } else if (res != 0) {
                                pk_pmsg("You have BLACKJACK!!");
                                gameTick(0x14);
                                pk_sbet(&g_pcbet, 1, 0);
                                if (pk_quit != NO) {
                                        pk_pmsg("I'm all out!!");
                                        gameTick(0x14);
                                        goto cleanup;
                                }
                        } else if (rv != 0) {
                                pk_pmsg("I have BLACKJACK!!");
                                gameTick(10);
                                pk_drcs(pk_ch[0], 0, 0);
                                gameTick(0x14);
                                pk_pmsg("I win double the bet.");
                                gameTick(0x14);
                                pk_sbet(&g_pcbet, 0, 0);
                                if (pk_quit != NO) {
                                        pk_pmsg("Game's over. I win.");
                                        gameTick(0x14);
                                        goto cleanup;
                                }
                        } else {
                                /* Neither had a natural.  Split, double-down,
                                   hit/stand, dealer -- the meat of the game. */
                                pk_phase = 0;
                                if ((short) pk_ph[0] % 13 ==
                                    (short) pk_ph[1] % 13) {
                                        pk_pmsg("Do you wish to split?");
                                        plEr(225, 10, 319, 60);
                                        strPr("F1 Split",    225, 18, COLOR_red);
                                        strPr("F3 No split", 225, 26, COLOR_red);
                                        ikey = 0;
                                        while (ikey != 1 && ikey != 2 && ikey != -1) {
                                                gameTick(0);
                                                ikey = pk_inph(KEY_F1, KEY_F3, 0);
                                        }
                                        if (mg_tofl != NO) goto cleanup;
                                        if (ikey == 1) {
                                                pk_phase = 1;
                                                pk_psh[0] = pk_ph[1];
                                                pk_ph[1]  = CARD_NONE;
                                                pk_pmsg("Here is your first hand.");
                                                pk_wpr = g_pcbet;
                                                pk_drcs(CARD_HIGHLIGHT, 1, 1);
                                                gameTick(8);
                                                pk_dchd(pk_ph, 0);
                                                pk_c1bj = NO;
                                                pk_c2bj = NO;
                                                res = pk_cnbj(pk_ph);
                                                if (res != 0) {
                                                        pk_pmsg("You have BLACKJACK!!");
                                                        gameTick(0x14);
                                                        pk_sbet(&g_pcbet, 1, 0);
                                                        if (pk_quit != NO) {
                                                                pk_pmsg("I'm all out!!");
                                                                gameTick(20);
                                                                goto cleanup;
                                                        }
                                                        pk_c1bj = YES;
                                                }
                                                gameTick(20);
                                                pk_drcs(CARD_HIGHLIGHT, 0, 1);
                                                pk_drcs(CARD_HIGHLIGHT, 1, 1);
                                                g_ppbet = 0;
                                                pk_dbhi(2);
                                                pk_pmsg("Here is your second hand.");
                                                pk_drcs(pk_psh[0], 0, 1);
                                                gameTick(10);
                                                pk_dchd(pk_psh, 0);
                                                while (g_ppbet != pk_wpr) {
                                                        if (g_ppmon == 0) {
                                                                pk_quit = YES;
                                                                break;
                                                        }
                                                        g_ppmon = g_ppmon - 1;
                                                        pk_dppm();
                                                        g_ppbet = g_ppbet + 1;
                                                        pk_dbhi(2);
                                                        gameTick(0);
                                                }
                                                if (pk_quit != NO) {
                                                        pk_pmsg("Sorry, you're all out!!");
                                                        gameTick(20);
                                                        goto cleanup;
                                                }
                                                res = pk_cnbj(pk_psh);
                                                if (res != 0) {
                                                        pk_pmsg("You have BLACKJACK!!");
                                                        gameTick(20);
                                                        pk_sbet(&g_ppbet, 1, 0);
                                                        if (pk_quit != NO) {
                                                                pk_pmsg("I'm all out!!");
                                                                gameTick(0x14);
                                                                goto cleanup;
                                                        }
                                                        pk_c2bj = YES;
                                                }
                                                gameTick(0x14);
                                        }
                                }

                                /* Double-down / hit-loop phase. */
                                if (pk_phase == 0 ||
                                    pk_c1bj == NO || pk_c2bj == NO) {
                                        pk_wrf = NO;
                                        pk_wcs = NO;
                                        pk_pcc  = CARD_BJ_MAX;
                                        pk_pscc = CARD_BJ_MAX;
                                        plEr(225, 10, 319, 60);
                                        strPr("F1 Double",    225, 18, COLOR_red);
                                        strPr("F3 No double", 225, 26, COLOR_red);
                                        if (pk_phase == 0) {
                                                if (g_ppmon < g_pcbet) ikey = 2;
                                                else {
                                                        pk_pmsg("Do you wish to double-down?");
                                                        ikey = 0;
                                                }
                                                while (ikey != 1 && ikey != 2 && ikey != -1) {
                                                        gameTick(0);
                                                        ikey = pk_inph(KEY_F1, KEY_F3, 0);
                                                }
                                                if (mg_tofl != NO) goto cleanup;
                                                if (ikey == 1) {
                                                        pk_pcc = CARD_BJ_STEP;
                                                        i      = g_pcbet;
                                                        pk_wrf = YES;
                                                        while (i != 0) {
                                                                if (g_ppmon == 0) {
                                                                        pk_quit = YES;
                                                                        break;
                                                                }
                                                                g_ppmon = g_ppmon - 1;
                                                                pk_dppm();
                                                                g_pcbet = g_pcbet + 1;
                                                                pk_dbhi(1);
                                                                gameTick(0);
                                                                i = i - 1;
                                                        }
                                                        if (pk_quit != NO) {
                                                                pk_pmsg("Game's over. I win.");
                                                                gameTick(0x14);
                                                                goto cleanup;
                                                        }
                                                }
                                        } else {
                                                if (pk_c1bj == NO) {
                                                        if (g_ppmon < g_pcbet) ikey = 2;
                                                        else {
                                                                pk_pmsg("Double-down on your first hand?");
                                                                pk_drcs(pk_ph[0], 0, 1);
                                                                pk_drcs(pk_ph[1], 1, 1);
                                                                gameTick(0);
                                                                ikey = 0;
                                                        }
                                                        while (ikey != 1 && ikey != 2 && ikey != -1) {
                                                                gameTick(0);
                                                                ikey = pk_inph(KEY_F1, KEY_F3, 0);
                                                        }
                                                        if (mg_tofl != NO) goto cleanup;
                                                        if (ikey == 1) {
                                                                pk_pcc = CARD_BJ_STEP;
                                                                i      = g_pcbet;
                                                                pk_wrf = YES;
                                                                while (i != 0) {
                                                                        if (g_ppmon == 0) {
                                                                                pk_quit = YES;
                                                                                break;
                                                                        }
                                                                        g_ppmon = g_ppmon - 1;
                                                                        pk_dppm();
                                                                        g_pcbet = g_pcbet + 1;
                                                                        pk_dbhi(1);
                                                                        gameTick(0);
                                                                        i = i - 1;
                                                                }
                                                                if (pk_quit != NO) {
                                                                        pk_pmsg("Games over. I win.");
                                                                        gameTick(0x14);
                                                                        goto cleanup;
                                                                }
                                                        }
                                                }
                                                if (pk_c2bj == NO) {
                                                        gameTick(10);
                                                        if (g_ppmon < g_ppbet) ikey = 2;
                                                        else {
                                                                pk_pmsg("Double-down on your second hand?");
                                                                pk_drcs(pk_psh[0], 0, 1);
                                                                pk_drcs(pk_psh[1], 1, 1);
                                                                gameTick(0);
                                                                ikey = 0;
                                                        }
                                                        while (ikey != 1 && ikey != 2 && ikey != -1) {
                                                                gameTick(0);
                                                                ikey = pk_inph(KEY_F1, KEY_F3, 0);
                                                        }
                                                        if (mg_tofl != NO) goto cleanup;
                                                        if (ikey == 1) {
                                                                pk_pscc = CARD_BJ_STEP;
                                                                pk_wcs  = YES;
                                                                i       = g_ppbet;
                                                                while (i != 0) {
                                                                        if (g_ppmon == 0) {
                                                                                pk_quit = YES;
                                                                                break;
                                                                        }
                                                                        g_ppmon = g_ppmon - 1;
                                                                        pk_dppm();
                                                                        g_ppbet = g_ppbet + 1;
                                                                        pk_dbhi(2);
                                                                        gameTick(0);
                                                                        i = i - 1;
                                                                }
                                                                if (pk_quit != NO) {
                                                                        pk_pmsg("Game's over. I win.");
                                                                        gameTick(0x14);
                                                                        goto cleanup;
                                                                }
                                                        }
                                                }
                                        }

                                        /* Hit/stand rounds. */
                                        if (pk_phase == 0) {
                                                res = pk_bjr(pk_ph, 1, "Do you want a hit?");
                                                if (res == -1) {
                                                        if (mg_tofl == NO) {
                                                                pk_pmsg("You've busted!!!");
                                                                gameTick(10);
                                                                while (g_pcbet != 0) {
                                                                        g_pcmon = g_pcmon + 1;
                                                                        g_pcbet = g_pcbet - 1;
                                                                        pk_awp();
                                                                        pk_dbhi(1);
                                                                        gameTick(0);
                                                                }
                                                                g_pcbet = -1;
                                                                goto after_settle;
                                                        }
                                                        goto cleanup;
                                                }
                                                plEr(225, 10, 319, 60);
                                        } else {
                                                for (br = 0; br < 5; br = br + 1)
                                                        pk_drcs(CARD_HIGHLIGHT, br, 1);
                                                pk_bs1 = NO;
                                                pk_bs2 = NO;
                                                if (pk_c1bj == NO) {
                                                        for (br = 0;
                                                             br < 5 && pk_ph[br] != CARD_NONE;
                                                             br = br + 1)
                                                                pk_drcs(pk_ph[br], br, 1);
                                                        pk_dbhi(1);
                                                        res = pk_bjr(pk_ph, 1,
                                                                              "Need a hit on your first hand?");
                                                        if (res == -1) {
                                                                if (mg_tofl != NO) goto cleanup;
                                                                pk_pmsg("Your first hand is busted !!");
                                                                gameTick(20);
                                                                pk_bs1 = YES;
                                                                while (res = g_pcbet - 1,
                                                                       game_over = (g_pcbet != 0),
                                                                       g_pcbet = res, game_over != NO) {
                                                                        g_pcmon = g_pcmon + 1;
                                                                        pk_awp();
                                                                        pk_dbhi(1);
                                                                        gameTick(0);
                                                                }
                                                        }
                                                }
                                                if (pk_c2bj == NO) {
                                                        for (br = 0; br < 5; br = br + 1)
                                                                pk_drcs(CARD_HIGHLIGHT, br, 1);
                                                        for (br = 0;
                                                             br < 5 && pk_psh[br] != CARD_NONE;
                                                             br = br + 1)
                                                                pk_drcs(pk_psh[br], br, 1);
                                                        pk_dbhi(2);
                                                        res = pk_bjr(pk_psh, 1,
                                                                              "Need a hit on your second hand?");
                                                        if (res == -1) {
                                                                if (mg_tofl != NO) goto cleanup;
                                                                pk_pmsg("Your second hand is busted!!");
                                                                gameTick(0x14);
                                                                pk_bs2 = YES;
                                                                while (res = g_ppbet - 1,
                                                                       game_over = (g_ppbet != 0),
                                                                       g_ppbet = res, game_over != NO) {
                                                                        g_pcmon = g_pcmon + 1;
                                                                        pk_awp();
                                                                        pk_dbhi(2);
                                                                        gameTick(0);
                                                                }
                                                        }
                                                }
                                        }
                                        plEr(225, 10, 319, 60);

                                        /* Dealer turn + settle. */
                                        if (pk_phase == 0 ||
                                            (pk_bs1 == NO && pk_c1bj == NO) ||
                                            (pk_bs2 == NO && pk_c2bj == NO)) {
                                                if (pk_phase != 0 && pk_bs1 == NO && pk_c1bj == NO) {
                                                        /* nothing extra */
                                                } else if (pk_phase != 0 &&
                                                           pk_bs2 == NO && pk_c2bj == NO) {
                                                        for (br = 0; br < 5; br = br + 1)
                                                                pk_drcs(CARD_HIGHLIGHT, br, 1);
                                                        pk_pmsg("Here is your second hand.");
                                                        gameTick(0x14);
                                                        for (br = 0;
                                                             br < 5 && pk_psh[br] != CARD_NONE;
                                                             br = br + 1)
                                                                pk_drcs(pk_psh[br], br, 1);
                                                        pk_dbhi(2);
                                                } else if (pk_phase != 0) {
                                                        for (br = 0; br < 5; br = br + 1)
                                                                pk_drcs(CARD_HIGHLIGHT, br, 1);
                                                        pk_pmsg("Here is your first hand again.");
                                                        gameTick(0x14);
                                                        for (br = 0;
                                                             br < 5 && pk_ph[br] != CARD_NONE;
                                                             br = br + 1)
                                                                pk_drcs(pk_ph[br], br, 1);
                                                        pk_dbhi(1);
                                                }

                                                pk_pmsg("Now here's my down card.");
                                                gameTick(10);
                                                pk_drcs(pk_ch[0], 0, 0);
                                                gameTick(0x14);

                                                round_ctr = 0;
                                                for (br = 0; br < 3; br = br + 1) {
                                                        pk_cscore = 0;
                                                        round_ctr = 0;
                                                        res = pk_chsc(pk_ch, 0);
                                                        rv  = pk_chsc(pk_ch, 1);
                                                        if (0x15 < res && 0x15 < rv) {
                                                                round_ctr = 1;
                                                                break;
                                                        }
                                                        pk_cscore = res;
                                                        if (rv < 0x16)
                                                                pk_cscore = rv;
                                                        if (0x10 < pk_cscore) {
                                                                pk_pmsg("I'll stand.");
                                                                gameTick(0x14);
                                                                break;
                                                        }
                                                        if (br == 0)
                                                                pk_pmsg("I'll take a hit.");
                                                        else if (br == 1)
                                                                pk_pmsg("I'll take another hit.");
                                                        else if (br == 2)
                                                                pk_pmsg("I'll take one more.");
                                                        gameTick(10);
                                                        pk_dchd(pk_ch, 0);
                                                }
                                                if (br == 3 && round_ctr == 0) {
                                                        pk_cscore = 0;
                                                        res = pk_chsc(pk_ch, 0);
                                                        pk_cscore = pk_chsc(pk_ch, 1);
                                                        if (res < 0x16) {
                                                                if (0x15 < pk_cscore) {
                                                                        pk_cscore = res;
                                                                        pk_pmsg("I'll stand.");
                                                                        gameTick(0x14);
                                                                }
                                                        } else {
                                                                round_ctr = 1;
                                                                pk_cscore = res;
                                                        }
                                                }
                                                if (round_ctr != 0) {
                                                        pk_pmsg("I've busted !!");
                                                        gameTick(0x14);
                                                }

                                                plEr(225, 10, 319, 60);
                                                if (pk_phase == 0) {
                                                        res = pk_chsc(pk_ph, 0);
                                                        rv  = pk_chsc(pk_ph, 1);
                                                        pk_pscore = res;
                                                        if (rv < 0x16)
                                                                pk_pscore = rv;
                                                        if (round_ctr == 0 &&
                                                            pk_pscore <= pk_cscore) {
                                                                if (pk_cscore == pk_pscore) {
                                                                        pk_pmsg("It's a tie and nobody wins.");
                                                                        gameTick(0x14);
                                                                        pk_sbet(&g_pcbet, 1, 0);
                                                                } else {
                                                                        pk_pmsg("I win.");
                                                                        gameTick(0x14);
                                                                        pk_sbet(&g_pcbet, 0, 0);
                                                                }
                                                        } else {
                                                                pk_pmsg("You win.");
                                                                gameTick(0x14);
                                                                pk_sbet(&g_pcbet, 1, 0);
                                                        }
                                                } else {
                                                        if (pk_bs1 == NO && pk_c1bj == NO) {
                                                                for (br = 0; br < 5; br = br + 1)
                                                                        pk_drcs(CARD_HIGHLIGHT, br, 1);
                                                                for (br = 0;
                                                                     br < 5 && pk_ph[br] != CARD_NONE;
                                                                     br = br + 1)
                                                                        pk_drcs(pk_ph[br], br, 1);
                                                                pk_dbhi(1);
                                                                res = pk_chsc(pk_ph, 0);
                                                                rv  = pk_chsc(pk_ph, 1);
                                                                pk_pscore = res;
                                                                if (rv < 0x16)
                                                                        pk_pscore = rv;
                                                                if (round_ctr == 0 &&
                                                                    pk_pscore <= pk_cscore) {
                                                                        if (pk_cscore == pk_pscore) {
                                                                                pk_pmsg("First hand ties, nobody wins.");
                                                                                gameTick(0x14);
                                                                                pk_sbet(&g_pcbet, 1, 0);
                                                                        } else {
                                                                                pk_pmsg("Your first hand loses.");
                                                                                gameTick(0x14);
                                                                                pk_sbet(&g_pcbet, 0, 0);
                                                                        }
                                                                } else {
                                                                        pk_pmsg("You win with your first hand.");
                                                                        gameTick(0x14);
                                                                        pk_sbet(&g_pcbet, 1, 0);
                                                                }
                                                        }
                                                        if (pk_bs2 == NO && pk_c2bj == NO) {
                                                                for (br = 0; br < 5; br = br + 1)
                                                                        pk_drcs(CARD_HIGHLIGHT, br, 1);
                                                                for (br = 0;
                                                                     br < 5 && pk_psh[br] != CARD_NONE;
                                                                     br = br + 1)
                                                                        pk_drcs(pk_psh[br], br, 1);
                                                                pk_dbhi(2);
                                                                res = pk_chsc(pk_psh, 0);
                                                                rv  = pk_chsc(pk_psh, 1);
                                                                pk_pscore = res;
                                                                if (rv < 0x16)
                                                                        pk_pscore = rv;
                                                                if (round_ctr == 0 &&
                                                                    pk_pscore <= pk_cscore) {
                                                                        if (pk_cscore == pk_pscore) {
                                                                                pk_pmsg("Second hand ties, nobody wins.");
                                                                                gameTick(0x14);
                                                                                pk_sbet(&g_ppbet, 1, 0);
                                                                        } else {
                                                                                pk_pmsg("Your second hand loses.");
                                                                                gameTick(0x14);
                                                                                pk_sbet(&g_ppbet, 0, 0);
                                                                        }
                                                                } else {
                                                                        pk_pmsg("You win with your second hand.");
                                                                        gameTick(0x14);
                                                                        pk_sbet(&g_ppbet, 1, 0);
                                                                }
                                                        }
                                                }
                                        }
                                }
                        }
after_settle:
                        gameTick(0x18);
                        continue;
                }
                pk_dpile[10] = CARD_BJ_STOP;
        }

cleanup:
        tx_sctm  = 0;
        no_keyin = NO;
        Mfree(crd_dat);
        moff();
}

/* ---- Word Puzzle helpers ------------------------------------------- */

/* wp_shwm: word-puzzle status message.  Clears the bottom prompt
   strip and prints `msg` in green at (8, 58).
   addr: word_puzzle_show_status_message() */

void
wp_shwm(msg)
char *  msg;
{
        plEr(0, 50, 319, 59);
        strPr(msg, 8, 58, COLOR_green);
}

/* wp_rtmp: render the current puzzle template with player answers
   substituted in place of '@' markers.  Walks the template one
   character at a time:
    - literal chars accumulated into an in-buffer word until the
      next space; then, if the word overruns column 0x26, wraps
      to the next row; otherwise prints char-by-char at (col*8, y)
      in blue.
    - '@' pulls the next wp_ans[i] string, wraps if needed, prints,
      then advances the cursor by its length.  The character
      immediately after '@' (already stashed at wp_ans[i][0] by
      the caller during parse) is treated as trailing punctuation
      and printed alongside, then column bumps past it.
   Preserves Ghidra's cursor_x = 1, cursor_y = 0x28 (40) start
   position for byte-comparable output.
   addr: word_puzzle_render_template_with_answers() */

void
wp_rtmp()
{
        short   ch;
        char *  ap;             /* answer_ptr / scratch */
        short   wlen;
        short   ci;             /* char_index within an in-buffer word */
        short   cy;
        short   cx;
        short   ai;             /* answer_index -> which wp_ans[] to use */
        char    cur;
        char *  tp;             /* template_ptr */

        plEr(0, 31, 319, 49);
        cx = 1;
        ai = 0;
        cy = 0x28;
        tp = g_ltlp[g_wpci + g_wpci];
        for (;;) {
                for (;;) {
                        for (;;) {
                                ap  = tp;
                                cur = *ap;
                                tp  = ap + 1;
                                if (cur < ' ')
                                        return;
                                if (cur != ' ') break;
                                if (1 < cx)
                                        cx = cx + 1;
                        }
                        if (cur == '@') break;
                        wlen = 1;
                        in_str[0] = cur;
                        for (ap = tp; wlen < 0x10 && ' ' < *ap;
                             ap = ap + 1) {
                                in_str[wlen] = *ap;
                                wlen = wlen + 1;
                        }
                        if (0x26 < (short)(wlen + cx)) {
                                cx = 1;
                                cy = cy + 8;
                        }
                        for (ci = 0; tp = ap, ci < wlen; ci = ci + 1) {
                                prCh((short) in_str[ci],
                                                cx << 3, cy, COLOR_blue);
                                cx = cx + 1;
                        }
                }
                for (wlen = 0;
                     wlen < 0xc && ' ' < wp_ans[ai][wlen];
                     wlen = wlen + 1) ;
                if (0x27 < (short)(wlen + cx)) {
                        cx = 1;
                        cy = cy + 8;
                }
                strPr(wp_ans[ai], cx << 3, cy, COLOR_blue);
                cx = wlen + cx;
                ai = ai + 1;
                ch = (short) ap[2];
                if (ch < 0x20)
                        return;
                tp = ap + 2;
                if (ch < 0x41) {
                        prCh(ch, cx * 8, cy, COLOR_blue);
                        cx = cx + 1;
                        tp = ap + 3;
                }
        }
}

/* wp_solv: solve phase.  For each of wp_blk blanks:
    - clear the input strip
    - pick a prompt string: word 0 -> random from wp_prm[0..4],
      later words -> wp_prm[word_index + 4]
    - keyboard loop: A-Z (uppercased via lcp_upp), max 10 chars;
      cursor-left erases; Enter confirms; F10 quits early
   After all blanks entered, walks the solution line (odd index of
   the current puzzle's line pair), comparing token-by-token with
   the player's answers.  On success: renders + shows a random
   wp_succ message.  On any mismatch: same but wp_fail.
   Preserves Ghidra's LAB_00017c4a (failure exit) as a labeled
   goto so the byte-for-byte control flow matches the 1985 source.
   addr: word_puzzle_solve_phase() */

void
wp_solv()
{
        short   ch;
        short   ri;
        char *  psp;            /* player_answer_ptr / scratch */
        char *  slp;            /* solution_line_ptr */
        short   wi;
        short   ilen;
        short   cwi;            /* current_word_index */
        char    pci;            /* player_char_iter */
        char *  scp;            /* scan_ptr */
        char    sci;            /* solution_char_iter */

        plEr(0, 10, 175, 26);
        plEr(176, 0, 319,  8);
        plEr(176, 8, 248, 18);
        cwi = 0;
        do {
                plEr(0, 60, 319, 69);
                if (cwi == 0)
                        wi = rndRng(0, 4);
                else
                        wi = cwi + 4;
                wp_shwm(wp_prm[wi]);
                ilen = 0;
                for (;;) {
                        gameTick(0);
                        ch = mg_wkev();
                        if (ch == KEY_F10)
                                return;
                        if (ch == KEY_CTRL_M)
                                break;
                        if (ch == KEY_CURSOR_LEFT && 0 < ilen) {
                                ilen = ilen - 1;
                                wp_ans[cwi][ilen] = '\0';
                                plEr(ilen * 8 + 8, 60,
                                                  ilen * 8 + 16, 68);
                        } else if (ilen < 10 && 0x40 < ch) {
                                ri = lcp_upp(ch);
                                wp_ans[cwi][ilen] = (char) ri;
                                ilen = ilen + 1;
                                wp_ans[cwi][ilen] = '\0';
                                strPr(wp_ans[cwi], 8, 68, COLOR_white);
                        }
                }
                wp_ans[cwi][ilen] = '\0';
                gameTick(8);
                cwi = cwi + 1;
                plEr(0, 60, 319, 69);
        } while (cwi < wp_blk);

        slp = g_ltlp[g_wpci + g_wpci + 1];
        wi  = 0;
        for (;;) {
                scp = slp;
                if (wp_blk <= wi) {
                        wp_rtmp();
                        gameTick(8);
                        ri = rndRng(0, 5);
                        wp_shwm(wp_succ[ri]);
                        return;
                }
                do {
                        slp = scp;
                        scp = slp + 1;
                } while (*slp < '!');
                psp = wp_ans[wi];
                for (;;) {
                        sci = *slp;
                        slp = slp + 1;
                        if (sci < '!') break;
                        pci = *psp;
                        psp = psp + 1;
                        if (pci != sci)
                                goto fail;
                }
                if (*psp != '\0')
                        break;
                wi = wi + 1;
        }
fail:
        wp_rtmp();
        gameTick(8);
        ri = rndRng(0, 5);
        wp_shwm(wp_fail[ri]);
}
