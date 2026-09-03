/*
 * parts/sp_updb.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x148fe object, after gameTick). Files under parts/ are
 * never compiled standalone.
 */
/* sp_updb: select body pose for lcp_st -> slot 3.  When carrying an
   object during walk states (< 25), uses arms-up frames from cy_frT.
   X = lcp_x - 4 (right) or lcp_x - 14 (left); Y = lcp_y + body_yof[st] - 21.
   addr: sp_updb() */

void
sp_updb()
{
        short   frame;

        while (g_sepef[HW_SLOT_LCP_BODY] == YES)
                ;

        frame = body_frT[lcp_st];
        /* The bound is spelled inclusively on the previous state
           (cmpi #24/bgt), not < 25. */
        if (g_lcyof != NO && lcp_st <= STATE_STR_BTM_F3)
                frame = cy_frT[lcp_st];

        /* Ghidra 0x2669a `muls.w #0x54, D0`: stride is 168 src, 84 dest. */
        /* STX multiplies in word width (muls.w) -- no (long) casts,
           so no call to the long-multiply helper. */
        sp_lcpf((short *) body_ptr[frame],
                (short *) body_shp[frame],
                (short *) g_lsimg,
                (short *) g_lsmas,
                2, 21, lcp_face, 1);

        if (lcp_face == FACING_RIGHT)
                g_seacx[HW_SLOT_LCP_BODY] = lcp_x - 4;
        else
                g_seacx[HW_SLOT_LCP_BODY] = lcp_x - 14;

        g_seacy[HW_SLOT_LCP_BODY] = lcp_y + body_yof[lcp_st] - 21;
        if (dbg_hide != NO)
                g_seacy[HW_SLOT_LCP_BODY] = 300;

        g_sepeh[HW_SLOT_LCP_BODY] = 21;
        g_sepew[HW_SLOT_LCP_BODY]  = 32;
        g_sepim[HW_SLOT_LCP_BODY]  = g_lsimg;
        g_sepms[HW_SLOT_LCP_BODY]   = g_lsmas;

        if (g_lssh != NO)
                g_sepim[HW_SLOT_LCP_BODY] = NULL;

        g_sepef[HW_SLOT_LCP_BODY] = YES;
}
