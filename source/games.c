/*
 * games.c -- mini-game entry points + shared setup helpers.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>

#ifdef HOST

#include "hostgem.h"

#else

#include <vdibind.h>

#endif
#include "obdefs1.h"
#include "ahouse.h"
#include "ai.h"
#include "aidle.h"
#include "alerts.h"
#include "asimple.h"
#include "cards.h"
#include "events.h"
#include "games.h"
#include "gfx_prim.h"
#include "vdiown.h"
#include "globals.h"
#include "keyboard.h"
#include "letload.h"
#include "movement.h"
#include "parser.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

#include "dat_games.c"


/* These statics are defined after their first caller.  A static
   called before its definition needs a forward declaration, or Alcyon
   treats the call as external and the linker resolves it to 0; the
   declaration also lets the assembler use a short call. */
static short pk_bjwr();
static short pk_chsc();
static short pk_bjr();
static short pk_cnbj();
static void  pk_show();  /* defined after pk_main, which calls it */
static short pk_dchd();
static void  pk_dbhi();
static void  pk_sbet();
static void  pk_ante();
static void  pk_evhs();
static void  pk_blf();
static short pk_cace();
static void  pk_ddec();
static short pk_cbet();
static void  pk_cdrw();


/* gamePlWQ does not exist in the original; Alcyon emits a static even
   when nothing calls it, so it must not be defined here. */


/* lcp_lgt and lcp_rgt live in parts/ and are included by stx_u2.c,
   not here: they belong to that object. */

/* mg_wkev: wait for a key while processing urgent game events.
   On 7200 idle frames (~15 min) sets mg_tofl=YES and returns KEY_F10. */

short

mg_wkev()
{
        short           key;
        unsigned short  idle;

        idle    = 0;
        mg_tofl = NO;

        /* Drain any keys the game accidentally left in the buffer. */
        do ; while (getKey() != KEY_NONE);

        while ((key = getKey()) == KEY_NONE) {
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
                /* The GLOBAL tank level, not the saved copy in the
                   lcp struct. */
                if (lcp.thirst_level > NEED_SATISFIED && lcp_watr != 0) {
                        lcp_lgt();
                        a_drink();
                        lcp_rgt();
                }
                if (idle++ > 7200) {
                        mg_tofl = YES;
                        return KEY_F10;
                }
                if (g_trel[0] != ACTION_NONE) {
                        lcp_lgt();
                        execEv(getEv());
                        lcp_rgt();
                }
                gameTick(0);
        }
        if (key == KEY_CTRL_A_ALARM  ||
            key == KEY_CTRL_B_BOOK    ||
            key == KEY_CTRL_C_CALL     ||
            key == KEY_CTRL_D_DOGFOOD    ||
            key == KEY_CTRL_F_FOOD  ||
            key == KEY_CTRL_W_WATER)
                deal_kc(key);
        return key;
}

/* rndRng lives in parts/; it belongs here, right after mg_wkev, in
   the minigame object, so the minigames reach it with a short call. */
#include "parts/rndRng.c"


/* ag_matc: character-by-character equality test for two C strings.
   Keeps walking both strings after a mismatch and reports at the end. */

short
ag_matc(a, b)
char *  a;
char *  b;
{
        /* No character temporaries: comparing through the
           post-increments makes Alcyon save and restore the condition
           codes around them, as the original does.  The unused short
           ahead of the flag must stay: removing it changes the
           compiled code. */
        short   unused;
        short   mismatch;

        mismatch = 0;
        while (*a != '\0' && *b != '\0') {
                if (*a++ != *b++)
                        mismatch = 1;
        }
        if (mismatch == 1 || *a != '\0' || *b != '\0')
                return 0;
        else
                return 1;
}
/* The order of these parts/ includes is the object's function order
   and must not change. */
#include "parts/mg_stp.c"
#include "parts/gameCln.c"
#include "parts/vst_h20.c"
#include "parts/rst_vsth.c"
#include "parts/initVdi.c"
#include "parts/exitVdi.c"


/* wp_main: WORD PUZZLE main loop.
   Loads wordpz.txt into a 2000-byte buffer, indexes 66 line pointers
   (33 puzzles x {template, solution}).  F1 next / F2 prev (wraps 0..0x20)
   / F5 solve / F10 quit.  The next_puzzle and cleanup gotos are the
   original's control flow and must stay. */

void
wp_main()
{
        /* Declaration order and the unused locals must stay: an unused
           short ahead of line_index, then ONE short reused for both
           the scanned character and the mini-game key, the parse
           pointer, and four more unreferenced bytes. */
        short   unused1;
        short   line_index;
        short   cur;
        char *  parse_ptr;
        long    unused2;
#define key     cur

        g_wpdb = (char *) Malloc(2000L);
        if (g_wpdb == (char *) 0)
                er_nomem();
        mg_stp();
        fr_reac("wordpz.txt",
                             (unsigned char *) g_wpdb, 1536);

        /* Index the 66 lines. */
        parse_ptr = g_wpdb;
        for (line_index = 0; line_index < 0x42; line_index++) {
                g_ltlp[line_index] = parse_ptr;
                /* Step once, then a plain `while` -- two increment
                   sites, not a do/while's one; the original has this
                   shape. */
                parse_ptr++;
                while (*parse_ptr >= ' ')
                        parse_ptr++;
                while (*parse_ptr < ' ')
                        parse_ptr++;
        }

        g_wpci = 0;
        strPr("**WORD PUZZLE #  **", 8, 8, COLOR_black);

        /* No outer loop: `next_puzzle` is a plain label and every arm
           of the key switch jumps back to it explicitly, as in the
           original. */
next_puzzle:
        strPr("Choose the puzzle",   8,  16, COLOR_black);
        strPr("you wish to solve.",  8,  24, COLOR_black);
        strPr("F1 Next, F5 Solve", 176,  8, COLOR_red);
        strPr("F2 Last, F10 Quit", 176, 16, COLOR_red);
        plEr(128,  0, 143,  8);
        plEr(  0, 50, 319, 69);

        /* Count '@' blanks; seed wp_ans[i][0] with char after '@'. */
        parse_ptr = g_ltlp[g_wpci << 1];
        wp_blk = 0;
        while (1) {
                cur = *parse_ptr;
                parse_ptr++;
                if (cur < ' ') break;
                if (cur == '@') {
                        wp_ans[wp_blk][0] = *parse_ptr;
                        wp_ans[wp_blk][1] = '\0';
                        wp_blk++;
                }
        }

        plEr(128, 0, 135, 8);
        sprintf(in_str, "%2d", g_wpci + 1);
        strPr(in_str, 128, 8, COLOR_black);
        wp_rtmp();

        while (1) {
                gameTick(0);
                key = mg_wkev();
                switch (key) {
                case KEY_F1:
                        g_wpci++;
                        if (g_wpci >= 33)
                                g_wpci = 0;
                        goto next_puzzle;
                case KEY_F2:
                        g_wpci--;
                        if (g_wpci < 0)
                                g_wpci = 0x20;
                        goto next_puzzle;
                case KEY_F5:
                        wp_solv();
                        if (mg_tofl != NO)
                                goto cleanup;
                        gameTick(0x28);
                        goto next_puzzle;
                case KEY_F10:
                        goto cleanup;
                }
        }

cleanup:
        no_keyin = NO;
        tx_sctm  = 0;
        Mfree(g_wpdb);          /* g_wpdb is deliberately not cleared */
}
#undef key

/* wp_solv must follow wp_main directly so the call from the key switch
   stays a short branch.
   wp_solv: solve phase.  Per blank: prompt, read A-Z (10-char max),
   Enter confirms, F10 quits.  Then walk solution line, compare
   token-by-token; show wp_succ or wp_fail. */

void
wp_solv()
{
        /* Exactly six locals; `ch` also carries the scanned solution
           and answer characters, as in the original. */
        short   cwi;            /* current_word_index */
        short   ilen;
        short   wi;
        short   ch;
        char *  slp;            /* solution_line_ptr */
        char *  psp;            /* player_answer_ptr */

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
                while (1) {
                        gameTick(0);
                        ch = mg_wkev();
                        if (ch == KEY_F10)
                                return;
                        /* The two `(long)` casts are deliberate: they
                           make Alcyon add the array base after the
                           column offset, as the original does.  Without
                           them the base is folded in first -- which is
                           what the other two stores here do. */
                        if (ch == KEY_CTRL_M) {
                                wp_ans[cwi][ilen] = '\0';
                                break;
                        } else if (ch == KEY_CURSOR_LEFT && ilen > 0) {
                                ilen--;
                                wp_ans[0][cwi * 12 + (long) ilen] = '\0';
                                plEr(ilen * 8 + 8, 60,
                                          ilen * 8 + 16, 68);
                        } else if (ilen < 10 && ch >= 'A') {
                                ch = lcp_upp(ch);
                                wp_ans[0][cwi * 12 + (long) ilen] = ch;
                                ilen++;
                                wp_ans[cwi][ilen] = '\0';
                                strPr(wp_ans[cwi], 8, 68, COLOR_white);
                        }
                }
                gameTick(8);
                cwi++;
                plEr(0, 60, 319, 69);
        } while (cwi < wp_blk);

        slp = g_ltlp[(g_wpci << 1) + 1];
        wi  = 0;
        while (wi < wp_blk) {
                /* The assignment is INSIDE the condition, so the
                   compare uses the loaded value rather than reloading
                   `ch` from the frame. */
                do {
                } while ((ch = *slp++) <= ' ');
                slp--;
                psp = wp_ans[wi];
                while (1) {
                        if ((ch = *slp++) <= ' ')
                                break;
                        if (*psp++ != ch)
                                goto fail;
                }
                if (*psp != '\0')
                        goto fail;
                wi++;
        }
        wp_rtmp();
        gameTick(8);
        wp_shwm(wp_succ[rndRng(0, 5)]);
        return;
fail:
        wp_rtmp();
        gameTick(8);
        wp_shwm(wp_fail[rndRng(0, 5)]);
}

/* wp_shwm and wp_rtmp must follow wp_solv directly so its calls to
   them stay short branches.
   wp_shwm: word-puzzle status message in green at (8,58). */

void
wp_shwm(msg)
char *  msg;
{
        plEr(0, 50, 319, 59);
        strPr(msg, 8, 58, COLOR_green);
}

/* wp_rtmp: render puzzle template with player answers substituted for '@'.
   Word-wraps at col 0x26 (literal) / 0x27 (answer).  Starts cursor at
   (x=1, y=0x28). */

void
wp_rtmp()
{
        /* Seven locals in this order; `cur` is a short, and there is
           no separate scan pointer -- tp is stepped in place. */
        short   ai;
        short   cx;
        short   cy;
        short   ci;
        short   wlen;
        short   cur;
        char *  tp;

        plEr(0, 31, 319, 49);
        tp = g_ltlp[g_wpci << 1];
        cx = 1;
        ai = 0;
        cy = 0x28;
        while (1) {
                cur = *tp;
                tp++;
                if (cur < ' ')
                        return;
                if (cur == ' ') {
                        if (cx > 1)
                                cx++;
                        continue;
                }
                if (cur == '@') {
                        for (wlen = 0; wlen < 12; wlen++)
                                if (wp_ans[ai][wlen] <= ' ')
                                        break;
                        if (cx + wlen > 0x27) {
                                cx = 1;
                                cy += 8;
                        }
                        strPr(wp_ans[ai], cx << 3, cy, COLOR_blue);
                        cx += wlen;
                        tp++;
                        ai++;
                        cur = *tp;
                        if (cur < ' ')
                                return;
                        if (cur < 'A') {
                                prCh(cur, cx << 3, cy, COLOR_blue);
                                cx++;
                                tp++;
                        }
                        continue;
                }

                /* Literal word. */
                in_str[0] = cur;
                for (wlen = 1; wlen < 0x10; wlen++) {
                        /* if/else, not an early break: the else arm's
                           `break` becomes a branch past the body's own
                           jump to the increment. */
                        if ((cur = *tp) > ' ') {
                                in_str[wlen] = cur;
                                tp++;
                        } else
                                break;
                }
                if (cx + wlen > 0x26) {
                        cx = 1;
                        cy += 8;
                }
                for (ci = 0; ci < wlen; ci++) {
                        prCh(in_str[ci], cx << 3, cy, COLOR_blue);
                        cx++;
                }
        }
}

