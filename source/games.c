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
#include "protos.h"
#include "rnd.h"
#include "events.h"
#include "vdiown.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprites.h"

/* ---- Word Puzzles messages.  The games object's initialized globals
   start here, in the original's data order: the position of these
   declarations and their order set the data layout. */

char *          wpzPrompts[9] = {
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

/* Word Puzzles: the six right-answer messages; wpzSolve shows one at
   random through wpzMessage. */
char *          wpzRightMsgs[6] = {
        "You got it!!",
        "Good going. That's right!",
        "Congratulations. That's it!",
        "I don't believe it!! You're right!",
        "You're pretty good. That's right!",
        "You got that one. How about another?"
};

/* Word Puzzles: the six wrong-answer messages, picked the same way. */
char *          wpzWrongMsgs[6] = {
        "Too bad. You missed it.",
        "Better luck next time.",
        "Good try, but that's the wrong answer.",
        "That's not it. How about another try?",
        "Nope.",
        "Not quite."
};

/* These statics are defined after their first caller.  A static
   called before its definition needs a forward declaration, or Alcyon
   treats the call as external and the linker resolves it to 0; the
   declaration also lets the assembler use a short call. */
static short warRound();
static short bjScore();
static short bjPlayHand();
static short bjIsNatural();
static void  pkrShowdown();  /* defined after playPoker, which calls it */
static short bjDealCard();
static void  bjShowBet();
static void  bjSettle();
static void  pkrAnte();
static void  pkrDealHands();
static void  pkrDecideBluff();
static short pkrOpenRank();
static void  pkrAddChips();
static short pkrPlyrBet();
static void  pkrCompDraw();

/* gamePlWQ does not exist in the original; Alcyon emits a static even
   when nothing calls it, so it must not be defined here. */

/* mgWaitKey: wait for a key while processing urgent game events.
   The minigames' key reader: it first drains stale keys, then, while
   no key is pressed, lets the resident answer the alarm, use the
   toilet, drink, or run a queued event (each wrapped in leaveGameTable /
   rejoinTable so he leaves and returns to the table) and advances the
   game by one gameTick.  On 7200 idle frames (~15 min) sets
   mgTimedOut=YES and returns KEY_F10, which the games treat as quit.
   The player's care keys (Ctrl-A/B/C/D/F/W) are passed to handleKey
   before the key is returned. */
short
mgWaitKey()
{
        short           key;
        unsigned short  idle;

        idle    = 0;
        mgTimedOut = NO;

        /* Drain any keys the game accidentally left in the buffer. */
        do ; while (getKey() != KEY_NONE);

        while ((key = getKey()) == KEY_NONE) {
                if (alarmRinging != NO) {
                        leaveGameTable();
                        wakeFromAlarm();
                        rejoinTable();
                }
                if (resident.bathroom_need != NO) {
                        leaveGameTable();
                        useToilet();
                        rejoinTable();
                }
                /* The GLOBAL tank level, not the saved copy in the
                   resident struct. */
                if (resident.thirst_level > NEED_SATISFIED && waterLevel != 0) {
                        leaveGameTable();
                        drinkWater();
                        rejoinTable();
                }
                if (idle++ > 7200) {
                        mgTimedOut = YES;
                        return KEY_F10;
                }
                if (eventQueue[0] != ACTION_NONE) {
                        leaveGameTable();
                        runEvent(nextEvent());
                        rejoinTable();
                }
                gameTick(0);
        }
        if (key == KEY_CTRL_A_ALARM  ||
            key == KEY_CTRL_B_BOOK    ||
            key == KEY_CTRL_C_CALL     ||
            key == KEY_CTRL_D_DOGFOOD    ||
            key == KEY_CTRL_F_FOOD  ||
            key == KEY_CTRL_W_WATER)
                handleKey(key);
        return key;
}

/* rndRng belongs here, right after mgWaitKey, in the minigame object,
   so the minigames reach it with a short call. */
#include "parts/rndRng.c"

/* anaStrMatch: character-by-character equality test for two C strings.
   Keeps walking both strings after a mismatch and reports at the end. */

short
anaStrMatch(a, b)
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
#include "parts/mgSetup.c"
#include "parts/gameCleanup.c"
#include "parts/textBig.c"
#include "parts/textNormal.c"
#include "parts/panelBegin.c"
#include "parts/panelEnd.c"

/* playWordPuzzle: WORD PUZZLE main loop.
   Loads wordpz.txt into a 2000-byte buffer, indexes 66 line pointers
   (33 puzzles x {template, solution}).  F1 next / F2 prev (wraps 0..0x20)
   / F5 solve / F10 quit.  The next_puzzle and cleanup gotos are the
   original's control flow and must stay. */

void
playWordPuzzle()
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

        wpzText = (char *) Malloc(2000L);
        if (wpzText == (char *) 0)
                outOfMemory();
        mgSetup();
        unpackFile("wordpz.txt",
                             (unsigned char *) wpzText, 1536);

        /* Index the 66 lines. */
        parse_ptr = wpzText;
        for (line_index = 0; line_index < 0x42; line_index++) {
                letterLines[line_index] = parse_ptr;
                /* Step once, then a plain `while` -- two increment
                   sites, not a do/while's one; the original has this
                   shape. */
                parse_ptr++;
                while (*parse_ptr >= ' ')
                        parse_ptr++;
                while (*parse_ptr < ' ')
                        parse_ptr++;
        }

        wpzIndex = 0;
        printString("**WORD PUZZLE #  **", 8, 8, COLOR_black);

        /* No outer loop: `next_puzzle` is a plain label and every arm
           of the key switch jumps back to it explicitly, as in the
           original. */
next_puzzle:
        printString("Choose the puzzle",   8,  16, COLOR_black);
        printString("you wish to solve.",  8,  24, COLOR_black);
        printString("F1 Next, F5 Solve", 176,  8, COLOR_red);
        printString("F2 Last, F10 Quit", 176, 16, COLOR_red);
        panelErase(128,  0, 143,  8);
        panelErase(  0, 50, 319, 69);

        /* Count '@' blanks; seed wpzAnswers[i][0] with char after '@'. */
        parse_ptr = letterLines[wpzIndex << 1];
        wpzBlanks = 0;
        while (1) {
                cur = *parse_ptr;
                parse_ptr++;
                if (cur < ' ') break;
                if (cur == '@') {
                        wpzAnswers[wpzBlanks][0] = *parse_ptr;
                        wpzAnswers[wpzBlanks][1] = '\0';
                        wpzBlanks++;
                }
        }

        panelErase(128, 0, 135, 8);
        sprintf(inputLine, "%2d", wpzIndex + 1);
        printString(inputLine, 128, 8, COLOR_black);
        wpzRender();

        while (1) {
                gameTick(0);
                key = mgWaitKey();
                switch (key) {
                case KEY_F1:
                        wpzIndex++;
                        if (wpzIndex >= 33)
                                wpzIndex = 0;
                        goto next_puzzle;
                case KEY_F2:
                        wpzIndex--;
                        if (wpzIndex < 0)
                                wpzIndex = 0x20;
                        goto next_puzzle;
                case KEY_F5:
                        wpzSolve();
                        if (mgTimedOut != NO)
                                goto cleanup;
                        gameTick(40);
                        goto next_puzzle;
                case KEY_F10:
                        goto cleanup;
                }
        }

cleanup:
        keysBlocked = NO;
        textTimer  = 0;
        Mfree(wpzText);          /* wpzText is deliberately not cleared */
}
#undef key

/* wpzSolve must follow playWordPuzzle directly so the call from the key switch
   stays a short branch.
   wpzSolve: solve phase.  Per blank: prompt, read A-Z (10-char max),
   Enter confirms, F10 quits.  Then walk solution line, compare
   token-by-token; show wpzRightMsgs or wpzWrongMsgs. */

void
wpzSolve()
{
        /* Exactly six locals; `ch` also carries the scanned solution
           and answer characters, as in the original. */
        short   cwi;            /* current_word_index */
        short   ilen;
        short   wi;
        short   ch;
        char *  slp;            /* solution_line_ptr */
        char *  psp;            /* player_answer_ptr */

        panelErase(0, 10, 175, 26);
        panelErase(176, 0, 319,  8);
        panelErase(176, 8, 248, 18);
        cwi = 0;
        do {
                panelErase(0, 60, 319, 69);
                if (cwi == 0)
                        wi = rndRng(0, 4);
                else
                        wi = cwi + 4;
                wpzMessage(wpzPrompts[wi]);
                ilen = 0;
                while (1) {
                        gameTick(0);
                        ch = mgWaitKey();
                        if (ch == KEY_F10)
                                return;
                        /* The two `(long)` casts are deliberate: they
                           make Alcyon add the array base after the
                           column offset, as the original does.  Without
                           them the base is folded in first -- which is
                           what the other two stores here do. */
                        if (ch == KEY_CTRL_M) {
                                wpzAnswers[cwi][ilen] = '\0';
                                break;
                        } else if (ch == KEY_CURSOR_LEFT && ilen > 0) {
                                ilen--;
                                wpzAnswers[0][cwi * 12 + (long) ilen] = '\0';
                                panelErase(ilen * 8 + 8, 60,
                                          ilen * 8 + 16, 68);
                        } else if (ilen < 10 && ch >= 'A') {
                                ch = toUpper(ch);
                                wpzAnswers[0][cwi * 12 + (long) ilen] = ch;
                                ilen++;
                                wpzAnswers[cwi][ilen] = '\0';
                                printString(wpzAnswers[cwi], 8, 68, COLOR_white);
                        }
                }
                gameTick(8);
                cwi++;
                panelErase(0, 60, 319, 69);
        } while (cwi < wpzBlanks);

        slp = letterLines[(wpzIndex << 1) + 1];
        wi  = 0;
        while (wi < wpzBlanks) {
                /* The assignment is INSIDE the condition, so the
                   compare uses the loaded value rather than reloading
                   `ch` from the frame. */
                do {
                } while ((ch = *slp++) <= ' ');
                slp--;
                psp = wpzAnswers[wi];
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
        wpzRender();
        gameTick(8);
        wpzMessage(wpzRightMsgs[rndRng(0, 5)]);
        return;
fail:
        wpzRender();
        gameTick(8);
        wpzMessage(wpzWrongMsgs[rndRng(0, 5)]);
}

/* wpzMessage and wpzRender must follow wpzSolve directly so its calls to
   them stay short branches.
   wpzMessage: word-puzzle status message in green at (8,58). */

void
wpzMessage(msg)
char *  msg;
{
        panelErase(0, 50, 319, 59);
        printString(msg, 8, 58, COLOR_green);
}

/* wpzRender: render puzzle template with player answers substituted for '@'.
   Word-wraps at col 0x26 (literal) / 0x27 (answer).  Starts cursor at
   (x=1, y=0x28). */

void
wpzRender()
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

        panelErase(0, 31, 319, 49);
        tp = letterLines[wpzIndex << 1];
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
                                if (wpzAnswers[ai][wlen] <= ' ')
                                        break;
                        if (cx + wlen > 0x27) {
                                cx = 1;
                                cy += 8;
                        }
                        printString(wpzAnswers[ai], cx << 3, cy, COLOR_blue);
                        cx += wlen;
                        tp++;
                        ai++;
                        cur = *tp;
                        if (cur < ' ')
                                return;
                        if (cur < 'A') {
                                printChar(cur, cx << 3, cy, COLOR_blue);
                                cx++;
                                tp++;
                        }
                        continue;
                }

                /* Literal word. */
                inputLine[0] = cur;
                for (wlen = 1; wlen < 0x10; wlen++) {
                        /* if/else, not an early break: the else arm's
                           `break` becomes a branch past the body's own
                           jump to the increment. */
                        if ((cur = *tp) > ' ') {
                                inputLine[wlen] = cur;
                                tp++;
                        } else
                                break;
                }
                if (cx + wlen > 0x26) {
                        cx = 1;
                        cy += 8;
                }
                for (ci = 0; ci < wlen; ci++) {
                        printChar(inputLine[ci], cx << 3, cy, COLOR_blue);
                        cx++;
                }
        }
}

