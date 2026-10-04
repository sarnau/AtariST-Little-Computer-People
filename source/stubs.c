/*
 * stubs.c -- no code is left here.
 *
 * The one former stub was cp_main, the copy-protection check.  It is
 * hand-written assembly in source/cp_asm.s.  Build with
 * -DSKIP_COPYPROT=1 to play the game under an emulator.
 */

#include "types.h"
#include "stubs.h"

/* How the copy protection works:

   * Enters supervisor mode via TRAP #1 (Super).
   * Locks `flock = 0xFF` to shut GEMDOS out of the disk.
   * Decrypts a self-modifying code block, then re-encrypts it after
     the check to defeat memory-dump analysis.
   * Selects the drive by poking PSG port A (YM2149 register 14)
     directly, bypassing every OS API.
   * Drives the WD1772 FDC via the DMA controller registers (buffer
     address, DMA mode 0x90, DMA start toggle) to read the raw track.
   * Polls MFP GPIP bit 5 for FDC completion with a 0x40000
     timeout.
   * Scans the resulting raw track buffer for a non-standard MFM
     signature: two 0xA1 sync marks + 0xFE ID mark + 0x4F data,
     surrounded by 0xFF gap-byte counts in two specific ranges
     (< 16 and >= 80).  A regular disk copier can only reproduce
     file-level data, not the raw MFM gap counts, so a copied disk
     fails the check silently -- the resident just sleeps forever.

   main() stores the result in cprot_r; cs_mvIn and gameLoop park the
   resident in an endless sleep loop when it is 0.  Under Hatari the
   check always fails, even with the original disk image. */
