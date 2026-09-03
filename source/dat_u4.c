/*
 * dat_u4.c -- the initialized globals that belong to the stx_u4
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
 * Not compiled standalone -- included by stx_u4.
 */


/* The first item of this object's data, at LCP_STX 0x1d68, and
   nothing in the program relocates it -- so its meaning rests on the
   bytes alone: 08 00 09 00 0a 00 ff 00.  8, 9 and 10 are the PSG's
   three amplitude registers and 0xff reads as the terminator, each
   entry followed by a zero byte.  Kept verbatim; do not "simplify" it
   to a short[4], which would lay the bytes down as 00 08 00 09 ...  */
char            psg_vrg[8] = { 8, 0, 9, 0, 10, 0, -1, 0 };


/* sf_pri: SOUND_EFFECT_ID -> priority, one byte per entry.  Lower
   value = higher priority (a new effect preempts the current one when
   its priority is <= the current one's).  SFX 12/13 (DOORBELL,
   DOORBELL_ECHO) at 0 beat everything; footsteps 0..5 at 30 lose to
   everything.

   TWENTY-SIX entries, not 32.  The port's table was dumped from the
   data segment with the wrong extent and swallowed the first six
   bytes of the file signature below -- which is why it ended in
   205, 77, 115, 116, 117, 100, i.e. "\315Mstud".  The reference puts
   g_momap 38 bytes after sf_pri, and those 38 are 26 + the
   signature's 12. */
char    sf_pri[26] = {
         30,  30,  30,  30,  30,  30,  15,  15,
         15,  15,  15,  15,   0,   0,  15,  15,
         15,  15,  15,  14,  16,   1,  15,   0,
          0,   0
};


/* The ten-byte header every SOUNDS.LCP and .SNG file starts with:
   0xCD, "Mstudio", 0xCD, 0x02 -- Activision's Music Studio signature.
   Declared but never referenced: sf_sl and mq_inti skip the header by
   a fixed byte count rather than comparing it.  Eleven bytes with the
   terminator, which Alcyon pads to twelve. */
char            mi_sig[12] = "\315Mstudio\315\002";



/* g_momap: the "maxPos" argument passed to
   mq_inis at song start.  0 means "no explicit end-of-song
   offset -- let the sequencer walk the event stream to its natural
   terminator" (in which case mq_setp stores -1 into
   g_msmap).  A .SNG file may carry a real byte offset
   here to trigger clean loop-back or fade-out at a specific point.
   Renamed from Ghidra's placeholder gSongMaxPosition_0. */
long            g_momap  = 0;
