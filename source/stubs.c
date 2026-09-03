/*
 * stubs.c -- residual stubs and intentional non-fidelity.
 *
 * As the port matured, every cross-file redirect that used to sit
 * here got resolved into a real .c file.  The only survivor is
 * cp_main: the ROM's copy-protection routine cannot be reproduced
 * under Hatari (see the plate comment for details), so we stub it
 * to the successful-verification outcome.
 */

#include "types.h"
#include "stubs.h"

/* cp_main -- INTENTIONAL non-fidelity vs Ghidra copyprot_main_check.
   The ROM's routine is not portable to Hatari and never will be:

   * Enters supervisor mode via TRAP #1 (Super).
   * Locks `flock = 0xFF` to shut GEMDOS out of the disk.
   * Decrypts a self-modifying code block via XOR key 0x1567, then
     re-encrypts after the check to defeat memory-dump analysis.
   * Selects drive A by poking PSG port E (YM2149 register 14)
     directly, bypassing every OS API.
   * Drives the WD1772 FDC via DMA controller registers (buffer
     addr, DMA mode 0x90, DMA start toggle) to read raw MFM.
   * Polls MFP GPIP bit 5 for FDC completion with a 0x40000
     timeout.
   * Scans the resulting raw track buffer for a non-standard MFM
     signature: two 0xA1 sync marks + 0xFE ID mark + 0x4F data,
     surrounded by 0xFF gap-byte counts in two specific ranges
     (< 16 and >= 80).  A regular disk copier can only reproduce
     file-level data, not the raw MFM gap counts, so a copied disk
     fails the check silently -- the resident just goes to sleep
     forever in gameLoop's else branch.

   Faithfully porting this would require either (a) an original
   disk image + Hatari's floppy hardware emulation being cycle-
   accurate enough to reproduce the non-standard MFM gaps -- which
   it isn't -- or (b) rewriting Hatari's WD1772 model.  Neither is
   in scope.  We stub it to return 1 so gameLoop takes the
   tight-loop branch every time.  main() writes the return value
   into cprot_r.

   addr: (matches copyprot_main_check @ ~0x122FC, behaviourally
   equivalent to the successful-verification outcome) */

/* cp_main lives in hand-written assembly -- see source/cp_asm.s. */
