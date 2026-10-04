/*
 * Sits ahead of gameLoop.
 *
 * Included by stx_u1.c; never compiled on its own.
 */
/* Restore a saved game: if the file "hyber" opens, its 128-byte image
   is read straight into the lcp record and the water level, every
   door/cabinet open flag, the dog-bowl state, food supply, record
   player and TV are unpacked from it into their working globals, and
   the sickness tint is applied to the palette (lcp_upal).  Returns 1
   if a save was loaded, 0 if there is none; main keeps it in g_lcldd. */
short
lc_load()
{
        /* The result goes through a second local and the whole body
           hangs off the open test, as in the original. */
        short   fhnd;
        short   ok;

        ok = 0;
        if ((fhnd = Fopen("hyber", RMODE_RD)) >= 0) {
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
