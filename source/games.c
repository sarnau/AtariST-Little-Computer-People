/*
 * games.c -- mini-game entry points + shared setup helpers.
 * addr: mg_stp(), plEr(), ag_main(), wp_main(), pk_main(),
 *       pk_wrMn(), pk_bjMn()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>

#include <vdibind.h>
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

/* LCP_STX defines pk_bjwr AFTER pk_wrMn (0xb784) and reaches it with
   a forward bsr; declaring it keeps the symbol local so as68 can
   shorten the call instead of leaving an external. */
static short pk_bjwr();




/* gamePlWQ is only reached from the ROM banner stubs; the default
   build must not emit it (Alcyon lays a static down regardless). */
#ifdef FAITHFUL
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
#endif

#ifdef FAITHFUL
#include "parts/plEr.c"
#include "parts/mg_stp.c"
#include "parts/vst_h20.c"
#include "parts/rst_vsth.c"
#include "parts/initVdi.c"
#include "parts/exitVdi.c"
#include "parts/gameCln.c"
/* ---- ROM minigame stubs (games.o 0x72ac-0x769a) ----------------
   The ST original has NO playable minigames: each handler loads its
   assets, prints a banner, waits at the table (gamePlWQ), and frees.
   The full minigames below the #else come from the other, larger
   game revision and are the default build. ---------------------- */

/* addr: ag_main() (ROM 0x72ac) */
void
ag_main()
{
        g_agwb = (char *) Malloc(10000L);
        if (g_agwb == (char *) 0)
                er_nomem();
        fr_reac("words", (unsigned char *) g_agwb, 10000);
        mg_stp();
        strPr("***ANAGRAMS***", 5, 8, 0);
        gamePlWQ();
        gameCln(g_agwb);
        g_agwb = (char *) 0;
}

/* addr: wp_main() (ROM 0x732a).  Reuses g_ltlp as the line table. */
void
wp_main()
{
        char *  i;
        short   linecount;

        g_wpdb = (char *) Malloc(2000L);
        if (g_wpdb == (char *) 0)
                er_nomem();
        mg_stp();
        fr_reac("wordpz.txt", (unsigned char *) g_wpdb, 1536);
        i = g_wpdb;
        for (linecount = 0; linecount < 66; linecount = linecount + 1) {
                g_ltlp[linecount] = i;
                do {
                        i = i + 1;
                } while (*i > 31);
                while (*i < ' ')
                        i = i + 1;
        }
        g_wpci = 0;
        strPr("**WORD PUZZLE #  **", 8, 8, 0);
        gamePlWQ();
        gameCln(g_wpdb);
        g_wpdb = (char *) 0;
}

