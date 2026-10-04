******************************************************************************
*
* blkcp_a.s -- hand-assembled copy of `count` 32-byte blocks.
*
* An unrolled loop Alcyon C cannot emit: eight post-increment long
* moves per iteration, driven by `dbf` on the count.  It still carries
* the C calling frame, so the arguments sit at the usual offsets
* (src 8, dst 12, count 16).

*
******************************************************************************

	.globl	_blkcp32

	.text

* blkcp32(src, dst, count): copy count 32-byte blocks from src to dst.
* count must be at least 1 (0 would wrap the dbf counter).
_blkcp32:
	link	a6,#-6
	move.w	16(a6),d0
	subq.w	#1,d0
	move.l	8(a6),a0
	move.l	12(a6),a1
bcp1:
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	dbf	d0,bcp1
	unlk	a6
	rts
