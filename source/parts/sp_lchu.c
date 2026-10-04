/*
 * parts/sp_lchu.c -- included by stx_u3.c; never compiled on its own.
 */
/* sp_lchu: pick head frame from PEx.LCP by happiness + g_hsfra,
   expand via sp_lcpf into slot 4.  Tracks body position; head lowers
   1 px while carrying on stair states 13..16. */

void
sp_lchu()
{
        short   headIndex;

        while (g_sepef[HW_SLOT_LCP_HEAD] == YES)
                ;

        headIndex = (g_hsfra & 0x7f) +
                    mood_hfo[lcp.happiness];

        /* Same 168-src/84-dest stride as sp_updb. */
        sp_lcpf((short *) pex_ptr[headIndex],
                (short *) hd_shp[headIndex],
                g_hsbuf, g_hsmas,
                2, 21, g_hsmif, 0);

        if (g_hsmif == NO)
                g_seacx[HW_SLOT_LCP_HEAD] = lcp_x + hd_xoff[lcp_st] - 4;
        else
                g_seacx[HW_SLOT_LCP_HEAD] = lcp_x + hd_xoff[lcp_st] - 14;

        g_seacy[HW_SLOT_LCP_HEAD] = (lcp_y + body_yof[lcp_st]) -
                             (hd_hgt[lcp_st] + 21);
        if (dbg_hide != NO)
                g_seacy[HW_SLOT_LCP_HEAD] = 300;

        /* The stair range is spelled inclusively and the y stepped in
           place; both shapes are the original's. */
        if (g_lcyof != NO &&
            lcp_st >= STATE_STR_TOP_F0 && lcp_st <= STATE_STR_TOP_F3S)
                g_seacy[HW_SLOT_LCP_HEAD]++;

        g_sepeh[HW_SLOT_LCP_HEAD] = 21;
        g_sepew[HW_SLOT_LCP_HEAD]  = 32;
        g_sepim[HW_SLOT_LCP_HEAD]  = g_hsbuf;
        g_sepms[HW_SLOT_LCP_HEAD]   = g_hsmas;

        if (g_lssh != NO)
                g_sepim[HW_SLOT_LCP_HEAD] = NULL;

        g_sepef[HW_SLOT_LCP_HEAD] = YES;
}
