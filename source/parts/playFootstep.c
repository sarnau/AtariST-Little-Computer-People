/* playFootstep: pick footstep SFX (carpet/wood/stairs) by floor + X.
   footstepDue is set by walkStep on foot-plant frames. */

void
playFootstep()
{
        short   floor;

        if (footstepDue == NO)
                return;

        if (onStairs != NO) {
                sfxSelect(SFX_FOOTSTEP_STAIRS, 2L);
                return;
        }

        floor = floorOfY(resY);
        switch (floor) {
        case FLOOR_BOTTOM:
                if (resX < 166)
                        sfxSelect(SFX_FOOTSTEP_CARPET, 2L);
                else
                        sfxSelect(SFX_FOOTSTEP_WOOD, 2L);
                break;
        case FLOOR_MIDDLE:
                if (resX > 146 && resX < 234)
                        sfxSelect(SFX_FOOTSTEP_CARPET, 2L);
                break;
        case FLOOR_TOP:
                if (resX > 136)
                        sfxSelect(SFX_FOOTSTEP_WOOD, 2L);
                break;
        }
}
