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


/* sf_pri (Ghidra 0x2b44c, 32-byte array indexed
   by SOUND_EFFECT_ID).  Lower value = higher priority (a new SFX
   preempts the current if the new one's priority <= the current's).
   Notable: SFX 12/13 (DOORBELL, DOORBELL_ECHO) at priority 0 beat
   everything; footsteps 0..5 at 30 lose to everything.
   Dumped verbatim from the data segment -- previous port had guessed
   values (0/5/3/8/etc) that gave wrong preemption. */
char    sf_pri[32] = {          /* STX: one byte per entry */
         30,  30,  30,  30,  30,  30,  15,  15,
         15,  15,  15,  15,   0,   0,  15,  15,
         15,  15,  15,  14,  16,   1,  15,   0,
          0,   0, 205,  77, 115, 116, 117, 100
};



/* g_momap: the "maxPos" argument passed to
   mq_inis at song start.  0 means "no explicit end-of-song
   offset -- let the sequencer walk the event stream to its natural
   terminator" (in which case mq_setp stores -1 into
   g_msmap).  A .SNG file may carry a real byte offset
   here to trigger clean loop-back or fade-out at a specific point.
   Renamed from Ghidra's placeholder gSongMaxPosition_0. */
long            g_momap  = 0;
