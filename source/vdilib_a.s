* vdilib_a.s -- hand-assembly helpers of the ROM's workstation module
* (0xe830-0xe85a): raw contrl writers and the trap-#2 dispatch on the
* second (runtime-patched) parameter block.  Linked directly after
* vdilib.o, exactly as in the ROM.

	.globl	_wr_src
	.globl	_wr_dst
	.globl	_vdi_go2
	.globl	_contrl
	.globl	_vdipb2

	.text

_wr_src:
	move.l	4(sp),_contrl+14
	rts

_wr_dst:
	move.l	4(sp),_contrl+18
	rts

_vdi_go2:
	move.l	#_contrl,_vdipb2
	move.l	#_vdipb2,d1
	moveq	#115,d0
	trap	#2
	rts
