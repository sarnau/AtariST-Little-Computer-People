/* Restore a saved game: if the file "hyber" opens, its 128-byte image
   is read straight into the resident record and the water level, every
   door/cabinet open flag, the dog-bowl state, food supply, record
   player and TV are unpacked from it into their working globals, and
   the sickness tint is applied to the palette (setSkinColor).  Returns 1
   if a save was loaded, 0 if there is none; main keeps it in loadedSave. */
short
loadSavedGame()
{
        /* The result goes through a second local and the whole body
           hangs off the open test, as in the original. */
        short   fhnd;
        short   ok;

        ok = 0;
        if ((fhnd = Fopen("hyber", RMODE_RD)) >= 0) {
                ok = 1;

                readFile(fhnd, 0x80L, &resident);
                Fclose(fhnd);

                waterLevel = resident.waterLevel;
                frontDoorOpen = resident.doorStatesAndFlags & DSF_FRONT_DOOR;
                dresserOpen = (resident.doorStatesAndFlags & DSF_DRESSER) >> 4;
                kitchenCabOpen = (resident.doorStatesAndFlags & DSF_KITCHEN_CABINET) >> 3;
                bedClosetOpen = (resident.doorStatesAndFlags & DSF_CLOSET_DOOR) >> 2;
                studyDoorOpen = (resident.doorStatesAndFlags & DSF_STUDY_DOOR) >> 1;
                toiletDoorOpen = (resident.doorStatesAndFlags & DSF_TOILET_DOOR) >> 5;
                filingCabOpen = (resident.doorStatesAndFlags & DSF_FILING_CABINET) >> 6;
                bowlLevel = (resident.doorStatesAndFlags & DSF_DOG_BOWL_MASK) >> 7;
                foodSupply = resident.foodSupply;
                recordPlaying = resident.recordPlaying;
                tvRunning = resident.tvOn;

                setSkinColor();
        }
        return ok;
}
