/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* drinkWater: get a drink of water.  The resident walks to the kitchen
   sink, picks up the glass and carries it to the water tap.  If the
   water tank (lcp_watr) is not empty he bends, draws 3 units
   (updateWaterTank(-3)), drinks for 16 ticks and rinses the glass at the sink
   (washAtSink).  Thirst is reset to satisfied with a full timer whether or
   not there was water, and startRecovery may start recovery from sickness. */
void
drinkWater()
{
        posToXY(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        if (walkToTarget() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();

        g_actif = YES;
        carryBehind(SPRITE_GLASS);
        posToXY(POS_BTM_WATER_TAP,
                              &g_wtx, &g_wty);
        walkToTarget();

        g_selaf[SPRITE_GLASS] = SPRITE_HIDDEN;
        layoutSlots();
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();

        if (lcp_watr != 0) {
                lcp_st = STATE_BEND_DOWN;
                lcp_face = FACING_RIGHT;
                gameTick(0);
                updateWaterTank(-3);
                g_hamod = HEAD_ANIM_DISABLED;
                lcp_st = STATE_DRINK_FROM_GLASS;
                gameTick(16);
                lcp_st = STATE_STAND_FACING_SCREEN;
                lcp_y++;
                gameTick(3);
                washAtSink(3);
        }

        lcp.thirst_level = NEED_SATISFIED;
        lcp.thirst_timer = lcp.thirst_timer_max;
        startRecovery();
        g_selaf[SPRITE_GLASS] = SPRITE_HIDDEN;
        layoutSlots();
        g_lcyof = NO;
        g_actif = NO;
}
