/*
 * parts/a_opecf.c -- included by stx_u2.c; never compiled on its own.
 */

/* a_opecf: put something back in the fridge.  Assumes the resident
   is already at it: he faces the screen, the fridge door is opened
   with the door sound, he reaches in, pauses, and the door is drawn
   shut again.  Both door movements play SFX_DOOR_OPEN.  Ends a_feedd
   and the food-delivery path (a_gesff). */
void
a_opecf()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        lcp_face = FACING_LEFT;
        lcp_st = STATE_REACH_INTO_CABINET;
        od_draw(OBJ_FRIDGE_CLOSED, FRIDGE_X, FRIDGE_Y);
        gameTick(1);
        od_draw(OBJ_FRIDGE_OPEN_1, FRIDGE_X, FRIDGE_Y);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);
        od_draw(OBJ_FRIDGE_OPEN_2, FRIDGE_X, FRIDGE_Y);
        gameTick(1);

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(2);

        lcp_face = FACING_LEFT;
        lcp_st = STATE_REACH_INTO_CABINET;
        gameTick(3);

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(8);

        od_draw(OBJ_FRIDGE_OPEN_1, FRIDGE_X, FRIDGE_Y);
        gameTick(1);
        od_draw(OBJ_FRIDGE_CLOSED, FRIDGE_X, FRIDGE_Y);
        sf_sele(SFX_DOOR_OPEN, 6L);   /* OPEN, not CLOSE: as in the original */
        gameTick(1);
}
