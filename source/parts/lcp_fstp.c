/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* lcp_fstp: pick footstep SFX (carpet/wood/stairs) by floor + X.
   fs_trg is set by lcp_path on foot-plant frames. */

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
        switch (floor) {
        case FLOOR_BOTTOM:
                if (lcp_x < 166)
                        sf_sele(SFX_FOOTSTEP_CARPET, 2L);
                else
                        sf_sele(SFX_FOOTSTEP_WOOD, 2L);
                break;
        case FLOOR_MIDDLE:
                if (lcp_x > 146 && lcp_x < 234)
                        sf_sele(SFX_FOOTSTEP_CARPET, 2L);
                break;
        case FLOOR_TOP:
                if (lcp_x > 136)
                        sf_sele(SFX_FOOTSTEP_WOOD, 2L);
                break;
        }
}
