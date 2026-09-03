/*
 * dat_games.c -- the initialized globals that belong to the games
 * OBJECT, in LCP_STX data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The object that
 * owns a stretch of anonymous data is not a guess: a switch table's
 * relocation points into its own function, and a string is emitted in
 * the object that references it.  See CLAUDE.md, "DATA and BSS
 * layout".
 *
 * Not compiled standalone -- included by games.
 */



char *          wp_prm[9] = {
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



char *          wp_succ[6] = {
        "You got it!!",
        "Good going. That's right!",
        "Congratulations. That's it!",
        "I don't believe it!! You're right!",
        "You're pretty good. That's right!",
        "You got that one. How about another?"
};



char *          wp_fail[6] = {
        "Too bad. You missed it.",
        "Better luck next time.",
        "Good try, but that's the wrong answer.",
        "That's not it. How about another try?",
        "Nope.",
        "Not quite."
};



/* anagram_guess_prompt_strings: shown per attempt.  Each is padded to
   19 characters so it overwrites the previous prompt in place.
   (0..8 -> "Guess #1?"..
   "Guess #9?").  Rendered by ag_sgp at (166, 57). */
/* Ten slots for nine prompts and five for three messages: LCP_STX
   sizes both arrays past their initializer lists and Alcyon zero-fills
   the tail (data 0xe08 and 0xe1a..0xe21 are NULL).  Do not shrink them
   to the initializer count. */
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


short           g_agacu          = 0;


char *          g_agwgm[5] = {
        "Nope, have another try.",
        "Sorry, try again.",
        "Missed, try again."
};



/* Card display positions -- 5 slots per row, extracted from Ghidra
   memory at 0x2a4fe / 0x2a508 / 0x2a512 / 0x2a51c.  Row A = computer
   (y=11 top strip), Row B = player (y=37 middle strip).  X columns
   are spaced 28 pixels apart (15-px card + 13-px gutter). */
short           crd_xa[5]         = { 70, 98, 126, 154, 182 };


short           crd_ya[5]         = { 11, 11, 11, 11, 11 };


short           crd_xb[5]         = { 70, 98, 126, 154, 182 };


short           crd_yb[5]         = { 37, 37, 37, 37, 37 };


char *          pk_rm     = "I'll raise __.";



/* Editable poker prompts, patched in place before each is shown.  The
   underscores are the digit slots the original ships -- pk_dbet and
   pk_dppm overwrite the two in pk_bm/pk_rm, pk_cdrw the one in
   pk_tcm (and the trailing "." becomes "s." for a plural draw).
   They are POINTERS, not arrays, so every patch goes through a
   movea.l of the variable first. */
char *          pk_bm     = "I'll bet __.";


char *          pk_tcm    = "I'll take _ cards.";
