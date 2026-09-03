/*
 * parts/a_brust.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * abathrm functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_brust()
{
        short           brush_cycles;   /* signed in STX: no clr.w */
        /* STX tests the call in place -- no local. */
        short           x_left;
        short           x_right;

        brush_cycles = (unsigned short) rndRng(24, 35);
        hs_posXY(POS_MID_BATHROOM_SINK,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        g_hamod = HEAD_ANIM_DISABLED;
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_BRUSH_TEETH;
        g_hatas = 10;
        lcp_y -= 2;
        lcp_hwt();

        g_selaf[SPRITE_STUDY_DOOR_FRAME] = SPRITE_BEHIND_LCP;
        sp_sprs(SPRITE_STUDY_DOOR_FRAME);
        x_left  = lcp_x + 8;
        x_right = lcp_x + 12;
        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
        g_sepey[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = lcp_y - 24;

        /* STX drives the loop from a post-decrement, so the body
           sees the already-decremented value and tests it directly
           (btst #0) instead of subtracting first. */
        while (brush_cycles--) {
                if (brush_cycles & 1)
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
                else
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_right;
                gameTick(0);
        }

        g_selaf[SPRITE_STUDY_DOOR_FRAME] = SPRITE_HIDDEN;
        sp_upds();
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        lcp_y += 2;
        gameTick(0);
}
