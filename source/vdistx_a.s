* vdistx_a.s -- the tail of Activision's VDI binding module in
* LCP_STX: the two raw contrl writers plus the SINGLE trap-#2
* dispatcher, at 0x1771a / 0x17724 / 0x1772e.
*
* LCP_STX has one dispatcher where the port used to carry three
* (vdiown_a.s's vdi_go, vdilib_a.s's vdi_go2 and the linked VDIBIND
* gsx1) -- 22 bytes each, the port's whole text surplus.  Defining
* _gsx1 here keeps VDIBIND's own gsx1 member (and its private pblock)
* out of the link, so `vdipb` in globals.c is the one parameter block.


	.globl	_wr_src
	.globl	_wr_dst
	.globl	_gsx1
	.globl	_contrl
	.globl	_vdipb

	.text

_wr_src:
	move.l	4(sp),_contrl+14
	rts

_wr_dst:
	move.l	4(sp),_contrl+18
	rts

_gsx1:
	move.l	#_contrl,_vdipb
	move.l	#_vdipb,d1
	moveq	#115,d0
	trap	#2
	rts
