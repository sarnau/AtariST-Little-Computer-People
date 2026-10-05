******************************************************************************
*
* cp_asm.s -- the copy-protection check, checkCopyProt.
*
* This is hand-written assembly in the original: one self-contained
* object with no C in it and no external references at all.
*
* What it does:
*   - saves d1-d7/a0-a5 into its own static block (not the stack),
*   - goes supervisor if it is not already, sets TOS's flock byte at
*     $43e so the OS keeps its hands off the floppy,
*   - decrypts 96 bytes of itself in place (cpenc) under a raised
*     interrupt mask, with a fixed key built by the cpsum chain,
*   - selects the drive itself through YM2149 register 14 (I/O port A),
*     bypassing the OS,
*   - drives the 1772 FDC directly through $ff8604/$ff8606 and the DMA
*     address registers $ff8609/$ff860b/$ff860d: restore, seek, then
*     read the protected track into cpbuf, polling MFP GPIP bit 5 for
*     completion with a $40000 timeout,
*   - re-encrypts itself, clears flock, restores the registers, and
*     returns a LONG in d0.
*
* The return value is assembled by a chain of six mutually recursive
* stubs (cpsum1..cpsum6) that each add a constant -- an obfuscation,
* not a computation.  main stores the result in copyProtResult and moveInScene
* parks the resident asleep for ever if it is zero.
*
* The 96 bytes at cpenc..cpencend are written here as the decrypted
* instructions; the build encrypts them after assembling (see the note
* at cpenc), so the object carries the original's encrypted bytes.
*
* alcyon_link.sh links this file only for the default (shipped)
* build.  Branch optimisation must stay off when assembling it: every
* branch carries the size the original uses.
*
******************************************************************************

	.globl	_checkCo

	.text

* checkCopyProt(): the entry point, called once by main.  Returns a long in
* d0: zero means the disk check failed, anything else that it passed.
_checkCo:
	move.l	a6,cpa6
	lea	cptop,a6
	movem.l	d1-d7/a0-a5,-(a6)
	move.l	a6,cpsave
	move.w	sr,d0
	btst	#13,d0
	bne.s	cpsv1
	clr.l	-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.w	#6,sp
	move.l	d0,cpssp
	move.l	#-1,cpsvsr
	bra.s	cpgo
cpsv1:
	clr.l	cpsvsr
cpgo:
	move.b	#$ff,$43e
	move.w	sr,-(sp)
	ori.w	#$700,sr
	bsr.w	cpsetp
cpdec1:
	move.w	(a5),d7
	sub.w	d0,d7
	move.w	d7,(a5)+
	dbf	d6,cpdec1
	clr.l	d0
	clr.l	d7
	move.w	(sp)+,sr
	move.w	#$19,-(sp)
	trap	#1
	addq.w	#2,sp
	addq.b	#1,d0
	lsl.b	#1,d0
	or.w	#0,d0
	eori.b	#7,d0
	and.b	#7,d0
	moveq	#3,d5
	bsr.w	cpsel
cptrk:
	bsr.w	cpseek
	btst	#0,d6
	bne.w	cpnxt
	bsr.w	cprd
	btst	#0,d6
	bne.w	cpnxt
	moveq	#10,d7
	clr.l	d3
cpsec:
	move.l	d7,cpretv
	lea	cpbuf,a0
	move.l	a0,cpdma
	move.b	cpdma+3,$ff860d
	move.b	cpdma+2,$ff860b
	move.b	cpdma+1,$ff8609
	move.w	#$90,$ff8606
	move.w	#$190,$ff8606
	move.w	#$90,$ff8606
	move.w	#$1f,d7
	bsr.w	cpwcmd
	move.w	#$80,$ff8606
	move.w	#$e4,d7
	bsr.w	cpwcmd
	move.l	#$40000,d7
cpwt1:
	btst	#5,$fffa01
	beq.s	cpok1
	subq.l	#1,d7
	bne.s	cpwt1
	bsr.w	cprest2
	bra.w	cpnxt
cpok1:
	move.w	#$90,$ff8606
	move.w	$ff8606,d0
	btst	#0,d0
	beq.s	cpnxt

* The track check.  These 96 bytes are stored ENCRYPTED in the
* binary: cpdec1 decrypts them in place before they are reached, and
* cpenc1 re-encrypts them afterwards.  The source keeps them readable;
* tools/cp_encrypt.py encrypts the assembled object (every word plus
* $1567, the key cpsetp gets from the cpsum chain) so the shipped
* bytes are the original's.  Keep cpenc..cpencend exactly 48 words:
* that is the count cpsetp loads into d6.
*
* a0 points at the raw track just read.  Find a run of $a1 sync bytes
* followed by an $fe ID address mark for track 79 ($4f), then step
* back 18 bytes and count the $ff gap bytes between there and the
* start of the buffer.  Fewer than 15 sets bit 0 of d3, 80 or more
* sets bit 1; the disk passes once both have been seen (d3 == 3).
* The scan's end test compares a2 with the LONG stored at 512(a0),
* not with the address a0+512.
cpenc:
	clr.l	d7
	movea.l	a0,a2
cpsync:
	move.b	(a2)+,d6
	cmpa.l	512(a0),a2
	beq.s	cpnxt
	cmpi.b	#$a1,d6
	bne.s	cpsync
cpsync2:
	move.b	(a2)+,d6
	cmpi.b	#$a1,d6
	beq.s	cpsync2
	cmpi.b	#$fe,d6
	bne.s	cpnxt
	move.b	(a2)+,d6
	cmp.b	#$4f,d6
	bne.s	cpnxt
	suba.l	#18,a2
