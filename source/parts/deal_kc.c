/*
 * parts/deal_kc.c -- shared body; LCP_ORG links it in keyboard.o,
 * LCP_STX at 0x15d72 in the 0x148fe object (stx_u3.c), with
 * p_dobls and putEv right behind it.  Files under parts/ are never
 * compiled standalone.
 */
#ifdef FAITHFUL
/* addr: deal_kc() */
void
deal_kc(keycode)
short   keycode;
{
        char    ascii_char;
        unsigned short  food_count;

        switch (keycode) {
        case KEY_CTRL_A_ALARM:
                alarm_p = YES;
                return;

        case KEY_CTRL_B_BOOK:
                p_dobls();
                putEv(ACTION_EVENT_BOOK_DELIVERY);
                return;

        case KEY_CTRL_C_CALL:
                if (ph_ans == NO) {
                        ph_call = YES;
                        putEv(ACTION_EVENT_PHONE_CALL);
                }
                return;

        case KEY_CTRL_D_DOGFOOD:
                p_dobls();
                putEv(ACTION_EVENT_DOG_FOOD);
                return;

        case KEY_CTRL_F_FOOD:
                food_count = (lcp.door_states_and_flags >> 9) & 7;
                if (food_dlv != NO && food_count < 4)
                        food_dlv = NO;
                if (food_count == 4) {
                        food_dlv = YES;
                } else {
                        p_dobls();
                        putEv(ACTION_EVENT_FOOD_DELIVERY);
                }
                return;

        case KEY_CTRL_R_RECORD:
                if (g_inpmd == NO) {
                        p_dobls();
                        putEv(ACTION_EVENT_RECORD_DELIVERY);
                }
                return;

        case KEY_CTRL_W_WATER:
                if (lcp_watr != 10) {
                        sf_sele(SFX_WATER_TAP, -1L);
                        updWtLv(1);
                }
                return;

        case KEY_CTRL_P_PATTING:
                if (dg_petok != NO && g_ptdoa == NO) {
                        g_ptanf          = 0;
                        g_ptdoa          = YES;
                        lcp.happiness               = MOOD_HAPPY;
                        lcp.happiness_direction     = DIR_WORSENING;
                        lcp.happiness_duration_active =
                                lcp.happiness_initial_countdown;
                }
                return;

        case KEY_CTRL_M:
                if (g_inpmd == NO) {
                        prsCmd();
                        g_srsdc = 4;
                        g_cdibp = 0;
                }
                return;

        case KEY_CURSOR_LEFT:
                if (g_inpmd == NO && g_cdibp > 0) {
                        g_cdibp = g_cdibp - 1;
                        ascii_char = g_cdinb[g_cdibp];
                        g_cdinb[g_cdibp] = '\0';
                        prCh((short) ascii_char,
                                   g_cdibp << 3, 23,
                                   COLOR_white);
                }
                return;
        }

        /* Default: printable character in text-input mode. */
        if (g_inpmd == NO &&
            g_cdibp < 38 &&
            keycode > 0x1f) {
                g_cdinb[g_cdibp] =
                        (char) keycode;
                g_cdibp = g_cdibp + 1;
                g_cdinb[g_cdibp] = '\0';
                prCh(keycode,
                           (g_cdibp - 1) * 8, 23,
                           COLOR_black);
        }
}
#else   /* STX: link #-6 -- the selector is copied to a local, the
           cases appear in a different source order, the food counter
           is recomputed rather than latched, and the backspace case
           reuses the parameter as its scratch. */

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
                if (dg_petok != NO && g_ptdoa == NO) {
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
#endif