/* ag_cwda: clear the right-panel word display area (162,10)-(319,49). */

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

/* ag_cswa: clear the left-panel intro/instructions area (5,10)-(160,60). */

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

/* ag_cgpa: clear the "Guess #N?" prompt bar (166,50)-(319,65). */

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

/* ag_csb: clear the bottom info bar (5,62)-(319,75). */

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

/* ag_intr: draw the 5-line intro text in the left panel. */

void
ag_intr()
{
        strPr("I am thinking of",  5, 17, COLOR_black);
        strPr("a word.  Here it",  5, 25, COLOR_black);
        strPr("is jumbled up...",  5, 33, COLOR_black);
        strPr("See if you can ",   5, 41, COLOR_black);
        strPr("guess what it is.", 5, 49, COLOR_black);
}

/* ag_dwl: display a word in 20px text in right panel at (162,37), 12px pitch. */

void
ag_dwl(word, text_color)
char *  word;
short   text_color;
{
        short   x;

        ag_cwda();
        vst_h20();
        x = 0;
        /* The pointer is stepped inside the body, before the pitch;
           this order matches the original's code. */
        while (*word != '\0') {
                prCh((short) *word, x + 162, 37, text_color);
                word++;
                x += 12;
        }
        rst_vsth();
}

/* ag_sgp lives in parts/; it must sit here, after ag_dwl. */

#include "parts/ag_sgp.c"

/* ag_ssw: pick a random word from the 150-entry dictionary (11 bytes/row),
   copy into g_agscw, scramble 10..20 swaps.  Re-scrambles on identity.
   Plants '\0' at g_agwb row-tail so g_agorw reads as a C string. */

void
ag_ssw()
{
        /* One counter reused for the copy index and the shuffle round,
           and a local copy of the word length that the shuffle reads
           instead of g_agwol -- the original's locals exactly. */
        short   pos;
        short   len;
        short   ia;
        short   ib;
        char    tmp;
        char *  wp;

        g_agorw = g_agwb + rndRng(0, 0x95) * 11;        /* 0..149 */
        pos     = 0;
        for (wp = g_agorw; *wp > ' ' && *wp != '.'; ) {
                /* Index first: this makes Alcyon fold the base into the
                   address the way the original does. */
                *(pos + g_agscw) = *wp;
                wp++;
                pos++;
        }
        g_agscw[pos] = '\0';
        *wp          = '\0';
        len     = pos;
        g_agwol = len;

        while (ag_matc(g_agscw, g_agorw) != 0) {
                pos = 0;
                while (rndRng(10, 0x14) > pos) {
                        ia  = rndRng(0, len - 1);
                        ib  = rndRng(0, len - 1);
                        tmp = g_agscw[ib];
                        g_agscw[ib] = g_agscw[ia];
                        g_agscw[ia] = tmp;
                        pos++;
                }
        }
        ag_dwl(g_agscw, COLOR_green);
}

/* ag_main: full anagram game loop.  Outer per-word / middle per-guess /
   inner per-keypress.  The new_word/validate labels are the 1985
   code's own gotos and must stay. */

#include "dat_games2.c"

