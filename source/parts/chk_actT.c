/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* chk_actT: 9-priority AI ladder.
   1. Event queue -> execEv
   2. Alarm -> WAKE_FROM_ALARM   3. Bathroom -> USE_TOILET
   4. Thirst -> DRINK             5. Hunger -> KITCHEN_CABINET
   6. Lunch  7. Dinner  8. Wake  9. Bedtime (once/day scheduled)
   10. User command queue         11. Random time/mood-based */

void
chk_actT()
{
        /* Declaration order matters, and `unused` must stay: both
           are part of the original's stack frame. */
        short   index;
        short   food_slots;
        short   sickness_skip_probability;
        short   unused;
        /* P1: process any deferred event first */
        if (g_trel[0] != ACTION_NONE) {
                execEv(getEv());
                return;
        }
        /* P2: alarm clock */
        if (alarm_p != NO) {
                g_trac = ACTION_WAKE_FROM_ALARM;
                doAct();
                return;
        }
        /* P3: bathroom */
        if (lcp.bathroom_need != NO) {
                g_trac = ACTION_USE_TOILET;
                doAct();
                return;
        }

        /* Sickness bias: 66% skip healthy, 0% sick. */
        /* Tested this way round on purpose: it matches the original. */
        if (lcp.sickness_level > SICKNESS_HEALTHY)
                sickness_skip_probability = 0;
        else
                sickness_skip_probability = 66;
        /* P4: thirst.  The water gate is a disjunction of two
           conjunctions that re-tests the sickness level in the second
           arm.  Redundant, but kept on purpose: it is the original's
           shape. */
        if (lcp.thirst_level > NEED_SATISFIED) {
                if (rndRng(1, 100) > sickness_skip_probability &&
                    ((lcp.sickness_level != SICKNESS_HEALTHY &&
                      lcp_watr != 0) ||
                     lcp.sickness_level == SICKNESS_HEALTHY)) {
                        g_trac = ACTION_DRINK;
                        doAct();
                        return;
                }
        }

        food_slots = (lcp.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD;

        /* P5: hunger.  Same disjunctive shape; note that the lastAct
           gate applies ONLY to the healthy arm -- it is not
           `(healthy || food) && lastAct != KITCHEN`. */
        if (lcp.hunger_level > NEED_SATISFIED) {
                if (rndRng(1, 100) > sickness_skip_probability &&
                    ((lcp.sickness_level != SICKNESS_HEALTHY &&
                      food_slots != 0) ||
                     (lastAct != ACTION_KITCHEN_CABINET &&
                      lcp.sickness_level == SICKNESS_HEALTHY))) {
                        g_trac = ACTION_KITCHEN_CABINET;
                        doAct();
                        lastAct = ACTION_KITCHEN_CABINET;
                        return;
                }
        }

        /* P6-P9: once-per-day scheduled events */
        if (!lunT_trg && lcp.lunch_hour == t_hour) {
                g_trac = ACTION_EAT_MEAL;
                doAct();
                lunT_trg = YES;
                return;
        }
        if (!dinT_trg && lcp.dinner_hour == t_hour) {
                g_trac = ACTION_EAT_MEAL;
                doAct();
                dinT_trg = YES;
                return;
        }
        if (!wkT_trg && lcp.wake_hour == t_hour) {
                g_trac = ACTION_WAKE_UP_MORNING;
                doAct();
                wkT_trg = YES;
                return;
        }
        if (!bedT_trg && lcp.bedtime_hour == t_hour) {
                g_trac = ACTION_GO_TO_BED_NIGHT;
                doAct();
                bedT_trg = YES;
                return;
        }
        /* P10: command queue.  Low-priority (0..3) commands get shifted
           out on every rejected round; high-priority (>=8) fire
           immediately.  Middle-priority items get their priority
           incremented and stay in the queue for another shot. */
        /* The middle band is tested as `< 8` with the increment in the
           then-arm, and g_trac (not g_aqueu[0]) is re-read for the two
           game actions -- both as in the original. */
        if (g_aliss > 0) {
                if (g_apriq[0] < 4) {
                        for (index = 0; index < 9; index++) {
                                g_aqueu[index] = g_aqueu[index + 1];
                                g_apriq[index] =
                                        g_apriq[index + 1];
                        }
                } else if (g_apriq[0] < 8) {
                        g_apriq[0]++;
                } else {
                        g_trac = g_aqueu[0];
                        if (g_trac == ACTION_PLAY_A_GAME ||
                            g_trac == ACTION_PLAY_ORGAN)
                                a_nodok();
                        for (index = 0; index < 9; index++) {
                                g_aqueu[index] = g_aqueu[index + 1];
                                g_apriq[index] =
                                        g_apriq[index + 1];
                        }
                        g_aliss--;
                        doAct();
                        return;
                }
        }

        /* P11: time/mood-based random pick */
        if ((g_trac = chk_timA()) >= 0)
                doAct();
}
