/*
 * action_stubs.c -- placeholder bodies for do_action() handlers not
 *                   yet ported for real.
 *
 * Style: one-line bodies keep this file skimmable; real ports live in
 * per-group action_*.c files.  Delete each stub as its real .c comes
 * online.  Ported handlers get a leading comment marker so the diff
 * with a Ghidra-listed action_* set is trivially auditable.
 */

/* ---- Idle / gesture group -- see action_simple.c, actions_idle.c ---- */
/* action_hello                 -> action_simple.c */
/* action_yawn_and_stretch      -> action_simple.c */
/* action_nod_head              -> action_simple.c */
/* action_call_dog              -> action_simple.c */
/* action_wake_from_alarm       -> action_simple.c */
/* action_pet_dog               -> action_simple.c */
/* action_wander_idly           -> actions_idle.c  */
/* action_peek_around           -> actions_idle.c  */
/* action_pace_nervously        -> actions_idle.c  */
/* action_toggle_tv             -> actions_idle.c  */
/* action_sleep                 -> actions_idle.c  */

/* ---- Walk-and-interact -- see actions_house.c ----------------------- */
/* action_read_newspaper        -> actions_house.c */
/* action_get_in_out_of_bed     -> actions_house.c */
/* action_dance                 -> actions_house.c */
/* action_drink                 -> actions_house.c */
/* action_use_toilet            -> actions_house.c */
/* action_wake_up_morning       -> actions_house.c */
/* action_go_to_bed_night       -> actions_house.c */

/* ---- Leisure -- see actions_leisure.c ------------------------------- */
/* action_listen_song            -> actions_leisure.c */
/* action_play_piano             -> actions_leisure.c */
/* action_play_with_record       -> actions_leisure.c */
/* action_light_fireplace        -> actions_leisure.c */
/* action_sit_on_couch_with_dog  -> actions_leisure.c */
/* action_sit_and_exercise       -> actions_leisure.c */
/* action_check_front_door       -> actions_leisure.c */
/* action_tidy_house             -> actions_leisure.c */
/* action_clean_up               -> actions_leisure.c */
/* action_open_close_bedroom_closet -> actions_leisure.c */

/* ---- Still TODO ----------------------------------------------------- */
/* action_play_computer -> actions_games.c */
/* action_write_letter  -> actions_letter.c */
/* action_play_a_game   -> actions_games.c */
/* action_open_close_upstairs_closet -> actions_leisure.c */

/* Delivery event handlers moved to deliveries.c:
 *   event_receive_food_delivery, event_receive_book_delivery,
 *   event_receive_record_delivery, event_receive_dog_food,
 *   event_answer_phone
 */
