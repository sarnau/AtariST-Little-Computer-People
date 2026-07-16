/*
 * init.c -- boot-time init functions ported from Ghidra's main path.
 *
 * These wrap the parts of Ghidra's main() at 0x15546 that the port
 * had been silently skipping: lcp_create_random (populates the PLAYER
 * struct for a new game), cl_drini (paints the clock face),
 * and cutscene_new_lcp_move_in_stub (minimal seeding of lcp_x/y/state
 * and dog target so the AI loop can pick up on frame 1).
 *
 * addr: lcp_create_random @ 0x169D8, cl_drini @ 0x233B4,
 *       cutscene_new_lcp_move_in @ Ghidra (large, only stub here).
 */

#include "types.h"
#include "structs.h"
#include "enums.h"

extern PLAYER   lcp;
extern short    lcp_water_level;
extern short    lcp_record_playing;
extern short    lcp_tv_on;
extern short    lcp_food_count;
extern short    lcp_x;
extern short    lcp_y;
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    dog_x;
extern short    dog_y;
extern short    g_dtx;
extern short    g_dty;
extern short    g_dyx;
extern short    g_dyy;
extern short    dog_on_stairs_flag;
extern short    dog_idle_countdown;
extern short    dog_last_target_index;
extern short    dog_initialized;
extern short    intro_sequence_active;
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;

extern short    randomRange();
extern void     draw_line();
extern void     clock_redraw_hands();
extern void     sp_spud();

/* lcp_create_random (Ghidra 0x169D8): populate a fresh PLAYER struct
   for a new game.  The 1985 code also picks a random name from the
   "names" file -- we skip that so we don't need file_open here;
   character_name is left NUL-terminated. */

void
lcp_create_random()
{
        lcp.character_sprite_id       = randomRange(2, 6);
        lcp.character_name[0]         = 0;
        lcp.water_level               = 7;
        lcp_water_level               = 7;
        lcp.clothing_color            = randomRange(0, 15);
        lcp.skin_color                = randomRange(0, 7);
        lcp.bedtime_hour              = randomRange(22, 24);
        if (lcp.bedtime_hour > 23)
                lcp.bedtime_hour = lcp.bedtime_hour - 24;
        lcp.wake_hour                 = lcp.bedtime_hour + 6;
        if (lcp.wake_hour > 23)
                lcp.wake_hour = lcp.bedtime_hour - 18;
        lcp.lunch_hour                = randomRange(11, 13);
        lcp.dinner_hour               = randomRange(17, 19);
        lcp.personality_type          = randomRange(0, 3);
        lcp.activity_level            = randomRange(0, 7);
        lcp.happiness                 = MOOD_CONTENT;
        lcp.happiness_initial_countdown = randomRange(6, 24);
        lcp.happiness_duration_happy    = randomRange(6, 24);
        lcp.happiness_duration_content  = randomRange(6, 12);
        lcp.happiness_duration_active   = lcp.happiness_duration_happy;
        lcp.happiness_direction       = -1;             /* DIR_IMPROVING */
        lcp.sickness_level            = 0;              /* SICKNESS_HEALTHY */
        lcp.sickness_countdown        = 0;
        lcp.sickness_direction        = 0;              /* DIR_STABLE */
        lcp.is_sleeping               = NO;
        lcp.initiative_threshold      = randomRange(20, 80);
        lcp.thirst_level              = 0;              /* NEED_SATISFIED */
        lcp.thirst_timer_max          = randomRange(45, 75);
        lcp.thirst_timer              = lcp.thirst_timer_max;
        lcp.hunger_level              = 0;
        lcp.hunger_timer_max          = randomRange(75, 120);
        lcp.hunger_timer              = lcp.hunger_timer_max;
        lcp.bathroom_need             = NO;
        lcp.bathroom_timer_max        = randomRange(20, 40);
        lcp.bathroom_timer            = lcp.bathroom_timer_max;
        lcp.record_playing            = NO;
        lcp_record_playing            = NO;
        lcp.tv_on                     = NO;
        lcp_tv_on                     = NO;
        lcp.food_supply               = 4;
        lcp_food_count                = 4;
        lcp.door_states_and_flags     = 0x0800;         /* DSF_INIT_FOOD_FULL */
}

/* cl_drini (Ghidra 0x233B4): paint the clock face's center
   dot then delegate to clock_redraw_hands. */

void
cl_drini()
{
        draw_line(278, 83, 281, 83, COLOR_white);
        clock_redraw_hands();
}

/* cutscene_new_lcp_move_in_stub: minimal replacement for the doorbell/
   door-open/room-tour cutscene in Ghidra.  We SKIP the tour animation
   but reproduce the exit state so the AI loop starts from valid
   positions:
     lcp at (300, 190)   -- right side of ground floor
     lcp_state = STAND_SIDE_VIEW, facing right
     head anim initialised so sp_lcha doesn't loop
     dog at (273, 190), initial wander target seeded
     intro_sequence_active released so the event queue can drain.
   TODO: port the full cutscene once the AI-loop path is stable. */

void
cutscene_new_lcp_move_in_stub()
{
        lcp_x                     = 300;
        lcp_y                     = 190;
        lcp_facing_direction      = FACING_RIGHT;
        lcp_state                 = STATE_STAND_SIDE_VIEW;
        g_hatas                   = 8;
        g_hacur                   = 8;
        g_hamod                   = HEAD_ANIM_DISABLED;

        dog_x                     = 273;
        dog_y                     = 190;
        g_dtx                     = 0;
        g_dty                     = 0;
        g_dyx                     = 0;
        g_dyy                     = 0;
        dog_on_stairs_flag        = NO;
        dog_idle_countdown        = 20;
        dog_last_target_index     = 0;   /* Ghidra: dog_initial_target_index, not yet ported */
        dog_initialized           = NO;

        /* Push initial dog sprite (lay-down pose) into the dog slot. */
        sp_spud(SPRITE_DOG_LAY_DOWN, -1, YES);

        intro_sequence_active     = NO;
}
