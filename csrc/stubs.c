/*
 * stubs.c -- placeholder bodies for functions not yet ported.
 *
 * Every extern referenced from a ported .c but not yet implemented has
 * an empty body here so the link stays clean.  Delete each stub as its
 * real .c comes online.
 */

#include "types.h"

/* AI                                                                     */
/* chk_timA moved to airandom.c */

/* Cross-file helper stubs still pending real ports.                       */
/* tt_on, tt_off, updWtLv,
   sc_drfc    -> render.c
   hideLcp, showLcp -> sprites.c
   lcp_rcov          -> health.c
   a_driwa-> abathrm.c
   a_clotd, a_clocd,
   a_opecf, a_opcfc,
   a_opecd, a_watat -> adoors.c
   li_lool/right    -> ahouse.c
   a_lists, a_takes, a_brust
                                -> aleisure.c / abathrm.c */
/* a_eatm, a_kitcc, a_feedd,
   a_gesff          -> afood.c */
/* a_opcbc -> aleisure.c
   a_opecf, a_opcfc,
   a_opecd, a_clocd,
   a_watat -> adoors.c
   li_loor -> ahouse.c */
/* sgPlay -> sound.c
   pa_cloc, pa_skic,
   td_line, sc_sctd -> renderx.c */
/* mq_inis, mq_parh, mq_resp,
   mq_skip, mq_setp, mq_stap,
   mq_pacm, mq_bust,
   mq_sepc -> midi_seq.c

   mq_dise -> midi_seq.c

   mowrit, psg_cpE, psg_wr,
   psg_mix -> psg_io.c */

/* drwLine, sc_sdtb/frontbuffer,
   sc_firw, blkcp32 -> gfx_prim.c */

/* vsl_color, v_pline, vst_color, vswr_mode, v_gtext, vsf_interior,
   vsf_style, vsf_color, vro_cpyfm -> vdi.c */

/* sc_firs, sc_firb -> gfx_prim.c */

/* drwPixel -> gfx_prim.c */

/* cl_drwH -> clock.c
   g_momap -> globals.c (renamed from Ghidra's
   gSongMaxPosition_0; passed as the maxPos arg to
   mq_inis, 0 meaning "no explicit end-of-song offset --
   let the sequencer walk until the natural stream terminator"). */
/* in_str defined in globals.c */

/* Letter subsystem helpers (letter template + text rendering). */
/* fl_ltpl -> letload.c */
/* prCh                          -> renderx.c */
/* lt_sets      -> sound.c */
/* sfClick           -> sound.c */
/* er_nomem             -> alerts.c */

/* Movement -- lcp_wkD, lcp_path,
   lcp_flwp, dg_wkPth, lcp_fstp
   all ported in walk.c */

/* Sprite pipeline                          -- all ported in sprites.c / sprhead.c */

/* Study / save   -- lcp_std, lcp_save, lc_load, fr_read,
   crFile all ported in save.c.  Only the peripheral error / cosmetic
   helpers remain stubs. */
/* er_write, er_nomem -> alerts.c */
/* showLcp -> sprites.c */

/* Keyboard -> keyboard.c
   p_dobls -> sound.c
   prsCmd -> ai.c */

/* chk_encm, cmd_upp, chk_vwd,
   lcp_upp -> parser.c */

/* sp_draw -> sprender.c */

/* sf_irqp -> sfx_irq.c */

/* ag_main, wp_main, pk_main, pk_wrMn,
   pk_bjMn -> games.c
   mg_stp, plEr       -> games.c */

/* pk_ldCrd -> cards.c */

/* initVdi, exitVdi -> gfx_prim.c */

/* strPr       -> renderx.c
   tv_scrc, tv_boul, tv_patl
                      -> tvanim.c
   sp_draw, sp_iniM -> sprender.c */

/* v_bar -> vdi.c */

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

short
cp_main()
{
        return 1;
}
