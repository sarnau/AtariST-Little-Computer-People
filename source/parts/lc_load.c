/*
 * parts/lc_load.c -- shared body; LCP_STX links it in the 0x400c
 * object, ahead of gameLoop (0x5ac8). Files under parts/ are never
 * compiled standalone.
 */
short
lc_load()
{
        /* STX: link #-8 -- the result goes through a second local, the
           open mode is a word, and the whole body hangs off the open
           test. */
        short   fhnd;
        short   ok;

        ok = 0;
        if ((fhnd = Fopen("hyber", 0)) >= 0) {
                ok = 1;

                fr_read(fhnd, 0x80L, &lcp);
                Fclose(fhnd);

                lcp_watr         = lcp.water_level;
                lcp_frdO     = lcp.door_states_and_flags & DSF_FRONT_DOOR;
                lcp_drsO        = (lcp.door_states_and_flags & DSF_DRESSER)          >> 4;
                lcp_cabO        = (lcp.door_states_and_flags & DSF_KITCHEN_CABINET)  >> 3;
                lcp_clsO    = (lcp.door_states_and_flags & DSF_CLOSET_DOOR)      >> 2;
                studyDrO     = (lcp.door_states_and_flags & DSF_STUDY_DOOR)       >> 1;
                lcp_toiO    = (lcp.door_states_and_flags & DSF_TOILET_DOOR)      >> 5;
                lcp_flcO = (lcp.door_states_and_flags & DSF_FILING_CABINET)   >> 6;
                lcp_bwlS     = (lcp.door_states_and_flags & DSF_DOG_BOWL_MASK)    >> 7;
                lcp_food          = lcp.food_supply;
                lcp_recP      = lcp.record_playing;
                lcp_tv               = lcp.tv_on;

                lcp_upal();
        }
        return ok;
}
