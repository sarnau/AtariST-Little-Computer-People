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
