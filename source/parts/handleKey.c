/*
 * Dispatches a typed key: the Ctrl delivery/call/water/alarm/pat
 * commands, Return, erase, and plain characters into the command line.
 *
 * Included by stx_u3.c; never compiled on its own.
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
                if (g_inpmd != NO)
                        return;
                playDoorbell();
                queueEvent(ACTION_EVENT_RECORD_DELIVERY);
                return;

        case KEY_CTRL_F_FOOD:
                if (food_dlv != NO &&
                    ((lcp.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD) < FOOD_PACKS_MAX)
                        food_dlv = NO;
                if (((lcp.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD) == FOOD_PACKS_MAX) {
                        food_dlv = YES;
                        return;
                }
                playDoorbell();
                queueEvent(ACTION_EVENT_FOOD_DELIVERY);
                return;

        case KEY_CTRL_P_PATTING:
                if (pat_ok != NO && g_ptdoa == NO) {
                        g_ptanf          = 0;
                        g_ptdoa          = YES;
                        lcp.happiness               = MOOD_HAPPY;
                        lcp.happiness_direction     = DIR_WORSENING;
                        lcp.happiness_duration_active =
                                lcp.happiness_initial_countdown;
                }
                return;

        case KEY_CTRL_C_CALL:
                if (ph_ans != NO)
                        return;
                ph_call = YES;
                queueEvent(ACTION_EVENT_PHONE_CALL);
                return;

        case KEY_CTRL_D_DOGFOOD:
                playDoorbell();
                queueEvent(ACTION_EVENT_DOG_FOOD);
                return;

        case KEY_CTRL_W_WATER:
                if (lcp_watr == WATER_MAX)
                        return;
                sfxSelect(SFX_WATER_TAP, -1L);
                updateWaterTank(1);
                return;

        case KEY_CTRL_A_ALARM:
                alarm_p = YES;
                return;

        case KEY_CTRL_M:
                if (g_inpmd != NO)
                        return;
                submitCommand();
                g_srsdc = 4;
                g_cdibp = 0;
                return;

        case KEY_CURSOR_LEFT:
                if (g_inpmd != NO)
                        return;
                if (g_cdibp > 0) {
                        g_cdibp--;
                        keycode = g_cdinb[g_cdibp];
                        g_cdinb[g_cdibp] = '\0';
                        printChar(keycode, g_cdibp << 3, 23, COLOR_white);
                }
                return;

        default:
                /* Printable character in text-input mode. */
                if (g_inpmd != NO)
                        return;
                /* One compound `if`, not two early returns, on purpose:
                   both tests must branch to the `break` below rather
                   than straight to the function end. */
                if (g_cdibp < 38 && keycode >= 32) {
                /* Index named first on purpose: this spelling
                   reproduces the original's addressing code. */
                        *(g_cdibp + g_cdinb) = keycode;
                        g_cdibp++;
                        g_cdinb[g_cdibp] = '\0';
                        printChar(keycode, (g_cdibp - 1) << 3, 23, COLOR_black);
                }
                break;
        }
}
