/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* checkFrontDoor: check the front door.  The resident walks to the front
   door, opens it if shut, steps outside (the sitting-dog sprite waits
   on the porch, the resident is hidden) for `value` ticks, comes back
   in and the dog sprite is removed.  If a random roll beats
   lcp.initiative_threshold he walks back and shuts the door again.
   g_actif keeps the inner walks from being preempted. */
void
checkFrontDoor(value)
short   value;
{

        posToXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        if (walkToTarget() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();
        if (lcp_frdO == NO)
                openFrontDoor(DOOR_OPEN);
        g_actif = YES;

        posToXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx -= 10;
        walkToTarget();

        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        posToXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        walkToTarget();
        hideResident();
        gameTick(value);
        showResident();

        posToXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx -= 10;
        walkToTarget();
        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        layoutSlots();

        if (lcp.initiative_threshold < rndRng(0, 100)) {
                g_actif = YES;
                posToXY(POS_BTM_FRONT_DOOR,
                                      &g_wtx, &g_wty);
                walkToTarget();
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                waitHeadTurn();
                openFrontDoor(DOOR_CLOSE);
        }
        g_actif = NO;
}
