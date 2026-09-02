* vdiown_a.s -- the game's own trap-#2 dispatch, as a SEPARATE object.
*
* LCP_ORG keeps this routine inside vdiown.o, so as68 shortens every
* call to it into a bsr; LCP_STX calls it with jsr, which means it
* lived in another object there.  This file supplies it for the STX
* configuration; under FAITHFUL alcyon_build.sh injects the same code
* into vdiown.s instead and this file is not linked.

	.globl	_vdi_go
	.globl	_vdipb

	.text

_vdi_go:
	link	a6,#-4
	move.l	#_vdipb,d1
	moveq	#115,d0
	trap	#2
	unlk	a6
	rts
