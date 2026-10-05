******************************************************************************
*
* mq_tick.s -- the MFP Timer-A interrupt: the clock for the MIDI
*              sequencer and the PSG software envelopes.
*
* hookTimerA installs it with Xbtimer(timer A, prescaler /64, data $28):
* 2.4576 MHz / 64 / 40 = 960 interrupts a second.  Each one
*   - counts timerTicks,
*   - every fourth tick (envDivider) runs stepEnvelopes -- 240 Hz -- if
*     a song is playing or a PSG note is still sounding,
*   - while a song is playing, counts seqCountdown down and runs
*     seqAdvance when it reaches zero or below.  On a tick where the
*     envelope divider wraps the sequencer is not looked at; the count
*     has gone negative by the next tick, which then runs it.
*
* Interrupt levels: the CPU enters at IPL 6, the MFP's level; the
* handler raises that to 7 at once, and drops to IPL 5 for the two long
* calls.  Before each it clears Timer A's in-service bit
* (bit 5 of the MFP's ISRA; the MFP runs in software end-of-interrupt
* mode), so the other MFP interrupts -- keyboard/MIDI ACIA, the other
* timers, RS-232, all at level 6 -- can be serviced meanwhile.  VBL
* (level 4) and HBL stay blocked until the rte.
*
* Each long call has a busy flag so a slow step is never re-entered:
* envBusy for stepEnvelopes (it also holds off the sequencer), seqBusy
* for seqAdvance.  seqBusy doubles as "running inside the interrupt" for
* sendMidiEvent, which then pokes the ACIA and PSG directly (aciaWrite,
* psgWrite) instead of going through the XBIOS.
*
* Hand assembly because C cannot express it: `move dn,sr` to change the
* interrupt level, `rte` to return.
*
* Symbols (linkage names keep 8 characters):
*     timerTicks     long   tick counter, reset when a song starts
*     envDivider     word   ticks until the next stepEnvelopes, reloaded
*                           with 4
*     seqCountdown   word   ticks until the next seqAdvance, reloaded by
*                           the sequencer
*     envBusy        word   stepEnvelopes running
*     seqBusy        word   seqAdvance running
*     songActive     byte   a song is playing
*     psgActive      byte   a PSG note's envelope is still running
*     stepEnvelopes         the PSG envelope step (midi_seq.c)
*     seqAdvance            the MIDI sequencer step (midi_seq.c)
*
******************************************************************************

	.globl	_timerAI
	.globl	_timerTi
	.globl	_songAct
	.globl	_psgActi
	.globl	_seqCoun
	.globl	_envDivi
	.globl	_envBusy
	.globl	_seqBusy
	.globl	_seqAdva
	.globl	_stepEnv

* timerAIsr: see the header.
_timerAI:
	ori.w	#$0700,sr		* IPL 7 while the counters change
	addq.l	#1,_timerTi		* ++timerTicks

	tst.b	_songAct		* test songActive (a BYTE here)
	bne.s	L_seqA			* sequencer active
	tst.b	_psgActi		* test psgActive (a BYTE here)
	beq	L_ack			* nothing to do -> ack
	subq.w	#1,_envDivi		* --envDivider
	bne	L_ack			* not the fourth tick
	bra.s	L_psg			* step the envelopes

L_seqA:
	subq.w	#1,_seqCoun		* --seqCountdown
	subq.w	#1,_envDivi		* --envDivider
	bne.s	L_seq			* not the fourth tick: sequencer

* -----------------------------------------------------------------------
* stepEnvelopes, every fourth tick
* -----------------------------------------------------------------------

L_psg:
	move.w	#4,_envDivi		* reset divider
	cmpi.w	#1,_envBusy		* still running from an earlier tick?
	beq	L_ack			* yes -> skip
	addq.w	#1,_envBusy		* ++envBusy
	bclr.b	#5,$fffffa0f		* end Timer A in service (ISRA bit 5)
	movem.l	d0-d7/a0-a6,-(sp)	* save every reg
	move.w	sr,d0
	andi.w	#$f8ff,d0		* clear the IPL bits
	ori.w	#$0500,d0		* IPL 5: level 6 (the MFP) may interrupt
	move.w	d0,sr
	jsr	_stepEnv		* advance PSG envelopes
	movem.l	(sp)+,d0-d7/a0-a6	* restore regs
	subq.w	#1,_envBusy		* --envBusy
	bra	L_ack

* -----------------------------------------------------------------------
* seqAdvance, once seqCountdown is zero or below
* -----------------------------------------------------------------------

L_seq:
	tst.w	_seqCoun		* test seqCountdown
	beq.s	L_seq2			* zero -> step
	bpl	L_ack			* positive -> not yet; negative -> step

L_seq2:
	cmpi.w	#1,_seqBusy		* sequencer still running?
	bge	L_ack			* yes -> skip
	cmpi.w	#1,_envBusy		* envelopes running?
	beq	L_ack			* yes -> skip
	addq.w	#1,_seqBusy		* ++seqBusy
	bclr.b	#5,$fffffa0f		* end Timer A in service (ISRA bit 5)
	movem.l	d0-d7/a0-a6,-(sp)	* save every reg
	move.w	sr,d0
	andi.w	#$f8ff,d0		* clear the IPL bits
	ori.w	#$0500,d0		* IPL 5: level 6 (the MFP) may interrupt
	move.w	d0,sr
	jsr	_seqAdva		* advance MIDI sequencer
	movem.l	(sp)+,d0-d7/a0-a6	* restore
	subq.w	#1,_seqBusy		* --seqBusy

L_ack:
	bclr.b	#5,$fffffa0f		* end Timer A in service
	rte				* back, with the interrupted SR

* -----------------------------------------------------------------------
* The original keeps these five in the TEXT segment, immediately behind
* the ISR, rather than in data with the other globals -- so they are
* defined here and globals.c leaves them out of the default build.
* The last two are real bytes, not BOOL16 words, which is why the tests
* above address them directly.
* -----------------------------------------------------------------------

* seqBusy: non-zero while seqAdvance is running; a tick that finds it set
* skips the sequencer step, and sendMidiEvent writes to the hardware
* directly while it is set.
_seqBusy:	.ds.w	1
* envBusy: non-zero while stepEnvelopes is running; blocks both sub-calls.
_envBusy:	.ds.w	1
* seqCountdown: ticks left until the next sequencer step; the sequencer
* reloads it (beatTicks / ticksToNext).
_seqCoun:	.ds.w	1
* psgActive: non-zero while a PSG note's envelope is running, so the
* envelopes keep being stepped when no song is playing.
_psgActi:	.ds.b	1
* songActive: non-zero while a song is playing (set by the sequencer's
* start, cleared when it ends or is stopped).
_songAct:	.ds.b	1
