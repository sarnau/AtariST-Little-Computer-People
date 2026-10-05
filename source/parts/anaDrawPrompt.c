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
        printString(anaPrompts[guess - 1], 166, 57, COLOR_black);
}
