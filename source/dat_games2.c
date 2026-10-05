/*
 * dat_games2.c -- the anagram globals.
 *
 * Alcyon defers a compilation unit's string literals to a pool at the
 * end of its data, in the order it met them.  In the original,
 * g_aggpr's nine prompts and g_agwgm's three messages sit between
 * playWordPuzzle's screen text and playAnagrams's, so the declarations are
 * between those two functions.  The card-geometry tables come along
 * because the globals region keeps the same order.  The position of
 * this file's inclusion must not change.  Never compiled standalone.
 */

#include "types.h"
#include "enums.h"

/* anagram_guess_prompt_strings: shown per attempt.  Each is padded to
   19 characters so it overwrites the previous prompt in place.
   (0..8 -> "Guess #1?"..
   "Guess #9?").  Rendered by anaDrawPrompt at (166, 57). */
/* Ten slots for nine prompts and five for three messages: the original
   sizes both arrays past their initializer lists and Alcyon zero-fills
   the tail with NULLs.  Do not shrink them to the initializer count. */
char *          g_aggpr[10] = {
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

short           g_agacu          = 0;    /* anagram: set when a clue pushed the guess count to 9, allowing one more guess (cleared per round) */

/* Anagram wrong-guess messages; playAnagrams picks one of the three with
   rndRng(0, 2).  The last two slots are NULL. */
char *          g_agwgm[5] = {
        "Nope, have another try.",
        "Sorry, try again.",
        "Missed, try again."
};

/* Card display positions -- 5 slots per row.  Row A = computer
   (y=11 top strip), Row B = player (y=37 middle strip).  X columns
   are spaced 28 pixels apart (15-px card + 13-px gutter). */
short           crd_xa[5]         = { 70, 98, 126, 154, 182 };

short           crd_ya[5]         = { 11, 11, 11, 11, 11 };    /* row A (computer) y per card slot */

short           crd_xb[5]         = { 70, 98, 126, 154, 182 };    /* row B (player) x per card slot */

short           crd_yb[5]         = { 37, 37, 37, 37, 37 };    /* row B (player) y per card slot; all four read by cardDraw */
