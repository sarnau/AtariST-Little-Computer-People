/* 9-priority AI ladder.
   1. Event queue -> runEvent
   2. Alarm -> WAKE_FROM_ALARM   3. Bathroom -> USE_TOILET
   4. Thirst -> DRINK             5. Hunger -> KITCHEN_CABINET
   6. Lunch  7. Dinner  8. Wake  9. Bedtime (once/day scheduled)
   10. User command queue         11. Random time/mood-based */

void
chooseAction()
{
        /* Declaration order matters, and `unused` must stay: both
           are part of the original's stack frame. */
        short   index;
        short   foodSlots;
        short   sicknessSkipProbability;
        short   unused;
        /* P1: process any deferred event first */
        if (eventQueue[0] != ACTION_NONE) {
                runEvent(nextEvent());
                return;
        }
        /* P2: alarm clock */
        if (alarmRinging != NO) {
                nextAction = ACTION_WAKE_FROM_ALARM;
                runAction();
                return;
        }
        /* P3: bathroom */
        if (resident.bathroomNeed != NO) {
                nextAction = ACTION_USE_TOILET;
                runAction();
                return;
        }

        /* Sickness bias: 66% skip healthy, 0% sick. */
        /* Tested this way round on purpose: it matches the original. */
        if (resident.sicknessLevel > SICKNESS_HEALTHY)
                sicknessSkipProbability = 0;
        else
                sicknessSkipProbability = 66;
        /* P4: thirst.  The water gate is a disjunction of two
           conjunctions that re-tests the sickness level in the second
           arm.  Redundant, but kept on purpose: it is the original's
           shape. */
        if (resident.thirstLevel > NEED_SATISFIED) {
                if (rndRng(1, 100) > sicknessSkipProbability &&
                    ((resident.sicknessLevel != SICKNESS_HEALTHY &&
                      waterLevel != 0) ||
                     resident.sicknessLevel == SICKNESS_HEALTHY)) {
                        nextAction = ACTION_DRINK;
                        runAction();
                        return;
                }
        }

        foodSlots = (resident.doorStatesAndFlags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD;

        /* P5: hunger.  Same disjunctive shape; note that the lastAction
           gate applies ONLY to the healthy arm -- it is not
           `(healthy || food) && lastAction != KITCHEN`. */
        if (resident.hungerLevel > NEED_SATISFIED) {
                if (rndRng(1, 100) > sicknessSkipProbability &&
                    ((resident.sicknessLevel != SICKNESS_HEALTHY &&
                      foodSlots != 0) ||
                     (lastAction != ACTION_KITCHEN_CABINET &&
                      resident.sicknessLevel == SICKNESS_HEALTHY))) {
                        nextAction = ACTION_KITCHEN_CABINET;
                        runAction();
                        lastAction = ACTION_KITCHEN_CABINET;
                        return;
                }
        }

        /* P6-P9: once-per-day scheduled events */
        if (!lunchDone && resident.lunchHour == t_hour) {
                nextAction = ACTION_EAT_MEAL;
                runAction();
                lunchDone = YES;
                return;
        }
        if (!dinnerDone && resident.dinnerHour == t_hour) {
                nextAction = ACTION_EAT_MEAL;
                runAction();
                dinnerDone = YES;
                return;
        }
        if (!wakeupDone && resident.wakeHour == t_hour) {
                nextAction = ACTION_WAKE_UP_MORNING;
                runAction();
                wakeupDone = YES;
                return;
        }
        if (!bedtimeDone && resident.bedtimeHour == t_hour) {
                nextAction = ACTION_GO_TO_BED_NIGHT;
                runAction();
                bedtimeDone = YES;
                return;
        }
        /* P10: command queue.  Low-priority (0..3) commands get shifted
           out on every rejected round; high-priority (>=8) fire
           immediately.  Middle-priority items get their priority
           incremented and stay in the queue for another shot. */
        /* The middle band is tested as `< 8` with the increment in the
           then-arm, and nextAction (not queueActions[0]) is re-read for the two
           game actions -- both as in the original. */
        if (queueCount > 0) {
                if (queuePriority[0] < 4) {
                        for (index = 0; index < 9; index++) {
                                queueActions[index] = queueActions[index + 1];
                                queuePriority[index] =
                                        queuePriority[index + 1];
                        }
                } else if (queuePriority[0] < 8) {
                        queuePriority[0]++;
                } else {
                        nextAction = queueActions[0];
                        if (nextAction == ACTION_PLAY_A_GAME ||
                            nextAction == ACTION_PLAY_ORGAN)
                                nodOk();
                        for (index = 0; index < 9; index++) {
                                queueActions[index] = queueActions[index + 1];
                                queuePriority[index] =
                                        queuePriority[index + 1];
                        }
                        queueCount--;
                        runAction();
                        return;
                }
        }

        /* P11: time/mood-based random pick */
        if ((nextAction = pickIdleAction()) >= 0)
                runAction();
}
