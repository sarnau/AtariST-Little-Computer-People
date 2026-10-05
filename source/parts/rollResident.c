/* Roll a new resident: appearance, schedule, personality and needs,
   plus a random name. */

void
rollResident()
{
        /* Two locals; the first is both the record offset and the
           loop index.  The name is a random record of the 266-entry,
           10-byte NAMES file; padding bytes below 'A' are cleared. */
        short   tmp;
        short   fhnd;

        resident.characterSpriteId = rndRng(2, 6);

        tmp  = rndRng(0, 265) * 10;
        fhnd = openFile("names", RMODE_RD);
        Fseek((long) tmp, fhnd, 0);
        readFile(fhnd, 10L, resident.characterName);
        Fclose(fhnd);
        for (tmp = 0; tmp < 10; tmp++)
                if (resident.characterName[tmp] < 'A')
                        resident.characterName[tmp] = 0;

        resident.waterLevel = WATER_START;
        waterLevel = resident.waterLevel;
        resident.clothingColor = rndRng(0, 15);
        resident.skinColor = rndRng(0, 7);
        resident.bedtimeHour = rndRng(22, 24);
        if (resident.bedtimeHour >= 24)
                resident.bedtimeHour -= 24;
        resident.wakeHour = resident.bedtimeHour + 6;
        if (resident.wakeHour >= 24)
                resident.wakeHour -= 24;
        resident.lunchHour = rndRng(11, 13);
        resident.dinnerHour = rndRng(17, 19);
        resident.personalityType = rndRng(0, 3);
        resident.activityLevel = rndRng(0, 7);
        resident.happiness = MOOD_CONTENT;
        resident.moodDuration[MOOD_HAPPY] = rndRng(6, 24);
        resident.moodDuration[MOOD_CONTENT] = rndRng(6, 24);
        resident.moodDuration[MOOD_SAD] = rndRng(6, 12);
        resident.happinessDurationActive = resident.moodDuration[MOOD_CONTENT];
        resident.happinessDirection = DIR_IMPROVING;
        resident.sicknessLevel = SICKNESS_HEALTHY;
        resident.sicknessCountdown = 0;
        resident.sicknessDirection = DIR_STABLE;
        resident.isSleeping = NO;
        resident.initiativeThreshold = rndRng(20, 80);
        resident.thirstLevel = NEED_SATISFIED;
        resident.thirstTimerMax = rndRng(45, 75);
        resident.thirstTimer = resident.thirstTimerMax;
        resident.hungerLevel = NEED_SATISFIED;
        resident.hungerTimerMax = rndRng(75, 120);
        resident.hungerTimer = resident.hungerTimerMax;
        resident.bathroomNeed = NO;
        resident.bathroomTimerMax = rndRng(20, 40);
        resident.bathroomTimer = resident.bathroomTimerMax;
        /* The shadow globals are copied FROM the struct fields, not
           assigned the same literal. */
        resident.recordPlaying = NO;
        recordPlaying = resident.recordPlaying;
        resident.tvOn = NO;
        tvRunning = resident.tvOn;
        resident.foodSupply = 4;
        foodSupply = resident.foodSupply;
        resident.doorStatesAndFlags = FOOD_PACKS_MAX << DSF_FOOD_SHIFT;
}
