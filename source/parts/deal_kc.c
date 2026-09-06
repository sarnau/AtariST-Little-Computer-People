/*
 * parts/deal_kc.c -- shared body; LCP_STX links it in at 0x15d72 in
 * the 0x148fe object (stx_u3.c), with p_dobls and putEv right behind
 * it. Files under parts/ are never compiled standalone.
 */

void
deal_kc(keycode)
short   keycode;
{
        short   sel;

        sel = keycode;
        switch (sel) {
        case KEY_CTRL_B_BOOK:
                p_dobls();
                putEv(ACTION_EVENT_BOOK_DELIVERY);
                return;

        case KEY_CTRL_R_RECORD:
                if (g_inpmd != NO)
                        return;
                p_dobls();
                putEv(ACTION_EVENT_RECORD_DELIVERY);
                return;

        case KEY_CTRL_F_FOOD:
                if (food_dlv != NO &&
                    ((lcp.door_states_and_flags >> 9) & 7) < 4)
                        food_dlv = NO;
                if (((lcp.door_states_and_flags >> 9) & 7) == 4) {
                        food_dlv = YES;
                        return;
                }
                p_dobls();
                putEv(ACTION_EVENT_FOOD_DELIVERY);
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
                putEv(ACTION_EVENT_PHONE_CALL);
                return;

        case KEY_CTRL_D_DOGFOOD:
                p_dobls();
                putEv(ACTION_EVENT_DOG_FOOD);
                return;

        case KEY_CTRL_W_WATER:
                if (lcp_watr == 10)
                        return;
                sf_sele(SFX_WATER_TAP, -1L);
                updWtLv(1);
                return;

        case KEY_CTRL_A_ALARM:
                alarm_p = YES;
                return;

        case KEY_CTRL_M:
                if (g_inpmd != NO)
                        return;
                prsCmd();
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
                        prCh(keycode, g_cdibp << 3, 23, COLOR_white);
                }
                return;

        default:
                /* Printable character in text-input mode. */
                if (g_inpmd != NO)
                        return;
                /* STX guards the append with one compound `if`, not
                   two early returns: both tests branch to the `break`
                   below rather than straight to the epilogue. */
                if (g_cdibp < 38 && keycode >= 32) {
                        /* Naming the index first folds the array base
                           into `add.l #base,An` instead of loading it
                           into a second address register. */
                        *(g_cdibp + g_cdinb) = keycode;
                        g_cdibp++;
                        g_cdinb[g_cdibp] = '\0';
                        prCh(keycode, (g_cdibp - 1) << 3, 23, COLOR_black);
                }
                break;
        }
}