/* addr: pk_main() (ROM 0x740a) */
void
pk_main()
{
        crd_dat = (short *) Malloc(10400L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCr();
        mg_stp();
        g_pcbet = 0;
        g_ppbet = 0;
        g_pcmon = 400;
        g_ppmon = 400;
        g_ppppa = 0;
        strPr("***POKER***", 5, 8, 0);
        gamePlWQ();
        gameCln(crd_dat);
        crd_dat = (short *) 0;
}

/* addr: pk_wrMn() (ROM 0x7498, WAR).  Shuffles a 52-card deck with
   400 random swaps and splits it into the two hands. */
void
pk_wrMn()
{
        short   a;
        short   b;
        short   t;
        short   i;
        short   swaps;
        short   j;

        crd_dat = (short *) Malloc(10400L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCr();
        mg_stp();
        g_pcmon = 26;
        g_ppmon = 26;
        g_ppppa = 0;
        for (i = 0; i < 52; i = i + 1)
                pk_dsc[i] = i;
        swaps = 400;
        while (swaps != 0) {
                a = rndRng(0, 51);
                do {
                        b = rndRng(0, 51);
                } while (a == b);
                t = pk_dsc[b];
                pk_dsc[b] = pk_dsc[a];
                pk_dsc[a] = t;
                swaps = swaps - 1;
        }
        j = 0;
        for (i = 0; i < 52; i = i + 2) {
                pk_ch[j] = pk_dsc[i];
                pk_ph[j] = pk_dsc[i + 1];
                j = j + 1;
        }
        strPr("***WAR***", 5, 8, 0);
        gamePlWQ();
        gameCln(crd_dat);
        crd_dat = (short *) 0;
}



/* addr: pk_bjMn() (ROM 0x761e) */
void
pk_bjMn()
{
        crd_dat = (short *) Malloc(10400L);
        if (crd_dat == (short *) 0)
                er_nomem();
        pk_ldCr();
        mg_stp();
        g_pcmon = 400;
        g_ppmon = 400;
        strPr("***BLACKJACK***", 5, 8, 0);
        gamePlWQ();
        gameCln(crd_dat);
        crd_dat = (short *) 0;
}

#else   /* !FAITHFUL: the kept other-revision minigames */

/* lcp_lgt -> parts/lcp_lgt.c (STX puts it at the head of the
   0xdece object, ahead of sp_sprs). */
#ifdef FAITHFUL
#include "parts/lcp_lgt.c"
#endif

/* lcp_rgt -> parts/lcp_rgt.c (STX puts it at the head of the
   0xdece object, ahead of sp_sprs). */
#ifdef FAITHFUL
#include "parts/lcp_rgt.c"
#endif

/* mg_wkev: wait for a key while processing urgent game events.
   On 7200 idle frames (~15 min) sets mg_tofl=YES and returns KEY_F10.
   addr: minigame_wait_for_key_with_events() */

short
#ifdef FAITHFUL
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
        } while (key != KEY_NONE);

        for (;;) {
                key = getKey();
                if (key != KEY_NONE) {
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

#else   /* STX: link #-8 -- key and idle only; the event id, the drain
           key and the timeout test are all consumed in place. */

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
                if (lcp.thirst_level > 0 && lcp.water_level != 0) {
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

/* rndRng -> parts/rndRng.c: LCP_STX links it here (0x74fc), right
   after mg_wkev, so the minigames reach it with bsr.  random.c keeps
   it for FAITHFUL. */
#ifndef FAITHFUL
#include "parts/rndRng.c"
#endif
#endif


/* ag_matc: character-by-character equality test for two C strings.
   Preserves 1985 shape: walks both strings after mismatch, reports
   final result -- byte-comparable with the original.
   addr: anagram_match_result() */

short
ag_matc(a, b)
char *  a;
char *  b;
{
#ifdef FAITHFUL
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
#else
        /* STX keeps no character temporaries: it compares through the
           post-increments, which makes Alcyon save and restore the
           condition codes around them (move sr,d0 / move d0,ccr).
           Its frame is -8, with an unused short ahead of the flag. */
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
#endif
}
#include "parts/mg_stp.c"    /* 0x759c */
#include "parts/gameCln.c"   /* 0x75c8 */
#include "parts/vst_h20.c"   /* 0x75dc */
#include "parts/rst_vsth.c"  /* 0x761e */
#include "parts/initVdi.c"   /* 0x764e */
#include "parts/exitVdi.c"   /* 0x76d0 */


/* wp_main: WORD PUZZLE main loop.
   Loads wordpz.txt into a 2000-byte buffer, indexes 66 line pointers
   (33 puzzles x {template, solution}).  F1 next / F2 prev (wraps 0..0x20)
   / F5 solve / F10 quit.  Preserves goto LAB_000177ac (next_puzzle)
   and LAB_0001797c (cleanup) verbatim.
   addr: wp_main() (== word_puzzle_main) */

void
wp_main()
{
        /* STX's frame is -18: an unused short ahead of line_index,
           then ONE short reused for both the scanned character and
           the mini-game key, the parse pointer, and four more
           unreferenced bytes. */
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
                /* STX steps once, then runs a plain `while` -- two
                   increment sites, not a do/while's one. */
                parse_ptr++;
                while (*parse_ptr >= ' ')
                        parse_ptr++;
                while (*parse_ptr < ' ')
                        parse_ptr++;
        }

        g_wpci = 0;
        strPr("**WORD PUZZLE #  **", 8, 8, COLOR_black);

        /* STX has no outer loop: `next_puzzle` is a plain label and
           every arm of the key switch jumps back to it explicitly. */
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
        Mfree(g_wpdb);          /* STX does not clear g_wpdb here */
}
#undef key

/* STX links wp_solv immediately after wp_main (0x799e): the
   call from the key switch is a bsr.s. */
/* wp_solv: solve phase.  Per blank: prompt, read A-Z (10-char max),
   Enter confirms, F10 quits.  Then walk solution line, compare
   token-by-token; show wp_succ or wp_fail.  Preserves LAB_00017c4a.
   addr: word_puzzle_solve_phase() */

void
wp_solv()
{
        /* STX's frame is -20: six locals, and ONE of them (`ch`) also
           carries the scanned solution and answer characters, so
           there is no ri/pci/sci/scp. */
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
                        /* The two `(long)` casts are real: they keep
                           the muls.w result long, which puts the array
                           base AFTER the column offset.  Without them
                           Alcyon folds the base in first -- which is
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

/* STX links these immediately after wp_solv: wp_shwm 0x7c78,
   wp_rtmp 0x7cac -- both bsr.s targets from it. */
/* STX places wp_shwm between exitVdi (0x76d0) and the anagram
   helpers (ag_cwda 0x7e9c). */
/* wp_shwm: word-puzzle status message in green at (8,58).
   addr: word_puzzle_show_status_message() */

void
wp_shwm(msg)
char *  msg;
{
        plEr(0, 50, 319, 59);
        strPr(msg, 8, 58, COLOR_green);
}

/* wp_rtmp: render puzzle template with player answers substituted for '@'.
   Word-wraps at col 0x26 (literal) / 0x27 (answer).  Starts cursor at
   (x=1, y=0x28) -- byte-comparable.
   addr: word_puzzle_render_template_with_answers() */

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

/* ag_cswa: clear the left-panel intro/instructions area (5,10)-(160,60).
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

/* ag_cgpa: clear the "Guess #N?" prompt bar (166,50)-(319,65).
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

/* ag_csb: clear the bottom info bar (5,62)-(319,75).
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

/* ag_dwl: display a word in 20px text in right panel at (162,37), 12px pitch.
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
#ifdef FAITHFUL
        for (; *word != '\0'; word = word + 1) {
                prCh((short) *word, x + 162, 37, text_color);
                x = x + 12;
        }
#else
        /* STX steps the pointer inside the body, before the pitch,
           and both steps are memory-direct. */
        while (*word != '\0') {
                prCh((short) *word, x + 162, 37, text_color);
                word++;
                x += 12;
        }
#endif
        rst_vsth();
}

/* ag_sgp -> parts/ag_sgp.c (STX: 0x8052, after ag_intr). */
#ifdef FAITHFUL
#include "parts/ag_sgp.c"
#endif

#ifndef FAITHFUL
#include "parts/ag_sgp.c"   /* STX: 0x8052, after ag_intr */
#endif

/* ag_ssw: pick a random word from the 150-entry dictionary (11 bytes/row),
   copy into g_agscw, scramble 10..20 swaps.  Re-scrambles on identity.
   Plants '\0' at g_agwb row-tail so g_agorw reads as a C string.
   addr: anagram_select_and_scramble_word() */

void
ag_ssw()
{
#ifdef FAITHFUL
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
#else
        /* STX's frame is -18: no `idx`, one counter reused for the
           copy index and the shuffle round, and a local copy of the
           word length that the shuffle reads instead of g_agwol. */
        short   pos;
        short   len;
        short   ia;
        short   ib;
        char    tmp;
        char *  wp;

        g_agorw = g_agwb + rndRng(0, 0x95) * 11;        /* 0..149 */
        pos     = 0;
        for (wp = g_agorw; *wp > ' ' && *wp != '.'; ) {
                /* index first: the base folds into add.l #base,An */
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
#endif
}

/* ag_main: full anagram game loop.  Outer per-word / middle per-guess /
   inner per-keypress.  Labels new_word/validate mirror the two
   goto LAB_00018210 / LAB_00018562 jumps in the 1985 source.
   addr: ag_main() (== anagram_main) */

void
ag_main()
{
        /* STX's frame is -28: twelve shorts, only six of which the
           body touches. */
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

        /* STX runs the round prologue ONCE per word and folds the
           guess-count guard into a `while` condition; the
           "too many guesses" tail lives inside the loop and ends with
           `goto new_word`. */
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
                                /* Braced: a bare `continue` folds into
                                   the conditional branch, braces make
                                   Alcyon emit beq-over-bra. */
                                if (ag_matc(g_agorw, g_agscw) != 0) {
                                        continue;
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
                        } else if (word_complete != NO)
                                goto validate;
                }

validate:
                if (word_complete != NO)
                        goto new_word;
                /* The second test is a bare truthiness test, which
                   is what makes Alcyon reach it with an indexed EA
                   (movea.w idx,a0 / movea.l #base,a1 /
                   tst.b (0,a0,a1.l)); spelling it `!= '\0'` emits the
                   base+add form instead. */
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
                        goto new_word;
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

/* STX orders plEr after the anagram helpers (0x86e0, past ag_intr
   at 0x7f84); see parts/plEr.c. */
#ifndef FAITHFUL
#include "parts/plEr.c"
#include "parts/plErCol.c"   /* 0x871a, right after plEr */
#endif

/* pk_ante: opening prompt "Ante up to play." + F1 Ante / F10 Quit.
   On F1: both players contribute 1 chip.  On F10/timeout: sets pk_quit.
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

/* pk_evh: evaluate a 5-card hand.  *hand_rank <- 0=high card..9=royal flush.
   rank_flags[i]=1 for winning combo cards.  suit_flags: rank-sorted hand copy.
   Preserves Ghidra shape (two goto exits) -- byte-comparable.
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

/* pk_evhs: deal 5-card hands.  Draws 10 unique random cards; player
   face-up, computer face-down.
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

/* pk_blf: 1/15 chance of bluff when hand rank < 2.  Sets pk_bluff.
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

/* pk_cace: should the computer open?  Bluffing -> yes (0).
   Otherwise "Jacks or better" -- returns best rank >= Q, else -1.
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

/* pk_dbet: computer call/raise decision.  Returns 'c' or 'r'.
   On raise: pk_dpos = money/10 clamped [1,20].
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

/* pk_ddec: animated chip transfer.  who=0 computer / 1 player.
   Caps pk_bet at 20.
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

/* pk_cbet: player betting UI: F1 Bet (hold), F3 Enter, F5 Pass/Clr.
   Returns 0 normally, -1 on timeout.
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

/* pk_cdrw: computer AI draw phase.
   Discard count by rank: 0->4, 1->3, 2->1, 3->2, >=4->stay.
   Bluffing: 0..2 discards from non-rank cards.  Re-draws unique
   replacements and animates the swap.
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


/* pk_main: 5-card draw poker main loop.
   Init: Malloc, load cards, mg_stp, money=400 each.
   Per-round: ante, deal, bet, discard/draw, computer draw, final bet,
   showdown.  Preserves goto LAB_00018da0 (cleanup), LAB_00018d72,
   LAB_00019082, LAB_00019514, LAB_00019950 verbatim.
   addr: pk_main() (== poker_main) */

void
pk_main()
{
        /* Six locals: loc8 doubles as the second key variable and as
           the raise countdown; there is no `res`. */
        short   ikey;           /* -2  */
        short   loc8;           /* -4  */
        short   i;              /* -6  */
        short   card;           /* -8  */
        short   dcount;         /* -10 */
        BOOL16  in_use;         /* -12 */

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
                while ((ikey = pk_inph(KEY_F1, KEY_F3, 0)) != 2) {
                        if (ikey == -1)
                                break;
                        for (i = 0; i < 5; i++)
                                if (pk_sel[i] == 1)
                                        break;
                        if (i == 5)
                                strPr("F3 Stay", 225, 26, COLOR_red);
                        else
                                strPr("F3 Stay", 225, 26, COLOR_lt_grey);
                        if (ikey == 1) {
                                for (i = 0; i < 5; i++)
                                        if (pk_sel[i] == 1)
                                                break;
                                if (i == 5)
                                        continue;
                                break;
                        }
                        if (ikey < 4)
                                continue;
                        if (ikey > 8)
                                continue;
                        if (pk_sel[ikey - 4]) {
                                pk_sel[ikey - 4] = 0;
                                pk_drcs(pk_ph[ikey - 4], ikey - 4, 1);
                        } else {
                                pk_sel[ikey - 4] = 1;
                                pk_drcs(CARD_HIGHLIGHT, ikey - 4, 1);
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
                if (ikey == 1) {
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
                if (ikey == 2) {
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
                        if (pk_bluff == NO && pk_chrk == 0) {
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
                                ikey = pk_inph(KEY_F1, 255, KEY_F3);
                                if (ikey == -1) goto cleanup;
                                if (ikey == 3) {
                                        plEr(225, 10, 319, 60);
                                        pk_pmsg("My pot.");
                                        gameTick(8);
                                        pk_annr(0);
                                } else if (ikey == 1) {
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
                                                if (loc8 == -1)
                                                        break;
                                                if (loc8 == 3) {
                                                        pk_show();
                                                        goto next_round;
                                                }
                                                if (loc8 == 1 && g_ppmon != 0) {
                                                        pk_bet = 0;
                                                        pk_ddec(1, 1);
                                                        pk_dpos++;
                                                        break;
                                                }
                                        }
                                        if (mg_tofl != NO) goto cleanup;
                                        while (1) {
                                                loc8 = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                                                if (loc8 == -1)
                                                        break;
                                                if (loc8 == 2)
                                                        break;
                                                if (loc8 == 1) {
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
                        } else {
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
                                        ikey = pk_inph(KEY_F1, 255, KEY_F3);
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
                                                        if (loc8 == -1)
                                                                break;
                                                        if (loc8 == 3) {
                                                                pk_show();
                                                                goto next_round;
                                                        }
                                                        if (loc8 == 1 && g_ppmon != 0) {
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
                                                        if (loc8 == -1)
                                                                break;
                                                        if (loc8 == 2)
                                                                break;
                                                        if (loc8 == 1) {
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
                                        }
                                }
                        }
                }

        goto next_round;
}

/* pk_show: showdown.  Reveal computer hand, evaluate both, walk the
   per-rank tiebreak ladder.  Winner blinks 5x then pk_annr transfers.
   Sets pk_round=1.  Preserves Ghidra tiebreak shape (byte-comparable).
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

/* pk_dbhi: display bet with highlight.  sel=1 -> computer bet, else
   player bet.  3-digit format as pk_awp/dppm/dpot.
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

/* pk_chsc: blackjack card value.  ace_mode=0 all aces=1; ace_mode=1
   one ace=11 (soft), rest=1.  Called mode 0 then 1 to pick better
   score without busting.  Rank 12=Ace, 6..11=10, 0..5=rank+2.
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

/* pk_dchd: deal one card into hand at next CARD_NONE slot.
   Rejects dups vs pk_ch/pk_ph/pk_psh.  Returns -1 if full.
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

/* pk_cnbj: check for natural blackjack (Ace + T/J/Q/K in first two).
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

/* pk_sbet: settle a bet.  winner: 0=computer, 1=player.
   mode: 0=normal, 1=natural blackjack double-collect,
         2=split -- suppress the second (player) transfer.
   Sets pk_quit on mid-transfer bankruptcy.
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

/* pk_bjr: play one blackjack round for `hand` at row.
   pk_wrf/pk_wcs forced-single-hit modes auto-deal one card + return.
   Otherwise F1 Hit / F3 Stand.  Returns 0 on stand, -1 on bust/timeout.
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
                        pk_pmsg(" ");   /* s__0002b422 verified via
                                          Ghidra HTTP /read_memory:
                                          the F3-stand path just
                                          blanks the message strip. */
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

/* pk_annr: transfer pot to winner one chip per tick
   (winner=0 -> computer, winner=1 -> player).
   addr: poker_ante_and_new_round() */

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

/* pk_drcs: blit one card sprite (15x23) at slot xi of row yi.
   card=CARD_BACK selects crd_mfdb[52]; 0..51 index directly.
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
        vroCpyD(vdihnd, S_ONLY,
                              (long) &crd_mfdb[card], (long) &mf_scb_c,
                              0, 0, 15, 23,
                              x, y, x + 15, y + 23);
}

/* STX: pk_ldCrd sits here (0xab04), ahead of pk_awp. */
#ifndef FAITHFUL
#include "parts/pk_ldCrd.c"
#endif

/* pk_inph: wait for one of F-keys a/b/c or digits 1..5.
   Returns 1..8 for a/b/c/1/2/3/4/5, or -1 on timeout.
   addr: poker_input_handler() */

short
pk_inph(a, b, c)
short   a;
short   b;
short   c;
{
        short   ch;

#ifdef FAITHFUL
        for (;;) {
#else
        while (1) {
#endif
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

/* pk_awp: display computer money count in the top-left panel.
   Preserves the 3-digit hand-formatted, space-padded byte layout
   from Ghidra verbatim (byte-comparable).
   addr: poker_award_pot() */

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

/* pk_dppm: display player money (same 3-digit format as pk_awp).
   addr: poker_display_player_money() */

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

/* pk_dpot: display the pot amount in the middle panel.
   addr: poker_display_pot() */

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

/* pk_pmsg: print a green status message in the bottom info bar.
   addr: poker_print_message() */

void
pk_pmsg(str)
char *  str;
{
        plEr(5, 63, 319, 75);
        strPr(str, 5, 71, COLOR_green);
}

/* pk_rmch: pop card from top of `pile`; shift remaining entries down.
   Returns CARD_NONE if empty.
   addr: poker_remove_card_from_hand() */

short
pk_rmch(pile, count)
short * pile;
short * count;
{
#ifdef FAITHFUL
        short   card;
        short   n;
        short   i;
#else
        /* STX's frame is 12 bytes: `card` and `i` are followed by two
           more declared shorts the body never touches. */
        short   card;
        short   i;
        short   unused1;
        short   unused2;
#endif

        if (*count == 0)
                return -1;
        card    = *pile;
#ifdef FAITHFUL
        n       = *count;
        *count  = n - 1;
        if ((short)(n - 1) != 0) {
                for (i = 0; i < 51; i = i + 1)
                        pile[i] = pile[i + 1];
        }
#else
        /* STX decrements the count in place and returns early when the
           pile is emptied, so `card` is returned from two places. */
        if (--*count == 0)
                return card;
        for (i = 0; i < 51; i++)
                *(pile + i) = *(pile + i + 1);
#endif
        return card;
}

/* pk_actd: append val at pile[*idx]; increment idx.
   addr: poker_add_card_to_discard() */

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
   Per-round: reveal cards, compare mod-13, resolve win/loss/tie.
   addr: pk_wrMn() (== poker_war_main) */

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

        /* Per-round loop (Ghidra LAB_0001b29c): a label and explicit
           gotos, not a for(;;) -- every round-end branches straight
           back here rather than to a loop-bottom edge.  The bare
           `pk_dppm;` is in the original: a call whose parentheses
           were left off, so Alcyon just loads its address into d0. */
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
                while (ikey != 1 && ikey != 2)
                        ikey = pk_inph(KEY_F1, KEY_F10, 255);
                if (ikey == 2)
                        goto cleanup;

                pk_drcs(pk_pwc[0], 0, 1);
                plEr(225, 10, 319, 60);
                gameTick(5);

                /* Both ranks land in locals before the compare, and
                   the loser's branch recomputes them the other way
                   round. */
                if ((ikey = pk_pwc[0] % 13) > (cidx = pk_cwc[0] % 13)) {
                        /* Player wins. */
                        if (ikey == 12) {
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
                } else if ((ikey = pk_cwc[0] % 13) > (cidx = pk_pwc[0] % 13)) {
                        /* Computer wins by margin (ikey - cidx). */
                        if (ikey == 12) {
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
   Returns 0 = normal, -1 = computer out / user quit, -2 = player out.
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

/* pk_bjMn: BLACKJACK main game loop.
   Bet-entry (F1 add, F3 enter, F5 clear, 20 cap), deal, natural check,
   optional split, double-down, hit/stand rounds, dealer plays, settle.
   Preserves Ghidra gotos LAB_0001bcbe, LAB_0001bd9e, LAB_0001beb6.
   Alcyon 8-char link-name truncation prevents a body/wrapper split.
   addr: pk_bjMn() (== poker_blackjack_main) */

void
pk_bjMn()
{
        /* Ten locals in LCP_STX; four of them are never referenced and
           the last is written once and never read. */
        short   br;             /* -2, also every countdown */
        short   hit;            /* -4  */
        short   unused1;        /* -6  */
        short   unused2;        /* -8  */
        short   unused3;        /* -10 */
        short   unused4;        /* -12 */
        short   res;            /* -14 */
        short   rv;             /* -16 */
        short   round_ctr;      /* -18 */
        short   phase_snap;     /* -20, written once, never read */

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
                while (bj_key != 1 && bj_key != 3)
                        bj_key = pk_inph(KEY_F1, 255, KEY_F10);
                if (bj_key == 3) {
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
                        while (bj_key != 1 && bj_key != 2 &&
                               bj_key != 3 && bj_key != -1) {
                                gameTick(0);
                                bj_key = pk_inph(KEY_F1, KEY_F3, KEY_F5);
                        }
                        if (mg_tofl != NO) goto cleanup;
                        if (bj_key == 3) {
                                g_ppmon += pk_bet;
                                pk_bet  = 0;
                                g_pcbet = 0;
                                pk_dbhi(1);
                                pk_dppm();
                                pk_dpile[10] = CARD_BJ_STEP;
                                break;
                        }
                        if (bj_key == 1) {
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
                        if (bj_key == 2) {
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
                if ((short) pk_ph[0] % 13 ==
                    (short) pk_ph[1] % 13) {
                        pk_pmsg("Do you wish to split?");
                        plEr(225, 10, 319, 60);
                        strPr("F1 Split",    225, 18, COLOR_red);
                        strPr("F3 No split", 225, 26, COLOR_red);
                        bj_key = 0;
                        while (bj_key != 1 && bj_key != 2 && bj_key != -1) {
                                gameTick(0);
                                bj_key = pk_inph(KEY_F1, KEY_F3, 255);
                        }
                        if (mg_tofl != NO) goto cleanup;
                        if (bj_key == 1) {
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
                        while (bj_key != 1 && bj_key != 2 && bj_key != -1) {
                                gameTick(0);
                                bj_key = pk_inph(KEY_F1, KEY_F3, 255);
                        }
                        if (mg_tofl != NO) goto cleanup;
                        if (bj_key == 1) {
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
                        /* A redundant re-test of pk_phase, already
                           implied by the else. */
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
                                while (bj_key != 1 && bj_key != 2 && bj_key != -1) {
                                        gameTick(0);
                                        bj_key = pk_inph(KEY_F1, KEY_F3, 255);
                                }
                                if (mg_tofl != NO) goto cleanup;
                                if (bj_key == 1) {
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
                                while (bj_key != 1 && bj_key != 2 && bj_key != -1) {
                                        gameTick(0);
                                        bj_key = pk_inph(KEY_F1, KEY_F3, 255);
                                }
                                if (mg_tofl != NO) goto cleanup;
                                if (bj_key == 1) {
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




#endif  /* FAITHFUL */
