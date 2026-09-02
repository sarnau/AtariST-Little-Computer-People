/*
 * parts/lcp_fstp.c -- shared body; LCP_ORG links it in walk.c,
 * LCP_STX in the 0xdece object (0x4fec, in the 0x400c object with getFlrY).  Files under parts/
 * are never compiled standalone.
 */
/* lcp_fstp: pick footstep SFX (carpet/wood/stairs) by floor + X.
   fs_trg is set by lcp_path on foot-plant frames.
   addr: lcp_fstp() */

void
lcp_fstp()
{
        short   floor;

        if (fs_trg == NO)
                return;

        if (lcp_stR != NO) {
                sf_sele(SFX_FOOTSTEP_STAIRS, 2L);
                return;
        }

        floor = getFlrY(lcp_y);
#ifdef FAITHFUL
        if (floor == 1) {
                if (lcp_x < 166)
                        sf_sele(SFX_FOOTSTEP_CARPET, 2L);
                else
                        sf_sele(SFX_FOOTSTEP_WOOD, 2L);
        } else if (floor == 2) {
                if (lcp_x > 146 && lcp_x < 234)
                        sf_sele(SFX_FOOTSTEP_CARPET, 2L);
        } else if (floor == 3 && lcp_x > 136) {
                sf_sele(SFX_FOOTSTEP_WOOD, 2L);
        }
#else
        switch (floor) {
        case 1:
                if (lcp_x < 166)
                        sf_sele(SFX_FOOTSTEP_CARPET, 2L);
                else
                        sf_sele(SFX_FOOTSTEP_WOOD, 2L);
                break;
        case 2:
                if (lcp_x > 146 && lcp_x < 234)
                        sf_sele(SFX_FOOTSTEP_CARPET, 2L);
                break;
        case 3:
                if (lcp_x > 136)
                        sf_sele(SFX_FOOTSTEP_WOOD, 2L);
                break;
        }
#endif
}
