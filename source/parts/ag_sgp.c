/*
 * Draws "Guess #N?" for the current anagram attempt.
 *
 * Included by games.c; never compiled on its own.
 */

void
ag_sgp(guess)
short   guess;
{
        ag_cgpa();
        strPr(g_aggpr[guess - 1], 166, 57, COLOR_black);
}
