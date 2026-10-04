* vdistx_a.s -- the tail of Activision's VDI binding module: the two
* raw contrl writers plus the SINGLE trap-#2 dispatcher.
*
* Defining _gsx1 here keeps VDIBIND's own gsx1 member (and its private
* pblock) out of the link, so `vdipb` in globals.c is the one
* parameter block and vdi_go/vdi_go2 map onto this one dispatcher.


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
