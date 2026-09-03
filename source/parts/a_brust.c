/*
 * parts/a_brust.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from abathrm.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in abathrm.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_brust()
{
#ifdef FAITHFUL
        unsigned short  brush_cycles;
#else
        short           brush_cycles;   /* signed in STX: no clr.w */
#endif
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short           result;
#endif
        short           x_left;
        short           x_right;

        brush_cycles = (unsigned short) rndRng(24, 35);
        hs_posXY(POS_MID_BATHROOM_SINK,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        g_hamod = HEAD_ANIM_DISABLED;
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_BRUSH_TEETH;
        g_hatas = 10;
#ifdef FAITHFUL
        lcp_y = lcp_y - 2;
#else
        lcp_y -= 2;
#endif
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
#ifdef FAITHFUL
        while (brush_cycles != 0) {
                if (((brush_cycles - 1) & 1) == 0)
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_right;
                else
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
                gameTick(0);
                brush_cycles = brush_cycles - 1;
        }
#else
        while (brush_cycles--) {
                if (brush_cycles & 1)
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_left;
                else
                        g_sepex[g_seslm[SPRITE_STUDY_DOOR_FRAME]] = x_right;
                gameTick(0);
        }
#endif

        g_selaf[SPRITE_STUDY_DOOR_FRAME] = SPRITE_HIDDEN;
        sp_upds();
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
#ifdef FAITHFUL
        lcp_y = lcp_y + 2;
#else
        lcp_y += 2;
#endif
        gameTick(0);
}
