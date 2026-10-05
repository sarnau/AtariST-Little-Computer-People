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

        resident.character_sprite_id       = rndRng(2, 6);

        tmp  = rndRng(0, 265) * 10;
        fhnd = openFile("names", RMODE_RD);
        Fseek((long) tmp, fhnd, 0);
        readFile(fhnd, 10L, resident.character_name);
        Fclose(fhnd);
        for (tmp = 0; tmp < 10; tmp++)
                if (resident.character_name[tmp] < 'A')
                        resident.character_name[tmp] = 0;

        resident.water_level               = WATER_START;
        waterLevel               = resident.water_level;
        resident.clothing_color            = rndRng(0, 15);
        resident.skin_color                = rndRng(0, 7);
        resident.bedtime_hour              = rndRng(22, 24);
        if (resident.bedtime_hour >= 24)
                resident.bedtime_hour -= 24;
        resident.wake_hour                 = resident.bedtime_hour + 6;
        if (resident.wake_hour >= 24)
                resident.wake_hour -= 24;
        resident.lunch_hour                = rndRng(11, 13);
        resident.dinner_hour               = rndRng(17, 19);
        resident.personality_type          = rndRng(0, 3);
        resident.activity_level            = rndRng(0, 7);
        resident.happiness                 = MOOD_CONTENT;
        resident.mood_duration[MOOD_HAPPY]   = rndRng(6, 24);
        resident.mood_duration[MOOD_CONTENT] = rndRng(6, 24);
        resident.mood_duration[MOOD_SAD]     = rndRng(6, 12);
        resident.happiness_duration_active   = resident.mood_duration[MOOD_CONTENT];
        resident.happiness_direction       = DIR_IMPROVING;
        resident.sickness_level            = SICKNESS_HEALTHY;
        resident.sickness_countdown        = 0;
        resident.sickness_direction        = DIR_STABLE;
        resident.is_sleeping               = NO;
        resident.initiative_threshold      = rndRng(20, 80);
        resident.thirst_level              = NEED_SATISFIED;
        resident.thirst_timer_max          = rndRng(45, 75);
        resident.thirst_timer              = resident.thirst_timer_max;
        resident.hunger_level              = NEED_SATISFIED;
        resident.hunger_timer_max          = rndRng(75, 120);
        resident.hunger_timer              = resident.hunger_timer_max;
        resident.bathroom_need             = NO;
        resident.bathroom_timer_max        = rndRng(20, 40);
        resident.bathroom_timer            = resident.bathroom_timer_max;
        /* The shadow globals are copied FROM the struct fields, not
           assigned the same literal. */
        resident.record_playing            = NO;
        recordPlaying            = resident.record_playing;
        resident.tv_on                     = NO;
        tvRunning                     = resident.tv_on;
        resident.food_supply               = 4;
        foodSupply                = resident.food_supply;
        resident.door_states_and_flags     = FOOD_PACKS_MAX << DSF_FOOD_SHIFT;
}
