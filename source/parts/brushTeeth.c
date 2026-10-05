/*
 * parts/brushTeeth.c -- the resident brushes his teeth at the bathroom sink.
 * Included by stx_u2.c; never compiled on its own.
 */

void
brushTeeth()
{
        short           brush_cycles;   /* signed: unsigned compiles differently */
        /* walkToTarget()'s result is tested in place, not kept in a local. */
        short           x_left;
        short           x_right;

        brush_cycles = (unsigned short) rndRng(24, 35);
        posToXY(POS_MID_BATHROOM_SINK,
                              &g_wtx, &g_wty);
        if (walkToTarget() != 0)
                return;

        g_hamod = HEAD_ANIM_DISABLED;
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_BRUSH_TEETH;
        g_hatas = 10;
        lcp_y -= 2;
        waitHeadTurn();

        g_selaf[SPRITE_STUDY_DOOR_FRAME] = SPRITE_BEHIND_LCP;
        activateSprite(SPRITE_STUDY_DOOR_FRAME);
        x_left  = lcp_x + 8;
        x_right = lcp_x + 12;
        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
        g_sepey[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = lcp_y - 24;

        /* The loop is driven by a post-decrement, so the body sees the
           already-decremented value.  Keep this shape: it is the
           original's. */
        while (brush_cycles--) {
                if (brush_cycles & 1)
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
                else
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_right;
                gameTick(0);
        }

        g_selaf[SPRITE_STUDY_DOOR_FRAME] = SPRITE_HIDDEN;
        layoutSlots();
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        lcp_y += 2;
        gameTick(0);
}
