******************************************************************************
*
* mq_tick.s -- MFP Timer-A interrupt handler for the MIDI sequencer and
*              the PSG envelopes.
*
* Cannot be written in Alcyon C: it uses `move sr,dn` and `move dn,sr`
* (privileged 68000 instructions) to raise the IPL to 7 on entry, lower
* it to 5 during the long seqAdvance / stepEnvelopes sub-calls (so higher-
* priority interrupts -- VBL, RS-232 -- can still preempt them), then
* restore it on exit.  Ends with `rte`, not `rts`.
*
* Installed by hookTimerA via
*     xbios(31, 0, 5, 0x28, (long) timerAIsr);
*
* Symbols (Alcyon truncates linkage names to 8 characters):
*     timerTicks    long     master tick counter
*     songActive    byte     MIDI sequencer active
*     psgActive   byte     PSG notes active
*     seqCountdown    word     MIDI tick prescaler
*     envDivider    word     tick divider (stepEnvelopes runs when it wraps)
*     envBusy   word     re-entrancy lock
*     seqBusy    word     MIDI direct-write mode
*     stepEnvelopes            advance the PSG envelopes
*     seqAdvance             advance the MIDI sequencer
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

* timerAIsr: the Timer-A interrupt routine.  Counts timerTicks, steps the
* PSG envelopes every 4th tick (via envDivider) when the sequencer or a PSG
* note is active, and steps the MIDI sequencer each time seqCountdown runs
* out.  Each sub-call is guarded so it is never re-entered.
_timerAI:
	ori.w	#$0700,sr		* mask all interrupts (IPL=7)
	addq.l	#1,_timerTi		* ++timerTicks

	tst.b	_songAct		* test songActive (a BYTE here)
	bne.s	L_seqA			* sequencer active
	tst.b	_psgActi		* test psgActive (a BYTE here)
	beq	L_ack			* psg idle -> just ack
	subq.w	#1,_envDivi		* --envDivider
	bne	L_ack			* not yet -> ack
	bra.s	L_psg			* fall to PSG call

L_seqA:
	subq.w	#1,_seqCoun		* --seqCountdown
	subq.w	#1,_envDivi		* --envDivider
	bne.s	L_seq			* divider still ticking

* -----------------------------------------------------------------------
* Sub-call 1: stepEnvelopes (called every 4 ticks when envDivider wraps)
* -----------------------------------------------------------------------

L_psg:
	move.w	#4,_envDivi		* reset divider
	cmpi.w	#1,_envBusy		* re-entered?
	beq	L_ack			* yes -> skip
	addq.w	#1,_envBusy		* ++envBusy
	bclr.b	#5,$fffffa0f		* ack MFP ISRA before long call
	movem.l	d0-d7/a0-a6,-(sp)	* save every reg
	move.w	sr,d0			* save current SR
	andi.w	#$f8ff,d0		* clear IPL bits
	ori.w	#$0500,d0		* set IPL = 5
	move.w	d0,sr			* install
	jsr	_stepEnv		* advance PSG envelopes
	movem.l	(sp)+,d0-d7/a0-a6	* restore regs
	subq.w	#1,_envBusy		* --envBusy
	bra	L_ack

* -----------------------------------------------------------------------
* Sub-call 2: seqAdvance (called when seqCountdown reaches 0 while active)
* -----------------------------------------------------------------------

L_seq:
	tst.w	_seqCoun		* test seqCountdown
	beq.s	L_seq2			* 0 -> advance
	bpl	L_ack			* positive -> not yet

L_seq2:
	cmpi.w	#1,_seqBusy		* seqBusy >= 1 ?
	bge	L_ack			* yes -> skip
	cmpi.w	#1,_envBusy		* re-entered ?
	beq	L_ack			* yes -> skip
	addq.w	#1,_seqBusy		* ++seqBusy
	bclr.b	#5,$fffffa0f		* ack MFP ISRA
	movem.l	d0-d7/a0-a6,-(sp)	* save
	move.w	sr,d0			* save SR
	andi.w	#$f8ff,d0		* clear IPL
	ori.w	#$0500,d0		* set IPL = 5
	move.w	d0,sr			* install
	jsr	_seqAdva		* advance MIDI sequencer
	movem.l	(sp)+,d0-d7/a0-a6	* restore
	subq.w	#1,_seqBusy		* --seqBusy

L_ack:
	bclr.b	#5,$fffffa0f		* final ISRA ack
	rte				* return from exception

* -----------------------------------------------------------------------
* The original keeps these five in the TEXT segment, immediately behind
* the ISR, rather than in data with the other globals -- so they are
* defined here and globals.c leaves them out of the default build.
* The last two are real bytes, not BOOL16 words, which is why the tests
* above address them directly.
* -----------------------------------------------------------------------

* seqBusy: non-zero while seqAdvance is running; a tick that finds it set
* skips the sequencer step.
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
