/*
 * stubs.c -- placeholder bodies for functions not yet ported.
 *
 * Every extern referenced from a ported .c but not yet implemented has
 * an empty body here so the link stays clean.  Delete each stub as its
 * real .c comes online.
 */

#include "types.h"

/* AI                                                                     */
/* check_time_based_actions moved to ai_random.c */

/* Cross-file helper stubs still pending real ports.                       */
/* tt_on, tt_off, update_water_level_bar,
   sc_drfc    -> render.c
   hide_lcp_sprites, show_lcp_sprites -> sprites.c
   lcp_check_recovery          -> health.c
   a_driwa-> actions_bathroom.c
   a_clotd, a_clocd,
   a_opecf, a_opcfc,
   a_opecd, a_watat -> actions_doors.c
   li_lool/right    -> actions_house.c
   a_lists, a_takes, a_brust
                                -> actions_leisure.c / actions_bathroom.c */
/* a_eatm, a_kitcc, a_feedd,
   a_gesff          -> actions_food.c */
/* a_opcbc -> actions_leisure.c
   a_opecf, a_opcfc,
   a_opecd, a_clocd,
   a_watat -> actions_doors.c
   li_loor -> actions_house.c */
/* song_play -> sound.c
   pa_cloc, pa_skic,
   td_line, sc_sctd -> render_extra.c */
/* mq_inis, mq_parh, mq_resp,
   mq_skip, mq_setp, mq_stap,
   mq_pacm, mq_bust,
   mq_sepc -> midi_seq.c

   mq_dise -> midi_seq.c

   mowrit, psg_copy_envelope_params, psg_write_register,
   psg_set_mixer -> psg_io.c */

/* draw_line, sc_sdtb/frontbuffer,
   sc_firw, blkcopy32 -> gfx_prim.c */

/* vsl_color, v_pline, vst_color, vswr_mode, v_gtext, vsf_interior,
   vsf_style, vsf_color, vdi_copy_rect -> vdi.c */

/* sc_firs, sc_firb -> gfx_prim.c */

/* _draw_pixel -> gfx_prim.c */

/* clock_draw_hands -> clock.c
   g_momap -> globals.c (renamed from Ghidra's
   gSongMaxPosition_0; passed as the maxPosition arg to
   mq_inis, 0 meaning "no explicit end-of-song offset --
   let the sequencer walk until the natural stream terminator"). */
/* input_string defined in globals.c */

/* Letter subsystem helpers (letter template + text rendering). */
/* file_load_letter_template -> letter_load.c */
/* print_char                          -> render_extra.c */
/* lt_sets      -> sound.c */
/* select_random_click_sound           -> sound.c */
/* error_not_enough_memory             -> alerts.c */

/* Movement -- lcp_walk_to_destination, lcp_pathfind_one_step,
   lcp_calc_floor_waypoint, dog_calc_walk_path, lcp_play_footstep_sound
   all ported in walk.c */

/* Sprite pipeline                          -- all ported in sprites.c / sprites_head.c */

/* Study / save   -- lcp_enter_study_and_save, lcp_save, lc_load, fr_read,
   create_file all ported in save.c.  Only the peripheral error / cosmetic
   helpers remain stubs. */
/* error_unable_to_write, error_not_enough_memory -> alerts.c */
/* show_lcp_sprites -> sprites.c */

/* Keyboard -> keyboard.c
   p_dobls -> sound.c
   parse_command_to_action -> ai.c */

/* check_entered_command, command_upperstr, check_valid_word_input,
   lcp_toupper -> parser.c */

/* sp_draw -> sprite_render.c */

/* sf_irqp -> sfx_irq.c */

/* ag_main, wp_main, poker_main, poker_war_main,
   poker_blackjack_main -> games.c
   minigame_setup_screen, play_erase_rect       -> games.c */

/* poker_load_card_graphics -> cards.c */

/* init_vdi_and_screen, exit_vdi_and_screen -> gfx_prim.c */

/* string_print       -> render_extra.c
   tv_scrc, tv_boul, tv_patl
                      -> tv_animate.c
   sp_draw, sp_iniM -> sprite_render.c */

/* v_bar -> vdi.c */
