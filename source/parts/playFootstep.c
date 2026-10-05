/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* playFootstep: pick footstep SFX (carpet/wood/stairs) by floor + X.
   fs_trg is set by walkStep on foot-plant frames. */

void
playFootstep()
{
        short   floor;

        if (fs_trg == NO)
                return;

        if (lcp_stR != NO) {
                sfxSelect(SFX_FOOTSTEP_STAIRS, 2L);
                return;
        }

        floor = floorOfY(lcp_y);
        switch (floor) {
        case FLOOR_BOTTOM:
                if (lcp_x < 166)
                        sfxSelect(SFX_FOOTSTEP_CARPET, 2L);
                else
                        sfxSelect(SFX_FOOTSTEP_WOOD, 2L);
                break;
        case FLOOR_MIDDLE:
                if (lcp_x > 146 && lcp_x < 234)
                        sfxSelect(SFX_FOOTSTEP_CARPET, 2L);
                break;
        case FLOOR_TOP:
                if (lcp_x > 136)
                        sfxSelect(SFX_FOOTSTEP_WOOD, 2L);
                break;
        }
}
