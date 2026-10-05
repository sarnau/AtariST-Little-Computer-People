/*
 * parts/playDoorbell.c -- included by stx_u3.c immediately after handleKey;
 * never compiled on its own.
 */
void playDoorbell() { sfxSelect(SFX_DOORBELL,  4L); }
