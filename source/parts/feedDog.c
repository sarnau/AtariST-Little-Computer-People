/*
 * parts/feedDog.c -- included by stx_u2.c; never compiled on its own.
 */

/* feedDog: fill the dog's bowl.  With value 0 the resident first
   fetches a food package from the fridge (door opens, he reaches in);
   with value non-zero the caller (a food delivery) has already put the
   package in his hands.  He carries it to the dog bowl, bends and
   fills it (lcp_bwlS = BOWL_FULL, dg_bwlch flags the change for the
   tick loop), then carries the package back and stores it (putInFridge). */
void
feedDog(value)
short   value;
{
        /* The call is tested in place, with no local. */

        if (value == 0) {
                posToXY(POS_BTM_FRIDGE,
                                      &g_wtx, &g_wty);
                if (walkToTarget() != 0)
                        return;

                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                waitHeadTurn();

                lcp_face = FACING_LEFT;
                lcp_st            = STATE_REACH_INTO_CABINET;
                drawObject(OBJ_FRIDGE_CLOSED, FRIDGE_X, FRIDGE_Y);
                gameTick(1);
                drawObject(OBJ_FRIDGE_OPEN_1, FRIDGE_X, FRIDGE_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(1);
                drawObject(OBJ_FRIDGE_OPEN_2, FRIDGE_X, FRIDGE_Y);
                gameTick(1);

                lcp_face = FACING_RIGHT;
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);

                lcp_face = FACING_LEFT;
                lcp_st = STATE_REACH_INTO_CABINET;
                gameTick(3);

                lcp_face = FACING_RIGHT;
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);

                drawObject(OBJ_FRIDGE_OPEN_1, FRIDGE_X, FRIDGE_Y);
                gameTick(1);
                drawObject(OBJ_FRIDGE_CLOSED, FRIDGE_X, FRIDGE_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(1);

                carryBehind(SPRITE_FOOD_PACKAGE);
        }

        posToXY(POS_BTM_DOG_BOWL,
                              &g_wtx, &g_wty);
        g_actif = YES;
        walkToTarget();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        layoutSlots();
        g_lcyof = NO;
        waitHeadTurn();

        lcp_st = STATE_BEND_DOWN;    gameTick(1);
        lcp_st = STATE_REACH_FORWARD;gameTick(2);
        lcp_st = STATE_BEND_DOWN;    gameTick(1);

        dg_bwlch = 1;
        lcp_bwlS  = BOWL_FULL;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        carryBehind(SPRITE_FOOD_PACKAGE);
        posToXY(POS_BTM_FRIDGE,
                              &g_wtx, &g_wty);
        g_actif = YES;
        walkToTarget();

        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        layoutSlots();
        g_lcyof = NO;
        putInFridge();
        g_actif = NO;
}
