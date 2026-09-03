/*
 * parts/lt_tyca.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aletter.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aletter.c.
 * Files under parts/ are never compiled standalone.
 */

void
lt_tyca(ch)
short   ch;
{
        /* One local: STX consumes every rndRng result in place. */
        short   i;

        if (ch < ' ') {                 /* CR */
                lcp_st = STATE_DESK_TYPE_L;
                gameTick(0);
                if (rndRng(0, 5) == 0) {
                        lcp_st = STATE_DESK_TYPE_R;
                        gameTick(0);
                        lcp_st = STATE_DESK_TYPE_L;
                        gameTick(0);
                }
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_WRITE_AT_DESK;
                gameTick(0);

                g_srsdc = 4;
                g_cdibp = 0;

                g_selaf[SPRITE_TYPING_1] = SPRITE_HIDDEN;
                g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
                g_selaf[SPRITE_TYPING_3] = SPRITE_HIDDEN;
                g_selaf[SPRITE_TYPING_4] = SPRITE_HIDDEN;
                sp_upds();

                /* Width-bracket sprite for buffer_pos (0..9/10..19/20..29/30+).
                   i resolves to 0 here (buffer_pos just cleared);
                   preserved verbatim. */
                i = 3;
                if (g_cdibp < 10)      i = 0;
                else if (g_cdibp < 20) i = 1;
                else if (g_cdibp < 30) i = 2;

                g_selaf[g_ltcwt[i]] = SPRITE_IN_FRONT;
                sp_sprs(g_ltcwt[i]);
                g_sepex[g_seslm[g_ltcwt[i]]] = 211;
                g_sepey[g_seslm[g_ltcwt[i]]] =  44;
                lt_sets();
                gameTick(6);
                return;
        }

        /* Printable char. */
        if (ch == ' ') {
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_DESK_TYPE_L;
                gameTick(0);
        } else {
                lcp_face = rndRng(0, 1);
                lcp_st = STATE_DESK_TYPE_L;
                gameTick(0);
                if (rndRng(0, 5) == 0) {
                        lcp_st = STATE_DESK_TYPE_R;
                        gameTick(0);
                        lcp_st = STATE_DESK_TYPE_L;
                        gameTick(0);
                }
        }
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_WRITE_AT_DESK;
        sfClick();
        gameTick(0);
        prCh(ch, g_cdibp << 3, 23, COLOR_black);
        g_cdibp++;

        g_selaf[SPRITE_TYPING_1] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_3] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_4] = SPRITE_HIDDEN;
        sp_upds();

        i = 3;
        if (g_cdibp < 10)      i = 0;
        else if (g_cdibp < 20) i = 1;
        else if (g_cdibp < 30) i = 2;

        g_selaf[g_ltcwt[i]] = SPRITE_IN_FRONT;
        sp_sprs(g_ltcwt[i]);
        g_sepex[g_seslm[g_ltcwt[i]]] = 211;
        g_sepey[g_seslm[g_ltcwt[i]]] =  44;

        /* 1/21 chance of a short pause between keystrokes. */
        if (rndRng(0, 20) == 0)
                gameTick(rndRng(0, 3));
}
