******************************************************************************
*
* mq_tick.s -- MFP Timer-A ISR, byte-faithful port of Ghidra 0x1219a
*              (midi_seq_tick_handler).
*
* Replaces the earlier mq_hlpr.s C-callable wrapper + C mq_tick() combo.
* Cannot be written in Alcyon C 4.14: the ROM version uses `move sr,dn`
* and `move dn,sr` (privileged 68000 instructions) to raise IPL to 7 on
* entry, lower to 5 during the long mq_advs / psg_upEn sub-calls (so
* higher-priority interrupts -- VBL, RS-232 -- can still preempt them),
* then restore on exit.  Ends with `rte`, not `rts`.
*
* Installed by mq_intim in init.c via
*     xbios(31, 0, 5, 0x28, (long) mq_tick);
*
* Ghidra symbol map:
*     0x0004b7ac  g_mtcou    long     master tick counter
*     0x00012271  g_msmsa    byte     midi_sequencer_active
*     0x00012270  psg_ntAc   byte     psg_notes_active
*                                     (Alcyon truncates to _psg_ntA)
*     0x0001226e  g_mtpre    word     midi_tick_prescaler
*     0x0003d130  g_mtdiv    word     midi_tick_divider
*     0x0001226c  mi_rlock   word     midi_reentrant_lock
*     0x0001226a  mi_dwrm    word     midi_direct_write_mode
*     0x000115ae  psg_upEn            psg_process_envelopes
*     0x00010ec2  mq_advs             midi_seq_advance_sequencer
*
* Ghidra reads g_msmsa and psg_ntAc as bytes; our port declares them as
* BOOL16 (short = 2 bytes).  On big-endian 68000 the low byte
* (containing the 0/1 value) sits at address+1, so `tst.b _sym+1` is
* the semantically-correct byte-faithful port of Ghidra's `tst.b`.
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

* -----------------------------------------------------------------------
* _mq_tick @ Ghidra 0x0001219a
* -----------------------------------------------------------------------

_mq_tick:
	ori.w	#$0700,sr		* 1219a  mask all interrupts (IPL=7)
	addq.l	#1,_g_mtcou		* 1219e  ++g_mtcou

	tst.b	_g_msmsa		* 121a4  test g_msmsa (a BYTE here)
	bne.s	L_seqA			* 121aa  sequencer active
	tst.b	_psg_ntA		* 121ac  test psg_ntAc (a BYTE here)
	beq	L_ack			* 121b2  psg idle -> just ack
	subq.w	#1,_g_mtdiv		* 121b6  --g_mtdiv
	bne	L_ack			* 121bc  not yet -> ack
	bra.s	L_psg			* 121c0  fall to PSG call

L_seqA:
	subq.w	#1,_g_mtpre		* 121c2  --g_mtpre
	subq.w	#1,_g_mtdiv		* 121c8  --g_mtdiv
	bne.s	L_seq			* 121ce  divider still ticking

* -----------------------------------------------------------------------
* Sub-call 1: psg_upEn (called every 4 ticks when g_mtdiv wraps)
* -----------------------------------------------------------------------

L_psg:
	move.w	#4,_g_mtdiv		* 121d0  reset divider
	cmpi.w	#1,_mi_rloc		* 121d8  re-entered?
	beq	L_ack			* 121e0  yes -> skip
	addq.w	#1,_mi_rloc		* 121e4  ++mi_rlock
	bclr.b	#5,$fffffa0f		* 121ea  ack MFP ISRA before long call
	movem.l	d0-d7/a0-a6,-(sp)	* 121f2  save every reg
	move.w	sr,d0			* 121f6  save current SR
	andi.w	#$f8ff,d0		* 121f8  clear IPL bits
	ori.w	#$0500,d0		* 121fc  set IPL = 5
	move.w	d0,sr			* 12200  install
	jsr	_psg_upE		* 12202  advance PSG envelopes
	movem.l	(sp)+,d0-d7/a0-a6	* 12208  restore regs
	subq.w	#1,_mi_rloc		* 1220c  --mi_rlock
	bra	L_ack			* 12212

* -----------------------------------------------------------------------
* Sub-call 2: mq_advs (called when g_mtpre reaches 0 while active)
* -----------------------------------------------------------------------

L_seq:
	tst.w	_g_mtpre		* 12214  test g_mtpre
	beq.s	L_seq2			* 1221a  0 -> advance
	bpl	L_ack			* 1221c  positive -> not yet

L_seq2:
	cmpi.w	#1,_mi_dwrm		* 1221e  mi_dwrm >= 1 ?
	bge	L_ack			* 12226  yes -> skip
	cmpi.w	#1,_mi_rloc		* 12228  re-entered ?
	beq	L_ack			* 12230  yes -> skip
	addq.w	#1,_mi_dwrm		* 12232  ++mi_dwrm
	bclr.b	#5,$fffffa0f		* 12238  ack MFP ISRA
	movem.l	d0-d7/a0-a6,-(sp)	* 12240  save
	move.w	sr,d0			* 12244  save SR
	andi.w	#$f8ff,d0		* 12246  clear IPL
	ori.w	#$0500,d0		* 1224a  set IPL = 5
	move.w	d0,sr			* 1224e  install
	jsr	_mq_advs		* 12250  advance MIDI sequencer
	movem.l	(sp)+,d0-d7/a0-a6	* 12256  restore
	subq.w	#1,_mi_dwrm		* 1225a  --mi_dwrm

L_ack:
	bclr.b	#5,$fffffa0f		* 12260  final ISRA ack
	rte				* 12268  return from exception

* -----------------------------------------------------------------------
* LCP_STX keeps these five in the TEXT segment, immediately behind the
* ISR at 0x226a-0x2271, rather than in data with the other globals --
* so they are defined here and globals.c leaves them out of the
* default build.  The last two are real bytes, not BOOL16 words,
* which is why the tests above address them directly.
* -----------------------------------------------------------------------

_mi_dwrm:	.ds.w	1		* 226a
_mi_rloc:	.ds.w	1		* 226c
_g_mtpre:	.ds.w	1		* 226e
_g_msmsa:	.ds.b	1		* 2270
_psg_ntA:	.ds.b	1		* 2271