void
ag_main()
{
        /* Twelve locals, only six of which the body touches.  The
           unused ones must stay: removing them changes the compiled
           code. */
        short   index;
        short   unused1;
        short   unused2;
        short   key_pressed;
        short   unused3;
        short   walk_result;
        char    typed_char;
        short   clue_count;
        short   guess_count;
        short   unused5;
        short   unused6;
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

        /* The round prologue runs ONCE per word and the guess-count
           guard is folded into a `while` condition.  A wrong guess
           re-enters HERE, past the prologue, so the word is kept --
           only a solved or abandoned word goes back to new_word. */
same_word:
        g_agacu = 0;
        ag_clue = 0;
        strPr("F1 Clue, F10 Quit", 183, 8, COLOR_blue);
        ag_sgp(g_aggun);
        for (index = 0; index < 10; index++)
                g_aginb[index] = ' ';
        g_aginb[10]   = '\0';
        gameTick(0);
        word_complete = NO;
        while (g_aggun < 9 || (g_aggun < 10 && g_agacu != 0)) {
                index         = 0;
                key_pressed   = 0;
                while (key_pressed != KEY_CTRL_M) {
                        strPr(g_aginb, 239, 57, COLOR_green);
                        key_pressed = mg_wkev();
                        if (key_pressed >= 'A' && key_pressed <= 'Z')
                                key_pressed += 0x20;
                        if (key_pressed <= 'z' && key_pressed >= 'a') {
                                g_aginb[index] = key_pressed;
                                if (++index >= 10) {
                                        index = 9;
                                        ag_sgp(g_aggun);
                                }
                        }
                        if (key_pressed == KEY_CURSOR_LEFT) {
                                if (index != 0) {
                                        if (index == 9 &&
                                            g_aginb[index] != ' ')
                                                g_aginb[index] = ' ';
                                        else {
                                                index--;
                                                *(index + g_aginb) = ' ';
                                        }
                                } else
                                        g_aginb[index] = ' ';
                                ag_sgp(g_aggun);
                                continue;
                        }
                        if (key_pressed == KEY_F10) {
                                tx_sctm  = 0;
                                no_keyin = NO;
                                Mfree(g_agwb);
                                return;
                        }
                        if (key_pressed == KEY_F1 && ag_clue == 0) {
                                /* A goto, not `continue`: the original
                                   jumps to a label sitting ON the else
                                   arm's statement, one test ahead of the
                                   loop's own condition.  The braces are
                                   deliberate too; without them Alcyon
                                   emits a different branch shape. */
                                if (ag_matc(g_agorw, g_agscw) != 0) {
                                        goto again;
                                }
                                /* Clue path: reveal one letter. */
                                g_agclc++;
                                g_aggun++;
                                ag_sgp(g_aggun);
                                if (g_aggun == 9)
                                        g_agacu = 1;
                                ag_clue = 1;
                                plEr(182, 0, 319, 9);
                                strPr("         F10 Quit", 183, 8,
                                      COLOR_blue);
                                for (guess_count = 0;
                                     guess_count < g_agwol;
                                     guess_count++)
                                        if (g_agorw[guess_count] !=
                                            g_agscw[guess_count])
                                                break;
                                if (guess_count != g_agwol) {
                                        clue_count = g_agwol - 1;
                                        for (;;) {
                                                if (g_agorw[guess_count] ==
                                                    g_agscw[clue_count])
                                                        break;
                                                clue_count--;
                                        }
                                        typed_char = g_agscw[clue_count];
                                        g_agscw[clue_count] =
                                                g_agscw[guess_count];
                                        g_agscw[guess_count] = typed_char;
                                }
                                ag_dwl(g_agscw, COLOR_green);
                                if (ag_matc(g_agorw, g_agscw) != 0) {
                                        ag_csb();
                                        strPr("You took too many clues!",
                                              5, 69, COLOR_black);
                                        ag_dwl(g_agorw, COLOR_black);
                                        gameTick(0x14);
                                        word_complete = YES;
                                }
                                if (word_complete != NO)
                                        goto validate;
                        } else
again:                          if (word_complete != NO)
                                        goto validate;
                }

validate:
                if (word_complete != NO)
                        goto new_word;
                /* The second test is deliberately a bare truthiness
                   test: spelling it `!= '\0'` makes Alcyon address the
                   array differently from the original. */
                for (index = 0;
                     g_aginb[index] != ' ' && g_aginb[index];
                     index++) ;
                if (g_aginb[index] == ' ')
                        g_aginb[index] = '\0';
                if (ag_matc(g_aginb, g_agorw) != 0) {
                        strPr("YOU GOT IT!!!!!!",
                                             5, 69, COLOR_black);
                        ag_dwl(g_agorw, COLOR_black);
                        gameTick(0x1e);
                        ag_csb();
                        goto new_word;
                /* The guess counter steps in the condition itself,
                   so both arms see it incremented. */
                } else if (g_aggun++ < 8) {
                        strPr(g_agwgm[rndRng(0, 2)], 5, 69, COLOR_black);
                        gameTick(0x14);
                        ag_csb();
                        goto same_word;
                } else {

                /* Too many wrong guesses: show the answer, start a
                   new word. */
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
        }
}

/* plEr and plErCol (in parts/) sit here, after the anagram code;
   plErCol must follow plEr directly. */
#include "parts/plEr.c"
#include "parts/plErCol.c"


/* pk_dbet: computer call/raise decision.  Returns 'c' or 'r'.
   On raise: pk_dpos = money/10 clamped [1,20]. */

static short
pk_dbet()
{
        /* No temporary, just early returns.  The redundant `else` is
           kept on purpose: Alcyon emits its skip branch even though
           the then arm returns, and the original has it. */
        if (g_pcmon == 0)
                return 'c';
        if (pk_bluff == NO && pk_chrk < HAND_TWO_PAIR)
                return 'c';
        else {
                pk_dpos = g_pcmon / 10;
                if (pk_dpos == 0)
                        pk_dpos = 1;
                else if (pk_dpos > 20)
                        pk_dpos = 20;
                return 'r';
        }
}

/* pk_evh: evaluate a 5-card hand.  *hand_rank <- 0=high card..9=royal flush.
   rank_flags[i]=1 for winning combo cards.  suit_flags: rank-sorted hand copy.
   The two goto exits are the original's control flow. */

static void
pk_evh(hand, rank_flags, suit_flags, hand_rank)
short * hand;
short * rank_flags;
short * suit_flags;
short * hand_rank;
{
        /* Eleven declarations in this order, four of them zeroed on
           entry; the order and the unused one must stay, or the
           compiled code changes. */
        short    straight;
        short    flush;
        short    unused;            /* written once, never read */
        short    ace_high;
        short    i;
        short    j;                 /* doubles as the sort flag */
        short    tmp;               /* doubles as the wheel flag */
        short    rc[5];             /* rank_flags scratch */
        unsigned short  sc[5];      /* pair-slot flag scratch */
        short    hc;
        short    bp;

        straight = 0;
        flush    = 0;
        unused   = 0;
        ace_high = 0;
        *hand_rank = HAND_HIGH_CARD;
        for (i = 0; i < 5; i++)
                suit_flags[i] = hand[i];

        /* Bubble sort suit_flags[] by rank ascending. */
        j = 1;
        while (j) {
                j = 0;
                for (i = 0; i < 4; i++) {
                        if (suit_flags[i]     % CARDS_PER_SUIT >
                            suit_flags[i + 1] % CARDS_PER_SUIT) {
                                tmp = suit_flags[i + 1];
                                suit_flags[i + 1] = suit_flags[i];
                                suit_flags[i] = tmp;
                                j = 1;
                        }
                }
        }

        /* Ace-high is latched BEFORE the straight scan, and the wheel
           flag borrows the sort's tmp variable. */
        if (suit_flags[4] % CARDS_PER_SUIT == CARD_RANK_ACE)
                ace_high = 1;
        straight = 1;
        for (i = 0; i < 3; i++) {
                if (suit_flags[i] % CARDS_PER_SUIT != suit_flags[i + 1] % CARDS_PER_SUIT - 1)
                        straight = 0;
        }
        tmp = 0;
        /* Wheel straight A-2-3-4-5 lives with Ace-high sorted last. */
        if (ace_high != 0 && straight != 0 && suit_flags[0] % CARDS_PER_SUIT == CARD_RANK_2)
                tmp = 1;
        if (tmp == 0 &&
            suit_flags[3] % CARDS_PER_SUIT != suit_flags[4] % CARDS_PER_SUIT - 1)
                straight = 0;

        /* Flush: all same suit (card / 13). */
        flush = YES;
        for (i = 0; i < 4; i++) {
                if (suit_flags[i]     / CARDS_PER_SUIT !=
                    suit_flags[i + 1] / CARDS_PER_SUIT)
                        flush = NO;
        }
        if (straight != NO) *hand_rank = HAND_STRAIGHT;
        if (flush != NO)    *hand_rank = HAND_FLUSH;
        if (straight != NO && flush != NO)
                *hand_rank = HAND_STRAIGHT_FLUSH;
        /* Royal: T-J-Q-K-A of one suit -- rank[0] == 8 (ten). */
        if (*hand_rank == HAND_STRAIGHT_FLUSH && suit_flags[0] % CARDS_PER_SUIT == CARD_RANK_10)
                *hand_rank = HAND_ROYAL_FLUSH;
        if (*hand_rank != HAND_HIGH_CARD)
                return;

        for (i = 0; i < 5; i++) {
                rank_flags[i] = 0;
                rc[i]         = 0;
                sc[i]         = 0;
        }

        hc = 0;                 /* first-hit rank (pair/trip/quad) */
        bp = 0;                 /* second-hit rank (two pair / full) */
        i = 0;
        while (i < 13) {
                tmp = 0;
                for (j = 0; j < 5; j++) {
                        if ((short) hand[j] % CARDS_PER_SUIT == i) {
                                if (hc == 0)
                                        rc[j] = 1;
                                else if (bp == 0)
                                        sc[j] = 1;
                                tmp++;
                        }
                }
                /* Four of a kind: rank 7, flags = rc (the 4 matched
                   cards).  Handled inline with a jump out rather than
                   a break to a tail block, as in the original. */
                if (tmp == 4) {
                        hc = 7;
                        for (i = 0; i < 5; i++)
                                rank_flags[i] = rc[i];
                        goto rank_from_hc_bp;
                }
                if (tmp == 3) {
                        if (hc == 0) {
                                hc = 3;
                                for (j = 0; j < 5; j++)
                                        rank_flags[j] = rc[j];
                        } else if (bp == 0) {
                                bp = 3;
                                for (j = 0; j < 5; j++)
                                        rank_flags[j] = sc[j];
                                goto rank_from_hc_bp;
                        }
                }
                if (tmp == 1) {
                        if (hc == 0) {
                                for (j = 0; j < 5; j++)
                                        rc[j] = 0;
                        } else if (bp == 0) {
                                for (j = 0; j < 5; j++)
                                        sc[j] = 0;
                        }
                }
                if (tmp == 2) {
                        if (hc == 0) {
                                hc = 1;
                                for (j = 0; j < 5; j++)
                                        rank_flags[j] = rc[j];
                        } else if (bp == 0) {
                                if (hc == 1) {
                                        bp = 1;
                                        for (j = 0; j < 5; j++)
                                                rank_flags[j] |= sc[j];
                                } else if (hc == 3) {
                                        bp = 1;
                                }
                        }
                }
                i++;
        }

rank_from_hc_bp:
        if (hc + bp == 7) *hand_rank = HAND_FOUR_OF_A_KIND;
        if (hc + bp == 3) *hand_rank = HAND_THREE_OF_A_KIND;
        if (hc + bp == 4) *hand_rank = HAND_FULL_HOUSE;
        if (hc + bp == 2) *hand_rank = HAND_TWO_PAIR;
        if (hc + bp != 1) return;
        *hand_rank = HAND_ONE_PAIR;
}


/* pk_main: 5-card draw poker main loop.
   Init: Malloc, load cards, mg_stp, money=400 each.
   Per-round: ante, deal, bet, discard/draw, computer draw, final bet,
   showdown.  The labels and gotos are the original's control flow and
   must stay. */

#include "dat_games3.c"

void
pk_main()
{
        /* Six locals in this order: loc8 doubles as the second key
           variable and as the raise countdown. */
        short   ikey;
        short   loc8;
        short   i;
        short   card;
        short   dcount;
        BOOL16  in_use;

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

        /* A label and gotos, not a loop statement: the round tick sits
           at the top and is skipped on the first pass.  The cleanup is
           a label inside the first exit test. */
        goto round;
next_round:
        gameTick(0x18);
round:
                plEr(70, 10, 219, 62);
                pk_ante();
                if (pk_quit == YES) {
cleanup:
                        tx_sctm  = 0;
                        no_keyin = NO;
                        Mfree(crd_dat);
                        moff();
                        return;
                }
                pk_evhs();

                if (pk_cbet("Do you feel lucky today?") == -1) {
                        if (mg_tofl != NO)
                                goto cleanup;
                        pk_pmsg("Sorry, you're all out!");
                        gameTick(10);
                        goto cleanup;
                }
                if (pk_pass != NO) {
                        pk_pmsg("That's all right with me.");
                        gameTick(10);
                } else {
                        pk_pmsg("I'll see your bet.");
                        while (pk_bet--) {
                                if (g_pcmon == 0) {
                                        pk_pmsg("Sorry, I'm all out!");
                                        gameTick(10);
                                        goto cleanup;
                                }
                                g_pcmon--;
                                pk_awp();
                                g_ppppa++;
                                pk_dpot();
                                gameTick(0);
                        }
                }
                gameTick(0x10);

                pk_disc = 0;
                pk_pmsg("Do you want any cards?");
                plEr(225, 10, 319, 60);
                strPr("F1 Draw", 225, 18, COLOR_red);
                strPr("F3 Stay", 225, 26, COLOR_red);
                for (i = 0; i < 5; i++)
                        pk_sel[i] = 0;

                /* One while loop: the key read is the condition, and
                   every re-prompt is a `continue`. */
discard_loop:
                while ((ikey = pk_inph(KEY_F1, KEY_F3, 0)) != PK_IN_ARG_B) {
                        if (ikey == PK_IN_TIMEOUT)
                                break;
                        for (i = 0; i < 5; i++)
                                if (pk_sel[i] == 1)
                                        break;
                        if (i == 5)
                                strPr("F3 Stay", 225, 26, COLOR_red);
                        else
                                strPr("F3 Stay", 225, 26, COLOR_lt_grey);
                        if (ikey == PK_IN_ARG_A) {
                                for (i = 0; i < 5; i++)
                                        if (pk_sel[i] == 1)
                                                break;
                                if (i == 5)
                                        continue;
                                break;
                        }
                        if (ikey < PK_IN_DIGIT_1)
                                continue;
                        if (ikey > PK_IN_DIGIT_5)
                                continue;
                        if (pk_sel[ikey - PK_IN_DIGIT_1]) {
                                pk_sel[ikey - PK_IN_DIGIT_1] = 0;
                                pk_drcs(pk_ph[ikey - PK_IN_DIGIT_1], ikey - PK_IN_DIGIT_1, 1);
                        } else {
                                pk_sel[ikey - PK_IN_DIGIT_1] = 1;
                                pk_drcs(CARD_HIGHLIGHT, ikey - PK_IN_DIGIT_1, 1);
                        }
                        for (i = 0; i < 5; i++)
                                if (pk_sel[i] == 1)
                                        break;
                        if (i == 5)
                                strPr("F3 Stay", 225, 26, COLOR_red);
                        else
                                strPr("F3 Stay", 225, 26, COLOR_lt_grey);
                }
                if (mg_tofl != NO)
                        goto cleanup;
                if (ikey == PK_IN_ARG_A) {
                        for (i = 0; i < 5; i++) {
                                if (pk_sel[i] != 1) continue;
                                in_use = YES;
                                while (in_use != NO) {
                                        card = rndRng(0, 51);
                                        in_use = NO;
                                        for (dcount = 0; dcount < 5; dcount++) {
                                                if (pk_ch[dcount] == card)
                                                        in_use = YES;
                                                if (pk_ph[dcount] == card)
                                                        in_use = YES;
                                        }
                                        dcount = pk_disc;
                                        while (dcount--)
                                                if (pk_dpile[dcount] == card)
                                                        in_use = YES;
                                }
                                pk_dpile[pk_disc] = pk_ph[i];
                                pk_disc++;
                                pk_ph[i] = card;
                                pk_drcs(card, i, 1);
                                gameTick(3);
                        }
                }
                if (ikey == PK_IN_ARG_B) {
                        for (i = 0; i < 5; i++)
                                if (pk_sel[i] == 1)
                                        break;
                        if (i != 5)
                                goto discard_loop;
                }

                pk_cdrw();
                if (pk_cbet("Want to make a bet?") == -1) {
                        if (mg_tofl != NO)
                                goto cleanup;
                        pk_pmsg("Sorry, you're all out!");
                        gameTick(10);
                        goto cleanup;
                }
                if (pk_pass != NO) {
                        if (pk_bluff == NO && pk_chrk == HAND_HIGH_CARD) {
                                pk_pmsg("Ok, I'll call.");
                                gameTick(10);
                                pk_show();
                                goto next_round;
                        } else {
                                i = rndRng(5, 15);
                                if (i > g_pcmon)
                                        i = g_pcmon;
                                pk_bm[9] = i / 10 + '0';
                                if (pk_bm[9] == '0')
                                        pk_bm[9] = ' ';
                                pk_bm[10] = i % 10 + '0';
                                pk_pmsg(pk_bm);
                                pk_bet = 0;
                                while (i--) {
                                        pk_ddec(0, 1);
                                        gameTick(0);
                                }
                                gameTick(10);
                                pk_pmsg("Will you see my bet?");
                                pk_phv = pk_bet;
                                plEr(225, 10, 319, 60);
                                strPr("F1 See",  225, 18, COLOR_red);
                                strPr("F3 Fold", 225, 34, COLOR_red);
                                ikey = pk_inph(KEY_F1, PK_IN_UNUSED, KEY_F3);
                                if (ikey == PK_IN_TIMEOUT) goto cleanup;
                                if (ikey == PK_IN_ARG_C) {
                                        plEr(225, 10, 319, 60);
                                        pk_pmsg("My pot.");
                                        gameTick(8);
                                        pk_annr(0);
                                        goto next_round;
                                }
                                if (ikey == PK_IN_ARG_A) {
                                        loc8   = pk_phv;
                                        pk_bet = 0;
                                        while (loc8--) {
                                                pk_ddec(1, 1);
                                                gameTick(0);
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
                                        while (1) {
                                                loc8 = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                                                if (loc8 == PK_IN_TIMEOUT)
                                                        break;
                                                if (loc8 == PK_IN_ARG_C) {
                                                        pk_show();
                                                        goto next_round;
                                                }
                                                if (loc8 == PK_IN_ARG_A && g_ppmon != 0) {
                                                        pk_bet = 0;
                                                        pk_ddec(1, 1);
                                                        pk_dpos++;
                                                        break;
                                                }
                                        }
                                        if (mg_tofl != NO) goto cleanup;
                                        while (1) {
                                                loc8 = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                                                if (loc8 == PK_IN_TIMEOUT)
                                                        break;
                                                if (loc8 == PK_IN_ARG_B)
                                                        break;
                                                if (loc8 == PK_IN_ARG_A) {
                                                        pk_ddec(1, 1);
                                                        if (g_ppmon != 0)
                                                                pk_dpos++;
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
                                        while (loc8--) {
                                                pk_ddec(0, 1);
                                                gameTick(0);
                                        }
                                        gameTick(5);
                                        pk_pmsg("I'll call.");
                                        gameTick(8);
                                        pk_show();
                                        goto next_round;
                                }
                        }
                } else {
                        if (pk_cace() == -1) {
                                pk_pmsg("I feel unlucky. I fold.");
                                gameTick(8);
                                pk_pmsg("Your pot.");
                                pk_annr(1);
                                goto next_round;
                        }
                        if (g_pcmon < pk_bet) {
                                        pk_pmsg("Sorry, I'm all out!");
                                        gameTick(10);
                                        goto cleanup;
                                }
                                pk_pmsg("Ok. I'll see your bet.");
                                i = pk_bet;
                                pk_bet = 0;
                                while (i--) {
                                        pk_ddec(0, 1);
                                        gameTick(0);
                                }
                                if (pk_dbet() == 'c') {
                                        pk_pmsg("I'll call.");
                                        gameTick(8);
                                        pk_show();
                                        goto next_round;
                                } else {
                                        pk_rm[11] = pk_dpos / 10 + '0';
                                        if (pk_rm[11] == '0')
                                                pk_rm[11] = ' ';
                                        pk_rm[12] = pk_dpos % 10 + '0';
                                        pk_pmsg(pk_rm);
                                        ikey = pk_dpos;
                                        pk_bet    = 0;
                                        while (ikey--) {
                                                pk_ddec(0, 1);
                                                gameTick(0);
                                        }
                                        gameTick(8);
                                        pk_pmsg("You think I'm bluffin'?");
                                        pk_phv = pk_dpos;
                                        plEr(225, 10, 319, 60);
                                        strPr("F1 See",  225, 18, COLOR_red);
                                        strPr("F3 Fold", 225, 34, COLOR_red);
                                        ikey = pk_inph(KEY_F1, PK_IN_UNUSED, KEY_F3);
                                        if (ikey == PK_IN_TIMEOUT) goto cleanup;
                                        if (ikey == PK_IN_ARG_C) {
                                                plEr(225, 10, 319, 60);
                                                pk_pmsg("My pot.");
                                                gameTick(8);
                                                pk_annr(0);
                                                goto next_round;
                                        }
                                        if (ikey == PK_IN_ARG_A) {
                                                loc8   = pk_phv;
                                                pk_bet = 0;
                                                while (loc8--) {
                                                        pk_ddec(1, 1);
                                                        gameTick(0);
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
                                                while (1) {
                                                        loc8 = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                                                        if (loc8 == PK_IN_TIMEOUT)
                                                                break;
                                                        if (loc8 == PK_IN_ARG_C) {
                                                                pk_show();
                                                                goto next_round;
                                                        }
                                                        if (loc8 == PK_IN_ARG_A && g_ppmon != 0) {
                                                                pk_bet = 0;
                                                                pk_dpos = 0;
                                                                pk_ddec(1, 1);
                                                                pk_dpos++;
                                                                break;
                                                        }
                                                }
                                                if (mg_tofl != NO) goto cleanup;
                                                while (1) {
                                                        loc8 = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                                                        if (loc8 == PK_IN_TIMEOUT)
                                                                break;
                                                        if (loc8 == PK_IN_ARG_B)
                                                                break;
                                                        if (loc8 == PK_IN_ARG_A) {
                                                                pk_ddec(1, 1);
                                                                if (g_ppmon != 0)
                                                                        pk_dpos++;
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
                                                while (loc8--) {
                                                        pk_ddec(0, 1);
                                                        gameTick(0);
                                                }
                                                gameTick(5);
                                                pk_pmsg("I'll call.");
                                                gameTick(8);
                                                pk_show();
                                                goto next_round;
                                        }
                                }
                        }
}

/* pk_show: showdown.  Reveal computer hand, evaluate both, walk the
   per-rank tiebreak ladder.  Winner blinks 5x then pk_annr transfers.
   Sets pk_round=1. */

static void
pk_show()
{
        /* The loop counter is declared first and the inner loops
           reuse `i`; the original has exactly these six locals. */
        short   i;
        short   br;         /* blink counter, tested with (br & 1) */
        short   pk;
        short   ck;
        short   ph;
        short   ch;

        /* Reveal computer hand, animated. */
        for (i = 0; i < 5; i++) {
                pk_drcs(pk_ch[i], i, 0);
                gameTick(2);
        }
        pk_evh(pk_ch, pk_hrf,  pk_hsf,  &pk_chrk);
        pk_evh(pk_ph, pk_phrf, pk_phsf, &pk_phrk);

        if (pk_chrk > pk_phrk) pk_dslot = 0;
        if (pk_chrk < pk_phrk) pk_dslot = 1;

        if (pk_chrk == pk_phrk) {
                pk_dslot = 1;

                /* Straight/flush/straight-flush tiebreak: compare
                   highest sorted-hand card rank. */
                if ((pk_chrk == HAND_STRAIGHT_FLUSH || pk_chrk == HAND_FLUSH || pk_chrk == HAND_STRAIGHT) &&
                    pk_hsf[4] % CARDS_PER_SUIT > pk_phsf[4] % CARDS_PER_SUIT)
                        pk_dslot = 0;

                /* Trips, full house, quads: compare the pair/trip
                   card's rank. */
                if (pk_chrk == HAND_FOUR_OF_A_KIND || pk_chrk == HAND_FULL_HOUSE || pk_chrk == HAND_THREE_OF_A_KIND) {
                        for (br = 0; br < 5; br++)
                                if (pk_hrf[br] == 1)
                                        break;
                        for (i = 0; i < 5; i++)
                                if (pk_phrf[i] == 1)
                                        break;
                        if (pk_ch[br] % CARDS_PER_SUIT > pk_ph[i] % CARDS_PER_SUIT)
                                pk_dslot = 0;
                }

                /* Two pair tiebreak. */
                if (pk_chrk == HAND_TWO_PAIR) {
                        pk = 0; ck = 0; ph = 0; ch = 0;
                        for (i = 0; i < 5; i++) {
                                if (pk_hrf[i] &&
                                    pk_ch[i] % CARDS_PER_SUIT > pk % CARDS_PER_SUIT)
                                        pk = pk_ch[i];
                                if (pk_hrf[i] &&
                                    pk_ch[i] % CARDS_PER_SUIT < pk % CARDS_PER_SUIT)
                                        ck = pk_ch[i];
                                if (pk_phrf[i] &&
                                    pk_ph[i] % CARDS_PER_SUIT > ph % CARDS_PER_SUIT)
                                        ph = pk_ph[i];
                                if (pk_phrf[i] &&
                                    pk_ph[i] % CARDS_PER_SUIT < ph % CARDS_PER_SUIT)
                                        ch = pk_ph[i];
                        }
                        if (pk % CARDS_PER_SUIT > ph % CARDS_PER_SUIT) {
                                pk_dslot = 0;
                        } else if (pk % CARDS_PER_SUIT == ph % CARDS_PER_SUIT &&
                                   ck % CARDS_PER_SUIT > ch % CARDS_PER_SUIT) {
                                pk_dslot = 0;
                        } else if (pk % CARDS_PER_SUIT == ph % CARDS_PER_SUIT &&
                                   ck % CARDS_PER_SUIT == ch % CARDS_PER_SUIT) {
                                for (i = 0; i < 5; i++)
                                        if (!pk_hrf[i])
                                                break;
                                for (br = 0; br < 5; br++)
                                        if (!pk_phrf[br])
                                                break;
                                if (pk_ch[i] % CARDS_PER_SUIT > pk_ph[br] % CARDS_PER_SUIT)
                                        pk_dslot = 0;
                        }
                }

                /* One pair: compare pair rank, then kicker ladder. */
                if (pk_chrk == HAND_ONE_PAIR) {
                        pk = 0; ph = 0;
                        for (i = 0; i < 5; i++) {
                                if (pk_hrf[i])  pk = pk_ch[i];
                                if (pk_phrf[i]) ph = pk_ph[i];
                        }
                        if (pk % CARDS_PER_SUIT > ph % CARDS_PER_SUIT) {
                                pk_dslot = 0;
                        } else if (pk % CARDS_PER_SUIT == ph % CARDS_PER_SUIT) {
                                for (i = 4; i >= 0; i--) {
                                        if (pk_hsf[i] % CARDS_PER_SUIT >
                                            pk_phsf[i] % CARDS_PER_SUIT) {
                                                pk_dslot = 0; break;
                                        }
                                        if (pk_hsf[i] % CARDS_PER_SUIT <
                                            pk_phsf[i] % CARDS_PER_SUIT) {
                                                pk_dslot = 1; break;
                                        }
                                }
                        }
                }

                /* High card: pure kicker ladder from top down. */
                if (pk_chrk == HAND_HIGH_CARD) {
                        for (i = 4; i >= 0; i--) {
                                if (pk_hsf[i] % CARDS_PER_SUIT >
                                    pk_phsf[i] % CARDS_PER_SUIT) {
                                        pk_dslot = 0; break;
                                }
                                if (pk_hsf[i] % CARDS_PER_SUIT <
                                    pk_phsf[i] % CARDS_PER_SUIT) {
                                        pk_dslot = 1; break;
                                }
                        }
                }
        }

        pk_round = 1;

        if (pk_dslot == 0) {
                pk_pmsg("I win!!!");
                for (br = 0; br < 10; br++) {
                        gameTick(2);
                        if (br & 1) {
                                for (i = 0; i < 5; i++)
                                        pk_drcs(CARD_HIGHLIGHT, i, 0);
                        } else {
                                for (i = 0; i < 5; i++)
                                        pk_drcs(pk_ch[i], i, 0);
                        }
                }
                for (i = 0; i < 5; i++)
                        pk_drcs(pk_ch[i], i, 0);
                pk_annr(0);
        }
        if (pk_dslot == 1) {
                pk_pmsg("You're so lucky!!!");
                for (br = 0; br < 10; br++) {
                        gameTick(2);
                        if (br & 1) {
                                for (i = 0; i < 5; i++)
                                        pk_drcs(CARD_HIGHLIGHT, i, 1);
                        } else {
                                for (i = 0; i < 5; i++)
                                        pk_drcs(pk_ph[i], i, 1);
                        }
                }
                for (i = 0; i < 5; i++)
                        pk_drcs(pk_ph[i], i, 1);
                pk_annr(1);
        }
}

/* pk_cace: should the computer open?  Bluffing -> yes (0).
   Otherwise "Jacks or better" -- returns best rank >= Q, else -1. */

static short
pk_cace()
{
        /* The bare `return;` on the success path is deliberate: the
           last comparison leaves pk_ch[best] % 13 in the return
           register, which is what the caller reads.  The original has
           no explicit value there. */
        short   i;
        short   best;

        if (pk_bluff == NO && pk_chrk == HAND_HIGH_CARD) {
                for (best = 0, i = 0; i < 5; i++) {
                        if (pk_ch[i] % CARDS_PER_SUIT > pk_ch[best] % CARDS_PER_SUIT)
                                best = i;
                }
                if (pk_ch[best] % CARDS_PER_SUIT < CARD_RANK_ACE)
                        return -1;
                return;
        }
        return 0;
}

/* pk_blf: 1/15 chance of bluff when hand rank < 2.  Sets pk_bluff. */

static void
pk_blf()
{
        /* No local: the roll is tested in place. */
        pk_bluff = NO;
        if (rndRng(0, 14) == 0 && pk_chrk <= HAND_ONE_PAIR)
                pk_bluff = YES;
}

/* pk_cdrw: computer AI draw phase.
   Discard count by rank: 0->4, 1->3, 2->1, 3->2, >=4->stay.
   Bluffing: 0..2 discards from non-rank cards.  Re-draws unique
   replacements and animates the swap. */

#include "dat_games4.c"

static void
pk_cdrw()
{
        /* Five locals, counter first; the order must stay. */
        short   i;
        short   nc;                  /* card_in_use flag */
        short   dm;                  /* inner counter / scratch */
        short   card;
        short   n;                   /* new_card */

        for (i = 0; i < 5; i++)
                pk_sel[i] = 0;
        pk_evh(pk_ch, pk_hrf, pk_hsf, &pk_chrk);
        pk_blf();

        if (pk_bluff != NO) {
                nc = rndRng(0, 2);
                i  = nc;
                for (card = 0; card < 5; card++) {
                        if (i == 0)
                                break;
                        if (pk_hrf[card] == 0) {
                                pk_sel[card] = 1;
                                i--;
                        }
                }
        } else {
                if (pk_chrk >= HAND_STRAIGHT) {
                        nc = 0;
                } else {
                        if (pk_chrk == HAND_THREE_OF_A_KIND) {
                                nc = 2;
                                for (i = 0; i < 5; i++)
                                        if (pk_hrf[i] == 0)
                                                pk_sel[i] = 1;
                        } else if (pk_chrk == HAND_TWO_PAIR) {
                                nc = 1;
                                for (i = 0; i < 5; i++)
                                        if (pk_hrf[i] == 0)
                                                pk_sel[i] = 1;
                        } else if (pk_chrk == HAND_ONE_PAIR) {
                                nc = 3;
                                for (i = 0; i < 5; i++)
                                        if (pk_hrf[i] == 0)
                                                pk_sel[i] = 1;
                        } else if (pk_chrk == HAND_HIGH_CARD) {
                                nc = 4;
                                for (card = 0, i = 0; i < 5; i++) {
                                        if (pk_ch[i] % CARDS_PER_SUIT > pk_ch[card] % CARDS_PER_SUIT)
                                                card = i;
                                }
                                for (i = 0; i < 5; i++)
                                        if (i != card)
                                                pk_sel[i] = 1;
                        }
                        /* No final else: ranks 0..3 are all covered,
                           and >= 4 already cleared nc. */
                }
        }

        if (nc == 0) {
                pk_pmsg("I'll stay!");
                gameTick(8);
                return;
        }

        pk_tcm[10] = nc + '0';
        if (nc == 1) {
                pk_tcm[16] = '.';
                pk_tcm[17] = '\0';
        } else {
                pk_tcm[16] = 's';
                pk_tcm[17] = '.';
        }
        pk_pmsg(pk_tcm);
        gameTick(8);

        for (i = 0; i < 5; i++) {
                if (pk_sel[i] == 1) {
                        dm = YES;
                        while (dm != NO) {
                                card = rndRng(0, 51);
                                dm   = NO;
                                for (n = 0; n < 5; n++) {
                                        if (pk_ch[n] == card) dm = YES;
                                        if (pk_ph[n] == card) dm = YES;
                                }
                                n = pk_disc;
                                while (n--) {
                                        if (pk_dpile[n] == card) dm = YES;
                                }
                                pk_dpile[pk_disc] = pk_ch[i];
                                pk_disc++;
                                pk_ch[i] = card;
                                pk_drcs(CARD_HIGHLIGHT, i, 0);
                                gameTick(3);
                        }
                }
        }
        for (i = 0; i < 5; i++) {
                if (pk_sel[i] == 1) {
                        pk_drcs(CARD_BACK, i, 0);
                        gameTick(1);
                }
        }
}

/* pk_annr: transfer pot to winner one chip per tick
   (winner=0 -> computer, winner=1 -> player). */

void
pk_annr(winner)
short   winner;
{
        /* The pot decrement is the loop condition itself -- `while
           (n--)` loads the value, subtracts straight to memory and
           tests the OLD copy -- so it runs one past zero and the tail
           assignment puts the pot back to 0. */
        while (g_ppppa--) {
                if (winner == 0) {
                        g_pcmon++;
                        pk_awp();
                        pk_dpot();
                        gameTick(0);
                } else {
                        g_ppmon++;
                        pk_dppm();
                        pk_dpot();
                        gameTick(0);
                }
        }
        g_ppppa = 0;
}

/* pk_cbet: player betting UI: F1 Bet (hold), F3 Enter, F5 Pass/Clr.
   Returns 0 normally, -1 on timeout. */

static short
pk_cbet(str)
char *  str;
{
        /* Two locals: the key and a flag that ends the first prompt
           loop; the second loop is a plain while (1). */
        short   r;
        short   go;

        pk_bet  = 0;
        pk_pass = NO;
        pk_pmsg(str);
        plEr(225, 10, 319, 60);
        strPr("F1 Bet",       225, 18, COLOR_red);
        strPr("F3 Enter",     225, 26, COLOR_red);
        strPr("F5 Pass/Clr", 225, 34, COLOR_red);
        go = 0;
        while (!go) {
                r = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                if (r == PK_IN_TIMEOUT)
                        return -1;
                if (r == PK_IN_ARG_C) {
                        pk_pass = YES;
                        return 0;
                }
                if (r == PK_IN_ARG_A) {
                        if (g_ppmon == 0)
                                return -1;
                        pk_ddec(1, 1);
                        go = 1;
                        break;
                }
        }
        while (1) {
                r = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                if (r == PK_IN_TIMEOUT)
                        return -1;
                if (r == PK_IN_ARG_B && pk_bet != 0)
                        return 0;
                if (r == PK_IN_ARG_A)
                        pk_ddec(1, 1);
                if (r == PK_IN_ARG_C) {
                        if (pk_bet == 0) {
                                pk_pass = YES;
                                return 0;
                        }
                        g_ppmon += pk_bet;
                        g_ppppa -= pk_bet;
                        pk_bet   = 0;
                        pk_dppm();
                        pk_dpot();
                }
        }
}

/* pk_ddec: animated chip transfer.  who=0 computer / 1 player.
   Caps pk_bet at 20. */

static void
pk_ddec(who, n)
short   who;
short   n;
{
        /* No local: the count is decremented in the loop condition
           (the argument itself), and the two side branches are
           written out in order. */
        if (pk_bet == 20)
                return;
        while (n--) {
                if (who == 0 && g_pcmon == 0)
                        return;
                if (who == 1 && g_ppmon == 0)
                        return;
                if (who == 0) {
                        g_pcmon--;
                        pk_awp();
                        g_ppppa++;
                        pk_dpot();
                        pk_bet++;
                }
                if (who == 1) {
                        g_ppmon--;
                        pk_dppm();
                        g_ppppa++;
                        pk_dpot();
                        pk_bet++;
                }
        }
}

/* pk_evhs: deal 5-card hands.  Draws 10 unique random cards; player
   face-up, computer face-down. */

static void
pk_evhs()
{
        /* Declaration order must stay: counter first, then the drawn
           card, the inner counter and the duplicate flag last. */
        short   i;
        short   c;
        short   j;
        short   dup;

        for (i = 0; i < 5; i++) {
                pk_ch[i] = CARD_NONE;
                pk_ph[i] = CARD_NONE;
        }
        for (i = 0; i < 5; i++) {
                dup = YES;
                while (dup != NO) {
                        c   = rndRng(0, 51);
                        dup = NO;
                        for (j = 0; j < 5; j++) {
                                if (pk_ch[j] == c || pk_ph[j] == c)
                                        dup = YES;
                        }
                }
                pk_ch[i] = c;
                dup = YES;
                while (dup != NO) {
                        c   = rndRng(0, 51);
                        dup = NO;
                        for (j = 0; j < 5; j++) {
                                if (pk_ch[j] == c || pk_ph[j] == c)
                                        dup = YES;
                        }
                }
                pk_ph[i] = c;
        }
        plEr(70, 10, 219, 62);
        for (i = 0; i < 5; i++) {
                pk_drcs(pk_ph[i], i, 1);
                gameTick(3);
                pk_drcs(CARD_BACK, i, 0);
                gameTick(3);
        }
}

/* pk_drcs: blit one card sprite (15x23) at slot xi of row yi.
   card=CARD_BACK selects crd_mfdb[52]; 0..51 index directly. */

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
        vroCpyD(vdihnd, S_ONLY,
                              (long) &crd_mfdb[card], (long) &mf_scb_c,
                              0, 0, 15, 23,
                              x, y, x + 15, y + 23);
}

/* pk_ldCrd (in parts/) must sit here, ahead of pk_awp. */
#include "parts/pk_ldCrd.c"

/* pk_inph: wait for one of F-keys a/b/c or digits 1..5.
   Returns 1..8 for a/b/c/1/2/3/4/5, or -1 on timeout. */

short
pk_inph(a, b, c)
short   a;
short   b;
short   c;
{
        short   ch;

        while (1) {
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

/* pk_awp: display computer money count in the top-left panel, as
   3 hand-formatted, space-padded digits.  The assignments nested in
   expressions and the spare str[] cells are the original's shape. */

void
pk_awp()
{
        char    str[10];
        short   rem;

        plEr(5, 10, 31, 20);
        str[3] = '\0';
        str[0] = (str[8] = g_pcmon / 100) + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (rem = g_pcmon % 100) / 10;
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = rem % 10;
        str[2] = str[4] + '0';
        strPr(str, 5, 18, COLOR_black);
}

/* pk_dppm: display player money (same 3-digit format as pk_awp). */

void
pk_dppm()
{
        char    str[10];
        short   rem;

        plEr(5, 50, 31, 60);
        str[3] = '\0';
        str[0] = (str[8] = g_ppmon / 100) + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (rem = g_ppmon % 100) / 10;
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = rem % 10;
        str[2] = str[4] + '0';
        strPr(str, 5, 58, COLOR_black);
}

/* pk_dpot: display the pot amount in the middle panel. */

void
pk_dpot()
{
        char    str[10];
        short   rem;

        plEr(31, 30, 57, 40);
        str[3] = '\0';
        str[0] = (str[8] = g_ppppa / 100) + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (rem = g_ppppa % 100) / 10;
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = rem % 10;
        str[2] = str[4] + '0';
        strPr(str, 31, 38, COLOR_black);
}

/* pk_ante: opening prompt "Ante up to play." + F1 Ante / F10 Quit.
   On F1: both players contribute 1 chip.  On F10/timeout: sets pk_quit. */

static void
pk_ante()
{
        short   r;

        g_ppppa = 0;
        plEr(225, 10, 319, 60);
        strPr("F1  Ante", 225, 18, COLOR_red);
        strPr("F10 Quit", 225, 34, COLOR_red);
        pk_pmsg("Ante up to play.");
        r = 0;
        pk_quit = NO;
        while (r != PK_IN_ARG_A && r != PK_IN_ARG_C && r != PK_IN_TIMEOUT)
                r = pk_inph(KEY_F1, PK_IN_UNUSED, KEY_F10);
        if (r == PK_IN_ARG_C || r == PK_IN_TIMEOUT) {
                pk_quit = YES;
                return;
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
                g_ppmon--;
                pk_dppm();
                g_ppppa++;
                pk_dpot();
                g_pcmon--;
                pk_awp();
                g_ppppa++;
                pk_dpot();
        }
}

/* pk_pmsg: print a green status message in the bottom info bar. */

void
pk_pmsg(str)
char *  str;
{
        plEr(5, 63, 319, 75);
        strPr(str, 5, 71, COLOR_green);
}

/* pk_rmch: pop card from top of `pile`; shift remaining entries down.
   Returns -1 if empty (a plain -1, not CARD_NONE). */

short
pk_rmch(pile, count)
short * pile;
short * count;
{
        /* `card` and `i` are followed by two more declared shorts the
           body never touches.  They must stay: removing them changes
           the compiled code. */
        short   card;
        short   i;
        short   unused1;
        short   unused2;

        if (*count == 0)
                return -1;
        card    = *pile;
        /* The count is decremented in place with an early return when
           the pile is emptied, so `card` is returned from two places,
           as in the original. */
        if (--*count == 0)
                return card;
        for (i = 0; i < 51; i++)
                *(pile + i) = *(pile + i + 1);
        return card;
}

/* pk_actd: append val at pile[*idx]; increment idx. */

void
pk_actd(pile, idx, val)
short * pile;
short * idx;
short   val;
{
        pile[*idx] = val;
        (*idx)++;
}

static void     pk_show();


/* pk_wrMn: WAR mini-game main loop.
   Init: Malloc, load cards, mg_stp, 400-swap shuffle, split 26/26.
   Per-round: reveal cards, compare mod-13, resolve win/loss/tie. */

void
pk_wrMn()
{
        /* Seven locals: ikey and cidx double as the shuffle's and the
           deal loop's indices, and ikey also carries the war round's
           result. */
        short   ikey;
        short   cidx;
        short   j;
        short   t;
        char *  sp;
        short   saved_head_frame;
        short   saved_head_mode;

        crd_dat = (short *) Malloc(10400L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCrd();
        mg_stp();

        g_pcmon = 26;
        g_ppmon = 26;
        g_ppppa = 0;

        /* Deck 0..51 then Fisher-Yates-lite 400-swap shuffle. */
        for (ikey = 0; ikey < 52; ikey++)
                pk_dsc[ikey] = ikey;
        j = 400;
        while (j--) {
                ikey = rndRng(0, 51);
                do {
                        cidx = rndRng(0, 51);
                } while (ikey == cidx);
                t = pk_dsc[cidx];
                pk_dsc[cidx] = pk_dsc[ikey];
                pk_dsc[ikey] = t;
        }
        ikey = 0;
        for (cidx = 0; ikey < 52; cidx++) {
                g_pcdrp[cidx] = pk_dsc[ikey];
                ikey++;
                g_ppdrp[cidx] = pk_dsc[ikey];
                ikey++;
        }

        pk_awp();
        pk_dppm();
        pk_dpot();

        /* Per-round loop: a label and explicit gotos, not a for(;;)
           -- every round-end branches straight back here rather than
           to a loop-bottom edge, as in the original.
           1985 bug: the bare `pk_dppm;` is a call whose parentheses
           were left off, so Alcyon just loads its address and drops
           it.  Kept on purpose. */
round:
                pk_awp();
                pk_dppm;
                pk_dpot();
                plEr(5, 63, 319, 75);
                plEr(225, 10, 319, 60);
                plEr(70, 10, 219, 62);

                /* Every exit is a goto: the two message blocks and
                   the cleanup are labels the war round jumps back
                   into, and the cleanup returns. */
                if (g_pcmon == 0) {
out_of_cards:
                        pk_pmsg("I'm out of cards! You're too good!");
                        gameTick(0x14);
cleanup:
                        tx_sctm  = 0;
                        no_keyin = NO;
                        Mfree(crd_dat);
                        moff();
                        return;
                }
                if (g_ppmon == 0) {
no_cards:
                        pk_pmsg("No cards, huh? Better luck next time.");
                        gameTick(0x14);
                        goto cleanup;
                }

                gameTick(5);
                pk_pwc[0] = pk_rmch(g_ppdrp, &g_ppmon);
                g_ppppa++;
                pk_drcs(CARD_BACK, 0, 1);
                pk_dpot();
                pk_dppm();
                gameTick(3);
                pk_cwc[0] = pk_rmch(g_pcdrp, &g_pcmon);
                g_ppppa++;
                pk_drcs(pk_cwc[0], 0, 0);
                pk_dpot();
                pk_awp();

                pk_pmsg("Show me your card, Ace.");
                strPr("F1  Show", 225, 18, COLOR_red);
                strPr("F10 Quit", 225, 26, COLOR_red);
                ikey = 0;
                while (ikey != PK_IN_ARG_A && ikey != PK_IN_ARG_B)
                        ikey = pk_inph(KEY_F1, KEY_F10, PK_IN_UNUSED);
                if (ikey == PK_IN_ARG_B)
                        goto cleanup;

                pk_drcs(pk_pwc[0], 0, 1);
                plEr(225, 10, 319, 60);
                gameTick(5);

                /* Both ranks land in locals before the compare, and
                   the loser's branch recomputes them the other way
                   round. */
                if ((ikey = pk_pwc[0] % CARDS_PER_SUIT) > (cidx = pk_cwc[0] % CARDS_PER_SUIT)) {
                        /* Player wins. */
                        if (ikey == CARD_RANK_ACE) {
                                sp = "Ace? I don't believe it!";
                        } else {
                                switch (rndRng(1, 6)) {
                                case 1: sp = "You're awfully lucky!";       break;
                                case 2: sp = "Arrghh!";                        break;
                                case 3: sp = "You're tough.";                 break;
                                case 4: sp = "I'll get you next time.";     break;
                                case 5: sp = "Dog-gone it.";                   break;
                                case 6: sp = "All right. Slow down.";       break;
                                }
                        }
                        pk_pmsg(sp);
                        gameTick(8);
                        pk_annr(1);
                        plEr(70, 10, 219, 62);
                        g_ppmon -= 2;
                        pk_actd(g_ppdrp, &g_ppmon, pk_pwc[0]);
                        pk_actd(g_ppdrp, &g_ppmon, pk_cwc[0]);
                        goto round;
                } else if ((ikey = pk_cwc[0] % CARDS_PER_SUIT) > (cidx = pk_pwc[0] % CARDS_PER_SUIT)) {
                        /* Computer wins by margin (ikey - cidx). */
                        if (ikey == CARD_RANK_ACE) {
                                sp = "Ace takes it!";
                        } else if (ikey - cidx <= 2) {
                                if (rndRng(0, 1))
                                        sp = "Whew! That was too close.";
                                else
                                        sp = "Hmm... That's not too bad!";
                        } else if (ikey - cidx >= 7) {
                                switch (rndRng(1, 3)) {
                                case 1: sp = "No contest. You lose!"; break;
                                case 2: sp = "Beat you by a mile.";   break;
                                case 3: sp = "That was easy!";        break;
                                }
                        } else if (cidx <= 3) {
                                if (rndRng(0, 1))
                                        sp = "That's an easy card to beat.";
                                else
                                        sp = "Not a very high card, but I'll take it.";
                        } else if (cidx >= 9) {
                                sp = "Great, a face card, and it's mine now!";
                        } else {
                                switch (rndRng(1, 3)) {
                                case 1: sp = "Alright. I win!";        break;
                                case 2: sp = "Better luck next time."; break;
                                case 3: sp = "Hey... look at that!";   break;
                                }
                        }
                        pk_pmsg(sp);
                        saved_head_frame = g_hsfra;
                        saved_head_mode  = g_hamod;
                        a_peeka();
                        g_hamod = saved_head_mode;
                        gameTick(8);
                        pk_annr(0);
                        plEr(70, 10, 219, 62);
                        g_pcmon -= 2;
                        pk_actd(g_pcdrp, &g_pcmon, pk_pwc[0]);
                        pk_actd(g_pcdrp, &g_pcmon, pk_cwc[0]);
                        g_hsfra = saved_head_frame;
                        goto round;
                } else {
                        /* Tie -> war round. */
                        ikey = pk_bjwr();
                        if (mg_tofl != NO)
                                goto cleanup;
                        if (ikey == -1)
                                goto out_of_cards;
                        if (ikey == -2)
                                goto no_cards;
                        goto round;
                }
}

/* pk_bjwr: nested war round.  Draw 3 face-down + 1 face-up each.
   On tie, loops with g_pchc++.
   Returns 0 = normal, -1 = computer out / user quit, -2 = player out. */

static short
pk_bjwr()
{
        /* Declaration order and the unused local must stay: removing
           or reordering them changes the compiled code. */
        short   idx;
        short   drawn;
        short   pot;
        short   prank;
        short   crank;
        short   unused;

        g_pchc = 0;
        for (idx = 1; idx < 52; idx++) {
                pk_cwc[idx] = -1;
                pk_pwc[idx] = -1;
        }
        for (;;) {
                pk_pmsg("... WAR!! ...");
                gameTick(10);
                if (g_pcmon == 0)
                        return -1;
                if (g_ppmon == 0)
                        return -2;

                for (idx = 1; idx < 4; idx++) {
                        if (g_ppmon == 1)
                                break;
                        if (g_pcmon == 1)
                                break;
                        drawn = pk_rmch(g_ppdrp, &g_ppmon);
                        pk_pwc[g_pchc * 4 + idx] = drawn;
                        g_ppppa++;
                        pk_drcs(CARD_BACK, idx, 1);
                        pk_dpot();
                        pk_dppm();
                        gameTick(3);
                        drawn = pk_rmch(g_pcdrp, &g_pcmon);
                        pk_cwc[g_pchc * 4 + idx] = drawn;
                        g_ppppa++;
                        pk_drcs(CARD_BACK, idx, 0);
                        pk_dpot();
                        pk_awp();
                        gameTick(3);
                }

                /* Final face-up card each. */
                drawn = pk_rmch(g_ppdrp, &g_ppmon);
                pk_pwc[g_pchc * 4 + idx] = drawn;
                g_ppppa++;
                pk_drcs(CARD_BACK, idx, 1);
                pk_dpot();
                pk_dppm();
                gameTick(3);
                drawn = pk_rmch(g_pcdrp, &g_pcmon);
                pk_cwc[g_pchc * 4 + idx] = drawn;
                g_ppppa++;
                pk_drcs(drawn, idx, 0);
                pk_dpot();
                pk_awp();
                gameTick(3);

                pk_pmsg("Let's see what you've got...");
                strPr("F1 Show", 225, 18, COLOR_red);
                while (pk_inph(KEY_F1, PK_IN_UNUSED, PK_IN_UNUSED) != PK_IN_ARG_A) {
                        if (mg_tofl != NO)
                                return -1;
                }
                pk_drcs(pk_pwc[g_pchc * 4 + idx], idx, 1);
                plEr(225, 10, 319, 60);
                gameTick(5);

                if ((prank = pk_pwc[g_pchc * 4 + idx] % CARDS_PER_SUIT) >
                    (crank = pk_cwc[g_pchc * 4 + idx] % CARDS_PER_SUIT)) {
                        /* Player wins the war round. */
                        pk_pmsg("You win the war!!!");
                        gameTick(8);
                        while (--idx) {
                                pk_drcs(pk_cwc[g_pchc * 4 + idx], idx, 0);
                                gameTick(1);
                        }
                        gameTick(10);
                        pot = g_ppppa;
                        pk_annr(1);
                        g_ppmon -= pot;
                        for (idx = 0; pk_cwc[idx] != -1; idx++) {
                                pk_actd(g_ppdrp, &g_ppmon, pk_pwc[idx]);
                                pk_actd(g_ppdrp, &g_ppmon, pk_cwc[idx]);
                        }
                        return 0;
                }
                if ((prank = pk_pwc[g_pchc * 4 + idx] % CARDS_PER_SUIT) <
                    (crank = pk_cwc[g_pchc * 4 + idx] % CARDS_PER_SUIT)) {
                        /* Computer wins the war round. */
                        pk_pmsg("I win the war!!!");
                        gameTick(8);
                        while (--idx) {
                                pk_drcs(pk_pwc[g_pchc * 4 + idx], idx, 1);
                                gameTick(1);
                        }
                        gameTick(10);
                        pot = g_ppppa;
                        pk_annr(0);
                        g_pcmon -= pot;
                        for (idx = 0; pk_cwc[idx] != -1; idx++) {
                                pk_actd(g_pcdrp, &g_pcmon, pk_pwc[idx]);
                                pk_actd(g_pcdrp, &g_pcmon, pk_cwc[idx]);
                        }
                        return 0;
                }
                g_pchc++;
        }
}

/* pk_bjMn: BLACKJACK main game loop.
   Bet-entry (F1 add, F3 enter, F5 clear, 20 cap), deal, natural check,
   optional split, double-down, hit/stand rounds, dealer plays, settle.
   The labels and gotos are the original's control flow and must stay.
   Alcyon 8-char link-name truncation prevents a body/wrapper split. */

void
pk_bjMn()
{
        /* Ten locals; four of them are never referenced and the last
           is written once and never read.  All must stay, in this
           order: removing them changes the compiled code. */
        short   br;             /* also every countdown */
        short   hit;
        short   unused1;
        short   unused2;
        short   unused3;
        short   unused4;
        short   res;
        short   rv;
        short   round_ctr;
        short   phase_snap;     /* written once, never read */

        crd_dat = (short *) Malloc(0x28a0L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCrd();
        mg_stp();
        g_pcmon = 400;
        g_ppmon = 400;
        pk_awp();
        pk_dppm();

        /* A label and gotos, not a loop statement: the round tick sits
           at the top and is skipped on the first pass. */
        goto round;
next_round:
        gameTick(0x18);
round:
                plEr(70, 10, 219, 62);
                plEr(31, 43, 57, 53);
                g_pcbet = 0;
                g_ppbet = 0;
                pk_phase = 0;
                plEr(225, 10, 319, 60);
                strPr("F1  Bet",  225, 18, COLOR_red);
                strPr("F10 Quit", 225, 34, COLOR_red);
                pk_pmsg("What's your bet?");
                bj_key  = 0;
                pk_quit = NO;
                while (bj_key != PK_IN_ARG_A && bj_key != PK_IN_ARG_C)
                        bj_key = pk_inph(KEY_F1, PK_IN_UNUSED, KEY_F10);
                if (bj_key == PK_IN_ARG_C) {
cleanup:
                        tx_sctm  = 0;
                        no_keyin = NO;
                        Mfree(crd_dat);
                        moff();
                        return;
                }

                for (br = 0; br < 5; br++) {
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
                g_ppmon--;
                pk_dppm();
                g_pcbet++;
                pk_dbhi(1);
                pk_bet = 1;
                strPr("F3  Enter", 225, 26, COLOR_red);
                strPr("F10 Quit",  225, 34, COLOR_lt_grey);
                strPr("F5  Clear", 225, 34, COLOR_red);

                bj_key = 0;
                while (1) {
                        bj_key = 0;
                        while (bj_key != PK_IN_ARG_A && bj_key != PK_IN_ARG_B &&
                               bj_key != PK_IN_ARG_C && bj_key != PK_IN_TIMEOUT) {
                                gameTick(0);
                                bj_key = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                        }
                        if (mg_tofl != NO) goto cleanup;
                        if (bj_key == PK_IN_ARG_C) {
                                g_ppmon += pk_bet;
                                pk_bet  = 0;
                                g_pcbet = 0;
                                pk_dbhi(1);
                                pk_dppm();
                                pk_dpile[10] = CARD_BJ_STEP;
                                break;
                        }
                        if (bj_key == PK_IN_ARG_A) {
                                if (g_ppmon == 0) {
                                        pk_pmsg("Game's over. I win.");
                                        pk_quit = YES;
                                        break;
                                }
                                if (pk_bet == 20)
                                        continue;
                                g_ppmon--;
                                pk_dppm();
                                g_pcbet++;
                                pk_dbhi(1);
                                pk_bet++;
                        }
                        if (bj_key == PK_IN_ARG_B) {
                                break;
                        }
                }

                if (pk_dpile[10] != CARD_BJ_STOP) {
                        pk_dpile[10] = CARD_BJ_STOP;
                        goto round;
                }
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
                br = pk_cnbj(pk_ph);
                hit  = pk_cnbj(pk_ch);
                if (br != 0 && hit != 0) {
                        pk_pmsg("You have BLACKJACK...but so do I !!");
                        pk_drcs(pk_ch[0], 0, 0);
                        gameTick(0x14);
                        pk_sbet(&g_pcbet, 1, 2);
                        if (pk_quit != NO) {
                                pk_pmsg("Game's over. I win.");
                                gameTick(0x14);
                                goto cleanup;
                        }
                        goto next_round;
                } else if (br != 0) {
                        pk_pmsg("You have BLACKJACK!!");
                        gameTick(0x14);
                        phase_snap = g_pcbet;
                        pk_sbet(&g_pcbet, 1, 1);
                        if (pk_quit != NO) {
                                pk_pmsg("I'm all out!!");
                                gameTick(0x14);
                                goto cleanup;
                        }
                        goto next_round;
                } else if (hit != 0) {
                        pk_pmsg("I have BLACKJACK!!");
                        gameTick(10);
                        pk_drcs(pk_ch[0], 0, 0);
                        gameTick(0x14);
                        pk_pmsg("I win double the bet.");
                        gameTick(0x14);
                        pk_sbet(&g_pcbet, 0, 1);
                        if (pk_quit != NO) {
                                pk_pmsg("Game's over. I win.");
                                gameTick(0x14);
                                goto cleanup;
                        }
                        goto next_round;
                }
                /* Neither had a natural.  Split, double-down,
                   hit/stand, dealer -- the meat of the game. */
                pk_phase = 0;
                if ((short) pk_ph[0] % CARDS_PER_SUIT ==
                    (short) pk_ph[1] % CARDS_PER_SUIT) {
                        pk_pmsg("Do you wish to split?");
                        plEr(225, 10, 319, 60);
                        strPr("F1 Split",    225, 18, COLOR_red);
                        strPr("F3 No split", 225, 26, COLOR_red);
                        bj_key = 0;
                        while (bj_key != PK_IN_ARG_A && bj_key != PK_IN_ARG_B && bj_key != PK_IN_TIMEOUT) {
                                gameTick(0);
                                bj_key = pk_inph(KEY_F1, KEY_F3, PK_IN_UNUSED);
                        }
                        if (mg_tofl != NO) goto cleanup;
                        if (bj_key == PK_IN_ARG_A) {
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
                                if (pk_cnbj(pk_ph)) {
                                        pk_pmsg("You have BLACKJACK!!");
                                        gameTick(0x14);
                                        pk_sbet(&g_pcbet, 1, 1);
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
                                        g_ppmon--;
                                        pk_dppm();
                                        g_ppbet++;
                                        pk_dbhi(2);
                                        gameTick(0);
                                }
                                if (pk_quit != NO) {
                                        pk_pmsg("Sorry, you're all out!!");
                                        gameTick(20);
                                        goto cleanup;
                                }
                                if (pk_cnbj(pk_psh)) {
                                        pk_pmsg("You have BLACKJACK!!");
                                        gameTick(20);
                                        pk_sbet(&g_ppbet, 1, 1);
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
                if (pk_phase != 0 && pk_c1bj != NO && pk_c2bj != NO) {
                        goto next_round;
                }
                pk_wrf = NO;
                pk_wcs = NO;
                pk_pcc  = CARD_BJ_MAX;
                pk_pscc = CARD_BJ_MAX;
                plEr(225, 10, 319, 60);
                strPr("F1 Double",    225, 18, COLOR_red);
                strPr("F3 No double", 225, 26, COLOR_red);
                if (pk_phase == 0) {
                        if (g_ppmon < g_pcbet) bj_key = 2;
                        else {
                                pk_pmsg("Do you wish to double-down?");
                                bj_key = 0;
                        }
                        while (bj_key != PK_IN_ARG_A && bj_key != PK_IN_ARG_B && bj_key != PK_IN_TIMEOUT) {
                                gameTick(0);
                                bj_key = pk_inph(KEY_F1, KEY_F3, PK_IN_UNUSED);
                        }
                        if (mg_tofl != NO) goto cleanup;
                        if (bj_key == PK_IN_ARG_A) {
                                pk_pcc = CARD_BJ_STEP;
                                br      = g_pcbet;
                                pk_wrf = YES;
                                while (br--) {
                                        if (g_ppmon == 0) {
                                                pk_quit = YES;
                                                break;
                                        }
                                        g_ppmon--;
                                        pk_dppm();
                                        g_pcbet++;
                                        pk_dbhi(1);
                                        gameTick(0);
                                }
                                if (pk_quit != NO) {
                                        pk_pmsg("Game's over. I win.");
                                        gameTick(0x14);
                                        goto cleanup;
                                }
                        }
                        /* Redundant re-test of pk_phase, already
                           implied by the else.  Kept on purpose: it
                           is part of the original code. */
                } else if (pk_phase != 0) {
                        if (pk_c1bj == NO) {
                                if (g_ppmon < g_pcbet) bj_key = 2;
                                else {
                                        pk_pmsg("Double-down on your first hand?");
                                        pk_drcs(pk_ph[0], 0, 1);
                                        pk_drcs(pk_ph[1], 1, 1);
                                        gameTick(0);
                                        bj_key = 0;
                                }
                                while (bj_key != PK_IN_ARG_A && bj_key != PK_IN_ARG_B && bj_key != PK_IN_TIMEOUT) {
                                        gameTick(0);
                                        bj_key = pk_inph(KEY_F1, KEY_F3, PK_IN_UNUSED);
                                }
                                if (mg_tofl != NO) goto cleanup;
                                if (bj_key == PK_IN_ARG_A) {
                                        pk_pcc = CARD_BJ_STEP;
                                        br      = g_pcbet;
                                        pk_wrf = YES;
                                        while (br--) {
                                                if (g_ppmon == 0) {
                                                        pk_quit = YES;
                                                        break;
                                                }
                                                g_ppmon--;
                                                pk_dppm();
                                                g_pcbet++;
                                                pk_dbhi(1);
                                                gameTick(0);
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
                                if (g_ppmon < g_ppbet) bj_key = 2;
                                else {
                                        pk_pmsg("Double-down on your second hand?");
                                        pk_drcs(pk_psh[0], 0, 1);
                                        pk_drcs(pk_psh[1], 1, 1);
                                        gameTick(0);
                                        bj_key = 0;
                                }
                                while (bj_key != PK_IN_ARG_A && bj_key != PK_IN_ARG_B && bj_key != PK_IN_TIMEOUT) {
                                        gameTick(0);
                                        bj_key = pk_inph(KEY_F1, KEY_F3, PK_IN_UNUSED);
                                }
                                if (mg_tofl != NO) goto cleanup;
                                if (bj_key == PK_IN_ARG_A) {
                                        pk_pscc = CARD_BJ_STEP;
                                        pk_wcs  = YES;
                                        br       = g_ppbet;
                                        while (br--) {
                                                if (g_ppmon == 0) {
                                                        pk_quit = YES;
                                                        break;
                                                }
                                                g_ppmon--;
                                                pk_dppm();
                                                g_ppbet++;
                                                pk_dbhi(2);
                                                gameTick(0);
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
                        if (pk_bjr(pk_ph, 1, "Do you want a hit?") == -1) {
                                if (mg_tofl != NO)
                                        goto cleanup;
                                pk_pmsg("You've busted!!!");
                                gameTick(10);
                                while (g_pcbet--) {
                                        g_pcmon++;
                                        pk_awp();
                                        pk_dbhi(1);
                                        gameTick(0);
                                }
                                goto next_round;
                        }
                        plEr(225, 10, 319, 60);
                } else {
                        for (br = 0; br < 5; br++)
                                pk_drcs(CARD_HIGHLIGHT, br, 1);
                        pk_bs1 = NO;
                        pk_bs2 = NO;
                        if (pk_c1bj == NO) {
                                for (br = 0; br < 5; br++) {
                                        if (pk_ph[br] == CARD_NONE)
                                                break;
                                        pk_drcs(pk_ph[br], br, 1);
                                }
                                pk_dbhi(1);
                                if (pk_bjr(pk_ph, 1,
                                                      "Need a hit on your first hand?") == -1) {
                                        if (mg_tofl != NO) goto cleanup;
                                        pk_pmsg("Your first hand is busted !!");
                                        gameTick(20);
                                        pk_bs1 = YES;
                                        while (g_pcbet--) {
                                                g_pcmon++;
                                                pk_awp();
                                                pk_dbhi(1);
                                                gameTick(0);
                                        }
                                }
                        }
                        if (pk_c2bj == NO) {
                                for (br = 0; br < 5; br++)
                                        pk_drcs(CARD_HIGHLIGHT, br, 1);
                                for (br = 0; br < 5; br++) {
                                        if (pk_psh[br] == CARD_NONE)
                                                break;
                                        pk_drcs(pk_psh[br], br, 1);
                                }
                                pk_dbhi(2);
                                if (pk_bjr(pk_psh, 1,
                                                      "Need a hit on your second hand?") == -1) {
                                        if (mg_tofl != NO) goto cleanup;
                                        pk_pmsg("Your second hand is busted!!");
                                        gameTick(0x14);
                                        pk_bs2 = YES;
                                        while (g_ppbet--) {
                                                g_pcmon++;
                                                pk_awp();
                                                pk_dbhi(2);
                                                gameTick(0);
                                        }
                                }
                        }
                }
                plEr(225, 10, 319, 60);

                /* Dealer turn + settle. */
                if (pk_phase != 0 && (pk_bs1 != NO || pk_c1bj != NO) &&
                    (pk_bs2 != NO || pk_c2bj != NO))
                        goto next_round;
                if (pk_phase != 0 && pk_bs1 == NO && pk_c1bj == NO) {
                        for (br = 0; br < 5; br++)
                                pk_drcs(CARD_HIGHLIGHT, br, 1);
                        pk_pmsg("Here is your first hand again.");
                        gameTick(0x14);
                        for (br = 0; br < 5; br++) {
                                if (pk_ph[br] == CARD_NONE)
                                        break;
                                pk_drcs(pk_ph[br], br, 1);
                        }
                        pk_dbhi(1);
                } else if (pk_phase != 0 &&
                           pk_bs2 == NO && pk_c2bj == NO) {
                        for (br = 0; br < 5; br++)
                                pk_drcs(CARD_HIGHLIGHT, br, 1);
                        pk_pmsg("Here is your second hand.");
                        gameTick(0x14);
                        for (br = 0; br < 5; br++) {
                                if (pk_psh[br] == CARD_NONE)
                                        break;
                                pk_drcs(pk_psh[br], br, 1);
                        }
                        pk_dbhi(2);
                }

                pk_pmsg("Now here's my down card.");
                gameTick(10);
                pk_drcs(pk_ch[0], 0, 0);
                gameTick(0x14);

                for (br = 0; br < 3; br++) {
                        pk_cscore = 0;
                        round_ctr = 0;
                        res = pk_chsc(pk_ch, 0);
                        rv  = pk_chsc(pk_ch, 1);
                        if (0x15 < res && 0x15 < rv) {
                                round_ctr = 1;
                                break;
                        }
                        if (rv <= 21)
                                pk_cscore = rv;
                        else
                                pk_cscore = res;
                        if (pk_cscore >= 17) {
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
                        rv  = pk_chsc(pk_ch, 1);
                        pk_cscore = res;
                        if (res > 21)
                                round_ctr = 1;
                        else if (rv <= 21)
                                pk_cscore = rv;
                        else {
                                pk_pmsg("I'll stand.");
                                gameTick(0x14);
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
                        if (rv <= 21)
                                pk_pscore = rv;
                        else
                                pk_pscore = res;
                        if (round_ctr != 0 || pk_cscore < pk_pscore) {
                                pk_pmsg("You win.");
                                gameTick(0x14);
                                pk_sbet(&g_pcbet, 1, 0);
                        } else if (pk_cscore == pk_pscore) {
                                pk_pmsg("It's a tie and nobody wins.");
                                gameTick(0x14);
                                pk_sbet(&g_pcbet, 1, 2);
                        } else {
                                pk_pmsg("I win.");
                                gameTick(0x14);
                                pk_sbet(&g_pcbet, 0, 0);
                        }
                        goto next_round;
                } else {
                        if (pk_bs1 == NO && pk_c1bj == NO) {
                                for (br = 0; br < 5; br++)
                                        pk_drcs(CARD_HIGHLIGHT, br, 1);
                                for (br = 0; br < 5; br++) {
                                        if (pk_ph[br] == CARD_NONE)
                                                break;
                                        pk_drcs(pk_ph[br], br, 1);
                                }
                                pk_dbhi(1);
                                res = pk_chsc(pk_ph, 0);
                                rv  = pk_chsc(pk_ph, 1);
                                if (rv <= 21)
                                        pk_pscore = rv;
                                else
                                        pk_pscore = res;
                                if (round_ctr != 0 || pk_cscore < pk_pscore) {
                                                pk_pmsg("You win with your first hand.");
                                                gameTick(0x14);
                                                pk_sbet(&g_pcbet, 1, 0);

                                } else if (pk_cscore == pk_pscore) {
                                                pk_pmsg("First hand ties, nobody wins.");
                                                gameTick(0x14);
                                                pk_sbet(&g_pcbet, 1, 2);

                                } else {
                                                pk_pmsg("Your first hand loses.");
                                                gameTick(0x14);
                                                pk_sbet(&g_pcbet, 0, 0);

                                }
                        }
                        if (pk_bs2 == NO && pk_c2bj == NO) {
                                for (br = 0; br < 5; br++)
                                        pk_drcs(CARD_HIGHLIGHT, br, 1);
                                for (br = 0; br < 5; br++) {
                                        if (pk_psh[br] == CARD_NONE)
                                                break;
                                        pk_drcs(pk_psh[br], br, 1);
                                }
                                pk_dbhi(2);
                                res = pk_chsc(pk_psh, 0);
                                rv  = pk_chsc(pk_psh, 1);
                                if (rv <= 21)
                                        pk_pscore = rv;
                                else
                                        pk_pscore = res;
                                if (round_ctr != 0 || pk_cscore < pk_pscore) {
                                                pk_pmsg("You win with your second hand.");
                                                gameTick(0x14);
                                                pk_sbet(&g_ppbet, 1, 0);

                                } else if (pk_cscore == pk_pscore) {
                                                pk_pmsg("Second hand ties, nobody wins.");
                                                gameTick(0x14);
                                                pk_sbet(&g_ppbet, 1, 2);

                                } else {
                                                pk_pmsg("Your second hand loses.");
                                                gameTick(0x14);
                                                pk_sbet(&g_ppbet, 0, 0);

                                }
                        }
                }
        goto next_round;
}

/* pk_chsc: blackjack card value.  ace_mode=0 all aces=1; ace_mode=1
   one ace=11 (soft), rest=1.  Called mode 0 then 1 to pick better
   score without busting.  Rank 12=Ace, 6..11=10, 0..5=rank+2. */

static short
pk_chsc(hand, ace_mode)
short * hand;
short   ace_mode;
{
        /* Counter first; the scan terminates inside the loop body. */
        short   i;
        short   ace_high;
        short   score;

        ace_high = NO;
        score    = 0;
        for (i = 0; i < 5; i++) {
                if (hand[i] == CARD_NONE)
                        return score;
                if (hand[i] % CARDS_PER_SUIT == CARD_RANK_ACE) {
                        if (ace_mode == 0)
                                score++;
                        else if (ace_high == NO) {
                                score += 11;
                                ace_high = YES;
                        } else
                                score++;
                } else if (hand[i] % CARDS_PER_SUIT <= CARD_RANK_KING && hand[i] % CARDS_PER_SUIT >= CARD_RANK_10) {
                        score += 10;
                } else {
                        score += hand[i] % CARDS_PER_SUIT + 2;
                }
        }
        return score;
}

/* pk_bjr: play one blackjack round for `hand` at row.
   pk_wrf/pk_wcs forced-single-hit modes auto-deal one card + return.
   Otherwise F1 Hit / F3 Stand.  Returns 0 on stand, -1 on bust/timeout. */

static short
pk_bjr(hand, row, prompt)
short * hand;
short   row;
char *  prompt;
{
        /* Seven declarations, two of them never referenced; they must
           stay, in this order, or the compiled code changes.  cnt_ptr
           is deliberately not initialised, as in the original. */
        short   i;
        short   unused1;
        short   j;
        short   unused2;
        short   score;
        short * cnt_ptr;
        short   forced;

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
        for (i = 0; hand[i] != CARD_NONE; i++)
                pk_drcs(hand[i], i, row);

        if (forced == NO)
                pk_pmsg(prompt);
        else
                pk_pmsg("Here's your card.");
        if (forced != NO) {
                gameTick(0x10);
                (*cnt_ptr)--;
                pk_dchd(hand, 0);
                score = 0;
                for (j = 0; j < 5; j++) {
                        if (hand[j] == CARD_NONE)
                                break;
                        if ((short) hand[j] % CARDS_PER_SUIT == CARD_RANK_ACE)
                                score++;
                        else if ((short) hand[j] % CARDS_PER_SUIT <= CARD_RANK_KING &&
                                 (short) hand[j] % CARDS_PER_SUIT >= CARD_RANK_10)
                                score += 10;
                        else
                                score += hand[j] % CARDS_PER_SUIT + 2;
                }
                if (score > 21)
                        return -1;
                else
                        return 0;
        } else while (1) {
                bj_key = 0;
                while (bj_key != PK_IN_ARG_A && bj_key != PK_IN_ARG_B && bj_key != PK_IN_TIMEOUT) {
                        gameTick(0);
                        bj_key = pk_inph(KEY_F1, KEY_F3, PK_IN_UNUSED);
                }
                if (mg_tofl != NO)
                        return -1;
                if (bj_key == PK_IN_ARG_B) {
                        pk_pmsg(" ");   /* the F3-stand path just
                                          blanks the message strip */
                        return 0;
                } else if (bj_key == PK_IN_ARG_A) {
                        (*cnt_ptr)--;
                        pk_dchd(hand, 0);
                        score = 0;
                        for (j = 0; j < 5; j++) {
                                if (hand[j] == CARD_NONE)
                                        break;
                                if ((short) hand[j] % CARDS_PER_SUIT == CARD_RANK_ACE)
                                        score++;
                                else if ((short) hand[j] % CARDS_PER_SUIT <= CARD_RANK_KING &&
                                         (short) hand[j] % CARDS_PER_SUIT >= CARD_RANK_10)
                                        score += 10;
                                else
                                        score += hand[j] % CARDS_PER_SUIT + 2;
                        }
                        if (score > 21)
                                return -1;
                }
                if (*cnt_ptr == CARD_BJ_STOP) {
                        pk_pmsg("You cannot take any more cards.");
                        gameTick(0xf);
                        return 0;
                }
        }
}

/* pk_cnbj: check for natural blackjack (Ace + T/J/Q/K in first two). */

static short
pk_cnbj(hand)
short * hand;
{
        short   result;
        short   r0;
        short   r1;

        result = 0;
        r0 = (short) hand[0] % CARDS_PER_SUIT;
        r1 = (short) hand[1] % CARDS_PER_SUIT;
        if (r0 == CARD_RANK_ACE && r1 <= CARD_RANK_KING && r1 >= CARD_RANK_10)
                result = 1;
        else if (r1 == CARD_RANK_ACE && r0 <= CARD_RANK_KING && r0 >= CARD_RANK_10)
                result = 1;
        return result;
}

/* pk_dchd: deal one card into hand at next CARD_NONE slot.
   Rejects dups vs pk_ch/pk_ph/pk_psh.  Returns -1 if full. */

static short
pk_dchd(hand, face_down)
short * hand;
short   face_down;
{
        short   i;
        short   dup;
        short   j;
        short   card;
        short   row;

        for (i = 0; i < 5; i++) {
                if (hand[i] == CARD_NONE)
                        break;
        }
        if (i == 5)
                return -1;

        dup = 1;
        while (dup) {
                card = rndRng(0, 51);
                dup  = 0;
                for (j = 0; j < 5; j++) {
                        if (pk_ch[j]  == card ||
                            pk_ph[j]  == card ||
                            pk_psh[j] == card)
                                dup = 1;
                }
        }
        hand[i] = card;
        if (hand == pk_ch)
                row = 0;
        else
                row = 1;
        if (face_down)
                pk_drcs(CARD_BACK, i, row);
        else
                pk_drcs(card, i, row);
        gameTick(3);
        return 0;
}

/* pk_dbhi: display bet with highlight.  sel=1 -> computer bet, else
   player bet.  3-digit format as pk_awp/dppm/dpot. */

static void
pk_dbhi(sel)
short   sel;
{
        char    hund;
        char    tens;
        char    ones;
        char    str[4];
        short   rem;
        short   val;

        if (sel == 1)
                val = g_pcbet;
        else
                val = g_ppbet;
        plEr(31, 43, 57, 53);
        str[3] = '\0';
        str[0] = (hund = val / 100) + '0';
        if (str[0] == '0')
                str[0] = ' ';
        tens = (rem = val % 100) / 10;
        if (str[0] == ' ' && tens == 0)
                str[1] = ' ';
        else
                str[1] = tens + '0';
        ones = rem % 10;
        str[2] = ones + '0';
        strPr(str, 31, 51, COLOR_black);
}

/* pk_sbet: settle a bet.  winner: 0=computer, 1=player.
   mode: 0=normal, 1=natural blackjack double-collect,
         2=split -- suppress the second (player) transfer.
   Sets pk_quit on mid-transfer bankruptcy. */

static void
pk_sbet(bet_ptr, winner, mode)
short * bet_ptr;
short   winner;
short   mode;
{
        short   orig;
        short   loc8;

        if (winner == 0) {
                orig = *bet_ptr;
                while ((*bet_ptr)--) {
                        g_pcmon++;
                        pk_awp();
                        if (bet_ptr == &g_pcbet)
                                pk_dbhi(1);
                        else
                                pk_dbhi(2);
                        gameTick(0);
                }
                if (mode != 0) {
                        while (orig--) {
                                if (g_ppmon == 0) {
                                        pk_quit = YES;
                                        return;
                                }
                                g_ppmon--;
                                pk_dppm();
                                g_pcmon++;
                                pk_awp();
                                gameTick(0);
                        }
                }
        } else if (winner == 1) {
                orig = *bet_ptr;
                loc8 = *bet_ptr;
                while ((*bet_ptr)--) {
                        g_ppmon++;
                        pk_dppm();
                        if (bet_ptr == &g_pcbet)
                                pk_dbhi(1);
                        else
                                pk_dbhi(2);
                        gameTick(0);
                }
                if (mode != 2) {
                        while (loc8--) {
                                if (g_pcmon == 0) {
                                        pk_quit = YES;
                                        break;
                                }
                                g_pcmon--;
                                pk_awp();
                                g_ppmon++;
                                pk_dppm();
                                gameTick(0);
                        }
                }
                if (mode == 1) {
                        while (orig--) {
                                if (g_pcmon == 0) {
                                        pk_quit = YES;
                                        return;
                                }
                                g_pcmon--;
                                pk_awp();
                                g_ppmon++;
                                pk_dppm();
                                gameTick(0);
                        }
                }
        }
}


