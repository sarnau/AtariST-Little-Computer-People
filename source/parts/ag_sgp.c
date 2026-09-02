/*
 * parts/ag_sgp.c -- shared body; LCP_ORG links it in games.c,
 * LCP_STX in the 0xdece object (0x8052, after ag_intr).  Files under parts/
 * are never compiled standalone.
 */
/* ag_sgp: draw "Guess #N?" for the current attempt.
   addr: anagram_show_guess_prompt() */

void
ag_sgp(guess)
short   guess;
{
        ag_cgpa();
        strPr(g_aggpr[guess - 1], 166, 57, COLOR_black);
}
