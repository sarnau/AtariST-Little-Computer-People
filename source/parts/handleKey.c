/*
 * Dispatches a typed key: the Ctrl delivery/call/water/alarm/pat
 * commands, Return, erase, and plain characters into the command line.
 */

void
handleKey(keycode)
short   keycode;
{
        short   sel;

        sel = keycode;
        switch (sel) {
        case KEY_CTRL_B_BOOK:
                playDoorbell();
                queueEvent(ACTION_EVENT_BOOK_DELIVERY);
                return;

        case KEY_CTRL_R_RECORD:
                if (typingOff != NO)
                        return;
                playDoorbell();
                queueEvent(ACTION_EVENT_RECORD_DELIVERY);
                return;

        case KEY_CTRL_F_FOOD:
                if (pantryFull != NO &&
                    ((resident.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD) < FOOD_PACKS_MAX)
                        pantryFull = NO;
                if (((resident.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD) == FOOD_PACKS_MAX) {
                        pantryFull = YES;
                        return;
                }
                playDoorbell();
                queueEvent(ACTION_EVENT_FOOD_DELIVERY);
                return;

        case KEY_CTRL_P_PATTING:
                if (patAllowed != NO && patActive == NO) {
                        patFrame          = 0;
                        patActive          = YES;
                        resident.happiness               = MOOD_HAPPY;
                        resident.happiness_direction     = DIR_WORSENING;
                        resident.happiness_duration_active =
                                resident.mood_duration[MOOD_HAPPY];
                }
                return;

        case KEY_CTRL_C_CALL:
                if (phoneAnswered != NO)
                        return;
                phoneRinging = YES;
                queueEvent(ACTION_EVENT_PHONE_CALL);
                return;

        case KEY_CTRL_D_DOGFOOD:
                playDoorbell();
                queueEvent(ACTION_EVENT_DOG_FOOD);
                return;

        case KEY_CTRL_W_WATER:
                if (waterLevel == WATER_MAX)
                        return;
                sfxSelect(SFX_WATER_TAP, -1L);
                updateWaterTank(1);
                return;

        case KEY_CTRL_A_ALARM:
                alarmRinging = YES;
                return;

        case KEY_CTRL_M:
                if (typingOff != NO)
                        return;
                submitCommand();
                stripScroll = 4;
                typedCursor = 0;
                return;

        case KEY_CURSOR_LEFT:
                if (typingOff != NO)
                        return;
                if (typedCursor > 0) {
                        typedCursor--;
                        keycode = typedLine[typedCursor];
                        typedLine[typedCursor] = '\0';
                        printChar(keycode, typedCursor << 3, 23, COLOR_white);
                }
                return;

        default:
                /* Printable character in text-input mode. */
                if (typingOff != NO)
                        return;
                /* One compound `if`, not two early returns, on purpose:
                   both tests must branch to the `break` below rather
                   than straight to the function end. */
                if (typedCursor < 38 && keycode >= 32) {
                /* Index named first on purpose: this spelling
                   reproduces the original's addressing code. */
                        *(typedCursor + typedLine) = keycode;
                        typedCursor++;
                        typedLine[typedCursor] = '\0';
                        printChar(keycode, (typedCursor - 1) << 3, 23, COLOR_black);
                }
                break;
        }
}
