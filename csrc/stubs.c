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
/* tv_turn_on, tv_turn_off, update_water_level_bar,
   screen_draw_food_cabinet    -> render.c
   hide_lcp_sprites, show_lcp_sprites -> sprites.c
   lcp_check_recovery          -> health.c
   action_drink_water_animation-> actions_bathroom.c
   action_close_toilet_door, action_close_closet_door,
   action_open_close_fridge, action_open_close_filing_cabinet,
   action_open_close_dresser, action_walk_to_and_turn -> actions_doors.c
   lcp_idle_look_left/right    -> actions_house.c
   action_listen_song, action_take_shower, action_brush_teeth
                                -> actions_leisure.c / actions_bathroom.c */
/* action_eat_meal, action_kitchen_cabinet, action_feed_dog,
   action_get_snack_from_fridge          -> actions_food.c */
/* action_open_close_bedroom_closet -> actions_leisure.c
   action_open_close_fridge, action_open_close_filing_cabinet,
   action_open_close_dresser, action_close_closet_door,
   action_walk_to_and_turn -> actions_doors.c
   lcp_idle_look_right -> actions_house.c */
/* song_play -> sound.c
   palette_apply_clothing_colors, palette_apply_skin_colors,
   tv_draw_static_line, screen_scroll_text_down -> render_extra.c */
/* midi_seq_init_song, midi_seq_parse_header, midi_seq_reset_programs,
   midi_seq_skip_padding, midi_seq_set_position, midi_seq_start_playback,
   midi_seq_parse_channel_map, midi_seq_build_scale_table,
   midi_seq_send_program_change -> midi_seq.c

   midi_seq_dispatch_event -> midi_seq.c

   midi_out_write_byte, psg_copy_envelope_params, psg_write_register,
   psg_set_mixer -> psg_io.c */

/* draw_line, screen_set_draw_to_backbuffer/frontbuffer,
   screen_fill_row_white, blkcopy32 -> gfx_prim.c */

/* vsl_color, v_pline, vst_color, vswr_mode, v_gtext, vsf_interior,
   vsf_style, vsf_color, vdi_copy_rect -> vdi.c */

/* screen_fill_row_striped, screen_fill_row_black -> gfx_prim.c */

/* _draw_pixel -> gfx_prim.c */

/* clock_draw_hands -> clock.c
   midi_song_max_position -> globals.c (renamed from Ghidra's
   gSongMaxPosition_0; passed as the maxPosition arg to
   midi_seq_init_song, 0 meaning "no explicit end-of-song offset --
   let the sequencer walk until the natural stream terminator"). */
/* input_string defined in globals.c */

/* Letter subsystem helpers (letter template + text rendering). */
/* file_load_letter_template -> letter_load.c */
/* print_char                          -> render_extra.c */
/* letter_select_typewriter_sound      -> sound.c */
/* select_random_click_sound           -> sound.c */
/* error_not_enough_memory             -> alerts.c */

/* Movement -- lcp_walk_to_destination, lcp_pathfind_one_step,
   lcp_calc_floor_waypoint, dog_calc_walk_path, lcp_play_footstep_sound
   all ported in walk.c */

/* Sprite pipeline                          -- all ported in sprites.c / sprites_head.c */

/* Study / save   -- lcp_enter_study_and_save, lcp_save, lcp_load, file_read,
   create_file all ported in save.c.  Only the peripheral error / cosmetic
   helpers remain stubs. */
/* error_unable_to_write, error_not_enough_memory -> alerts.c */
/* show_lcp_sprites -> sprites.c */

/* Keyboard -> keyboard.c
   play_doorbell_sound -> sound.c
   parse_command_to_action -> ai.c */

/* check_entered_command, command_upperstr, check_valid_word_input,
   lcp_toupper -> parser.c */

/* sprite_draw -> sprite_render.c */

/* soundeffect_irq_play -> sfx_irq.c */

/* anagram_main, word_puzzle_main, poker_main, poker_war_main,
   poker_blackjack_main -> games.c
   minigame_setup_screen, play_erase_rect       -> games.c */

/* poker_load_card_graphics -> cards.c */

/* init_vdi_and_screen, exit_vdi_and_screen -> gfx_prim.c */

/* string_print       -> render_extra.c
   tv_show_screen_clear, tv_show_bouncing_line, tv_show_pattern_lines
                      -> tv_animate.c
   sprite_draw, sprite_init_MFDB -> sprite_render.c */

/* v_bar -> vdi.c */
