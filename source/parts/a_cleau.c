/*
 * parts/a_cleau.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aleisure.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aleisure.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_cleau()
{
        /* STX consumes every call result in place -- no local. */
#ifdef FAITHFUL
        short   result;
#endif

        if (lcp_flcO != NO) {
                hs_posXY(POS_TOP_FILING_CABINET,
                                      &g_wtx, &g_wty);
#ifdef FAITHFUL
                if ((result = lcp_wkD()) != 0)
#else
                if (lcp_wkD() != 0)
#endif
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfc();
        }
        if (studyDrO != NO) {
                hs_posXY(POS_TOP_STUDY_DOOR,
                                      &g_wtx, &g_wty);
#ifdef FAITHFUL
                if ((result = lcp_wkD()) != 0)
#else
                if (lcp_wkD() != 0)
#endif
                        return;
                lcp_face = FACING_RIGHT;
                lcp_st            = STATE_STAND_FACING_SCREEN;
                /* The original really does compute g_hatas - 12 here
                   and throw it away (STX 0xe9a0 = the Ghidra 0x1e9a0
                   this comment already noted).  It is an expression
                   statement in the source, not a compiler artifact,
                   so the STX build emits it. */
#ifndef FAITHFUL
                g_hatas - 12;
#endif
                lcp_hwt();
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(od_sto1, 178, 23);
                gameTick(2);
                od_draw(od_stcl,  178, 23);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                studyDrO = NO;
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(0);
        }
        if (lcp_toiO != NO) {
                hs_posXY(POS_MID_TOILET_DOOR,
                                      &g_wtx, &g_wty);
#ifdef FAITHFUL
                if ((result = lcp_wkD()) != 0)
#else
                if (lcp_wkD() != 0)
#endif
                        return;
                a_clotd();
        }
        if (lcp_clsO != NO) {
                hs_posXY(POS_MID_BEDROOM_CLOSET,
                                      &g_wtx, &g_wty);
#ifdef FAITHFUL
                if ((result = lcp_wkD()) != 0)
#else
                if (lcp_wkD() != 0)
#endif
                        return;
                a_clocd();
        }
        if (lcp_drsO != NO) {
                hs_posXY(POS_MID_DRESSER,
                                      &g_wtx, &g_wty);
#ifdef FAITHFUL
                if ((result = lcp_wkD()) != 0)
#else
                if (lcp_wkD() != 0)
#endif
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opecd(1);
        }
        if (lcp_cabO != NO) {
                hs_posXY(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
#ifdef FAITHFUL
                if ((result = lcp_wkD()) != 0)
#else
                if (lcp_wkD() != 0)
#endif
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opecc(1);
        }
        if (lcp_frdO != NO) {
                wkFrDr();
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfd(1);
        }
}
