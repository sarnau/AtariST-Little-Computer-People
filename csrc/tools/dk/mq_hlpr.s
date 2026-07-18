******************************************************************************
*
* mq_hlpr.s -- MFP Timer-A interrupt-mode wrapper for the C mq_tick handler.
*
* K&R C functions return via RTS.  MFP interrupt handlers must
* return via RTE.  Xbtimer expects a raw asm entry point that
* preserves scratch registers around the C call and RTEs at the end.
*
* Alcyon C's calling convention: D0-D2 and A0-A2 are caller-saved
* (scratch); D3-D7 and A3-A6 are callee-saved.  So the wrapper only
* needs to preserve the scratch set before calling mq_tick.
*
* Named _mqisr (mq interrupt-service-routine) rather than
* _mq_tickA -- as68's 8-char symbol truncation would map
* _mq_tickA -> _mq_tick and collide with the C entry point.
*
* Both _mqisr and _mq_tick need `.globl`: as68 treats it as
* export-if-defined-here / import-if-not-defined-here.  Without the
* import declaration on _mq_tick, as68 creates a spurious BSS
* symbol for it locally and the JSR resolves to BSS offset 0
* instead of the real C function.
*
* Installed by mq_intim in init.c:
*     xbios(31, 0, 5, 0x28, (long) mqisr);
*
******************************************************************************

	.globl	_mqisr
	.globl	_mq_tick

_mqisr:
	movem.l	d0-d2/a0-a2,-(sp)	* save scratch regs
	jsr	_mq_tick		* call C handler
	movem.l	(sp)+,d0-d2/a0-a2	* restore
	rte				* return from exception
