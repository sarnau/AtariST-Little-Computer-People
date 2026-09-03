/*
 * parts/chk_actT.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x5ce2, immediately after gameLoop). Files under parts/ are
 * never compiled standalone.
 */
/* chk_actT: 9-priority AI ladder.
   1. Event queue -> execEv
   2. Alarm -> WAKE_FROM_ALARM   3. Bathroom -> USE_TOILET
   4. Thirst -> DRINK             5. Hunger -> KITCHEN_CABINET
   6. Lunch  7. Dinner  8. Wake  9. Bedtime (once/day scheduled)
   10. User command queue         11. Random time/mood-based
   addr: chk_actT() */

void
chk_actT()
{
        /* STX's frame is also -12, but its four shorts are ordered
           index, food_slots, skip-probability, unused: it keeps no
           `event`/`rnd` temporaries and hoists the food-slot count
           into a local of its own. */
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
        /* STX tests the other way round, so the arms swap. */
        if (lcp.sickness_level > 0)
                sickness_skip_probability = 0;
        else
                sickness_skip_probability = 66;
        /* P4: thirst.  STX spells the water gate as a disjunction of
           two conjunctions, re-testing the sickness level in the
           second arm -- the three tst.w and their branch targets pin
           the shape. */
        if (lcp.thirst_level > 0) {
                if (rndRng(1, 100) > sickness_skip_probability &&
                    ((lcp.sickness_level != SICKNESS_HEALTHY &&
                      lcp_watr != 0) ||
                     lcp.sickness_level == SICKNESS_HEALTHY)) {
                        g_trac = ACTION_DRINK;
                        doAct();
                        return;
                }
        }

        food_slots = (lcp.door_states_and_flags >> 9) & 7;

        /* P5: hunger.  Same disjunctive shape, and note that STX's
           lastAct gate applies ONLY to the healthy arm -- it is not
           the ROM's `(healthy || food) && lastAct != KITCHEN`. */
        if (lcp.hunger_level > 0) {
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
        /* STX tests the middle band as `< 8` and puts the increment in
           the then-arm, and it re-reads g_trac (not g_aqueu[0]) for
           the two game actions. */
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
                            g_trac == ACTION_PLAY_WITH_RECORD)
                                a_getd();
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