cpgap:
	move.b	-(a2),d6
	cmpa.l	a0,a2
	beq.s	cpgapn
	cmpi.b	#$ff,d6
	bne.s	cpgap
	addq.l	#1,d7
	bra.s	cpgap
cpgapn:
	subi.l	#16,d7
	bmi.s	cpshort
	subi.l	#64,d7
	bmi.s	cpnxt
	ori.b	#2,d3
	bra.s	cpgaps
cpshort:
	addq.l	#1,d7
	bpl.s	cpnxt
	ori.b	#1,d3
cpgaps:
	cmpi.b	#3,d3
cpencend:
	beq.s	cpgood
	move.l	cpretv,d7
	dbf	d7,cpsec
cpnxt:
	dbf	d5,cptrk
	bsr.w	cpfdcw
	bsr.w	cpenc1
	cmpi.l	#0,cpsvsr
	beq.s	cpfail
	move.l	cpssp,-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.w	#6,sp
cpfail:
	bsr.s	cprest
	clr.l	d0
	rts

cpgood:
	bsr.s	cpfdcw
	bsr.s	cpenc1
	cmpi.l	#0,cpsvsr
	beq.s	cpg2
	move.l	cpssp,-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.w	#6,sp
cpg2:
	bsr.s	cprest
	move.l	cpretv,d0
	ori.l	#$f0000000,d0
	rts

* The obfuscated return-value chain.
cpsum1:
	bsr.w	cpsum2
cps1b:
	bsr.w	cpsum3
cps1c:
	bsr.w	cpsum5
cps1d:
	bsr.w	cpsum6
	add.l	#$4e,d0
	rts

cprest:
	movea.l	cpsave,a6
	movem.l	(a6)+,d1-d7/a0-a5
	movea.l	cpa6,a6
	rts

* Wait for the FDC to go idle, drop flock, restore the drive select.
cpfdcw:
	move.w	#$80,$ff8606
	bsr.s	cprds
	btst	#7,d0
	bne.s	cpfdcw
	move.b	d2,d0
	move.b	#0,$43e
	bsr.s	cpsel
	rts

* Re-encrypt the 96 bytes.
cpenc1:
	bsr.s	cpsetp
cpenc2:
	move.w	(a5),d7
	add.w	d0,d7
	move.w	d7,(a5)+
	dbf	d6,cpenc2
	rts

* Point a5/d6 at the encrypted block and d0 at the key.
cpsetp:
	clr.l	d0
	bsr.s	cpsum1
	moveq	#$2f,d6
	lea	cpenc,a5
	rts

* Write d7 to the FDC command/data register, with settle delays.
cpwcmd:
	bsr.s	cpdly
	move.w	d7,$ff8604
cpdly:
	move.w	sr,-(sp)
	move.w	d7,-(sp)
	move.w	#$20,d7
cpdly1:
	dbf	d7,cpdly1
	move.w	(sp)+,d7
	move.w	(sp)+,sr
	rts

* PSG port A: select the drive/side bits in d0.
cpsel:
	move.w	sr,-(sp)
	ori.w	#$700,sr
	move.b	#$e,$ff8800
	move.b	$ff8800,d1
	move.b	d1,d2
	and.b	#$f8,d1
	or.b	d0,d1
	move.b	d1,$ff8802
	move.w	(sp)+,sr
	rts

* Read the FDC status register into d0.
cprds:
	bsr.s	cpdly
	move.w	$ff8604,d0
	bra.s	cpdly

* Seek: issue the command, wait for IRQ, check for an error.
cpseek:
	move.w	#3,d7
	bsr.s	cpcmd
cpsk1:
	subq.l	#1,d6
	beq.s	cpsk2
	btst	#5,$fffa01
	bne.s	cpsk1
	clr.l	d6
	move.w	#$80,$ff8606
	bsr.s	cprds
	btst	#2,d0
	bne.s	cpsk3
cpsk2:
	bsr.s	cprest2
	moveq	#1,d6
cpsk3:
	rts

* Restore to track 0.
cprest2:
	move.w	#$80,$ff8606
	move.w	#$d0,d7
	bsr.w	cpwcmd
	bsr.s	cpdly
	rts

* Read the protected sector.
cprd:
	move.w	#$4f,d7
	move.w	#$86,$ff8606
	bsr.w	cpwcmd
	move.w	#$13,d7
	bsr.s	cpcmd
cprd1:
	subq.l	#1,d6
	beq.s	cpsk2
	btst	#5,$fffa01
	bne.s	cprd1
	clr.l	d6
	rts

* Issue command d7 with a timeout in d6 that depends on motor state.
cpcmd:
	move.l	#$40000,d6
	move.w	#$80,$ff8606
	bsr.w	cprds
	btst	#7,d0
	bne.s	cpcmd2
	move.l	#$60000,d6
cpcmd2:
	bsr.w	cpwcmd
	rts

cpsum2:
	bsr.w	cps1b
cps2b:
	bsr.w	cpsum4
cps2c:
	bsr.w	cps1d
	add.l	#$c9,d0
	rts

* --- statics -------------------------------------------------------
cpsvsr:	.ds.l	1
cpretv:	.ds.l	1
cpssp:	.ds.l	1
cpdma:	.ds.l	1
cpa6:	.ds.l	1
cpsave:	.ds.l	1
	.ds.b	8
	.ds.b	52
cptop:	.ds.l	1
cpbuf:	.ds.b	6560

cpsum3:
	bsr.w	cps2b
cpsum4:
	bsr.w	cps1c
cpsum5:
	bsr.w	cps2c
cpsum6:
	add.l	#$16,d0
	rts
