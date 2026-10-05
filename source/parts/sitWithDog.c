/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* ACTION_SIT_ON_COUCH_WITH_DOG.  callDog walks the resident to the
   couch beside the phone (uninterruptibly, g_actif) and lets him be
   patted; he sits upright, the SPRITE_READING_1 prop appears beside
   him, and he pets the dog for 30..50 three-tick rounds unless a new
   action is queued.  Then he sits up, the prop is hidden, he crouches
   to stand, waits for a running Ctrl-P pat (g_ptdoa) to finish, and
   clears pat_ok so the player can no longer pat him. */
void
sitWithDog()
{
        short   ticks;

        g_actif = YES;
        callDog();
        g_actif = NO;
        if (g_trel[0] != ACTION_NONE) {
                lcp_st = STATE_STAND_SIDE_VIEW;
                gameTick(0);
                return;
        }

        /* The +9 is deliberately split into +3 and +6 around the state
           assignment; merging them changes the compiled code. */
        lcp_y += 3;
        lcp_st = STATE_SIT_COUCH_UPRIGHT;
        lcp_y += 6;
        g_hatas = 8;
        waitHeadTurn();
        gameTick(3);

        lcp_y -= 3;
        g_selaf[SPRITE_READING_1] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_READING_1);
        g_sepex[g_seslm[SPRITE_READING_1]] = 221;
        g_sepey[g_seslm[SPRITE_READING_1]] = 172;

        ticks = rndRng(30, 50);
        lcp_st = STATE_SIT_COUCH_PETTING_DOG;
        /* Post-decrement loop with the break in the body, and the
           trailing +3 / state assignment split the same way as the
           entry sequence -- both shapes are what the original compiles
           to. */
        while (ticks--) {
                if (g_trel[0] != ACTION_NONE)
                        break;
                gameTick(3);
        }

        lcp_y += 3;
        lcp_st = STATE_SIT_COUCH_UPRIGHT;
        g_selaf[SPRITE_READING_1] = SPRITE_HIDDEN;
        layoutSlots();
        g_hatas = 8;
        waitHeadTurn();
        gameTick(3);

        /* The -9 is deliberately two separate steps. */
        lcp_y -= 3;
        lcp_y -= 6;
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(8);
        while (g_ptdoa != NO)
                gameTick(0);

        pat_ok = NO;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_face = FACING_RIGHT;
        gameTick(1);
}
