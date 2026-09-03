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



/* anagram_guess_prompt_strings: shown per attempt (0..8 -> "Guess #1?"..
   "Guess #9?").  Rendered by ag_sgp at (166, 57). */
char *          g_aggpr[9] = {
        "Guess #1?",
        "Guess #2?",
        "Guess #3?",
        "Guess #4?",
        "Guess #5?",
        "Guess #6?",
        "Guess #7?",
        "Guess #8?",
        "Guess #9?"
};


short           g_agacu          = 0;


char *          g_agwgm[3] = {
        "Nope, try again!",
        "Not quite...",
        "Sorry, wrong guess."
};



/* Card display positions -- 5 slots per row, extracted from Ghidra
   memory at 0x2a4fe / 0x2a508 / 0x2a512 / 0x2a51c.  Row A = computer
   (y=11 top strip), Row B = player (y=37 middle strip).  X columns
   are spaced 28 pixels apart (15-px card + 13-px gutter). */
short           crd_xa[5]         = { 70, 98, 126, 154, 182 };


short           crd_ya[5]         = { 11, 11, 11, 11, 11 };


short           crd_xb[5]         = { 70, 98, 126, 154, 182 };


short           crd_yb[5]         = { 37, 37, 37, 37, 37 };


char *          pk_rm     = "I'll raise 00.";



/* Editable poker prompts.  pk_bm / pk_rm have single-space digit
   slots at fixed offsets; pk_tcm's card count digit + trailing
   period/'s.' get patched in by pk_cdrw.  Buffer widths sized so
   the biggest overwrite (a 2-digit prefix like "20") still fits. */
char *          pk_bm     = "I'll bet 00.  ";


char *          pk_tcm    = "I'll take 0 cards.";