/* anaClrWord: clear the right-panel word display area (162,10)-(319,49). */

void
anaClrWord()
{
        short   rect[4];
        rect[0] = 162; rect[1] = 10;
        rect[2] = 319; rect[3] = 49;
        panelBegin();
        v_bar(vdiHandle, rect);
        panelEnd();
}

/* anaClrIntro: clear the left-panel intro/instructions area (5,10)-(160,60). */

void
anaClrIntro()
{
        short   rect[4];
        rect[0] = 5;   rect[1] = 10;
        rect[2] = 160; rect[3] = 60;
        panelBegin();
        v_bar(vdiHandle, rect);
        panelEnd();
}

/* anaClrGuess: clear the "Guess #N?" prompt bar (166,50)-(319,65). */

void
anaClrGuess()
{
        short   rect[4];
        rect[0] = 166; rect[1] = 50;
        rect[2] = 319; rect[3] = 65;
        panelBegin();
        v_bar(vdiHandle, rect);
        panelEnd();
}

/* anaClrBottom: clear the bottom info bar (5,62)-(319,75). */

void
anaClrBottom()
{
        short   rect[4];
        rect[0] = 5;   rect[1] = 62;
        rect[2] = 319; rect[3] = 75;
        panelBegin();
        v_bar(vdiHandle, rect);
        panelEnd();
}

/* anaIntroText: draw the 5-line intro text in the left panel. */

void
anaIntroText()
{
        printString("I am thinking of",  5, 17, COLOR_black);
        printString("a word.  Here it",  5, 25, COLOR_black);
        printString("is jumbled up...",  5, 33, COLOR_black);
        printString("See if you can ",   5, 41, COLOR_black);
        printString("guess what it is.", 5, 49, COLOR_black);
}

/* anaShowWord: display a word in 20px text in right panel at (162,37), 12px pitch. */

void
anaShowWord(word, text_color)
char *  word;
short   text_color;
{
        short   x;

        anaClrWord();
        textBig();
        x = 0;
        /* The pointer is stepped inside the body, before the pitch;
           this order matches the original's code. */
        while (*word != '\0') {
                printChar((short) *word, x + 162, 37, text_color);
                word++;
                x += 12;
        }
        textNormal();
}

/* anaDrawPrompt must sit here, after anaShowWord. */

#include "parts/anaDrawPrompt.c"

/* anaPickWord: pick a random word from the 150-entry dictionary (11 bytes/row),
   copy into anaScrambled, scramble 10..20 swaps.  Re-scrambles on identity.
   Plants '\0' at anaDict row-tail so anaAnswer reads as a C string. */

void
anaPickWord()
{
        /* One counter reused for the copy index and the shuffle round,
           and a local copy of the word length that the shuffle reads
           instead of anaWordLen -- the original's locals exactly. */
        short   pos;
        short   len;
        short   ia;
        short   ib;
        char    tmp;
        char *  wp;

        anaAnswer = anaDict + rndRng(0, 149) * 11;        /* 0..149 */
        pos     = 0;
        for (wp = anaAnswer; *wp > ' ' && *wp != '.'; ) {
                /* Index first: this makes Alcyon fold the base into the
                   address the way the original does. */
                *(pos + anaScrambled) = *wp;
                wp++;
                pos++;
        }
        anaScrambled[pos] = '\0';
        *wp          = '\0';
        len     = pos;
        anaWordLen = len;

        while (anaStrMatch(anaScrambled, anaAnswer) != 0) {
                pos = 0;
                while (rndRng(10, 20) > pos) {
                        ia  = rndRng(0, len - 1);
                        ib  = rndRng(0, len - 1);
                        tmp = anaScrambled[ib];
                        anaScrambled[ib] = anaScrambled[ia];
                        anaScrambled[ia] = tmp;
                        pos++;
                }
        }
        anaShowWord(anaScrambled, COLOR_green);
}

/* playAnagrams: full anagram game loop.  Outer per-word / middle per-guess /
   inner per-keypress.  The new_word/validate labels are the 1985
   code's own gotos and must stay. */

/* ---- Anagram prompts and messages, card positions.  Alcyon pools a
   unit's string literals in the order it meets them; in the original
   these strings sit between playWordPuzzle's screen text and
   playAnagrams's, so the declarations must stay right here, between the
   two functions. */

/* anagram_guess_prompt_strings: shown per attempt.  Each is padded to
   19 characters so it overwrites the previous prompt in place.
   (0..8 -> "Guess #1?"..
   "Guess #9?").  Rendered by anaDrawPrompt at (166, 57). */
/* Ten slots for nine prompts and five for three messages: the original
   sizes both arrays past their initializer lists and Alcyon zero-fills
   the tail with NULLs.  Do not shrink them to the initializer count. */
char *          anaPrompts[10] = {
        "Guess #1?          ",
        "Guess #2?          ",
        "Guess #3?          ",
        "Guess #4?          ",
        "Guess #5?          ",
        "Guess #6?          ",
        "Guess #7?          ",
        "Guess #8?          ",
        "Guess #9?          "
};

short           anaExtraGuess          = 0;    /* anagram: set when a clue pushed the guess count to 9, allowing one more guess (cleared per round) */

/* Anagram wrong-guess messages; playAnagrams picks one of the three with
   rndRng(0, 2).  The last two slots are NULL. */
char *          anaWrongMsgs[5] = {
        "Nope, have another try.",
        "Sorry, try again.",
        "Missed, try again."
};

/* Card display positions -- 5 slots per row.  Row A = computer
   (y=11 top strip), Row B = player (y=37 middle strip).  X columns
   are spaced 28 pixels apart (15-px card + 13-px gutter). */
short           cardXComp[5]         = { 70, 98, 126, 154, 182 };

short           cardYComp[5]         = { 11, 11, 11, 11, 11 };    /* row A (computer) y per card slot */

short           cardXPlyr[5]         = { 70, 98, 126, 154, 182 };    /* row B (player) x per card slot */

short           cardYPlyr[5]         = { 37, 37, 37, 37, 37 };    /* row B (player) y per card slot; all four read by cardDraw */

/* The Anagrams minigame.  Loads the "words" file into a 10000-byte
   Malloc buffer (freed on F10), then loops: anaPickWord picks and scrambles
   a word, the player types up to ten letters (cursor-left erases) and
   Return submits.  A right answer or too many guesses/clues starts a
   new word; a wrong one keeps the word and counts a guess (anaGuessNum).
   F1 once per word swaps one letter of the scramble into place and
   costs a guess.  F10 (or mgWaitKey's idle timeout) leaves the game. */
void
playAnagrams()
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

        anaDict = (char *) Malloc(10000L);
        if (anaDict == (char *) 0)
                outOfMemory();
        unpackFile("words", (unsigned char *) anaDict, 10000);
        mgSetup();
        printString("***ANAGRAMS***", 5, 8, COLOR_black);
        anaIntroText();

new_word:
        anaClrBottom();
        anaNumClues = 0;
        anaGuessNum = 1;
        anaPickWord();

        /* The round prologue runs ONCE per word and the guess-count
           guard is folded into a `while` condition.  A wrong guess
           re-enters HERE, past the prologue, so the word is kept --
           only a solved or abandoned word goes back to new_word. */
