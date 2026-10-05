/*
 * parts/lightFire.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* lightFire: light the fireplace; does nothing if a fire is already
   burning.  The resident opens the front door, steps outside (the
   sitting-dog sprite waits on the porch) for 40 ticks, returns carrying
   firewood, perhaps shuts the door (random roll against
   lcp.initiative_threshold), and walks to the fireplace, where he
   bends, stokes and fidgets for ten ticks.  Then fire_act is set and
   fire_dur is 2500..5000 ticks, which the tick loop counts down. */
void
lightFire()
{
        /* walkToTarget()'s result is tested in place, with no local for it. */
        short   i;

        if (fire_act != NO)
                return;

        posToXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        if (walkToTarget() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();
        openFrontDoor(DOOR_OPEN);
        g_actif = YES;

        posToXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx -= 10;
        walkToTarget();

        /* Sit-dog sprite waits at the porch. */
        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        posToXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        walkToTarget();
        hideResident();
        gameTick(40);
        showResident();

        carryBehind(SPRITE_FIREWOOD);
        posToXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx -= 10;
        walkToTarget();

        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        layoutSlots();

        if (lcp.initiative_threshold < rndRng(0, 100))
                openFrontDoor(DOOR_CLOSE);

        posToXY(POS_BTM_FIREPLACE_LOGS,
                              &g_wtx, &g_wty);
        g_actif = YES;
        walkToTarget();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_FIREWOOD] = SPRITE_HIDDEN;
        layoutSlots();
        g_lcyof = NO;
        waitHeadTurn();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_BEND_DOWN;      gameTick(1);
        lcp_st = STATE_REACH_FORWARD;  gameTick(1);
        lcp_st = STATE_STOKE_FIREPLACE;gameTick(1);

        /* Random-direction shrug for 10 ticks (feeding kindling). */
        for (i = 0; i < 10; i++) {
                lcp_face = rndRng(0, 1);
                gameTick(0);
        }

        fire_act        = YES;
        fire_dur = rndRng(2500, 5000);

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
        g_actif = NO;
}
