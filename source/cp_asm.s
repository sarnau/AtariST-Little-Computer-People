******************************************************************************
*
* cp_asm.s -- the copy-protection check, checkCopyProt.
*
* Hand-written assembly in the original, and Activision's rather than
* LCP's: 97% of it is byte-identical to the routine in The Music Studio.
* It is one self-contained object with no external references; every
* address it touches is either its own or ST hardware.
*
* THE IDEA
*
* The original disk carries a specially mastered track 79.  The routine
* reads that track raw with the 1772's READ TRACK command and measures
* the gap in front of the first sector header: how many $ff bytes the
* controller delivers between the index pulse and the header.  It asks
* for two different answers -- fewer than 15 on one read, 80 or more on
* another -- within at most eleven reads of the same track.  A track
* written by an ordinary formatter or disk copier reads back the same
* way every revolution, so it can produce at most one of the two; the
* original passes only because its gap reads back differently from one
* revolution to the next.
*
* THE FLOW
*
*   1. Save d1-d7/a0-a5 in a private block, switch to supervisor mode
*      if needed, and set TOS's flock so the VBL floppy handler keeps
*      off the controller.
*   2. Decrypt the gap-measuring code (cpenc) in place.
*   3. Select the boot drive directly through the YM2149's port A.
*   4. Up to four times: RESTORE to track 0, SEEK to track 79, then up
*      to eleven times READ TRACK into cpbuf and measure the gap.
*   5. Wait for the motor to stop, release flock and the drive select,
*      re-encrypt cpenc, leave supervisor mode, restore the registers.
*
* It returns a LONG in d0: zero if the check failed, otherwise the
* read's remaining retry count with the top four bits set, so never
* zero.  main stores it in copyProtResult; moveInScene parks the
* resident asleep for ever when it is zero.
*
* THE HARDWARE
*
*   $ff8604  DMA chip: FDC register or DMA sector count, depending on
*            what $ff8606 last selected (word access)
*   $ff8606  DMA mode (write) / DMA status (read):
*              $80  select the FDC command/status register
*              $86  select the FDC data register
*              $90  select the DMA sector count, transfer = read
*              $190 the same with the write bit set; writing $90,
*                   $190, $90 toggles the direction bit, which resets
*                   the DMA chip and empties its FIFO
*            status bit 0 is set when the transfer had NO error
*   $ff8609/$ff860b/$ff860d  DMA address, high/middle/low byte
*   $ff8800/$ff8802  YM2149 register select / write; register 14 is
*            port A, whose low three bits are the floppy side select
*            and the drive A and B selects, all active low
*   $fffa01  MFP GPIP: bit 5 is the FDC/ACSI interrupt line, which goes
*            LOW when the controller finishes a command
*   $43e     TOS's flock: non-zero keeps the VBL floppy handler out
*
* 1772 commands used: $03 RESTORE and $13 SEEK (both with step-rate
* bits 11, 3 ms per step on the 1772, and motor spin-up enabled), $e4
* READ TRACK with the 15 ms head-settle delay, $d0 FORCE INTERRUPT
* (abort whatever is running, no interrupt).  Type I status: bit 7
* motor on, bit 2 head at track 0.  The 1772 switches the motor off by
* itself after ten index pulses without a command.
*
* THE ENCRYPTION
*
* The 96 bytes cpenc..cpencend are stored encrypted.  cpsetp builds the
* key, $1567, by calling the cpsum chain on a cleared d0, and cpdec1
* subtracts it from each of the 48 words before the code is reached;
* cpenc1 adds it back on every way out.  The source carries the
* decrypted instructions, and tools/cp_encrypt.py encrypts them in the
* assembled object, so the binary holds the original's bytes.
*
* alcyon_link.sh links this file only for the default (shipped) build.
* Branch optimisation must stay off when assembling it (as68 -n):
* every branch carries the size the original uses.
*
* Register use: d5 = RESTORE/SEEK attempts left, d7 = READ TRACK
* attempts left (copied to cpretv, because the gap count reuses d7),
* d3 = which gap results have been seen, d6 = command timeouts and the
* failure flag of cpseek/cprd, d2 = port A as found.
*
******************************************************************************

	.globl	_checkCo

	.text

* checkCopyProt(): the entry point, called once by main.
*
* Save the caller's registers.  a6 itself goes to cpa6; the other
* thirteen are pushed downward from cptop into the 52-byte block below
* it, and cpsave remembers where they start.
_checkCo:
	move.l	a6,cpa6
	lea	cptop,a6
	movem.l	d1-d7/a0-a5,-(a6)
	move.l	a6,cpsave
* Supervisor mode is needed for the hardware.  If the S bit (13) of SR
* is clear, Super(0) switches over and returns the old supervisor stack
* pointer, which is kept for the way back; cpsvsr = -1 records that the
* switch was made, 0 that the caller was supervisor already.
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
* flock: keep TOS's VBL floppy handler away from the FDC from here on.
	move.b	#$ff,$43e