same_word:
        anaExtraGuess = 0;
        anaClueUsed = 0;
        printString("F1 Clue, F10 Quit", 183, 8, COLOR_blue);
        anaDrawPrompt(anaGuessNum);
        for (index = 0; index < 10; index++)
                anaInput[index] = ' ';
        anaInput[10]   = '\0';
        gameTick(0);
        word_complete = NO;
        while (anaGuessNum < 9 || (anaGuessNum < 10 && anaExtraGuess != 0)) {
                index         = 0;
                key_pressed   = 0;
                while (key_pressed != KEY_CTRL_M) {
                        printString(anaInput, 239, 57, COLOR_green);
                        key_pressed = mgWaitKey();
                        if (key_pressed >= 'A' && key_pressed <= 'Z')
                                key_pressed += 0x20;
                        if (key_pressed <= 'z' && key_pressed >= 'a') {
                                anaInput[index] = key_pressed;
                                if (++index >= 10) {
                                        index = 9;
                                        anaDrawPrompt(anaGuessNum);
                                }
                        }
                        if (key_pressed == KEY_CURSOR_LEFT) {
                                if (index != 0) {
                                        if (index == 9 &&
                                            anaInput[index] != ' ')
                                                anaInput[index] = ' ';
                                        else {
                                                index--;
                                                *(index + anaInput) = ' ';
                                        }
                                } else
                                        anaInput[index] = ' ';
                                anaDrawPrompt(anaGuessNum);
                                continue;
                        }
                        if (key_pressed == KEY_F10) {
                                textTimer  = 0;
                                keysBlocked = NO;
                                Mfree(anaDict);
                                return;
                        }
                        if (key_pressed == KEY_F1 && anaClueUsed == 0) {
                                /* A goto, not `continue`: the original
                                   jumps to a label sitting ON the else
                                   arm's statement, one test ahead of the
                                   loop's own condition.  The braces are
                                   deliberate too; without them Alcyon
                                   emits a different branch shape. */
                                if (anaStrMatch(anaAnswer, anaScrambled) != 0) {
                                        goto again;
                                }
                                /* Clue path: reveal one letter. */
                                anaNumClues++;
                                anaGuessNum++;
                                anaDrawPrompt(anaGuessNum);
                                if (anaGuessNum == 9)
                                        anaExtraGuess = 1;
                                anaClueUsed = 1;
                                panelErase(182, 0, 319, 9);
                                printString("         F10 Quit", 183, 8,
                                      COLOR_blue);
                                for (guess_count = 0;
                                     guess_count < anaWordLen;
                                     guess_count++)
                                        if (anaAnswer[guess_count] !=
                                            anaScrambled[guess_count])
                                                break;
                                if (guess_count != anaWordLen) {
                                        clue_count = anaWordLen - 1;
                                        for (;;) {
                                                if (anaAnswer[guess_count] ==
                                                    anaScrambled[clue_count])
                                                        break;
                                                clue_count--;
                                        }
                                        typed_char = anaScrambled[clue_count];
                                        anaScrambled[clue_count] =
                                                anaScrambled[guess_count];
                                        anaScrambled[guess_count] = typed_char;
                                }
                                anaShowWord(anaScrambled, COLOR_green);
                                if (anaStrMatch(anaAnswer, anaScrambled) != 0) {
                                        anaClrBottom();
                                        printString("You took too many clues!",
                                              5, 69, COLOR_black);
                                        anaShowWord(anaAnswer, COLOR_black);
                                        gameTick(20);
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
                     anaInput[index] != ' ' && anaInput[index];
                     index++) ;
                if (anaInput[index] == ' ')
                        anaInput[index] = '\0';
                if (anaStrMatch(anaInput, anaAnswer) != 0) {
                        printString("YOU GOT IT!!!!!!",
                                             5, 69, COLOR_black);
                        anaShowWord(anaAnswer, COLOR_black);
                        gameTick(30);
                        anaClrBottom();
                        goto new_word;
                /* The guess counter steps in the condition itself,
                   so both arms see it incremented. */
                } else if (anaGuessNum++ < 8) {
                        printString(anaWrongMsgs[rndRng(0, 2)], 5, 69, COLOR_black);
                        gameTick(20);
                        anaClrBottom();
                        goto same_word;
                } else {

                /* Too many wrong guesses: show the answer, start a
                   new word. */
                printString("Sorry, too many guesses!",
                             5, 69, COLOR_black);
                gameTick(20);
                anaClrBottom();
                printString("Here is the word.",
                             5, 69, COLOR_black);
                anaShowWord(anaAnswer, COLOR_black);
                gameTick(30);
                anaClrWord();
                goto new_word;
                }
        }
}

/* panelErase and eraseRectColor (in parts/) sit here, after the anagram code;
   eraseRectColor must follow panelErase directly. */
#include "parts/panelErase.c"
#include "parts/eraseRectColor.c"

/* pkrCallOrRaise: computer call/raise decision.  Returns 'c' or 'r'.
   On raise: pkrRaiseAmt = money/10 clamped [1,20]. */

static short
pkrCallOrRaise()
{
        /* No temporary, just early returns.  The redundant `else` is
           kept on purpose: Alcyon emits its skip branch even though
           the then arm returns, and the original has it. */
        if (compChips == 0)
                return 'c';
        if (pkrBluffing == NO && compRank < HAND_TWO_PAIR)
                return 'c';
        else {
                pkrRaiseAmt = compChips / 10;
                if (pkrRaiseAmt == 0)
                        pkrRaiseAmt = 1;
                else if (pkrRaiseAmt > 20)
                        pkrRaiseAmt = 20;
                return 'r';
        }
}

/* pkrEvalHand: evaluate a 5-card hand.  *hand_rank <- 0=high card..9=royal flush.
   rank_flags[i]=1 for winning combo cards.  suit_flags: rank-sorted hand copy.
   The two goto exits are the original's control flow. */

static void
pkrEvalHand(hand, rank_flags, suit_flags, hand_rank)
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

/* playPoker: 5-card draw poker main loop.
   Init: Malloc, load cards, mgSetup, money=400 each.
   Per-round: ante, deal, bet, discard/draw, computer draw, final bet,
   showdown.  The labels and gotos are the original's control flow and
   must stay. */

/* ---- Poker's bet and raise templates.  Their strings are the last two
   before playPoker's "Do you feel lucky today?", so these declarations
   must stay immediately ahead of playPoker. */

/* The resident's raise announcement: playPoker writes his raise
   (pkrRaiseAmt, two digits, a leading zero blanked) over the underscores
   at [11] and [12] before showing it. */
char *          pkrMsgRaise     = "I'll raise __.";

/* Editable poker prompts, patched in place before each is shown.  The
   underscores are the digit slots the original ships -- pkrCallOrRaise and
   dispPlyrChips overwrite the two in pkrMsgBet/pkrMsgRaise, pkrCompDraw the one in
   pkrMsgTake (and the trailing "." becomes "s." for a plural draw).
   They are POINTERS, not arrays, so every patch loads the pointer
   first; declaring them as arrays changes the compiled code. */
char *          pkrMsgBet     = "I'll bet __.";

/* The Poker minigame (five-card draw against the resident).  Allocates
   and loads the card images into cardImages, gives both sides 400 chips,
   then plays rounds until pkrAnte sets cardQuit, a side runs out of
   chips, or mgWaitKey times out; every exit frees cardImages and hides the
   mouse through the cleanup label. */
void
playPoker()
{
        /* Six locals in this order: loc8 doubles as the second key
           variable and as the raise countdown. */
        short   ikey;
        short   loc8;
        short   i;
        short   card;
        short   dcount;
        BOOL16  in_use;

        cardImages = (short *) Malloc(10400L);
        if (cardImages == (short *) 0)
                outOfMemory();
        cardLoad();
        mgSetup();

        pkrRound = 0;
        cardQuit  = NO;
        compChips  = 400;
        plyrChips  = 400;
        potChips  = 0;
        dispCompChips();
        dispPlyrChips();
        dispPot();

        /* A label and gotos, not a loop statement: the round tick sits
           at the top and is skipped on the first pass.  The cleanup is
           a label inside the first exit test. */
        goto round;
next_round:
        gameTick(24);
round:
                panelErase(70, 10, 219, 62);
                pkrAnte();
                if (cardQuit == YES) {
cleanup:
                        textTimer  = 0;
                        keysBlocked = NO;
                        Mfree(cardImages);
                        hideMouse();
                        return;
                }
                pkrDealHands();

                if (pkrPlyrBet("Do you feel lucky today?") == -1) {
                        if (mgTimedOut != NO)
                                goto cleanup;
                        cardMessage("Sorry, you're all out!");
                        gameTick(10);
                        goto cleanup;
                }
                if (pkrPassed != NO) {
                        cardMessage("That's all right with me.");
                        gameTick(10);
                } else {
                        cardMessage("I'll see your bet.");
                        while (pkrBet--) {
                                if (compChips == 0) {
                                        cardMessage("Sorry, I'm all out!");
                                        gameTick(10);
                                        goto cleanup;
                                }
                                compChips--;
                                dispCompChips();
                                potChips++;
                                dispPot();
                                gameTick(0);
                        }
                }
                gameTick(16);

                pkrNumDisc = 0;
                cardMessage("Do you want any cards?");
                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                printString("F1 Draw", KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                printString("F3 Stay", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
                for (i = 0; i < 5; i++)
                        pkrSelected[i] = 0;

                /* One while loop: the key read is the condition, and
                   every re-prompt is a `continue`. */
discard_loop:
                while ((ikey = cardKeyInput(KEY_F1, KEY_F3, 0)) != PK_IN_ARG_B) {
                        if (ikey == PK_IN_TIMEOUT)
                                break;
                        for (i = 0; i < 5; i++)
                                if (pkrSelected[i] == 1)
                                        break;
                        if (i == 5)
                                printString("F3 Stay", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
                        else
                                printString("F3 Stay", KEYMENU_X, KEYMENU_LINE2, COLOR_lt_grey);
                        if (ikey == PK_IN_ARG_A) {
                                for (i = 0; i < 5; i++)
                                        if (pkrSelected[i] == 1)
                                                break;
                                if (i == 5)
                                        continue;
                                break;
                        }
                        if (ikey < PK_IN_DIGIT_1)
                                continue;
                        if (ikey > PK_IN_DIGIT_5)
                                continue;
                        if (pkrSelected[ikey - PK_IN_DIGIT_1]) {
                                pkrSelected[ikey - PK_IN_DIGIT_1] = 0;
                                cardDraw(plyrHand[ikey - PK_IN_DIGIT_1], ikey - PK_IN_DIGIT_1, 1);
                        } else {
                                pkrSelected[ikey - PK_IN_DIGIT_1] = 1;
                                cardDraw(CARD_HIGHLIGHT, ikey - PK_IN_DIGIT_1, 1);
                        }
                        for (i = 0; i < 5; i++)
                                if (pkrSelected[i] == 1)
                                        break;
                        if (i == 5)
                                printString("F3 Stay", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
                        else
                                printString("F3 Stay", KEYMENU_X, KEYMENU_LINE2, COLOR_lt_grey);
                }
                if (mgTimedOut != NO)
                        goto cleanup;
                if (ikey == PK_IN_ARG_A) {
                        for (i = 0; i < 5; i++) {
                                if (pkrSelected[i] != 1) continue;
                                in_use = YES;
                                while (in_use != NO) {
                                        card = rndRng(0, 51);
                                        in_use = NO;
                                        for (dcount = 0; dcount < 5; dcount++) {
                                                if (compHand[dcount] == card)
                                                        in_use = YES;
                                                if (plyrHand[dcount] == card)
                                                        in_use = YES;
                                        }
                                        dcount = pkrNumDisc;
                                        while (dcount--)
                                                if (pkrDiscPile[dcount] == card)
                                                        in_use = YES;
                                }
                                pkrDiscPile[pkrNumDisc] = plyrHand[i];
                                pkrNumDisc++;
                                plyrHand[i] = card;
                                cardDraw(card, i, 1);
                                gameTick(3);
                        }
                }
                if (ikey == PK_IN_ARG_B) {
                        for (i = 0; i < 5; i++)
                                if (pkrSelected[i] == 1)
                                        break;
                        if (i != 5)
                                goto discard_loop;
                }

                pkrCompDraw();
                if (pkrPlyrBet("Want to make a bet?") == -1) {
                        if (mgTimedOut != NO)
                                goto cleanup;
                        cardMessage("Sorry, you're all out!");
                        gameTick(10);
                        goto cleanup;
                }
                if (pkrPassed != NO) {
                        if (pkrBluffing == NO && compRank == HAND_HIGH_CARD) {
                                cardMessage("Ok, I'll call.");
                                gameTick(10);
                                pkrShowdown();
                                goto next_round;
                        } else {
                                i = rndRng(5, 15);
                                if (i > compChips)
                                        i = compChips;
                                pkrMsgBet[9] = i / 10 + '0';
                                if (pkrMsgBet[9] == '0')
                                        pkrMsgBet[9] = ' ';
                                pkrMsgBet[10] = i % 10 + '0';
                                cardMessage(pkrMsgBet);
                                pkrBet = 0;
                                while (i--) {
                                        pkrAddChips(0, 1);
                                        gameTick(0);
                                }
                                gameTick(10);
                                cardMessage("Will you see my bet?");
                                pkrLastBet = pkrBet;
                                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                                printString("F1 See",  KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                                printString("F3 Fold", KEYMENU_X, KEYMENU_LINE3, COLOR_red);
                                ikey = cardKeyInput(KEY_F1, PK_IN_UNUSED, KEY_F3);
                                if (ikey == PK_IN_TIMEOUT) goto cleanup;
                                if (ikey == PK_IN_ARG_C) {
                                        panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                                        cardMessage("My pot.");
                                        gameTick(8);
                                        potToWinner(0);
                                        goto next_round;
                                }
                                if (ikey == PK_IN_ARG_A) {
                                        loc8   = pkrLastBet;
                                        pkrBet = 0;
                                        while (loc8--) {
                                                pkrAddChips(1, 1);
                                                gameTick(0);
                                        }
                                        if (plyrChips == 0) {
                                                cardMessage("Sorry, you're all out!");
                                                gameTick(10);
                                                goto cleanup;
                                        }
                                        panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                                        panelErase(5, 63, 319, 75);
                                        printString("F1 Raise", KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                                        printString("F3 Enter", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
                                        printString("F5 Call",  KEYMENU_X, KEYMENU_LINE3, COLOR_red);
                                        pkrRaiseAmt = 0;
                                        while (1) {
                                                loc8 = cardKeyInput(KEY_F1, KEY_F3, KEY_F5);
                                                if (loc8 == PK_IN_TIMEOUT)
                                                        break;
                                                if (loc8 == PK_IN_ARG_C) {
                                                        pkrShowdown();
                                                        goto next_round;
                                                }
                                                if (loc8 == PK_IN_ARG_A && plyrChips != 0) {
                                                        pkrBet = 0;
                                                        pkrAddChips(1, 1);
                                                        pkrRaiseAmt++;
                                                        break;
                                                }
                                        }
                                        if (mgTimedOut != NO) goto cleanup;
                                        while (1) {
                                                loc8 = cardKeyInput(KEY_F1, KEY_F3, KEY_F5);
                                                if (loc8 == PK_IN_TIMEOUT)
                                                        break;
                                                if (loc8 == PK_IN_ARG_B)
                                                        break;
                                                if (loc8 == PK_IN_ARG_A) {
                                                        pkrAddChips(1, 1);
                                                        if (plyrChips != 0)
                                                                pkrRaiseAmt++;
                                                }
                                        }
                                        if (mgTimedOut != NO) goto cleanup;
                                        if (compChips < pkrRaiseAmt) {
                                                cardMessage("Sorry, I,m all out.");
                                                gameTick(10);
                                                goto cleanup;
                                        }
                                        cardMessage("Ok. I'll see your bet.");
                                        gameTick(8);
                                        loc8   = pkrRaiseAmt;
                                        pkrBet = 0;
                                        while (loc8--) {
                                                pkrAddChips(0, 1);
                                                gameTick(0);
                                        }
                                        gameTick(5);
                                        cardMessage("I'll call.");
                                        gameTick(8);
                                        pkrShowdown();
                                        goto next_round;
                                }
                        }
                } else {
                        if (pkrOpenRank() == -1) {
                                cardMessage("I feel unlucky. I fold.");
                                gameTick(8);
                                cardMessage("Your pot.");
                                potToWinner(1);
                                goto next_round;
                        }
                        if (compChips < pkrBet) {
                                        cardMessage("Sorry, I'm all out!");
                                        gameTick(10);
                                        goto cleanup;
                                }
                                cardMessage("Ok. I'll see your bet.");
                                i = pkrBet;
                                pkrBet = 0;
                                while (i--) {
                                        pkrAddChips(0, 1);
                                        gameTick(0);
                                }
                                if (pkrCallOrRaise() == 'c') {
                                        cardMessage("I'll call.");
                                        gameTick(8);
                                        pkrShowdown();
                                        goto next_round;
                                } else {
                                        pkrMsgRaise[11] = pkrRaiseAmt / 10 + '0';
                                        if (pkrMsgRaise[11] == '0')
                                                pkrMsgRaise[11] = ' ';
                                        pkrMsgRaise[12] = pkrRaiseAmt % 10 + '0';
                                        cardMessage(pkrMsgRaise);
                                        ikey = pkrRaiseAmt;
                                        pkrBet    = 0;
                                        while (ikey--) {
                                                pkrAddChips(0, 1);
                                                gameTick(0);
                                        }
                                        gameTick(8);
                                        cardMessage("You think I'm bluffin'?");
                                        pkrLastBet = pkrRaiseAmt;
                                        panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                                        printString("F1 See",  KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                                        printString("F3 Fold", KEYMENU_X, KEYMENU_LINE3, COLOR_red);
                                        ikey = cardKeyInput(KEY_F1, PK_IN_UNUSED, KEY_F3);
                                        if (ikey == PK_IN_TIMEOUT) goto cleanup;
                                        if (ikey == PK_IN_ARG_C) {
                                                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                                                cardMessage("My pot.");
                                                gameTick(8);
                                                potToWinner(0);
                                                goto next_round;
                                        }
                                        if (ikey == PK_IN_ARG_A) {
                                                loc8   = pkrLastBet;
                                                pkrBet = 0;
                                                while (loc8--) {
                                                        pkrAddChips(1, 1);
                                                        gameTick(0);
                                                }
                                                if (plyrChips == 0) {
                                                        cardMessage("Sorry, you're all out!");
                                                        gameTick(10);
                                                        goto cleanup;
                                                }
                                                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                                                panelErase(5, 63, 319, 75);
                                                printString("F1 Raise", KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                                                printString("F3 Enter", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
                                                printString("F5 Call",  KEYMENU_X, KEYMENU_LINE3, COLOR_red);
                                                while (1) {
                                                        loc8 = cardKeyInput(KEY_F1, KEY_F3, KEY_F5);
                                                        if (loc8 == PK_IN_TIMEOUT)
                                                                break;
                                                        if (loc8 == PK_IN_ARG_C) {
                                                                pkrShowdown();
                                                                goto next_round;
                                                        }
                                                        if (loc8 == PK_IN_ARG_A && plyrChips != 0) {
                                                                pkrBet = 0;
                                                                pkrRaiseAmt = 0;
                                                                pkrAddChips(1, 1);
                                                                pkrRaiseAmt++;
                                                                break;
                                                        }
                                                }
                                                if (mgTimedOut != NO) goto cleanup;
                                                while (1) {
                                                        loc8 = cardKeyInput(KEY_F1, KEY_F3, KEY_F5);
                                                        if (loc8 == PK_IN_TIMEOUT)
                                                                break;
                                                        if (loc8 == PK_IN_ARG_B)
                                                                break;
                                                        if (loc8 == PK_IN_ARG_A) {
                                                                pkrAddChips(1, 1);
                                                                if (plyrChips != 0)
                                                                        pkrRaiseAmt++;
                                                        }
                                                }
                                                if (mgTimedOut != NO) goto cleanup;
                                                if (compChips < pkrRaiseAmt) {
                                                        cardMessage("Sorry, I'm all out.");
                                                        gameTick(10);
                                                        goto cleanup;
                                                }
                                                cardMessage("Ok. I'll see your bet.");
                                                loc8   = pkrRaiseAmt;
                                                pkrBet = 0;
                                                while (loc8--) {
                                                        pkrAddChips(0, 1);
                                                        gameTick(0);
                                                }
                                                gameTick(5);
                                                cardMessage("I'll call.");
                                                gameTick(8);
                                                pkrShowdown();
                                                goto next_round;
                                        }
                                }
                        }
}

/* pkrShowdown: showdown.  Reveal computer hand, evaluate both, walk the
   per-rank tiebreak ladder.  Winner blinks 5x then potToWinner transfers.
   Sets pkrRound=1. */

static void
pkrShowdown()
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
                cardDraw(compHand[i], i, 0);
                gameTick(2);
        }
        pkrEvalHand(compHand, compScoring,  compSorted,  &compRank);
        pkrEvalHand(plyrHand, plyrScoring, plyrSorted, &plyrRank);

        if (compRank > plyrRank) pkrWinner = 0;
        if (compRank < plyrRank) pkrWinner = 1;

        if (compRank == plyrRank) {
                pkrWinner = 1;

                /* Straight/flush/straight-flush tiebreak: compare
                   highest sorted-hand card rank. */
                if ((compRank == HAND_STRAIGHT_FLUSH || compRank == HAND_FLUSH || compRank == HAND_STRAIGHT) &&
                    compSorted[4] % CARDS_PER_SUIT > plyrSorted[4] % CARDS_PER_SUIT)
                        pkrWinner = 0;

                /* Trips, full house, quads: compare the pair/trip
                   card's rank. */
                if (compRank == HAND_FOUR_OF_A_KIND || compRank == HAND_FULL_HOUSE || compRank == HAND_THREE_OF_A_KIND) {
                        for (br = 0; br < 5; br++)
                                if (compScoring[br] == 1)
                                        break;
                        for (i = 0; i < 5; i++)
                                if (plyrScoring[i] == 1)
                                        break;
                        if (compHand[br] % CARDS_PER_SUIT > plyrHand[i] % CARDS_PER_SUIT)
                                pkrWinner = 0;
                }

                /* Two pair tiebreak. */
                if (compRank == HAND_TWO_PAIR) {
                        pk = 0; ck = 0; ph = 0; ch = 0;
                        for (i = 0; i < 5; i++) {
                                if (compScoring[i] &&
                                    compHand[i] % CARDS_PER_SUIT > pk % CARDS_PER_SUIT)
                                        pk = compHand[i];
                                if (compScoring[i] &&
                                    compHand[i] % CARDS_PER_SUIT < pk % CARDS_PER_SUIT)
                                        ck = compHand[i];
                                if (plyrScoring[i] &&
                                    plyrHand[i] % CARDS_PER_SUIT > ph % CARDS_PER_SUIT)
                                        ph = plyrHand[i];
                                if (plyrScoring[i] &&
                                    plyrHand[i] % CARDS_PER_SUIT < ph % CARDS_PER_SUIT)
                                        ch = plyrHand[i];
                        }
                        if (pk % CARDS_PER_SUIT > ph % CARDS_PER_SUIT) {
                                pkrWinner = 0;
                        } else if (pk % CARDS_PER_SUIT == ph % CARDS_PER_SUIT &&
                                   ck % CARDS_PER_SUIT > ch % CARDS_PER_SUIT) {
                                pkrWinner = 0;
                        } else if (pk % CARDS_PER_SUIT == ph % CARDS_PER_SUIT &&
                                   ck % CARDS_PER_SUIT == ch % CARDS_PER_SUIT) {
                                for (i = 0; i < 5; i++)
                                        if (!compScoring[i])
                                                break;
                                for (br = 0; br < 5; br++)
                                        if (!plyrScoring[br])
                                                break;
                                if (compHand[i] % CARDS_PER_SUIT > plyrHand[br] % CARDS_PER_SUIT)
                                        pkrWinner = 0;
                        }
                }

                /* One pair: compare pair rank, then kicker ladder. */
                if (compRank == HAND_ONE_PAIR) {
                        pk = 0; ph = 0;
                        for (i = 0; i < 5; i++) {
                                if (compScoring[i])  pk = compHand[i];
                                if (plyrScoring[i]) ph = plyrHand[i];
                        }
                        if (pk % CARDS_PER_SUIT > ph % CARDS_PER_SUIT) {
                                pkrWinner = 0;
                        } else if (pk % CARDS_PER_SUIT == ph % CARDS_PER_SUIT) {
                                for (i = 4; i >= 0; i--) {
                                        if (compSorted[i] % CARDS_PER_SUIT >
                                            plyrSorted[i] % CARDS_PER_SUIT) {
                                                pkrWinner = 0; break;
                                        }
                                        if (compSorted[i] % CARDS_PER_SUIT <
                                            plyrSorted[i] % CARDS_PER_SUIT) {
                                                pkrWinner = 1; break;
                                        }
                                }
                        }
                }

                /* High card: pure kicker ladder from top down. */
                if (compRank == HAND_HIGH_CARD) {
                        for (i = 4; i >= 0; i--) {
                                if (compSorted[i] % CARDS_PER_SUIT >
                                    plyrSorted[i] % CARDS_PER_SUIT) {
                                        pkrWinner = 0; break;
                                }
                                if (compSorted[i] % CARDS_PER_SUIT <
                                    plyrSorted[i] % CARDS_PER_SUIT) {
                                        pkrWinner = 1; break;
                                }
                        }
                }
        }

        pkrRound = 1;

        if (pkrWinner == 0) {
                cardMessage("I win!!!");
                for (br = 0; br < 10; br++) {
                        gameTick(2);
                        if (br & 1) {
                                for (i = 0; i < 5; i++)
                                        cardDraw(CARD_HIGHLIGHT, i, 0);
                        } else {
                                for (i = 0; i < 5; i++)
                                        cardDraw(compHand[i], i, 0);
                        }
                }
                for (i = 0; i < 5; i++)
                        cardDraw(compHand[i], i, 0);
                potToWinner(0);
        }
        if (pkrWinner == 1) {
                cardMessage("You're so lucky!!!");
                for (br = 0; br < 10; br++) {
                        gameTick(2);
                        if (br & 1) {
                                for (i = 0; i < 5; i++)
                                        cardDraw(CARD_HIGHLIGHT, i, 1);
                        } else {
                                for (i = 0; i < 5; i++)
                                        cardDraw(plyrHand[i], i, 1);
                        }
                }
                for (i = 0; i < 5; i++)
                        cardDraw(plyrHand[i], i, 1);
                potToWinner(1);
        }
}

/* pkrOpenRank: should the computer open?  Bluffing -> yes (0).
   Otherwise "Jacks or better" -- returns best rank >= Q, else -1. */

static short
pkrOpenRank()
{
        /* The bare `return;` on the success path is deliberate: the
           last comparison leaves compHand[best] % 13 in the return
           register, which is what the caller reads.  The original has
           no explicit value there. */
        short   i;
        short   best;

        if (pkrBluffing == NO && compRank == HAND_HIGH_CARD) {
                for (best = 0, i = 0; i < 5; i++) {
                        if (compHand[i] % CARDS_PER_SUIT > compHand[best] % CARDS_PER_SUIT)
                                best = i;
                }
                if (compHand[best] % CARDS_PER_SUIT < CARD_RANK_ACE)
                        return -1;
                return;
        }
        return 0;
}

/* pkrDecideBluff: 1/15 chance of bluff when hand rank < 2.  Sets pkrBluffing. */

static void
pkrDecideBluff()
{
        /* No local: the roll is tested in place. */
        pkrBluffing = NO;
        if (rndRng(0, 14) == 0 && compRank <= HAND_ONE_PAIR)
                pkrBluffing = YES;
}

/* pkrCompDraw: computer AI draw phase.
   Discard count by rank: 0->4, 1->3, 2->1, 3->2, >=4->stay.
   Bluffing: 0..2 discards from non-rank cards.  Re-draws unique
   replacements and animates the swap. */

/* ---- Poker's draw template.  Its string lands between "You're so
   lucky!!!" and "I'll stay!" in the literal pool, so the declaration
   must stay just ahead of pkrCompDraw, its only user. */

/* The resident's draw announcement: pkrCompDraw writes the count into
   [10] and makes the ending "card." or "cards." from [16]. */
char *          pkrMsgTake    = "I'll take _ cards.";

/* The resident's draw.  Rates his hand with pkrEvalHand, marks the cards
   to throw in pkrSelected (the cards that are not part of the scoring
   rank, or 0..2 random ones when pkrDecideBluff decides to bluff), announces
   the count through the pkrMsgTake message, and deals each replacement
   from cards not in either hand or the discard pile, moving the old
   card onto pkrDiscPile. */
static void
pkrCompDraw()
{
        /* Five locals, counter first; the order must stay. */
        short   i;
        short   nc;                  /* card_in_use flag */
        short   dm;                  /* inner counter / scratch */
        short   card;
        short   n;                   /* new_card */

        for (i = 0; i < 5; i++)
                pkrSelected[i] = 0;
        pkrEvalHand(compHand, compScoring, compSorted, &compRank);
        pkrDecideBluff();

        if (pkrBluffing != NO) {
                nc = rndRng(0, 2);
                i  = nc;
                for (card = 0; card < 5; card++) {
                        if (i == 0)
                                break;
                        if (compScoring[card] == 0) {
                                pkrSelected[card] = 1;
                                i--;
                        }
                }
        } else {
                if (compRank >= HAND_STRAIGHT) {
                        nc = 0;
                } else {
                        if (compRank == HAND_THREE_OF_A_KIND) {
                                nc = 2;
                                for (i = 0; i < 5; i++)
                                        if (compScoring[i] == 0)
                                                pkrSelected[i] = 1;
                        } else if (compRank == HAND_TWO_PAIR) {
                                nc = 1;
                                for (i = 0; i < 5; i++)
                                        if (compScoring[i] == 0)
                                                pkrSelected[i] = 1;
                        } else if (compRank == HAND_ONE_PAIR) {
                                nc = 3;
                                for (i = 0; i < 5; i++)
                                        if (compScoring[i] == 0)
                                                pkrSelected[i] = 1;
                        } else if (compRank == HAND_HIGH_CARD) {
                                nc = 4;
                                for (card = 0, i = 0; i < 5; i++) {
                                        if (compHand[i] % CARDS_PER_SUIT > compHand[card] % CARDS_PER_SUIT)
                                                card = i;
                                }
                                for (i = 0; i < 5; i++)
                                        if (i != card)
                                                pkrSelected[i] = 1;
                        }
                        /* No final else: ranks 0..3 are all covered,
                           and >= 4 already cleared nc. */
                }
        }

        if (nc == 0) {
                cardMessage("I'll stay!");
                gameTick(8);
                return;
        }

        pkrMsgTake[10] = nc + '0';
        if (nc == 1) {
                pkrMsgTake[16] = '.';
                pkrMsgTake[17] = '\0';
        } else {
                pkrMsgTake[16] = 's';
                pkrMsgTake[17] = '.';
        }
        cardMessage(pkrMsgTake);
        gameTick(8);

        for (i = 0; i < 5; i++) {
                if (pkrSelected[i] == 1) {
                        dm = YES;
                        while (dm != NO) {
                                card = rndRng(0, 51);
                                dm   = NO;
                                for (n = 0; n < 5; n++) {
                                        if (compHand[n] == card) dm = YES;
                                        if (plyrHand[n] == card) dm = YES;
                                }
                                n = pkrNumDisc;
                                while (n--) {
                                        if (pkrDiscPile[n] == card) dm = YES;
                                }
                                pkrDiscPile[pkrNumDisc] = compHand[i];
                                pkrNumDisc++;
                                compHand[i] = card;
                                cardDraw(CARD_HIGHLIGHT, i, 0);
                                gameTick(3);
                        }
                }
        }
        for (i = 0; i < 5; i++) {
                if (pkrSelected[i] == 1) {
                        cardDraw(CARD_BACK, i, 0);
                        gameTick(1);
                }
        }
}

/* potToWinner: transfer pot to winner one chip per tick
   (winner=0 -> computer, winner=1 -> player). */

void
potToWinner(winner)
short   winner;
{
        /* The pot decrement is the loop condition itself -- `while
           (n--)` loads the value, subtracts straight to memory and
           tests the OLD copy -- so it runs one past zero and the tail
           assignment puts the pot back to 0. */
        while (potChips--) {
                if (winner == 0) {
                        compChips++;
                        dispCompChips();
                        dispPot();
                        gameTick(0);
                } else {
                        plyrChips++;
                        dispPlyrChips();
                        dispPot();
                        gameTick(0);
                }
        }
        potChips = 0;
}

/* pkrPlyrBet: player betting UI: F1 Bet (hold), F3 Enter, F5 Pass/Clr.
   Returns 0 normally, -1 on timeout. */

static short
pkrPlyrBet(str)
char *  str;
{
        /* Two locals: the key and a flag that ends the first prompt
           loop; the second loop is a plain while (1). */
        short   r;
        short   go;

        pkrBet  = 0;
        pkrPassed = NO;
        cardMessage(str);
        panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
        printString("F1 Bet",       KEYMENU_X, KEYMENU_LINE1, COLOR_red);
        printString("F3 Enter",     KEYMENU_X, KEYMENU_LINE2, COLOR_red);
        printString("F5 Pass/Clr", KEYMENU_X, KEYMENU_LINE3, COLOR_red);
        go = 0;
        while (!go) {
                r = cardKeyInput(KEY_F1, KEY_F3, KEY_F5);
                if (r == PK_IN_TIMEOUT)
                        return -1;
                if (r == PK_IN_ARG_C) {
                        pkrPassed = YES;
                        return 0;
                }
                if (r == PK_IN_ARG_A) {
                        if (plyrChips == 0)
                                return -1;
                        pkrAddChips(1, 1);
                        go = 1;
                        break;
                }
        }
        while (1) {
                r = cardKeyInput(KEY_F1, KEY_F3, KEY_F5);
                if (r == PK_IN_TIMEOUT)
                        return -1;
                if (r == PK_IN_ARG_B && pkrBet != 0)
                        return 0;
                if (r == PK_IN_ARG_A)
                        pkrAddChips(1, 1);
                if (r == PK_IN_ARG_C) {
                        if (pkrBet == 0) {
                                pkrPassed = YES;
                                return 0;
                        }
                        plyrChips += pkrBet;
                        potChips -= pkrBet;
                        pkrBet   = 0;
                        dispPlyrChips();
                        dispPot();
                }
        }
}

/* pkrAddChips: animated chip transfer.  who=0 computer / 1 player.
   Caps pkrBet at 20. */

static void
pkrAddChips(who, n)
short   who;
short   n;
{
        /* No local: the count is decremented in the loop condition
           (the argument itself), and the two side branches are
           written out in order. */
        if (pkrBet == 20)
                return;
        while (n--) {
                if (who == 0 && compChips == 0)
                        return;
                if (who == 1 && plyrChips == 0)
                        return;
                if (who == 0) {
                        compChips--;
                        dispCompChips();
                        potChips++;
                        dispPot();
                        pkrBet++;
                }
                if (who == 1) {
                        plyrChips--;
                        dispPlyrChips();
                        potChips++;
                        dispPot();
                        pkrBet++;
                }
        }
}

/* pkrDealHands: deal 5-card hands.  Draws 10 unique random cards; player
   face-up, computer face-down. */

static void
pkrDealHands()
{
        /* Declaration order must stay: counter first, then the drawn
           card, the inner counter and the duplicate flag last. */
        short   i;
        short   c;
        short   j;
        short   dup;

        for (i = 0; i < 5; i++) {
                compHand[i] = CARD_NONE;
                plyrHand[i] = CARD_NONE;
        }
        for (i = 0; i < 5; i++) {
                dup = YES;
                while (dup != NO) {
                        c   = rndRng(0, 51);
                        dup = NO;
                        for (j = 0; j < 5; j++) {
                                if (compHand[j] == c || plyrHand[j] == c)
                                        dup = YES;
                        }
                }
                compHand[i] = c;
                dup = YES;
                while (dup != NO) {
                        c   = rndRng(0, 51);
                        dup = NO;
                        for (j = 0; j < 5; j++) {
                                if (compHand[j] == c || plyrHand[j] == c)
                                        dup = YES;
                        }
                }
                plyrHand[i] = c;
        }
        panelErase(70, 10, 219, 62);
        for (i = 0; i < 5; i++) {
                cardDraw(plyrHand[i], i, 1);
                gameTick(3);
                cardDraw(CARD_BACK, i, 0);
                gameTick(3);
        }
}

/* cardDraw: blit one card sprite (15x23) at slot xi of row yi.
   card=CARD_BACK selects cardMfdb[52]; 0..51 index directly. */

void
cardDraw(card, xi, yi)
short   card;
short   xi;
short   yi;
{
        short   x;
        short   y;

        if (yi == 0) {
                x = cardXComp[xi];
                y = cardYComp[xi];
        } else {
                x = cardXPlyr[xi];
                y = cardYPlyr[xi];
        }
        blitRect(vdiHandle, S_ONLY,
                              (long) &cardMfdb[card], (long) &cardTableMfdb,
                              0, 0, 15, 23,
                              x, y, x + 15, y + 23);
}

/* cardLoad (in parts/) must sit here, ahead of dispCompChips. */
#include "parts/cardLoad.c"

/* cardKeyInput: wait for one of F-keys a/b/c or digits 1..5.
   Returns 1..8 for a/b/c/1/2/3/4/5, or -1 on timeout. */

short
cardKeyInput(a, b, c)
short   a;
short   b;
short   c;
{
        short   ch;

        while (1) {
                gameTick(0);
                ch = mgWaitKey();
                if (ch == a) return 1;
                if (ch == b) return 2;
                if (ch == c) return 3;
                if (ch == 0x31) return 4;         /* '1' */
                if (ch == 0x32) return 5;         /* '2' */
                if (ch == 0x33) return 6;         /* '3' */
                if (ch == 0x34) return 7;         /* '4' */
                if (ch == 0x35) return 8;         /* '5' */
                if (mgTimedOut != NO)
                        return -1;
        }
}

/* dispCompChips: display computer money count in the top-left panel, as
   3 hand-formatted, space-padded digits.  The assignments nested in
   expressions and the spare str[] cells are the original's shape. */

void
dispCompChips()
{
        char    str[10];
        short   rem;

        panelErase(5, 10, 31, 20);
        str[3] = '\0';
        str[0] = (str[8] = compChips / 100) + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (rem = compChips % 100) / 10;
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = rem % 10;
        str[2] = str[4] + '0';
        printString(str, 5, 18, COLOR_black);
}

/* dispPlyrChips: display player money (same 3-digit format as dispCompChips). */

void
dispPlyrChips()
{
        char    str[10];
        short   rem;

        panelErase(5, 50, 31, 60);
        str[3] = '\0';
        str[0] = (str[8] = plyrChips / 100) + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (rem = plyrChips % 100) / 10;
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = rem % 10;
        str[2] = str[4] + '0';
        printString(str, 5, 58, COLOR_black);
}

/* dispPot: display the pot amount in the middle panel. */

void
dispPot()
{
        char    str[10];
        short   rem;

        panelErase(31, 30, 57, 40);
        str[3] = '\0';
        str[0] = (str[8] = potChips / 100) + '0';
        if (str[0] == '0')
                str[0] = ' ';
        str[6] = (rem = potChips % 100) / 10;
        if (str[0] == ' ' && str[6] == '\0')
                str[1] = ' ';
        else
                str[1] = str[6] + '0';
        str[4] = rem % 10;
        str[2] = str[4] + '0';
        printString(str, 31, 38, COLOR_black);
}

/* pkrAnte: opening prompt "Ante up to play." + F1 Ante / F10 Quit.
   On F1: both players contribute 1 chip.  On F10/timeout: sets cardQuit. */

static void
pkrAnte()
{
        short   r;

        potChips = 0;
        panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
        printString("F1  Ante", KEYMENU_X, KEYMENU_LINE1, COLOR_red);
        printString("F10 Quit", KEYMENU_X, KEYMENU_LINE3, COLOR_red);
        cardMessage("Ante up to play.");
        r = 0;
        cardQuit = NO;
        while (r != PK_IN_ARG_A && r != PK_IN_ARG_C && r != PK_IN_TIMEOUT)
                r = cardKeyInput(KEY_F1, PK_IN_UNUSED, KEY_F10);
        if (r == PK_IN_ARG_C || r == PK_IN_TIMEOUT) {
                cardQuit = YES;
                return;
        } else if (plyrChips == 0) {
                cardMessage("Sorry, you're all out!!!");
                gameTick(30);
                cardQuit = YES;
        } else if (compChips == 0) {
                cardMessage("I'm all out!!!");
                gameTick(30);
                cardQuit = YES;
        } else {
                panelErase(5, 63, 319, 75);
                plyrChips--;
                dispPlyrChips();
                potChips++;
                dispPot();
                compChips--;
                dispCompChips();
                potChips++;
                dispPot();
        }
}

/* cardMessage: print a green status message in the bottom info bar. */

void
cardMessage(str)
char *  str;
{
        panelErase(5, 63, 319, 75);
        printString(str, 5, 71, COLOR_green);
}

/* popCard: pop card from top of `pile`; shift remaining entries down.
   Returns -1 if empty (a plain -1, not CARD_NONE). */

short
popCard(pile, count)
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

/* pushCard: append val at pile[*idx]; increment idx. */

void
pushCard(pile, idx, val)
short * pile;
short * idx;
short   val;
{
        pile[*idx] = val;
        (*idx)++;
}

static void     pkrShowdown();

/* playWar: WAR mini-game main loop.
   Init: Malloc, load cards, mgSetup, 400-swap shuffle, split 26/26.
   Per-round: reveal cards, compare mod-13, resolve win/loss/tie. */

void
playWar()
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

        cardImages = (short *) Malloc(10400L);
        if (cardImages == (short *) 0)
                outOfMemory();
        cardLoad();
        mgSetup();

        compChips = 26;
        plyrChips = 26;
        potChips = 0;

        /* Deck 0..51 then Fisher-Yates-lite 400-swap shuffle. */
        for (ikey = 0; ikey < 52; ikey++)
                warDeck[ikey] = ikey;
        j = 400;
        while (j--) {
                ikey = rndRng(0, 51);
                do {
                        cidx = rndRng(0, 51);
                } while (ikey == cidx);
                t = warDeck[cidx];
                warDeck[cidx] = warDeck[ikey];
                warDeck[ikey] = t;
        }
        ikey = 0;
        for (cidx = 0; ikey < 52; cidx++) {
                compPile[cidx] = warDeck[ikey];
                ikey++;
                plyrPile[cidx] = warDeck[ikey];
                ikey++;
        }

        dispCompChips();
        dispPlyrChips();
        dispPot();

        /* Per-round loop: a label and explicit gotos, not a for(;;)
           -- every round-end branches straight back here rather than
           to a loop-bottom edge, as in the original.
           1985 bug: the bare `dispPlyrChips;` is a call whose parentheses
           were left off, so Alcyon just loads its address and drops
           it.  Kept on purpose. */
round:
                dispCompChips();
                dispPlyrChips;
                dispPot();
                panelErase(5, 63, 319, 75);
                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                panelErase(70, 10, 219, 62);

                /* Every exit is a goto: the two message blocks and
                   the cleanup are labels the war round jumps back
                   into, and the cleanup returns. */
                if (compChips == 0) {
out_of_cards:
                        cardMessage("I'm out of cards! You're too good!");
                        gameTick(20);
cleanup:
                        textTimer  = 0;
                        keysBlocked = NO;
                        Mfree(cardImages);
                        hideMouse();
                        return;
                }
                if (plyrChips == 0) {
no_cards:
                        cardMessage("No cards, huh? Better luck next time.");
                        gameTick(20);
                        goto cleanup;
                }

                gameTick(5);
                plyrWarCards[0] = popCard(plyrPile, &plyrChips);
                potChips++;
                cardDraw(CARD_BACK, 0, 1);
                dispPot();
                dispPlyrChips();
                gameTick(3);
                compWarCards[0] = popCard(compPile, &compChips);
                potChips++;
                cardDraw(compWarCards[0], 0, 0);
                dispPot();
                dispCompChips();

                cardMessage("Show me your card, Ace.");
                printString("F1  Show", KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                printString("F10 Quit", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
                ikey = 0;
                while (ikey != PK_IN_ARG_A && ikey != PK_IN_ARG_B)
                        ikey = cardKeyInput(KEY_F1, KEY_F10, PK_IN_UNUSED);
                if (ikey == PK_IN_ARG_B)
                        goto cleanup;

                cardDraw(plyrWarCards[0], 0, 1);
                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                gameTick(5);

                /* Both ranks land in locals before the compare, and
                   the loser's branch recomputes them the other way
                   round. */
                if ((ikey = plyrWarCards[0] % CARDS_PER_SUIT) > (cidx = compWarCards[0] % CARDS_PER_SUIT)) {
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
                        cardMessage(sp);
                        gameTick(8);
                        potToWinner(1);
                        panelErase(70, 10, 219, 62);
                        plyrChips -= 2;
                        pushCard(plyrPile, &plyrChips, plyrWarCards[0]);
                        pushCard(plyrPile, &plyrChips, compWarCards[0]);
                        goto round;
                } else if ((ikey = compWarCards[0] % CARDS_PER_SUIT) > (cidx = plyrWarCards[0] % CARDS_PER_SUIT)) {
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
                        cardMessage(sp);
                        saved_head_frame = headFrame;
                        saved_head_mode  = headMode;
                        peekAround();
                        headMode = saved_head_mode;
                        gameTick(8);
                        potToWinner(0);
                        panelErase(70, 10, 219, 62);
                        compChips -= 2;
                        pushCard(compPile, &compChips, plyrWarCards[0]);
                        pushCard(compPile, &compChips, compWarCards[0]);
                        headFrame = saved_head_frame;
                        goto round;
                } else {
                        /* Tie -> war round. */
                        ikey = warRound();
                        if (mgTimedOut != NO)
                                goto cleanup;
                        if (ikey == -1)
                                goto out_of_cards;
                        if (ikey == -2)
                                goto no_cards;
                        goto round;
                }
}

/* warRound: nested war round.  Draw 3 face-down + 1 face-up each.
   On tie, loops with warDepth++.
   Returns 0 = normal, -1 = computer out / user quit, -2 = player out. */

static short
warRound()
{
        /* Declaration order and the unused local must stay: removing
           or reordering them changes the compiled code. */
        short   idx;
        short   drawn;
        short   pot;
        short   prank;
        short   crank;
        short   unused;

        warDepth = 0;
        for (idx = 1; idx < 52; idx++) {
                compWarCards[idx] = -1;
                plyrWarCards[idx] = -1;
        }
        for (;;) {
                cardMessage("... WAR!! ...");
                gameTick(10);
                if (compChips == 0)
                        return -1;
                if (plyrChips == 0)
                        return -2;

                for (idx = 1; idx < 4; idx++) {
                        if (plyrChips == 1)
                                break;
                        if (compChips == 1)
                                break;
                        drawn = popCard(plyrPile, &plyrChips);
                        plyrWarCards[warDepth * 4 + idx] = drawn;
                        potChips++;
                        cardDraw(CARD_BACK, idx, 1);
                        dispPot();
                        dispPlyrChips();
                        gameTick(3);
                        drawn = popCard(compPile, &compChips);
                        compWarCards[warDepth * 4 + idx] = drawn;
                        potChips++;
                        cardDraw(CARD_BACK, idx, 0);
                        dispPot();
                        dispCompChips();
                        gameTick(3);
                }

                /* Final face-up card each. */
                drawn = popCard(plyrPile, &plyrChips);
                plyrWarCards[warDepth * 4 + idx] = drawn;
                potChips++;
                cardDraw(CARD_BACK, idx, 1);
                dispPot();
                dispPlyrChips();
                gameTick(3);
                drawn = popCard(compPile, &compChips);
                compWarCards[warDepth * 4 + idx] = drawn;
                potChips++;
                cardDraw(drawn, idx, 0);
                dispPot();
                dispCompChips();
                gameTick(3);

                cardMessage("Let's see what you've got...");
                printString("F1 Show", KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                while (cardKeyInput(KEY_F1, PK_IN_UNUSED, PK_IN_UNUSED) != PK_IN_ARG_A) {
                        if (mgTimedOut != NO)
                                return -1;
                }
                cardDraw(plyrWarCards[warDepth * 4 + idx], idx, 1);
                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                gameTick(5);

                if ((prank = plyrWarCards[warDepth * 4 + idx] % CARDS_PER_SUIT) >
                    (crank = compWarCards[warDepth * 4 + idx] % CARDS_PER_SUIT)) {
                        /* Player wins the war round. */
                        cardMessage("You win the war!!!");
                        gameTick(8);
                        while (--idx) {
                                cardDraw(compWarCards[warDepth * 4 + idx], idx, 0);
                                gameTick(1);
                        }
                        gameTick(10);
                        pot = potChips;
                        potToWinner(1);
                        plyrChips -= pot;
                        for (idx = 0; compWarCards[idx] != -1; idx++) {
                                pushCard(plyrPile, &plyrChips, plyrWarCards[idx]);
                                pushCard(plyrPile, &plyrChips, compWarCards[idx]);
                        }
                        return 0;
                }
                if ((prank = plyrWarCards[warDepth * 4 + idx] % CARDS_PER_SUIT) <
                    (crank = compWarCards[warDepth * 4 + idx] % CARDS_PER_SUIT)) {
                        /* Computer wins the war round. */
                        cardMessage("I win the war!!!");
                        gameTick(8);
                        while (--idx) {
                                cardDraw(plyrWarCards[warDepth * 4 + idx], idx, 1);
                                gameTick(1);
                        }
                        gameTick(10);
                        pot = potChips;
                        potToWinner(0);
                        compChips -= pot;
                        for (idx = 0; compWarCards[idx] != -1; idx++) {
                                pushCard(compPile, &compChips, plyrWarCards[idx]);
                                pushCard(compPile, &compChips, compWarCards[idx]);
                        }
                        return 0;
                }
                warDepth++;
        }
}

/* playBlackjack: BLACKJACK main game loop.
   Bet-entry (F1 add, F3 enter, F5 clear, 20 cap), deal, natural check,
   optional split, double-down, hit/stand rounds, dealer plays, settle.
   The labels and gotos are the original's control flow and must stay.
   Alcyon 8-char link-name truncation prevents a body/wrapper split. */

void
playBlackjack()
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

        cardImages = (short *) Malloc(0x28a0L);
        if (cardImages == (short *) 0)
                outOfMemory();
        cardLoad();
        mgSetup();
        compChips = 400;
        plyrChips = 400;
        dispCompChips();
        dispPlyrChips();

        /* A label and gotos, not a loop statement: the round tick sits
           at the top and is skipped on the first pass. */
        goto round;
next_round:
        gameTick(24);
round:
                panelErase(70, 10, 219, 62);
                panelErase(31, 43, 57, 53);
                bjBetMain = 0;
                bjBetSplit = 0;
                bjDidSplit = 0;
                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                printString("F1  Bet",  KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                printString("F10 Quit", KEYMENU_X, KEYMENU_LINE3, COLOR_red);
                cardMessage("What's your bet?");
                bjKey  = 0;
                cardQuit = NO;
                while (bjKey != PK_IN_ARG_A && bjKey != PK_IN_ARG_C)
                        bjKey = cardKeyInput(KEY_F1, PK_IN_UNUSED, KEY_F10);
                if (bjKey == PK_IN_ARG_C) {
cleanup:
                        textTimer  = 0;
                        keysBlocked = NO;
                        Mfree(cardImages);
                        hideMouse();
                        return;
                }

                for (br = 0; br < 5; br++) {
                        compHand[br]  = CARD_NONE;
                        plyrHand[br]  = CARD_NONE;
                        bjSplitHand[br] = CARD_NONE;
                }
                panelErase(70, 10, 219, 62);
                if (plyrChips == 0) {
                        cardMessage("Game's over. I win.");
                        gameTick(20);
                        goto cleanup;
                }
                plyrChips--;
                dispPlyrChips();
                bjBetMain++;
                bjShowBet(1);
                pkrBet = 1;
                printString("F3  Enter", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
                printString("F10 Quit",  KEYMENU_X, KEYMENU_LINE3, COLOR_lt_grey);
                printString("F5  Clear", KEYMENU_X, KEYMENU_LINE3, COLOR_red);

                bjKey = 0;
                while (1) {
                        bjKey = 0;
                        while (bjKey != PK_IN_ARG_A && bjKey != PK_IN_ARG_B &&
                               bjKey != PK_IN_ARG_C && bjKey != PK_IN_TIMEOUT) {
                                gameTick(0);
                                bjKey = cardKeyInput(KEY_F1, KEY_F3, KEY_F5);
                        }
                        if (mgTimedOut != NO) goto cleanup;
                        if (bjKey == PK_IN_ARG_C) {
                                plyrChips += pkrBet;
                                pkrBet  = 0;
                                bjBetMain = 0;
                                bjShowBet(1);
                                dispPlyrChips();
                                pkrDiscPile[10] = CARD_BJ_STEP;
                                break;
                        }
                        if (bjKey == PK_IN_ARG_A) {
                                if (plyrChips == 0) {
                                        cardMessage("Game's over. I win.");
                                        cardQuit = YES;
                                        break;
                                }
                                if (pkrBet == 20)
                                        continue;
                                plyrChips--;
                                dispPlyrChips();
                                bjBetMain++;
                                bjShowBet(1);
                                pkrBet++;
                        }
                        if (bjKey == PK_IN_ARG_B) {
                                break;
                        }
                }

                if (pkrDiscPile[10] != CARD_BJ_STOP) {
                        pkrDiscPile[10] = CARD_BJ_STOP;
                        goto round;
                }
                if (cardQuit != NO) {
                        gameTick(20);
                        goto cleanup;
                }
                cardMessage(" ");
                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                bjDealCard(plyrHand, 0);
                bjDealCard(compHand, 1);
                bjDealCard(plyrHand, 0);
                bjDealCard(compHand, 0);
                gameTick(10);
                br = bjIsNatural(plyrHand);
                hit  = bjIsNatural(compHand);
                if (br != 0 && hit != 0) {
                        cardMessage("You have BLACKJACK...but so do I !!");
                        cardDraw(compHand[0], 0, 0);
                        gameTick(20);
                        bjSettle(&bjBetMain, 1, 2);
                        if (cardQuit != NO) {
                                cardMessage("Game's over. I win.");
                                gameTick(20);
                                goto cleanup;
                        }
                        goto next_round;
                } else if (br != 0) {
                        cardMessage("You have BLACKJACK!!");
                        gameTick(20);
                        phase_snap = bjBetMain;
                        bjSettle(&bjBetMain, 1, 1);
                        if (cardQuit != NO) {
                                cardMessage("I'm all out!!");
                                gameTick(20);
                                goto cleanup;
                        }
                        goto next_round;
                } else if (hit != 0) {
                        cardMessage("I have BLACKJACK!!");
                        gameTick(10);
                        cardDraw(compHand[0], 0, 0);
                        gameTick(20);
                        cardMessage("I win double the bet.");
                        gameTick(20);
                        bjSettle(&bjBetMain, 0, 1);
                        if (cardQuit != NO) {
                                cardMessage("Game's over. I win.");
                                gameTick(20);
                                goto cleanup;
                        }
                        goto next_round;
                }
                /* Neither had a natural.  Split, double-down,
                   hit/stand, dealer -- the meat of the game. */
                bjDidSplit = 0;
                if ((short) plyrHand[0] % CARDS_PER_SUIT ==
                    (short) plyrHand[1] % CARDS_PER_SUIT) {
                        cardMessage("Do you wish to split?");
                        panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                        printString("F1 Split",    KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                        printString("F3 No split", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
                        bjKey = 0;
                        while (bjKey != PK_IN_ARG_A && bjKey != PK_IN_ARG_B && bjKey != PK_IN_TIMEOUT) {
                                gameTick(0);
                                bjKey = cardKeyInput(KEY_F1, KEY_F3, PK_IN_UNUSED);
                        }
                        if (mgTimedOut != NO) goto cleanup;
                        if (bjKey == PK_IN_ARG_A) {
                                bjDidSplit = 1;
                                bjSplitHand[0] = plyrHand[1];
                                plyrHand[1]  = CARD_NONE;
                                cardMessage("Here is your first hand.");
                                bjMatchBet = bjBetMain;
                                cardDraw(CARD_HIGHLIGHT, 1, 1);
                                gameTick(8);
                                bjDealCard(plyrHand, 0);
                                bjNatMain = NO;
                                bjNatSplit = NO;
                                if (bjIsNatural(plyrHand)) {
                                        cardMessage("You have BLACKJACK!!");
                                        gameTick(20);
                                        bjSettle(&bjBetMain, 1, 1);
                                        if (cardQuit != NO) {
                                                cardMessage("I'm all out!!");
                                                gameTick(20);
                                                goto cleanup;
                                        }
                                        bjNatMain = YES;
                                }
                                gameTick(20);
                                cardDraw(CARD_HIGHLIGHT, 0, 1);
                                cardDraw(CARD_HIGHLIGHT, 1, 1);
                                bjBetSplit = 0;
                                bjShowBet(2);
                                cardMessage("Here is your second hand.");
                                cardDraw(bjSplitHand[0], 0, 1);
                                gameTick(10);
                                bjDealCard(bjSplitHand, 0);
                                while (bjBetSplit != bjMatchBet) {
                                        if (plyrChips == 0) {
                                                cardQuit = YES;
                                                break;
                                        }
                                        plyrChips--;
                                        dispPlyrChips();
                                        bjBetSplit++;
                                        bjShowBet(2);
                                        gameTick(0);
                                }
                                if (cardQuit != NO) {
                                        cardMessage("Sorry, you're all out!!");
                                        gameTick(20);
                                        goto cleanup;
                                }
                                if (bjIsNatural(bjSplitHand)) {
                                        cardMessage("You have BLACKJACK!!");
                                        gameTick(20);
                                        bjSettle(&bjBetSplit, 1, 1);
                                        if (cardQuit != NO) {
                                                cardMessage("I'm all out!!");
                                                gameTick(20);
                                                goto cleanup;
                                        }
                                        bjNatSplit = YES;
                                }
                                gameTick(20);
                        }
                }

                /* Double-down / hit-loop phase. */
                if (bjDidSplit != 0 && bjNatMain != NO && bjNatSplit != NO) {
                        goto next_round;
                }
                bjDblMain = NO;
                bjDblSplit = NO;
                bjHitsMain  = CARD_BJ_MAX;
                bjHitsSplit = CARD_BJ_MAX;
                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                printString("F1 Double",    KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                printString("F3 No double", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
                if (bjDidSplit == 0) {
                        if (plyrChips < bjBetMain) bjKey = 2;
                        else {
                                cardMessage("Do you wish to double-down?");
                                bjKey = 0;
                        }
                        while (bjKey != PK_IN_ARG_A && bjKey != PK_IN_ARG_B && bjKey != PK_IN_TIMEOUT) {
                                gameTick(0);
                                bjKey = cardKeyInput(KEY_F1, KEY_F3, PK_IN_UNUSED);
                        }
                        if (mgTimedOut != NO) goto cleanup;
                        if (bjKey == PK_IN_ARG_A) {
                                bjHitsMain = CARD_BJ_STEP;
                                br      = bjBetMain;
                                bjDblMain = YES;
                                while (br--) {
                                        if (plyrChips == 0) {
                                                cardQuit = YES;
                                                break;
                                        }
                                        plyrChips--;
                                        dispPlyrChips();
                                        bjBetMain++;
                                        bjShowBet(1);
                                        gameTick(0);
                                }
                                if (cardQuit != NO) {
                                        cardMessage("Game's over. I win.");
                                        gameTick(20);
                                        goto cleanup;
                                }
                        }
                        /* Redundant re-test of bjDidSplit, already
                           implied by the else.  Kept on purpose: it
                           is part of the original code. */
                } else if (bjDidSplit != 0) {
                        if (bjNatMain == NO) {
                                if (plyrChips < bjBetMain) bjKey = 2;
                                else {
                                        cardMessage("Double-down on your first hand?");
                                        cardDraw(plyrHand[0], 0, 1);
                                        cardDraw(plyrHand[1], 1, 1);
                                        gameTick(0);
                                        bjKey = 0;
                                }
                                while (bjKey != PK_IN_ARG_A && bjKey != PK_IN_ARG_B && bjKey != PK_IN_TIMEOUT) {
                                        gameTick(0);
                                        bjKey = cardKeyInput(KEY_F1, KEY_F3, PK_IN_UNUSED);
                                }
                                if (mgTimedOut != NO) goto cleanup;
                                if (bjKey == PK_IN_ARG_A) {
                                        bjHitsMain = CARD_BJ_STEP;
                                        br      = bjBetMain;
                                        bjDblMain = YES;
                                        while (br--) {
                                                if (plyrChips == 0) {
                                                        cardQuit = YES;
                                                        break;
                                                }
                                                plyrChips--;
                                                dispPlyrChips();
                                                bjBetMain++;
                                                bjShowBet(1);
                                                gameTick(0);
                                        }
                                        if (cardQuit != NO) {
                                                cardMessage("Games over. I win.");
                                                gameTick(20);
                                                goto cleanup;
                                        }
                                }
                        }
                        if (bjNatSplit == NO) {
                                gameTick(10);
                                if (plyrChips < bjBetSplit) bjKey = 2;
                                else {
                                        cardMessage("Double-down on your second hand?");
                                        cardDraw(bjSplitHand[0], 0, 1);
                                        cardDraw(bjSplitHand[1], 1, 1);
                                        gameTick(0);
                                        bjKey = 0;
                                }
                                while (bjKey != PK_IN_ARG_A && bjKey != PK_IN_ARG_B && bjKey != PK_IN_TIMEOUT) {
                                        gameTick(0);
                                        bjKey = cardKeyInput(KEY_F1, KEY_F3, PK_IN_UNUSED);
                                }
                                if (mgTimedOut != NO) goto cleanup;
                                if (bjKey == PK_IN_ARG_A) {
                                        bjHitsSplit = CARD_BJ_STEP;
                                        bjDblSplit  = YES;
                                        br       = bjBetSplit;
                                        while (br--) {
                                                if (plyrChips == 0) {
                                                        cardQuit = YES;
                                                        break;
                                                }
                                                plyrChips--;
                                                dispPlyrChips();
                                                bjBetSplit++;
                                                bjShowBet(2);
                                                gameTick(0);
                                        }
                                        if (cardQuit != NO) {
                                                cardMessage("Game's over. I win.");
                                                gameTick(20);
                                                goto cleanup;
                                        }
                                }
                        }
                }

                /* Hit/stand rounds. */
                if (bjDidSplit == 0) {
                        if (bjPlayHand(plyrHand, 1, "Do you want a hit?") == -1) {
                                if (mgTimedOut != NO)
                                        goto cleanup;
                                cardMessage("You've busted!!!");
                                gameTick(10);
                                while (bjBetMain--) {
                                        compChips++;
                                        dispCompChips();
                                        bjShowBet(1);
                                        gameTick(0);
                                }
                                goto next_round;
                        }
                        panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                } else {
                        for (br = 0; br < 5; br++)
                                cardDraw(CARD_HIGHLIGHT, br, 1);
                        bjBustMain = NO;
                        bjBustSplit = NO;
                        if (bjNatMain == NO) {
                                for (br = 0; br < 5; br++) {
                                        if (plyrHand[br] == CARD_NONE)
                                                break;
                                        cardDraw(plyrHand[br], br, 1);
                                }
                                bjShowBet(1);
                                if (bjPlayHand(plyrHand, 1,
                                                      "Need a hit on your first hand?") == -1) {
                                        if (mgTimedOut != NO) goto cleanup;
                                        cardMessage("Your first hand is busted !!");
                                        gameTick(20);
                                        bjBustMain = YES;
                                        while (bjBetMain--) {
                                                compChips++;
                                                dispCompChips();
                                                bjShowBet(1);
                                                gameTick(0);
                                        }
                                }
                        }
                        if (bjNatSplit == NO) {
                                for (br = 0; br < 5; br++)
                                        cardDraw(CARD_HIGHLIGHT, br, 1);
                                for (br = 0; br < 5; br++) {
                                        if (bjSplitHand[br] == CARD_NONE)
                                                break;
                                        cardDraw(bjSplitHand[br], br, 1);
                                }
                                bjShowBet(2);
                                if (bjPlayHand(bjSplitHand, 1,
                                                      "Need a hit on your second hand?") == -1) {
                                        if (mgTimedOut != NO) goto cleanup;
                                        cardMessage("Your second hand is busted!!");
                                        gameTick(20);
                                        bjBustSplit = YES;
                                        while (bjBetSplit--) {
                                                compChips++;
                                                dispCompChips();
                                                bjShowBet(2);
                                                gameTick(0);
                                        }
                                }
                        }
                }
                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);

                /* Dealer turn + settle. */
                if (bjDidSplit != 0 && (bjBustMain != NO || bjNatMain != NO) &&
                    (bjBustSplit != NO || bjNatSplit != NO))
                        goto next_round;
                if (bjDidSplit != 0 && bjBustMain == NO && bjNatMain == NO) {
                        for (br = 0; br < 5; br++)
                                cardDraw(CARD_HIGHLIGHT, br, 1);
                        cardMessage("Here is your first hand again.");
                        gameTick(20);
                        for (br = 0; br < 5; br++) {
                                if (plyrHand[br] == CARD_NONE)
                                        break;
                                cardDraw(plyrHand[br], br, 1);
                        }
                        bjShowBet(1);
                } else if (bjDidSplit != 0 &&
                           bjBustSplit == NO && bjNatSplit == NO) {
                        for (br = 0; br < 5; br++)
                                cardDraw(CARD_HIGHLIGHT, br, 1);
                        cardMessage("Here is your second hand.");
                        gameTick(20);
                        for (br = 0; br < 5; br++) {
                                if (bjSplitHand[br] == CARD_NONE)
                                        break;
                                cardDraw(bjSplitHand[br], br, 1);
                        }
                        bjShowBet(2);
                }

                cardMessage("Now here's my down card.");
                gameTick(10);
                cardDraw(compHand[0], 0, 0);
                gameTick(20);

                for (br = 0; br < 3; br++) {
                        bjDealerScore = 0;
                        round_ctr = 0;
                        res = bjScore(compHand, 0);
                        rv  = bjScore(compHand, 1);
                        if (0x15 < res && 0x15 < rv) {
                                round_ctr = 1;
                                break;
                        }
                        if (rv <= 21)
                                bjDealerScore = rv;
                        else
                                bjDealerScore = res;
                        if (bjDealerScore >= 17) {
                                cardMessage("I'll stand.");
                                gameTick(20);
                                break;
                        }
                        if (br == 0)
                                cardMessage("I'll take a hit.");
                        else if (br == 1)
                                cardMessage("I'll take another hit.");
                        else if (br == 2)
                                cardMessage("I'll take one more.");
                        gameTick(10);
                        bjDealCard(compHand, 0);
                }
                if (br == 3 && round_ctr == 0) {
                        bjDealerScore = 0;
                        res = bjScore(compHand, 0);
                        rv  = bjScore(compHand, 1);
                        bjDealerScore = res;
                        if (res > 21)
                                round_ctr = 1;
                        else if (rv <= 21)
                                bjDealerScore = rv;
                        else {
                                cardMessage("I'll stand.");
                                gameTick(20);
                        }
                }
                if (round_ctr != 0) {
                        cardMessage("I've busted !!");
                        gameTick(20);
                }

                panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
                if (bjDidSplit == 0) {
                        res = bjScore(plyrHand, 0);
                        rv  = bjScore(plyrHand, 1);
                        if (rv <= 21)
                                bjPlyrScore = rv;
                        else
                                bjPlyrScore = res;
                        if (round_ctr != 0 || bjDealerScore < bjPlyrScore) {
                                cardMessage("You win.");
                                gameTick(20);
                                bjSettle(&bjBetMain, 1, 0);
                        } else if (bjDealerScore == bjPlyrScore) {
                                cardMessage("It's a tie and nobody wins.");
                                gameTick(20);
                                bjSettle(&bjBetMain, 1, 2);
                        } else {
                                cardMessage("I win.");
                                gameTick(20);
                                bjSettle(&bjBetMain, 0, 0);
                        }
                        goto next_round;
                } else {
                        if (bjBustMain == NO && bjNatMain == NO) {
                                for (br = 0; br < 5; br++)
                                        cardDraw(CARD_HIGHLIGHT, br, 1);
                                for (br = 0; br < 5; br++) {
                                        if (plyrHand[br] == CARD_NONE)
                                                break;
                                        cardDraw(plyrHand[br], br, 1);
                                }
                                bjShowBet(1);
                                res = bjScore(plyrHand, 0);
                                rv  = bjScore(plyrHand, 1);
                                if (rv <= 21)
                                        bjPlyrScore = rv;
                                else
                                        bjPlyrScore = res;
                                if (round_ctr != 0 || bjDealerScore < bjPlyrScore) {
                                                cardMessage("You win with your first hand.");
                                                gameTick(20);
                                                bjSettle(&bjBetMain, 1, 0);

                                } else if (bjDealerScore == bjPlyrScore) {
                                                cardMessage("First hand ties, nobody wins.");
                                                gameTick(20);
                                                bjSettle(&bjBetMain, 1, 2);

                                } else {
                                                cardMessage("Your first hand loses.");
                                                gameTick(20);
                                                bjSettle(&bjBetMain, 0, 0);

                                }
                        }
                        if (bjBustSplit == NO && bjNatSplit == NO) {
                                for (br = 0; br < 5; br++)
                                        cardDraw(CARD_HIGHLIGHT, br, 1);
                                for (br = 0; br < 5; br++) {
                                        if (bjSplitHand[br] == CARD_NONE)
                                                break;
                                        cardDraw(bjSplitHand[br], br, 1);
                                }
                                bjShowBet(2);
                                res = bjScore(bjSplitHand, 0);
                                rv  = bjScore(bjSplitHand, 1);
                                if (rv <= 21)
                                        bjPlyrScore = rv;
                                else
                                        bjPlyrScore = res;
                                if (round_ctr != 0 || bjDealerScore < bjPlyrScore) {
                                                cardMessage("You win with your second hand.");
                                                gameTick(20);
                                                bjSettle(&bjBetSplit, 1, 0);

                                } else if (bjDealerScore == bjPlyrScore) {
                                                cardMessage("Second hand ties, nobody wins.");
                                                gameTick(20);
                                                bjSettle(&bjBetSplit, 1, 2);

                                } else {
                                                cardMessage("Your second hand loses.");
                                                gameTick(20);
                                                bjSettle(&bjBetSplit, 0, 0);

                                }
                        }
                }
        goto next_round;
}

/* bjScore: blackjack card value.  ace_mode=0 all aces=1; ace_mode=1
   one ace=11 (soft), rest=1.  Called mode 0 then 1 to pick better
   score without busting.  Rank 12=Ace, 6..11=10, 0..5=rank+2. */

static short
bjScore(hand, ace_mode)
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

/* bjPlayHand: play one blackjack round for `hand` at row.
   bjDblMain/bjDblSplit forced-single-hit modes auto-deal one card + return.
   Otherwise F1 Hit / F3 Stand.  Returns 0 on stand, -1 on bust/timeout. */

static short
bjPlayHand(hand, row, prompt)
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

        if (hand == plyrHand)  cnt_ptr = &bjHitsMain;
        if (hand == bjSplitHand) cnt_ptr = &bjHitsSplit;
        if (hand == compHand)  cnt_ptr = &bjHitsDealer;

        panelErase(KEYMENU_X, KEYMENU_TOP, KEYMENU_RIGHT, KEYMENU_BOTTOM);
        forced = NO;
        if ((hand == plyrHand  && bjDblMain != NO) ||
            (hand == bjSplitHand && bjDblSplit != NO))
                forced = YES;
        else {
                printString("F1 Hit",   KEYMENU_X, KEYMENU_LINE1, COLOR_red);
                printString("F3 Stand", KEYMENU_X, KEYMENU_LINE2, COLOR_red);
        }
        for (i = 0; hand[i] != CARD_NONE; i++)
                cardDraw(hand[i], i, row);

        if (forced == NO)
                cardMessage(prompt);
        else
                cardMessage("Here's your card.");
        if (forced != NO) {
                gameTick(16);
                (*cnt_ptr)--;
                bjDealCard(hand, 0);
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
                bjKey = 0;
                while (bjKey != PK_IN_ARG_A && bjKey != PK_IN_ARG_B && bjKey != PK_IN_TIMEOUT) {
                        gameTick(0);
                        bjKey = cardKeyInput(KEY_F1, KEY_F3, PK_IN_UNUSED);
                }
                if (mgTimedOut != NO)
                        return -1;
                if (bjKey == PK_IN_ARG_B) {
                        cardMessage(" ");   /* the F3-stand path just
                                          blanks the message strip */
                        return 0;
                } else if (bjKey == PK_IN_ARG_A) {
                        (*cnt_ptr)--;
                        bjDealCard(hand, 0);
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
                        cardMessage("You cannot take any more cards.");
                        gameTick(15);
                        return 0;
                }
        }
}

/* bjIsNatural: check for natural blackjack (Ace + T/J/Q/K in first two). */

static short
bjIsNatural(hand)
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

/* bjDealCard: deal one card into hand at next CARD_NONE slot.
   Rejects dups vs compHand/plyrHand/bjSplitHand.  Returns -1 if full. */

static short
bjDealCard(hand, face_down)
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
                        if (compHand[j]  == card ||
                            plyrHand[j]  == card ||
                            bjSplitHand[j] == card)
                                dup = 1;
                }
        }
        hand[i] = card;
        if (hand == compHand)
                row = 0;
        else
                row = 1;
        if (face_down)
                cardDraw(CARD_BACK, i, row);
        else
                cardDraw(card, i, row);
        gameTick(3);
        return 0;
}

/* bjShowBet: display bet with highlight.  sel=1 -> computer bet, else
   player bet.  3-digit format as dispCompChips/dppm/dpot. */

static void
bjShowBet(sel)
short   sel;
{
        char    hund;
        char    tens;
        char    ones;
        char    str[4];
        short   rem;
        short   val;

        if (sel == 1)
                val = bjBetMain;
        else
                val = bjBetSplit;
        panelErase(31, 43, 57, 53);
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
        printString(str, 31, 51, COLOR_black);
}

/* bjSettle: settle a bet.  winner: 0=computer, 1=player.
   mode: 0=normal, 1=natural blackjack double-collect,
         2=split -- suppress the second (player) transfer.
   Sets cardQuit on mid-transfer bankruptcy. */

static void
bjSettle(bet_ptr, winner, mode)
short * bet_ptr;
short   winner;
short   mode;
{
        short   orig;
        short   loc8;

        if (winner == 0) {
                orig = *bet_ptr;
                while ((*bet_ptr)--) {
                        compChips++;
                        dispCompChips();
                        if (bet_ptr == &bjBetMain)
                                bjShowBet(1);
                        else
                                bjShowBet(2);
                        gameTick(0);
                }
                if (mode != 0) {
                        while (orig--) {
                                if (plyrChips == 0) {
                                        cardQuit = YES;
                                        return;
                                }
                                plyrChips--;
                                dispPlyrChips();
                                compChips++;
                                dispCompChips();
                                gameTick(0);
                        }
                }
        } else if (winner == 1) {
                orig = *bet_ptr;
                loc8 = *bet_ptr;
                while ((*bet_ptr)--) {
                        plyrChips++;
                        dispPlyrChips();
                        if (bet_ptr == &bjBetMain)
                                bjShowBet(1);
                        else
                                bjShowBet(2);
                        gameTick(0);
                }
                if (mode != 2) {
                        while (loc8--) {
                                if (compChips == 0) {
                                        cardQuit = YES;
                                        break;
                                }
                                compChips--;
                                dispCompChips();
                                plyrChips++;
                                dispPlyrChips();
                                gameTick(0);
                        }
                }
                if (mode == 1) {
                        while (orig--) {
                                if (compChips == 0) {
                                        cardQuit = YES;
                                        return;
                                }
                                compChips--;
                                dispCompChips();
                                plyrChips++;
                                dispPlyrChips();
                                gameTick(0);
                        }
                }
        }
}

