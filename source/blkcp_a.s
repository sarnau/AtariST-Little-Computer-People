******************************************************************************
*
* blkcp_a.s -- LCP_STX's hand-assembly 32-byte block copy.
*
* At 0x17310 -- the last game function before the library -- LCP_STX
* has an unrolled dbf loop that Alcyon C cannot emit: eight
* post-increment long moves per iteration, driven by `dbf` on the
* count.  It still carries the C calling frame, so the arguments sit
* at the usual offsets (src 8, dst 12, count 16).

*
******************************************************************************

	.globl	_blkcp32

	.text

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
