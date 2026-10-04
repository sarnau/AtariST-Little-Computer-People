/*
 * parts/lcp_crnd.c -- included by stx_u1.c, after rnd; never compiled
 * on its own.
 */
/* Roll a new resident: appearance, schedule, personality and needs,
   plus a random name. */

void
lcp_crnd()
{
        /* Two locals; the first is both the record offset and the
           loop index.  The name is a random record of the 266-entry,
           10-byte NAMES file; padding bytes below 'A' are cleared. */
        short   tmp;
        short   fhnd;

        lcp.character_sprite_id       = rndRng(2, 6);

        tmp  = rndRng(0, 265) * 10;
        fhnd = fOpen("names", RMODE_RD);
        Fseek((long) tmp, fhnd, 0);
        fr_read(fhnd, 10L, lcp.character_name);
        Fclose(fhnd);
        for (tmp = 0; tmp < 10; tmp++)
                if (lcp.character_name[tmp] < 'A')
                        lcp.character_name[tmp] = 0;

        lcp.water_level               = WATER_START;
        lcp_watr               = lcp.water_level;
        lcp.clothing_color            = rndRng(0, 15);
        lcp.skin_color                = rndRng(0, 7);
        lcp.bedtime_hour              = rndRng(22, 24);
        if (lcp.bedtime_hour >= 24)
                lcp.bedtime_hour -= 24;
        lcp.wake_hour                 = lcp.bedtime_hour + 6;
        if (lcp.wake_hour >= 24)
                lcp.wake_hour -= 24;
        lcp.lunch_hour                = rndRng(11, 13);
        lcp.dinner_hour               = rndRng(17, 19);
        lcp.personality_type          = rndRng(0, 3);
        lcp.activity_level            = rndRng(0, 7);
        lcp.happiness                 = MOOD_CONTENT;
        lcp.happiness_initial_countdown = rndRng(6, 24);
        lcp.happiness_duration_happy    = rndRng(6, 24);
        lcp.happiness_duration_content  = rndRng(6, 12);
        lcp.happiness_duration_active   = lcp.happiness_duration_happy;
        lcp.happiness_direction       = DIR_IMPROVING;
        lcp.sickness_level            = SICKNESS_HEALTHY;
        lcp.sickness_countdown        = 0;
        lcp.sickness_direction        = DIR_STABLE;
        lcp.is_sleeping               = NO;
        lcp.initiative_threshold      = rndRng(20, 80);
        lcp.thirst_level              = NEED_SATISFIED;
        lcp.thirst_timer_max          = rndRng(45, 75);
        lcp.thirst_timer              = lcp.thirst_timer_max;
        lcp.hunger_level              = NEED_SATISFIED;
        lcp.hunger_timer_max          = rndRng(75, 120);
        lcp.hunger_timer              = lcp.hunger_timer_max;
        lcp.bathroom_need             = NO;
        lcp.bathroom_timer_max        = rndRng(20, 40);
        lcp.bathroom_timer            = lcp.bathroom_timer_max;
        /* The shadow globals are copied FROM the struct fields, not
           assigned the same literal. */
        lcp.record_playing            = NO;
        lcp_recP            = lcp.record_playing;
        lcp.tv_on                     = NO;
        lcp_tv                     = lcp.tv_on;
        lcp.food_supply               = 4;
        lcp_food                = lcp.food_supply;
        lcp.door_states_and_flags     = FOOD_PACKS_MAX << DSF_FOOD_SHIFT;
}
