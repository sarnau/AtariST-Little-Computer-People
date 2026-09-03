/*
 * parts/a_plawr.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aleisure.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aleisure.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_plawr()
{
#ifdef FAITHFUL
        short           result;
        _DTA *           dta_ptr;
        char *          filename;
        long            xres;
        unsigned char   psg_a, psg_b, psg_c;
        unsigned char   prev_a, prev_b, prev_c;
        short           i;
#else
        /* STX: link #-30 -- the walk result is tested in place;
           psg_* at -2/-4/-6, prev_* at -8/-10/-12, i at -14 and
           dta_ptr at -22. */
        unsigned char   psg_a, psg_b, psg_c;
        unsigned char   prev_a, prev_b, prev_c;
        short           i;
        char *          filename;
        _DTA *           dta_ptr;
        long            xres;
#endif

        pst_arr[0] = STATE_VINYL_REACH_R;
        pst_arr[1] = STATE_VINYL_IDLE;
#ifdef FAITHFUL
        pst_arr[2] = STATE_VINYL_REACH_R;  /* ROM: 40 again, not REACH_L */
#else
        pst_arr[2] = STATE_VINYL_REACH_L;
#endif
        pst_arr[3] = STATE_VINYL_PULL_OUT;

#ifdef FAITHFUL
        prev_a = prev_b = prev_c = 0;
#else
        prev_a = 0;
        prev_b = 0;
        prev_c = 0;
#endif
        g_actif = YES;
        if (lcp_recP != NO)
                a_playp();
        g_actif = NO;

        hs_posXY(POS_TOP_RECORD_SHELF,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
#else
        if (lcp_wkD() != 0)
#endif
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
#ifdef FAITHFUL
        Fsfirst("*.org", 0L);
#else
        Fsfirst("*.org", 0);
#endif
#ifdef FAITHFUL
        while ((i = i - 1) != 0)
#else
        while (--i != 0)
#endif
                Fsnext();
        dta_ptr = (_DTA *) Fgetdta();
        filename = dta_ptr->d_fname;
#ifdef FAITHFUL
        for (i = 0; filename[i] != '.'; i = i + 1)
#else
        for (i = 0; filename[i] != '.'; i++)
#endif
                ;
        filename[i + 4] = '\0';
        sgPlay(filename);

        g_hamod = HEAD_ANIM_WALKING;
        while (mi_play == NO)
                ;

        while (mi_play != NO) {
                /* ROM: bare word-arg xbios shape here (no 0L pad),
                   unlike sf_so's long-arg Giaccess writes. */
#ifdef FAITHFUL
                xres = xbios(0x1C, 0, 8);  psg_a = (unsigned char) xres & 0x1f;
                xres = xbios(0x1C, 0, 9);  psg_b = (unsigned char) xres & 0x1f;
                xres = xbios(0x1C, 0, 10); psg_c = (unsigned char) xres & 0x1f;
#else
                psg_a = Giaccess(0, 8) & 0x1f;
                psg_b = Giaccess(0, 9) & 0x1f;
                psg_c = Giaccess(0, 10) & 0x1f;
#endif

                lcp_st = pst_arr[0];
#ifdef FAITHFUL
                if (prev_a < psg_a || prev_b < psg_b || prev_c < psg_c) {
#else
                if (psg_a > prev_a || psg_b > prev_b || psg_c > prev_c) {
#endif
                        i = rndRng(1, 3);
                        while (pst_arr[i] == lcp_st)
                                i = rndRng(1, 3);
                        lcp_st = pst_arr[i];
                        if (pst_arr[3] == lcp_st) {
                                gameTick(0);
#ifdef FAITHFUL
                                result = rndRng(1, 2);
                                lcp_st = pst_arr[result];
#else
                                lcp_st = pst_arr[rndRng(1, 2)];
#endif
                        }
                }
#ifdef FAITHFUL
                gameTick(0);
                prev_c = psg_c; prev_b = psg_b; prev_a = psg_a;
#else
                prev_a = psg_a; prev_b = psg_b; prev_c = psg_c;
                gameTick(0);
#endif
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
