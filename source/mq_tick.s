******************************************************************************
*
* mq_tick.s -- MFP Timer-A interrupt handler for the MIDI sequencer and
*              the PSG envelopes.
*
* Cannot be written in Alcyon C: it uses `move sr,dn` and `move dn,sr`
* (privileged 68000 instructions) to raise the IPL to 7 on entry, lower
* it to 5 during the long mq_advs / psg_upEn sub-calls (so higher-
* priority interrupts -- VBL, RS-232 -- can still preempt them), then
* restore it on exit.  Ends with `rte`, not `rts`.
*
* Installed by mq_intim in init.c via
*     xbios(31, 0, 5, 0x28, (long) mq_tick);
*
* Symbols (Alcyon truncates linkage names to 8 characters):
*     g_mtcou    long     master tick counter
*     g_msmsa    byte     MIDI sequencer active
*     psg_ntAc   byte     PSG notes active
*     g_mtpre    word     MIDI tick prescaler
*     g_mtdiv    word     tick divider (psg_upEn runs when it wraps)
*     mi_rlock   word     re-entrancy lock
*     mi_dwrm    word     MIDI direct-write mode
*     psg_upEn            advance the PSG envelopes
*     mq_advs             advance the MIDI sequencer
*
******************************************************************************

	.globl	_mq_tick
	.globl	_g_mtcou
	.globl	_g_msmsa
	.globl	_psg_ntA
	.globl	_g_mtpre
	.globl	_g_mtdiv
	.globl	_mi_rloc
	.globl	_mi_dwrm
	.globl	_mq_advs
	.globl	_psg_upE

* mq_tick: the Timer-A interrupt routine.  Counts g_mtcou, steps the
* PSG envelopes every 4th tick (via g_mtdiv) when the sequencer or a PSG
* note is active, and steps the MIDI sequencer each time g_mtpre runs
* out.  Each sub-call is guarded so it is never re-entered.
_mq_tick:
	ori.w	#$0700,sr		* mask all interrupts (IPL=7)
	addq.l	#1,_g_mtcou		* ++g_mtcou

	tst.b	_g_msmsa		* test g_msmsa (a BYTE here)
	bne.s	L_seqA			* sequencer active
	tst.b	_psg_ntA		* test psg_ntAc (a BYTE here)
	beq	L_ack			* psg idle -> just ack
	subq.w	#1,_g_mtdiv		* --g_mtdiv
	bne	L_ack			* not yet -> ack
	bra.s	L_psg			* fall to PSG call

L_seqA:
	subq.w	#1,_g_mtpre		* --g_mtpre
	subq.w	#1,_g_mtdiv		* --g_mtdiv
	bne.s	L_seq			* divider still ticking

* -----------------------------------------------------------------------
* Sub-call 1: psg_upEn (called every 4 ticks when g_mtdiv wraps)
* -----------------------------------------------------------------------

L_psg:
	move.w	#4,_g_mtdiv		* reset divider
	cmpi.w	#1,_mi_rloc		* re-entered?
	beq	L_ack			* yes -> skip
	addq.w	#1,_mi_rloc		* ++mi_rlock
	bclr.b	#5,$fffffa0f		* ack MFP ISRA before long call
	movem.l	d0-d7/a0-a6,-(sp)	* save every reg
	move.w	sr,d0			* save current SR
	andi.w	#$f8ff,d0		* clear IPL bits
	ori.w	#$0500,d0		* set IPL = 5
	move.w	d0,sr			* install
	jsr	_psg_upE		* advance PSG envelopes
	movem.l	(sp)+,d0-d7/a0-a6	* restore regs
	subq.w	#1,_mi_rloc		* --mi_rlock
	bra	L_ack

* -----------------------------------------------------------------------
* Sub-call 2: mq_advs (called when g_mtpre reaches 0 while active)
* -----------------------------------------------------------------------

L_seq:
	tst.w	_g_mtpre		* test g_mtpre
	beq.s	L_seq2			* 0 -> advance
	bpl	L_ack			* positive -> not yet

L_seq2:
	cmpi.w	#1,_mi_dwrm		* mi_dwrm >= 1 ?
	bge	L_ack			* yes -> skip
	cmpi.w	#1,_mi_rloc		* re-entered ?
	beq	L_ack			* yes -> skip
	addq.w	#1,_mi_dwrm		* ++mi_dwrm
	bclr.b	#5,$fffffa0f		* ack MFP ISRA
	movem.l	d0-d7/a0-a6,-(sp)	* save
	move.w	sr,d0			* save SR
	andi.w	#$f8ff,d0		* clear IPL
	ori.w	#$0500,d0		* set IPL = 5
	move.w	d0,sr			* install
	jsr	_mq_advs		* advance MIDI sequencer
	movem.l	(sp)+,d0-d7/a0-a6	* restore
	subq.w	#1,_mi_dwrm		* --mi_dwrm

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

* mi_dwrm: non-zero while mq_advs is running; a tick that finds it set
* skips the sequencer step.
_mi_dwrm:	.ds.w	1
* mi_rlock: non-zero while psg_upEn is running; blocks both sub-calls.
_mi_rloc:	.ds.w	1
* g_mtpre: ticks left until the next sequencer step; the sequencer
* reloads it (mi_tpb / mi_nlp0).
_g_mtpre:	.ds.w	1
* psg_ntAc: non-zero while a PSG note's envelope is running, so the
* envelopes keep being stepped when no song is playing.
_psg_ntA:	.ds.b	1
* g_msmsa: non-zero while a song is playing (set by the sequencer's
* start, cleared when it ends or is stopped).
_g_msmsa:	.ds.b	1
