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
/* a_hello                 -> action_simple.c */
/* a_yawas      -> action_simple.c */
/* a_nodh              -> action_simple.c */
/* a_calld              -> action_simple.c */
/* a_wakfa       -> action_simple.c */
/* a_petd               -> action_simple.c */
/* a_wandi           -> actions_idle.c  */
/* a_peeka           -> actions_idle.c  */
/* a_pacen        -> actions_idle.c  */
/* a_toggt             -> actions_idle.c  */
/* a_sleep                 -> actions_idle.c  */

/* ---- Walk-and-interact -- see actions_house.c ----------------------- */
/* a_readn        -> actions_house.c */
/* a_gioob     -> actions_house.c */
/* a_dance                 -> actions_house.c */
/* a_drink                 -> actions_house.c */
/* a_uset            -> actions_house.c */
/* a_wakum       -> actions_house.c */
/* a_gotbn       -> actions_house.c */

/* ---- Leisure -- see actions_leisure.c ------------------------------- */
/* a_lists            -> actions_leisure.c */
/* a_playp             -> actions_leisure.c */
/* a_plawr       -> actions_leisure.c */
/* a_lighf        -> actions_leisure.c */
/* a_socwd  -> actions_leisure.c */
/* a_sitae       -> actions_leisure.c */
/* a_chefd       -> actions_leisure.c */
/* a_tidyh             -> actions_leisure.c */
/* a_cleau               -> actions_leisure.c */
/* a_opcbc -> actions_leisure.c */

/* ---- Still TODO ----------------------------------------------------- */
/* a_playc -> actions_games.c */
/* a_writl  -> actions_letter.c */
/* a_plaag   -> actions_games.c */
/* a_opcuc -> actions_leisure.c */

/* Delivery event handlers moved to deliveries.c:
 *   er_food, er_bood,
 *   er_recd, er_dogf,
 *   event_answer_phone
 */
