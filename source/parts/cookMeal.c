/*
 * parts/cookMeal.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* cookMeal: cook and eat a meal.  The resident walks to the kitchen
   cabinet, takes out the cooking pot and carries it to the stove,
   where the pot sits on the hob while random flame frames flicker
   for 30..50 ticks; the stove is then drawn off and he carries the
   cooked meal back to the cabinet and eats it through eatFromCabinet. */
void
cookMeal()
{
        /* walkToTarget()'s result is tested in place, with no local for it. */
        short   counter;

        posToXY(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        if (walkToTarget() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();

        lcp_st = STATE_BEND_DOWN;    gameTick(1);
        lcp_st = STATE_REACH_FORWARD;gameTick(2);
        lcp_st = STATE_STAND_FACING_SCREEN; gameTick(0);

        carryBehind(SPRITE_COOKING_POT);
        posToXY(POS_BTM_STOVE,
                              &g_wtx, &g_wty);
        g_actif = YES;
        walkToTarget();

        g_selaf[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        layoutSlots();
        carryBehind(SPRITE_COOKING_POT);
        g_lcyof = NO;
        g_sepex[g_seslm[SPRITE_COOKING_POT]] = 11;
        g_sepey[g_seslm[SPRITE_COOKING_POT]] = 172;

        lcp_face = FACING_LEFT;
        lcp_st            = STATE_BEND_AND_REACH;

        /* 30..50 tick cooking animation, rotating stove frames. */
        counter = rndRng(30, 50);
        while (counter-- != 0) {
                drawObject(g_obisa[rndRng(0, 2)], STOVE_X, STOVE_Y);
                gameTick(1);
        }
        drawObject(OBJ_STOVE_OFF, STOVE_X, STOVE_Y);

        g_selaf[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        layoutSlots();
        carryBehind(SPRITE_COOKED_MEAL);

        /* Back to cabinet, then chain into eatFromCabinet to eat. */
        posToXY(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        g_actif = YES;
        walkToTarget();
        g_selaf[SPRITE_COOKED_MEAL] = SPRITE_HIDDEN;
        layoutSlots();
        g_lcyof = NO;
        gameTick(0);
        eatFromCabinet();
        g_actif = NO;
}
