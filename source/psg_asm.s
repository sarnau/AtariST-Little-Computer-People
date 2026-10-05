******************************************************************************
*
* psg_asm.s -- PSG / MIDI byte pokes.
*
* In the original these three routines are hand-assembly, not C: they
* have no stack frame, address the hardware registers absolute-long,
* and read their arguments straight off the stack.  c168 cannot emit
* such frameless pokes, so they live here.
*
******************************************************************************

	.globl	_psgWrit
	.globl	_psgMixe
	.globl	_aciaWri

	.text

* psgWrite(val, reg): select register `reg` (the SECOND argument, 7(sp)),
* then write `val` (the first, 5(sp)) -- every caller passes data first.
* No frame: 4(sp) is the first argument word, so its byte is 5(sp).
_psgWrit:
	move.b	7(sp),$ffff8800
	move.b	5(sp),$ffff8802
	rts

* psgMixer(or_mask, and_mask): read-modify-write PSG register 7.
* d0 is saved, so the arguments move up by 4.
_psgMixe:
	move.l	d0,-(sp)
	move.b	#7,$ffff8800
	move.b	$ffff8800,d0
	and.b	11(sp),d0
	or.b	9(sp),d0
	move.b	d0,$ffff8802
	move.l	(sp)+,d0
	rts

* aciaWrite(byte): spin until the MIDI ACIA can accept a byte, then send.
_aciaWri:
	move.l	d0,-(sp)
mow1:
	move.b	$fffffc04,d0
	btst	#1,d0
	beq.s	mow1
	move.b	9(sp),$fffffc06
	move.l	(sp)+,d0
	rts
