/*
 * Draws "Guess #N?" for the current anagram attempt.
 *
 * Included by games.c; never compiled on its own.
 */

void
anaDrawPrompt(guess)
short   guess;
{
        anaClrGuess();
        printString(g_aggpr[guess - 1], 166, 57, COLOR_black);
}
