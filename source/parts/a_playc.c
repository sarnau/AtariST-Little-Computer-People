/*
 * parts/a_playc.c -- shared body; LCP_ORG links it in agames.o,
 * LCP_STX in the 0xdece object at 0x12e86, immediately before
 * tv_scrc.  Files under parts/ are never compiled standalone.
 */

#ifdef FAITHFUL
/* a_playc: sit-and-type.  pst_arr[0..1] typing poses; pst_arr[2] rest.
   addr: a_playc() */

void
a_playc()
{
        short           walk_result;
        unsigned short  random_seed;
        unsigned short  random_duration;
        unsigned short  random_anim;
        unsigned short  type_counter;
        short           is_even_frame;

        pst_arr[0] = STATE_HANDS_DOWN;
        pst_arr[1] = STATE_HANDS_UP;
        pst_arr[2] = STATE_SITTING_AT_DESK;

        hs_posXY(POS_MID_COMPUTER_DESK,
                              &g_wtx, &g_wty);
        walk_result = lcp_wkD();
        if (walk_result != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        g_hamod = HEAD_ANIM_COMPUTER;

        /* First Random() discarded (matches 1985 binary). */
        (void) Random();
        random_seed = (unsigned short) Random();

        lcp_st = pst_arr[2];
        gameTick(25);

        type_counter = 0;
        while ((short) type_counter <
               (short) ((random_seed & 0x1ff) | 0x80) &&
               introSeq == NO &&
               g_trel[0] == ACTION_NONE) {
                random_duration = (unsigned short) Random();
                is_even_frame   = ((type_counter & 1) == 0);

                if (is_even_frame) {
                        lcp_face = FACING_RIGHT;
                        lcp_st = pst_arr[1];
                } else {
                        random_anim = (unsigned short) Random();
                        lcp_face = (random_anim & 2) >> 1;
                        lcp_st = pst_arr[0];
                        sfClick();
                }
                /* 1985 flips is_even_frame here (even/odd swap for tick). */
                is_even_frame = !is_even_frame;
                if (is_even_frame)
                        gameTick(0);
                else
                        gameTick(random_duration & 3);

                /* Rare "clear the screen" gesture. */
                random_duration = (unsigned short) Random();
                if ((random_duration & 0x7f) < 3 && is_even_frame) {
                        g_hamod         = HEAD_ANIM_DISABLED;
                        lcp_st              = pst_arr[2];
                        g_hatas = 10;
                        lcp_face   = FACING_RIGHT;
                        lcp_hwt();
                        tv_scrc();
                        gameTick(5);
                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                        lcp_hwt();
                        g_hamod = HEAD_ANIM_COMPUTER;
                }

                type_counter = type_counter + 1;
        }

        lcp_st            = STATE_STAND_FACING_SCREEN;
        lcp_face = FACING_RIGHT;
        g_hamod       = HEAD_ANIM_DISABLED;
        gameTick(5);
}
#else   /* STX: link #-14 -- random_val, limit, typed, type_counter,
           is_even_frame; Random() stays long throughout. */

void
a_playc()
{
        short   random_val;
        short   limit;
        short   typed;
        short   type_counter;
        short   is_even_frame;

        pst_arr[0] = STATE_HANDS_DOWN;
        pst_arr[1] = STATE_HANDS_UP;
        pst_arr[2] = STATE_SITTING_AT_DESK;

        hs_posXY(POS_MID_COMPUTER_DESK,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        g_hamod = HEAD_ANIM_COMPUTER;

        /* The first draw is computed and thrown away. */
        random_val = (Random() & 7) | 3;
        limit      = (Random() & 0x1ff) | 0x80;

        lcp_st = pst_arr[2];
        gameTick(25);

        type_counter = 0;
        while (type_counter < limit) {
                if (introSeq != NO)
                        break;
                if (g_trel[0] != ACTION_NONE)
                        break;
                random_val = Random() & 3;
                if (type_counter & 1)
                        is_even_frame = 0;
                else
                        is_even_frame = 1;

                if (is_even_frame == 0) {
                        lcp_face = (Random() & 2) >> 1;
                        typed = 1;
                        lcp_st = pst_arr[0];
                        sfClick();
                } else {
                        lcp_face = FACING_RIGHT;
                        typed = 0;
                        lcp_st = pst_arr[1];
                }

                if (typed == 1)
                        gameTick(0);
                else
                        gameTick(random_val);

                /* Rare "clear the screen" gesture. */
                if ((Random() & 0x7f) < 3 && typed != 0) {
                        g_hamod         = HEAD_ANIM_DISABLED;
                        lcp_st              = pst_arr[2];
                        g_hatas = 10;
                        lcp_face   = FACING_RIGHT;
                        lcp_hwt();
                        tv_scrc();
                        gameTick(5);
                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                        lcp_hwt();
                        g_hamod = HEAD_ANIM_COMPUTER;
                }

                type_counter++;
        }

        lcp_st            = STATE_STAND_FACING_SCREEN;
        lcp_face = FACING_RIGHT;
        g_hamod       = HEAD_ANIM_DISABLED;
        gameTick(5);
}
#endif
