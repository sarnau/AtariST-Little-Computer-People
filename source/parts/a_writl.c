/*
 * parts/a_writl.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aletter.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aletter.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_writl()
{
        /* Nine scalars and the section array LAST: swap_a doubles as
           the paragraph count and line_spacing as the '-' test, and
           every call result is consumed in place. */
        short   section_id;             /* -2  */
        short   i;                      /* -4  */
        short   swap_a;                 /* -6  */
        short   swap_b;                 /* -8  */
        short   swap_temp;              /* -10 */
        short   template_index;         /* -12 */
        short   cursor_y;               /* -14 */
        short   line_spacing;           /* -16 */
        short   full_year;              /* -18 */
        short   section_order[4];       /* -26 */

        if (lcp_recP != NO)
                a_playp();

        hs_posXY(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        if (lcp_wkD())
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        a_watat();
        if (rndRng(0, 100) > lcp.initiative_threshold)
                a_opcfc();

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        if (lcp_wkD())
                return;

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_wtx -= 10;
        g_wty += 3;
        if (lcp_wkD())
                return;

        g_actif = YES;

        g_selaf[SPRITE_TYPEWRITER] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPEWRITER);
        g_sepex[g_seslm[SPRITE_TYPEWRITER]] = 201;
        g_sepey[g_seslm[SPRITE_TYPEWRITER]] =  51;
        g_selaf[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPING_2);
        g_sepex[g_seslm[SPRITE_TYPING_2]] = 211;
        g_sepey[g_seslm[SPRITE_TYPING_2]] =  44;

        hs_posXY(POS_TOP_DESK_CHAIR,
                              &g_wtx, &g_wty);
        g_wty -= 4;
        g_wtx -= 14;
        lcp_wkD();

        lcp_st              = STATE_STAND_SIDE_VIEW;
        lcp_face   = FACING_RIGHT;
        g_hatas = 8;
        lcp_hwt();

        lcp_x += 5;
        lcp_y += 6;
        lcp_st = STATE_WRITE_AT_DESK;
        gameTick(1);

        g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_TYPING_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPING_1);
        g_sepex[g_seslm[SPRITE_TYPING_1]] = 211;
        g_sepey[g_seslm[SPRITE_TYPING_1]] =  44;

        g_hamod         = HEAD_ANIM_READING;
        no_keyin = YES;
        fillTopR(0x1b);

        g_lttx = (char *) Malloc(0x2900L);
        if (g_lttx == (char *) 0)
                er_nomem();
        fl_ltpl();

        tx_sctm = 9999;
        gameTick(2);

        full_year = dt_year + 1900;
        sprintf(in_str, "%s %d, %4d",
                mo_names[dt_mon],
                date_day + 1, full_year);
        g_cdibp = 0;
        lt_tysa(in_str, -12);
        lt_tyca('\r');

        sprintf(in_str, "Dear %s,", lcp.owner_name);
        lt_tysa(in_str, 0);
        lt_tyca('\r');

        /* Shuffle the 4 section indices via 16 random swaps. */
        for (i = 0; i < 4; i++)
                section_order[i] = i;
        for (i = 0; i < 16; i++) {
                swap_a = rndRng(0, 3);
                swap_b = rndRng(0, 3);
                swap_temp = section_order[swap_a];
                section_order[swap_a] = section_order[swap_b];
                section_order[swap_b] = swap_temp;
        }

        /* Body: 2..4 paragraphs from the shuffled sections. */
        swap_a = rndRng(2, 4);
        for (i = 0; i < swap_a; i++) {
                section_id     = section_order[i];
                template_index = section_id * 0x60;
                if (section_id == 3)
                        template_index += rndRng(0, 5) * 0xc;
                else if (lcp.sickness_level > 0)
                        template_index += rndRng(0, 1) * 0x30 + 0x24;
                else
                        template_index += rndRng(0, 1) * 0x30 +
                                          lcp.happiness * 0xc;

                /* Opening line -- indent 5 spaces on the first
                   paragraph only; the whole call is duplicated. */
                if (i == 0)
                        cursor_y = lt_tysa(
                                g_ltlp[rndRng(0, 3) + template_index],
                                -5);
                else
                        cursor_y = lt_tysa(
                                g_ltlp[rndRng(0, 3) + template_index],
                                2);

                /* Middle line */
                if (cursor_y == '-')
                        line_spacing = 0;
                else
                        line_spacing = 1;
                cursor_y = lt_tysa(
                        g_ltlp[rndRng(0, 3) + template_index + 4],
                        line_spacing);

                /* Ending line */
                if (cursor_y == '-')
                        line_spacing = 0;
                else
                        line_spacing = 1;
                lt_tysa(
                        g_ltlp[rndRng(0, 3) + template_index + 8],
                        line_spacing);
        }

        /* Sign-off. */
        lt_tyca('\r');
        lt_tysa(g_ltg[rndRng(0, 3)], -8);
        lt_tyca('\r');

        sprintf(in_str, "%s", lcp.character_name);
        lt_tysa(in_str, -10);
        gameTick(60);

        /* Cleanup: free buffer, hide typing sprites, walk out. */
        tx_sctm        = 0;
        g_cdibp = 0;
        no_keyin   = NO;
        Mfree(g_lttx);

        g_selaf[SPRITE_TYPING_1] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_2] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_3] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_4] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_TYPING_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TYPING_2);
        g_sepex[g_seslm[SPRITE_TYPING_2]] = 211;
        g_sepey[g_seslm[SPRITE_TYPING_2]] =  44;
        gameTick(4);

        lcp_st      = STATE_STAND_SIDE_VIEW;
        g_hamod = HEAD_ANIM_DISABLED;
        lcp_y -= 6;
        gameTick(0);
        g_actif = YES;

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_wtx -= 10;
        g_wty += 3;
        lcp_wkD();
        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_TYPEWRITER] = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_1]   = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_2]   = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_3]   = SPRITE_HIDDEN;
        g_selaf[SPRITE_TYPING_4]   = SPRITE_HIDDEN;
        sp_upds();
        g_actif = NO;
}
