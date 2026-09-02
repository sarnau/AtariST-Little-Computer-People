******************************************************************************
*
* psg_asm.s -- the LCP_STX revision's PSG / MIDI byte pokes.
*
* In LCP_STX these three routines are hand-assembly, not C: they have
* no stack frame, address the hardware registers absolute-long, and
* read their arguments straight off the stack.  They sit immediately
* before the Timer-A ISR in the same object (0x2272, 0x2284, 0x22a6,
* mq_tick at 0x22c0).
*
* LCP_ORG has C versions instead -- psg_io.c keeps those under
* FAITHFUL, and alcyon_link.sh links this file only for the default
* build.
*
******************************************************************************

	.globl	_psg_wr
	.globl	_psg_mix
	.globl	_mowrit

	.text

* psg_wr(reg, val): select register `reg`, write `val`.
* No frame: 4(sp) is the first argument word, so its byte is 5(sp).
_psg_wr:
	move.b	7(sp),$ffff8800
	move.b	5(sp),$ffff8802
	rts

* psg_mix(or_mask, and_mask): read-modify-write PSG register 7.
* d0 is saved, so the arguments move up by 4.
_psg_mix:
	move.l	d0,-(sp)
	move.b	#7,$ffff8800
	move.b	$ffff8800,d0
	and.b	11(sp),d0
	or.b	9(sp),d0
	move.b	d0,$ffff8802
	move.l	(sp)+,d0
	rts

* mowrit(byte): spin until the MIDI ACIA can accept a byte, then send.
_mowrit:
	move.l	d0,-(sp)
mow1:
	move.b	$fffffc04,d0
	btst	#1,d0
	beq.s	mow1
	move.b	9(sp),$fffffc06
	move.l	(sp)+,d0
	rts
