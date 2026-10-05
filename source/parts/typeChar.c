/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Types one character of the letter at the desk.  ch below ' ' is a
   carriage return: a few typing frames, then the line buffer position
   g_cdibp is reset and g_srsdc = 4 makes gameTick scroll the paper strip
   (scrollStrip), with the typewriter key sound (typeKeySound).  Any other ch is
   typed with a random hand, a click (sfxClick) and printed at column
   g_cdibp, which then advances.  Both paths show the one SPRITE_TYPING_*
   sprite that matches how far along the line the carriage is. */
void
typeChar(ch)
short   ch;
{
        /* One local: every rndRng result is consumed in place. */
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
                layoutSlots();

                /* Width-bracket sprite for buffer_pos (0..9/10..19/20..29/30+).
                   i resolves to 0 here (buffer_pos just cleared);
                   preserved verbatim. */
                i = 3;
                if (g_cdibp < 10)      i = 0;
                else if (g_cdibp < 20) i = 1;
                else if (g_cdibp < 30) i = 2;

                g_selaf[g_ltcwt[i]] = SPRITE_IN_FRONT;
                activateSprite(g_ltcwt[i]);
                g_sepex[g_seslm[g_ltcwt[i]]] = 211;
                g_sepey[g_seslm[g_ltcwt[i]]] =  44;
                typeKeySound();
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
        sfxClick();
        gameTick(0);
        printChar(ch, g_cdibp << 3, 23, COLOR_black);
        g_cdibp++;

        g_selaf[SPRITE_TYPING_1] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_3] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_4] = SPRITE_HIDDEN;
        layoutSlots();

        i = 3;
        if (g_cdibp < 10)      i = 0;
        else if (g_cdibp < 20) i = 1;
        else if (g_cdibp < 30) i = 2;

        g_selaf[g_ltcwt[i]] = SPRITE_IN_FRONT;
        activateSprite(g_ltcwt[i]);
        g_sepex[g_seslm[g_ltcwt[i]]] = 211;
        g_sepey[g_seslm[g_ltcwt[i]]] =  44;

        /* 1/21 chance of a short pause between keystrokes. */
        if (rndRng(0, 20) == 0)
                gameTick(rndRng(0, 3));
}
