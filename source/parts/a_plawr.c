/*
 * parts/a_plawr.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* a_plawr: play the organ (ACTION_PLAY_ORGAN).  Any record playing is
   stopped first (a_playp).  The resident walks to the organ on the top
   floor (POS_TOP_ORGAN), a prop sprite is shown on the instrument, and a
   random *.ORG file is picked and started with sgPlay.  While it plays he switches
   to a new random reaching pose whenever any PSG channel's volume
   rises, so he moves in time with the notes.  g_rbact is set
   throughout to keep rp_anim's record-player animation still, and the
   song buffer is freed at the end. */
void
a_plawr()
{
        /* The walk result is tested in place, with no local for it.
           The declaration order of these locals sets their stack
           slots and must not change; xres is unused but must stay. */
        unsigned char   psg_a, psg_b, psg_c;
        unsigned char   prev_a, prev_b, prev_c;
        short           i;
        char *          filename;
        _DTA *           dta_ptr;
        long            xres;

        pst_arr[0] = STATE_ORGAN_REACH_R;
        pst_arr[1] = STATE_ORGAN_IDLE;
        pst_arr[2] = STATE_ORGAN_REACH_L;
        pst_arr[3] = STATE_ORGAN_PULL_OUT;

        prev_a = 0;
        prev_b = 0;
        prev_c = 0;
        g_actif = YES;
        if (lcp_recP != NO)
                a_playp();
        g_actif = NO;

        hs_posXY(POS_TOP_ORGAN,
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

        lcp_st = STATE_ORGAN_REACH_R;
        g_selaf[SPRITE_ORGAN_PROP] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_ORGAN_PROP);
        g_sepex[g_seslm[SPRITE_ORGAN_PROP]] = 146;
        g_sepey[g_seslm[SPRITE_ORGAN_PROP]] =  54;
        gameTick(1);

        i = rndRng(1, org_cnt);
        Fsfirst("*.org", F_NORMAL);
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
                /* Plain word arguments here (no 0L), unlike sf_so's
                   Giaccess writes: the argument shape changes the code. */
                psg_a = Giaccess(0, PSG_VOL_A) & 0x1f;
                psg_b = Giaccess(0, PSG_VOL_B) & 0x1f;
                psg_c = Giaccess(0, PSG_VOL_C) & 0x1f;

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
        g_selaf[SPRITE_ORGAN_PROP] = SPRITE_HIDDEN;
        sp_upds();
        gameTick(0);

        if (mi_sbuf != (char *) 0) {
                Mfree(mi_sbuf);
                mi_sbuf = (char *) 0;
        }
        g_rbact = NO;
}
