/*
 * Draws "Guess #N?" for the current anagram attempt.
 */

void
anaDrawPrompt(guess)
short   guess;
{
        anaClrGuess();
        printString(anaPrompts[guess - 1], 166, 57, COLOR_black);
}
