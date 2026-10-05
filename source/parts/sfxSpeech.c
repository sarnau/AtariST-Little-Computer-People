/* Plays SFX_SPEECH (duration 3) through sfxSelect.  The four wrappers
   sit in the original's order: sfxTvClick, sfxSpeech, sfxHeadNod,
   sfxGreeting. */
void sfxSpeech() { sfxSelect(SFX_SPEECH,    3L); }
