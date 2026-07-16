/*
 * astubs.c -- placeholder bodies for doAct() handlers not
 *                   yet ported for real.
 *
 * Style: one-line bodies keep this file skimmable; real ports live in
 * per-group action_*.c files.  Delete each stub as its real .c comes
 * online.  Ported handlers get a leading comment marker so the diff
 * with a Ghidra-listed action_* set is trivially auditable.
 */

/* ---- Idle / gesture group -- see asimple.c, aidle.c ---- */
/* a_hello                 -> asimple.c */
/* a_yawas      -> asimple.c */
/* a_nodh              -> asimple.c */
/* a_calld              -> asimple.c */
/* a_wakfa       -> asimple.c */
/* a_petd               -> asimple.c */
/* a_wandi           -> aidle.c  */
/* a_peeka           -> aidle.c  */
/* a_pacen        -> aidle.c  */
/* a_toggt             -> aidle.c  */
/* a_sleep                 -> aidle.c  */

/* ---- Walk-and-interact -- see ahouse.c ----------------------- */
/* a_readn        -> ahouse.c */
/* a_gioob     -> ahouse.c */
/* a_dance                 -> ahouse.c */
/* a_drink                 -> ahouse.c */
/* a_uset            -> ahouse.c */
/* a_wakum       -> ahouse.c */
/* a_gotbn       -> ahouse.c */

/* ---- Leisure -- see aleisure.c ------------------------------- */
/* a_lists            -> aleisure.c */
/* a_playp             -> aleisure.c */
/* a_plawr       -> aleisure.c */
/* a_lighf        -> aleisure.c */
/* a_socwd  -> aleisure.c */
/* a_sitae       -> aleisure.c */
/* a_chefd       -> aleisure.c */
/* a_tidyh             -> aleisure.c */
/* a_cleau               -> aleisure.c */
/* a_opcbc -> aleisure.c */

/* ---- Still TODO ----------------------------------------------------- */
/* a_playc -> agames.c */
/* a_writl  -> aletter.c */
/* a_plaag   -> agames.c */
/* a_opcuc -> aleisure.c */

/* Delivery event handlers moved to delivery.c:
 *   er_food, er_bood,
 *   er_recd, er_dogf,
 *   ev_ansPh
 */