* Decrypt cpenc with every interrupt masked (IPL 7): a5 = cpenc,
* d6 = 47 for 48 words, d0 = the key; each word minus the key.
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
* Drive select bits for port A from Dgetdrv(): drive 0 (A:) gives
* (0+1)*2 = %010, drive 1 (B:) %100.  Inverted, because the lines are
* active low, and masked to three bits: A: becomes %101 -- side 0,
* drive A selected, drive B not.  (The or.w #0 does nothing.)
	move.w	#$19,-(sp)
	trap	#1
	addq.w	#2,sp
	addq.b	#1,d0
	lsl.b	#1,d0
	or.w	#0,d0
	eori.b	#7,d0
	and.b	#7,d0
* Four RESTORE/SEEK attempts (dbf on d5).
	moveq	#3,d5
	bsr.w	cpsel
cptrk:
* Head to track 0, then out to track 79; either failing (d6 bit 0 set)
* costs this attempt.
	bsr.w	cpseek
	btst	#0,d6
	bne.w	cpnxt
	bsr.w	cprd
	btst	#0,d6
	bne.w	cpnxt
* Eleven READ TRACK attempts (dbf on d7), and no gap result seen yet.
	moveq	#10,d7
	clr.l	d3
cpsec:
	move.l	d7,cpretv
* Point the DMA at cpbuf: the low, middle and high address bytes come
* from the longword cpdma, byte 3 to byte 1.
	lea	cpbuf,a0
	move.l	a0,cpdma
	move.b	cpdma+3,$ff860d
	move.b	cpdma+2,$ff860b
	move.b	cpdma+1,$ff8609
* Reset the DMA chip ($90/$190/$90), leaving it in read direction with
* the sector count selected, and allow up to 31 512-byte blocks.
	move.w	#$90,$ff8606
	move.w	#$190,$ff8606
	move.w	#$90,$ff8606
	move.w	#$1f,d7
	bsr.w	cpwcmd
* READ TRACK, with head settle: the FDC streams the raw track, from
* index pulse to index pulse, through the DMA into cpbuf.
	move.w	#$80,$ff8606
	move.w	#$e4,d7
	bsr.w	cpwcmd
* Wait for the FDC interrupt (GPIP bit 5 low), at most $40000 polls;
* on a timeout abort the command and start a new RESTORE/SEEK attempt.
	move.l	#$40000,d7
cpwt1:
	btst	#5,$fffa01
	beq.s	cpok1
	subq.l	#1,d7
	bne.s	cpwt1
	bsr.w	cprest2
	bra.w	cpnxt
cpok1:
* DMA status bit 0 clear means the transfer failed: next attempt.
	move.w	#$90,$ff8606
	move.w	$ff8606,d0
	btst	#0,d0
	beq.s	cpnxt

* The gap measurement.  These 96 bytes are stored ENCRYPTED in the
* binary: cpdec1 decrypted them above, and cpenc1 re-encrypts them on
* the way out.  The source keeps them readable; tools/cp_encrypt.py
* encrypts the assembled object (every word plus $1567) so the shipped
* bytes are the original's.  Keep cpenc..cpencend exactly 48 words --
* the count cpsetp loads into d6 -- and free of relocated operands.
*
* a0 = cpbuf, the raw track.  In a READ TRACK dump a sector header is
* the $00 sync field, three $a1 address marks, the $fe ID mark, then
* track, side, sector, size and CRC.
cpenc:
	clr.l	d7
	movea.l	a0,a2
* Find the first $a1.  The end test compares a2 with the LONG stored
* at 512(a0), not with the address a0+512, so in practice the scan
* simply runs until it meets an $a1.
cpsync:
	move.b	(a2)+,d6
	cmpa.l	512(a0),a2
	beq.s	cpnxt
	cmpi.b	#$a1,d6
	bne.s	cpsync
* Skip the rest of the $a1 run; the next byte must be the $fe ID mark,
* and the one after it track 79.  Anything else: new RESTORE/SEEK.
cpsync2:
	move.b	(a2)+,d6
	cmpi.b	#$a1,d6
	beq.s	cpsync2
	cmpi.b	#$fe,d6
	bne.s	cpnxt
	move.b	(a2)+,d6
	cmp.b	#$4f,d6
	bne.s	cpnxt
* a2 is past the track byte.  Back 18 bytes is in front of the header
* (track, $fe, three $a1, twelve $00 sync bytes, and one more), i.e.
* at the end of the gap.  From there down to the start of the buffer,
* count every $ff byte into d7.
	suba.l	#18,a2
cpgap:
	move.b	-(a2),d6
	cmpa.l	a0,a2
	beq.s	cpgapn
	cmpi.b	#$ff,d6
	bne.s	cpgap
	addq.l	#1,d7
	bra.s	cpgap
* Classify the count: 80 or more sets bit 1 of d3, under 15 sets bit 0.
* 15..79 abandons this attempt for a new RESTORE/SEEK, which also
* clears d3 -- both results must come from one attempt's reads.
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
* Both results seen: the disk is the original.
cpgaps:
	cmpi.b	#3,d3
cpencend:

	beq.s	cpgood
* Otherwise read the track again (d7 was used for the count, so it is
* reloaded from cpretv first).
	move.l	cpretv,d7
	dbf	d7,cpsec
cpnxt:
	dbf	d5,cptrk
* Failed: tidy up exactly as on success, then return 0.
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

* Passed: motor off and flock released, cpenc re-encrypted, back to
* user mode with Super(old ssp) if the entry switched, registers
* restored.  The result is the remaining READ TRACK count with the top
* four bits set, so it is never zero.
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

* The key chain.  cpsum1..cpsum6 (here, after cpcmd and after the
* statics) call each other and fall through into one another, so a
* single call to cpsum1 runs a long, scattered sequence of additions of
* $16, $4e and $c9.  Starting from 0 it leaves $1567 in d0: the
* encryption key, kept out of the code as a constant.
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

* Restore the caller's registers, a6 last.
cprest:
	movea.l	cpsave,a6
	movem.l	(a6)+,d1-d7/a0-a5
	movea.l	cpa6,a6
	rts

* Wait until the FDC status shows the motor off (bit 7 clear; the 1772
* stops it by itself, see the header), then release flock and put port
* A's select bits back as they were found (d2).
cpfdcw:
	move.w	#$80,$ff8606
	bsr.s	cprds
	btst	#7,d0
	bne.s	cpfdcw
	move.b	d2,d0
	move.b	#0,$43e
	bsr.s	cpsel
	rts

* Re-encrypt cpenc: each word plus the key.
cpenc1:
	bsr.s	cpsetp
cpenc2:
	move.w	(a5),d7
	add.w	d0,d7
	move.w	d7,(a5)+
	dbf	d6,cpenc2
	rts

* Set up a cipher pass: d0 = the key from the cpsum chain, d6 = 47 (48
* words for dbf), a5 = cpenc.
cpsetp:
	clr.l	d0
	bsr.s	cpsum1
	moveq	#$2f,d6
	lea	cpenc,a5
	rts

* Write d7 to $ff8604 -- whichever register $ff8606 selected -- with a
* short delay before and after, as the DMA chip needs.
cpwcmd:
	bsr.s	cpdly
	move.w	d7,$ff8604
* The delay: 33 turns of a dbf loop, d7 and SR preserved.
cpdly:
	move.w	sr,-(sp)
	move.w	d7,-(sp)
	move.w	#$20,d7
cpdly1:
	dbf	d7,cpdly1
	move.w	(sp)+,d7
	move.w	(sp)+,sr
	rts

* Set the low three bits of YM2149 port A (side, drive A, drive B,
* active low) to d0, with interrupts masked so TOS's own sound and
* port A writes cannot interleave.  d2 receives port A as it was.
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

* Read the selected FDC register into d0, delays around it.
cprds:
	bsr.s	cpdly
	move.w	$ff8604,d0
	bra.s	cpdly

* RESTORE: step the head out to track 0, wait for the FDC interrupt
* (GPIP bit 5 low) while counting d6 down, and check the status's
* track-0 bit.  Returns d6 = 0 on success; on a timeout or without
* track 0, aborts the command and returns d6 = 1.
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

* FORCE INTERRUPT: abort whatever the FDC is doing.
cprest2:
	move.w	#$80,$ff8606
	move.w	#$d0,d7
	bsr.w	cpwcmd
	bsr.s	cpdly
	rts

* SEEK to track 79: the target goes into the data register, then the
* command; wait for the interrupt as cpseek does.  d6 = 0 on success,
* 1 (command aborted) on a timeout.  The status is not checked.
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

* Issue the type I command in d7, setting d6 to the time to wait for
* it: $40000 polls if the motor is already running (status bit 7),
* $60000 if it has to spin up first.
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

* More of the key chain (see cpsum1).
cpsum2:
	bsr.w	cps1b
cps2b:
	bsr.w	cpsum4
cps2c:
	bsr.w	cps1d
	add.l	#$c9,d0
	rts

* --- statics, inside the text segment ----------------------------
cpsvsr:	.ds.l	1
cpretv:	.ds.l	1
cpssp:	.ds.l	1
cpdma:	.ds.l	1
cpa6:	.ds.l	1
cpsave:	.ds.l	1
	.ds.b	8
* The caller's d1-d7/a0-a5, pushed downward from cptop.
	.ds.b	52
cptop:	.ds.l	1
* The raw track.  A double-density track is about 6250 bytes.
cpbuf:	.ds.b	6560

* The rest of the key chain (see cpsum1).
cpsum3:
	bsr.w	cps2b
cpsum4:
	bsr.w	cps1c
cpsum5:
	bsr.w	cps2c
cpsum6:
	add.l	#$16,d0
	rts
