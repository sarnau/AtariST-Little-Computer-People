/*
 * dat_sound.c -- the initialized globals that belong to the stx_u4
 * OBJECT, in the original's data order, which must not change.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The object that
 * owns a stretch of anonymous data is not a guess: a switch table's
 * relocation points into its own function, and a string is emitted in
 * the object that references it.  See docs/history.md, "DATA and BSS
 * layout".
 *
 */

/* The first item of this object's data, and nothing in the program
   references it -- so its meaning rests on the
   bytes alone: 08 00 09 00 0a 00 ff 00.  8, 9 and 10 are the PSG's
   three amplitude registers and 0xff reads as the terminator, each
   entry followed by a zero byte.  Kept verbatim; do not "simplify" it
   to a short[4], which would lay the bytes down as 00 08 00 09 ...  */
char            psgVolRegs[8] = { 8, 0, 9, 0, 10, 0, -1, 0 };

/* sfxPriority: SOUND_EFFECT_ID -> priority, one byte per entry.  Lower
   value = higher priority (a new effect preempts the current one when
   its priority is <= the current one's).  SFX 12/13 (DOORBELL,
   DOORBELL_ECHO) at 0 beat everything; footsteps 0..5 at 30 lose to
   everything.

   Twenty-six entries: one per sound effect.  The bytes that follow
   are the start of the file signature below, not more priorities. */
char    sfxPriority[26] = {
         30,  30,  30,  30,  30,  30,  15,  15,
         15,  15,  15,  15,   0,   0,  15,  15,
         15,  15,  15,  14,  16,   1,  15,   0,
          0,   0
};

/* The ten-byte header every SOUNDS.LCP and .SNG file starts with:
   0xCD, "Mstudio", 0xCD, 0x02 -- Activision's Music Studio signature.
   Declared but never referenced: loadSounds and mq_inti skip the header by
   a fixed byte count rather than comparing it.  Eleven bytes with the
   terminator, which Alcyon pads to twelve. */
char            studioSig[12] = "\315Mstudio\315\002";

/* songMaxPos: the "maxPos" argument passed to
   startSong at song start.  0 means "no explicit end-of-song
   offset -- let the sequencer walk the event stream to its natural
   terminator" (in which case initSongState stores -1 into
   g_msmap).  A .SNG file may carry a real byte offset
   here to trigger clean loop-back or fade-out at a specific point. */
long            songMaxPos  = 0;
