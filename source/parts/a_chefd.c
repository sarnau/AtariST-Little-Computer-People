/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* a_chefd: check the front door.  The resident walks to the front
   door, opens it if shut, steps outside (the sitting-dog sprite waits
   on the porch, the resident is hidden) for `value` ticks, comes back
   in and the dog sprite is removed.  If a random roll beats
   lcp.initiative_threshold he walks back and shuts the door again.
   g_actif keeps the inner walks from being preempted. */
void
a_chefd(value)
short   value;
{

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        if (lcp_frdO == NO)
                a_opcfd(DOOR_OPEN);
        g_actif = YES;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx -= 10;
        lcp_wkD();

        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hideLcp();
        gameTick(value);
        showLcp();

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx -= 10;
        lcp_wkD();
        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sp_upds();

        if (lcp.initiative_threshold < rndRng(0, 100)) {
                g_actif = YES;
                hs_posXY(POS_BTM_FRONT_DOOR,
                                      &g_wtx, &g_wty);
                lcp_wkD();
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfd(DOOR_CLOSE);
        }
        g_actif = NO;
}
