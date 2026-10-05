/* Plays SFX_GREETING (duration 2) through sfxSelect.  The four wrappers
   sit in the original's order: sfxTvClick, sfxSpeech, sfxHeadNod,
   sfxGreeting. */
void sfxGreeting() { sfxSelect(SFX_GREETING,  2L); }
