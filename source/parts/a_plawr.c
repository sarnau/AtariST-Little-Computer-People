/*
 * parts/a_plawr.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * aleisure functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_plawr()
{
        /* STX: link #-30 -- the walk result is tested in place;
           psg_* at -2/-4/-6, prev_* at -8/-10/-12, i at -14 and
           dta_ptr at -22. */
        unsigned char   psg_a, psg_b, psg_c;
        unsigned char   prev_a, prev_b, prev_c;
        short           i;
        char *          filename;
        _DTA *           dta_ptr;
        long            xres;

        pst_arr[0] = STATE_VINYL_REACH_R;
        pst_arr[1] = STATE_VINYL_IDLE;
        pst_arr[2] = STATE_VINYL_REACH_L;
        pst_arr[3] = STATE_VINYL_PULL_OUT;

        prev_a = 0;
        prev_b = 0;
        prev_c = 0;
        g_actif = YES;
        if (lcp_recP != NO)
                a_playp();
        g_actif = NO;

        hs_posXY(POS_TOP_RECORD_SHELF,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        g_rbact = YES;
        g_hamod = HEAD_ANIM_DISABLED;
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        gameTick(4);

        lcp_st = STATE_VINYL_REACH_R;
        g_selaf[SPRITE_VINYL_RECORD] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_VINYL_RECORD);
        g_sepex[g_seslm[SPRITE_VINYL_RECORD]] = 146;
        g_sepey[g_seslm[SPRITE_VINYL_RECORD]] =  54;
        gameTick(1);

        i = rndRng(1, org_cnt);
        Fsfirst("*.org", 0);
        while (--i != 0)
                Fsnext();
        dta_ptr = (_DTA *) Fgetdta();
        filename = dta_ptr->d_fname;
        for (i = 0; filename[i] != '.'; i++)
                ;
        filename[i + 4] = '\0';
        sgPlay(filename);

        g_hamod = HEAD_ANIM_WALKING;
        while (mi_play == NO)
                ;

        while (mi_play != NO) {
                /* ROM: bare word-arg xbios shape here (no 0L pad),
                   unlike sf_so's long-arg Giaccess writes. */
                psg_a = Giaccess(0, 8) & 0x1f;
                psg_b = Giaccess(0, 9) & 0x1f;
                psg_c = Giaccess(0, 10) & 0x1f;

                lcp_st = pst_arr[0];
                if (psg_a > prev_a || psg_b > prev_b || psg_c > prev_c) {
                        i = rndRng(1, 3);
                        while (pst_arr[i] == lcp_st)
                                i = rndRng(1, 3);
                        lcp_st = pst_arr[i];
                        if (pst_arr[3] == lcp_st) {
                                gameTick(0);
                                lcp_st = pst_arr[rndRng(1, 2)];
                        }
                }
                prev_a = psg_a; prev_b = psg_b; prev_c = psg_c;
                gameTick(0);
        }

        g_hamod = HEAD_ANIM_DISABLED;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_st = pst_arr[0];
        lcp_hwt();
        gameTick(8);

        lcp_st = STATE_STAND_FACING_SCREEN;
        g_selaf[SPRITE_VINYL_RECORD] = SPRITE_HIDDEN;
        sp_upds();
        gameTick(0);

        if (mi_sbuf != (char *) 0) {
                Mfree(mi_sbuf);
                mi_sbuf = (char *) 0;
        }
        g_rbact = NO;
}
