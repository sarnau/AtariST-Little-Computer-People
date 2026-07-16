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

/* vslCol, v_pline, vstCol, vswrMd, v_gtext, vsfInt,
   vsfSty, vsfCol, vdi_cpR -> vdi.c */

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

/* cp_main (Ghidra 0x122FC-ish, large): real routine
   drives the WD1772 FDC directly through a self-modifying,
   XOR-encrypted MFM-gap-count check on the protection track.
   Stubbed to a constant non-zero return here -- the port doesn't
   emulate an original disk, so we always take the "authentic"
   branch.  main() writes the return value into cprot_r
   which gameLoop tests to decide between the tight game
   loop and the anti-piracy sleep loop.
   addr: cp_main() */

short
cp_main()
{
        return 1;
}
