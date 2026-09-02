/*
 * parts/lcp_crnd.c -- shared body; LCP_ORG links it in init.c,
 * LCP_STX in the 0xdece object (0x69d8, in the 0x400c object after rnd).  Files under parts/
 * are never compiled standalone.
 */
/* lcp_crnd (Ghidra 0x169D8): populate PLAYER for a new game.
   1985 code also picks a random name from "names"; skipped here
   (avoids fOpen); character_name left NUL. */

void
lcp_crnd()
{
        /* Two locals; the first is reused as the loop index.  STX
           picks the resident's NAME out of a 266-entry, 10-byte
           record file rather than leaving it empty. */
        short   tmp;
        short   fhnd;

        lcp.character_sprite_id       = rndRng(2, 6);

        tmp  = rndRng(0, 265) * 10;
        fhnd = fOpen("names", 0);
        Fseek((long) tmp, fhnd, 0);
        fr_read(fhnd, 10L, lcp.character_name);
        Fclose(fhnd);
        for (tmp = 0; tmp < 10; tmp++)
                if (lcp.character_name[tmp] < 'A')
                        lcp.character_name[tmp] = 0;

        lcp.water_level               = 7;
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
        lcp.happiness_direction       = -1;             /* DIR_IMPROVING */
        lcp.sickness_level            = 0;              /* SICKNESS_HEALTHY */
        lcp.sickness_countdown        = 0;
        lcp.sickness_direction        = 0;              /* DIR_STABLE */
        lcp.is_sleeping               = NO;
        lcp.initiative_threshold      = rndRng(20, 80);
        lcp.thirst_level              = 0;              /* NEED_SATISFIED */
        lcp.thirst_timer_max          = rndRng(45, 75);
        lcp.thirst_timer              = lcp.thirst_timer_max;
        lcp.hunger_level              = 0;
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
        lcp.door_states_and_flags     = 0x0800;         /* DSF_INIT_FOOD_FULL */
}
