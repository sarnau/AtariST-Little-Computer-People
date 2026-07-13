/*
 * actions_leisure.c -- music, fireplace, couch, exercise, and the
 *                      lightweight house-upkeep handlers.
 *
 * Grouped because they all wire into the same music / dog / couch /
 * closet subsystems -- porting them one at a time forces the same
 * externs into each file.
 *
 * addr: action_listen_song(), action_play_piano(),
 *       action_play_with_record(), action_light_fireplace(),
 *       action_sit_on_couch_with_dog(), action_sit_and_exercise(),
 *       action_check_front_door(), action_clean_up(),
 *       action_tidy_house(), action_open_close_bedroom_closet()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     spritedata_select();
extern void     spritedata_select_carried_object_left();
extern void     sprite_update_slots();
extern void     soundeffect_select();
extern void     object_draw();
extern void     action_open_close_front_door();
extern void     action_open_close_filing_cabinet();
extern void     action_open_close_cabinet();
extern void     action_open_close_dresser();
extern void     action_close_toilet_door();
extern void     action_close_closet_door();
extern void     action_walk_to_and_turn();
extern void     action_call_dog();
extern void     action_play_piano();
extern void     hide_lcp_sprites();
extern void     show_lcp_sprites();
extern void     lcp_idle_look_right();
extern void     song_play();
extern void     midi_seq_init_song();
extern void     palette_apply_clothing_colors();
extern void     palette_apply_skin_colors();
extern void     walk_to_front_door();
/* host_gemdos_trap declared via _gemdos macro in osbind.h */
extern char     input_string[];         /* not used yet, letter subsystem */
extern void     lcp_enter_study_and_save();

/* Ghidra references. */
/* midi_song_max_position declared in globals.h */

/* DTA (Disk Transfer Address) layout used by GEMDOS Fsfirst/Fsnext.
   Only d_fname is read by the caller; the rest is padded. */
typedef struct {
        char    _reserved[30];
        char    d_fname[14];
} DTA;

/* action_listen_song: pick a random .sng file and start it playing.
   Uses lcp_food_count as a modulo index (yes, it's a bit hacky; the
   1985 code reused the field).

   .sng files on the LCP disk are byte-exact copies of Activision
   Music Studio 2.0 demo songs (AISLEDAN, BALLAD, BEBOP, BOSSA,
   CALYPSO, COUNTRY2, CANON, FIVEFOUR, MYSTERY, BOOGIE); the
   Music Studio disk shipped with the same files, so any tune the
   resident dances to originally came from Ed Bogas / Audio Light's
   1986 authoring tool.  See sound.c:song_play for the full file
   format details.
   addr: action_listen_song() */

void
action_listen_song()
{
        short   result;
        short   index;
        DTA *   dta_ptr;
        char *  filename;
        short   i;

        if (lcp_record_playing != NO)
                return;

        house_get_position_xy(POS_TOP_DANCE_FLOOR,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        game_tick_and_animate(2);
        lcp_idle_look_right();
        lcp_record_playing = YES;

        index = randomRange(0, lcp_food_count - 1) + 1;
        _gemdos(GEMDOS_Fsfirst, (long) "*.sng", 0L, 0L);
        while ((index = index - 1) != 0)
                _gemdos(GEMDOS_Fsnext, 0L, 0L, 0L);
        dta_ptr = (DTA *) _gemdos(GEMDOS_Fgetdta, 0L, 0L, 0L);
        filename = dta_ptr->d_fname;
        for (i = 0; filename[i] != '.'; i = i + 1)
                ;
        filename[i + 4] = '\0';
        song_play(filename);
}

/* action_play_piano: shortcut used mostly to *stop* a currently-playing
   record so the resident can start writing / typing.  Walks to the
   dance floor, spins the midi buffer to end, frees it, clears the
   record-playing flag.
   addr: action_play_piano() */

void
action_play_piano()
{
        short   result;

        if (lcp_record_playing == NO)
                return;

        house_get_position_xy(POS_TOP_DANCE_FLOOR,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        game_tick_and_animate(2);

        if (midi_is_playing != NO) {
                midi_seq_init_song(midi_song_buffer, midi_song_max_position);
                while (midi_is_playing != NO)
                        ;
        }
        lcp_idle_look_right();
        lcp_record_playing = NO;
        if (midi_song_buffer != (char *) 0) {
                _gemdos(GEMDOS_Mfree, (long) midi_song_buffer, 0L, 0L);
                midi_song_buffer = (char *) 0;
        }
}

/* action_play_with_record: browse the vinyl shelf and put on a random
   .org file.  The animation is amplitude-reactive: PSG channel volumes
   are polled via XBIOS Giaccess and if any channel got louder we
   randomise the browsing pose.  On the host the PSG stub returns 0
   forever so we fall through the amp check and hold the reach-right
   pose.

   .org files (FOLKSONG, MAPLE = Maple Leaf Rag, PRELUDE, REQUIEM,
   STARSPAN = Star-Spangled Banner) are cosmetically-renamed Music
   Studio 2.0 exports -- same file format as .sng, just organised
   into a classical/organ category for the vinyl-shelf UI.  PRELUDE
   and REQUIEM ship byte-identical to Music Studio's demo disk;
   STARSPAN is a shortened arrangement.  See sound.c:song_play.
   addr: action_play_with_record() */

void
action_play_with_record()
{
        short           result;
        DTA *           dta_ptr;
        char *          filename;
        long            xres;
        unsigned char   psg_a, psg_b, psg_c;
        unsigned char   prev_a, prev_b, prev_c;
        short           i;

        PLAYER_STATE_ARRAY[0] = STATE_BROWSE_VINYL_REACH_RIGHT;
        PLAYER_STATE_ARRAY[1] = STATE_BROWSE_VINYL_IDLE;
        PLAYER_STATE_ARRAY[2] = STATE_BROWSE_VINYL_REACH_LEFT;
        PLAYER_STATE_ARRAY[3] = STATE_BROWSE_VINYL_PULL_OUT;

        prev_a = prev_b = prev_c = 0;
        action_interruptible_flag = YES;
        if (lcp_record_playing != NO)
                action_play_piano();
        action_interruptible_flag = NO;

        house_get_position_xy(POS_TOP_RECORD_SHELF,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        record_browsing_active = YES;
        head_anim_mode = HEAD_ANIM_DISABLED;
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        game_tick_and_animate(4);

        lcp_state = STATE_BROWSE_VINYL_REACH_RIGHT;
        sprite_layer_flags[SPRITE_VINYL_RECORD] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_VINYL_RECORD);
        sprite_pending_x[sprite_slot_map[SPRITE_VINYL_RECORD]] = 146;
        sprite_pending_y[sprite_slot_map[SPRITE_VINYL_RECORD]] =  54;
        game_tick_and_animate(1);

        i = randomRange(1, org_song_file_count);
        _gemdos(GEMDOS_Fsfirst, (long) "*.org", 0L, 0L);
        while ((i = i - 1) != 0)
                _gemdos(GEMDOS_Fsnext, 0L, 0L, 0L);
        dta_ptr = (DTA *) _gemdos(GEMDOS_Fgetdta, 0L, 0L, 0L);
        filename = dta_ptr->d_fname;
        for (i = 0; filename[i] != '.'; i = i + 1)
                ;
        filename[i + 4] = '\0';
        song_play(filename);

        head_anim_mode = HEAD_ANIM_WALKING;
        while (midi_is_playing == NO)
                ;                        /* wait for playback to start */

        while (midi_is_playing != NO) {
                xres = Giaccess(0, 8);  psg_a = (unsigned char) xres & 0x1f;
                xres = Giaccess(0, 9);  psg_b = (unsigned char) xres & 0x1f;
                xres = Giaccess(0, 10); psg_c = (unsigned char) xres & 0x1f;

                lcp_state = PLAYER_STATE_ARRAY[0];
                if (prev_a < psg_a || prev_b < psg_b || prev_c < psg_c) {
                        i = randomRange(1, 3);
                        while (PLAYER_STATE_ARRAY[i] == lcp_state)
                                i = randomRange(1, 3);
                        lcp_state = PLAYER_STATE_ARRAY[i];
                        if (PLAYER_STATE_ARRAY[3] == lcp_state) {
                                game_tick_and_animate(0);
                                result = randomRange(1, 2);
                                lcp_state = PLAYER_STATE_ARRAY[result];
                        }
                }
                game_tick_and_animate(0);
                prev_c = psg_c; prev_b = psg_b; prev_a = psg_a;
        }

        head_anim_mode = HEAD_ANIM_DISABLED;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_state = PLAYER_STATE_ARRAY[0];
        lcp_wait_head_reach_target();
        game_tick_and_animate(8);

        lcp_state = STATE_STAND_FACING_SCREEN;
        sprite_layer_flags[SPRITE_VINYL_RECORD] = SPRITE_HIDDEN;
        sprite_update_slots();
        game_tick_and_animate(0);

        if (midi_song_buffer != (char *) 0) {
                _gemdos(GEMDOS_Mfree, (long) midi_song_buffer, 0L, 0L);
                midi_song_buffer = (char *) 0;
        }
        record_browsing_active = NO;
}

/* action_light_fireplace: firewood run from front-door pickup to the
   fireplace, then stoke animation with a random-facing shrug pattern
   and 2500..5000 tick fire-active countdown.
   addr: action_light_fireplace() */

void
action_light_fireplace()
{
        short   result;
        short   i;

        if (fire_active_flag != NO)
                return;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        action_open_close_front_door(0);
        action_interruptible_flag = YES;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &walk_target_x, &walk_target_y);
        walk_target_x = walk_target_x - 10;
        lcp_walk_to_destination();

        /* Sit-dog sprite waits at the porch. */
        sprite_layer_flags[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_DOG_SIT);
        sprite_pending_x[sprite_slot_map[SPRITE_DOG_SIT]] = 294;
        sprite_pending_y[sprite_slot_map[SPRITE_DOG_SIT]] = 151;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();
        hide_lcp_sprites();
        game_tick_and_animate(40);
        show_lcp_sprites();

        spritedata_select_carried_object_left(SPRITE_FIREWOOD);
        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &walk_target_x, &walk_target_y);
        walk_target_x = walk_target_x - 10;
        lcp_walk_to_destination();

        sprite_layer_flags[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sprite_update_slots();

        result = randomRange(0, 100);
        if (lcp.initiative_threshold < result)
                action_open_close_front_door(1);

        house_get_position_xy(POS_BTM_FIREPLACE_LOGS,
                              &walk_target_x, &walk_target_y);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        sprite_layer_flags[SPRITE_FIREWOOD] = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_carrying_object_flag = NO;
        lcp_wait_head_reach_target();

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_BEND_DOWN;      game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD;  game_tick_and_animate(1);
        lcp_state = STATE_STOKE_FIREPLACE;game_tick_and_animate(1);

        /* Random-direction shrug for 10 ticks so the resident looks
           like they're feeding kindling. */
        for (i = 0; i < 10; i = i + 1) {
                lcp_facing_direction = randomRange(0, 1);
                game_tick_and_animate(0);
        }

        fire_active_flag        = YES;
        fire_duration_countdown = randomRange(2500, 5000);

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
        action_interruptible_flag = NO;
}

/* action_sit_on_couch_with_dog: call the dog over, sit on the couch,
   pet the dog for 30..50 ticks then crouch back off the couch.
   addr: action_sit_on_couch_with_dog() */

void
action_sit_on_couch_with_dog()
{
        short   ticks;

        action_interruptible_flag = YES;
        action_call_dog();
        action_interruptible_flag = NO;
        if (triggered_event_list[0] != ACTION_NONE) {
                lcp_state = STATE_STAND_SIDE_VIEW;
                game_tick_and_animate(0);
                return;
        }

        lcp_state = STATE_SIT_COUCH_UPRIGHT;
        lcp_y = lcp_y + 9;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();
        game_tick_and_animate(3);

        lcp_y = lcp_y - 3;
        sprite_layer_flags[SPRITE_READING_1] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_READING_1);
        sprite_pending_x[sprite_slot_map[SPRITE_READING_1]] = 221;
        sprite_pending_y[sprite_slot_map[SPRITE_READING_1]] = 172;

        ticks = randomRange(30, 50);
        lcp_state = STATE_SIT_COUCH_PETTING_DOG;
        while (ticks != 0 && triggered_event_list[0] == ACTION_NONE) {
                game_tick_and_animate(3);
                ticks = ticks - 1;
        }

        lcp_y = lcp_y + 3;
        lcp_state = STATE_SIT_COUCH_UPRIGHT;
        sprite_layer_flags[SPRITE_READING_1] = SPRITE_HIDDEN;
        sprite_update_slots();
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();
        game_tick_and_animate(3);

        lcp_y = lcp_y - 9;
        lcp_state = STATE_CROUCH_DOWN;
        game_tick_and_animate(8);
        while (petting_dog_active != NO)
                game_tick_and_animate(0);

        dog_pettable_flag = NO;
        lcp_state = STATE_STAND_SIDE_VIEW;
        lcp_facing_direction = FACING_RIGHT;
        game_tick_and_animate(1);
}

/* action_sit_and_exercise: stretch arms in 4-frame cycle for a random
   number of iterations.
   addr: action_sit_and_exercise() */

void
action_sit_and_exercise()
{
        short           result;
        unsigned short  duration;
        unsigned short  i;

        PLAYER_STATE_ARRAY[0] = STATE_EXERCISE_ARMS_CENTER;
        PLAYER_STATE_ARRAY[1] = STATE_EXERCISE_ARMS_UP;
        PLAYER_STATE_ARRAY[2] = STATE_EXERCISE_ARMS_CENTER;
        PLAYER_STATE_ARRAY[3] = STATE_EXERCISE_ARMS_WIDE;

        house_get_position_xy(POS_MID_COUCH,
                              &walk_target_x, &walk_target_y);
        walk_target_y = walk_target_y - 5;
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();

        duration = (unsigned short) Random();
        i = 0;
        while (i < ((duration & 0x7f) | 8) &&
               triggered_event_list[0] == ACTION_NONE) {
                lcp_state = PLAYER_STATE_ARRAY[i & 3];
                if (lcp_state == STATE_EXERCISE_ARMS_CENTER)
                        game_tick_and_animate(0);
                else
                        game_tick_and_animate(3);
                i = i + 1;
        }
        lcp_state = STATE_STAND_SIDE_VIEW;
        game_tick_and_animate(0);
}

/* action_check_front_door: walk to the door, open it, look outside for
   `value` ticks, then optionally close.  value is passed by the do_action
   dispatcher as 40 (see actions.c switch).
   addr: action_check_front_door() */

void
action_check_front_door(value)
short   value;
{
        short   result;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        if (lcp_front_door_open == NO)
                action_open_close_front_door(0);
        action_interruptible_flag = YES;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &walk_target_x, &walk_target_y);
        walk_target_x = walk_target_x - 10;
        lcp_walk_to_destination();

        sprite_layer_flags[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_DOG_SIT);
        sprite_pending_x[sprite_slot_map[SPRITE_DOG_SIT]] = 294;
        sprite_pending_y[sprite_slot_map[SPRITE_DOG_SIT]] = 151;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();
        hide_lcp_sprites();
        game_tick_and_animate(value);
        show_lcp_sprites();

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &walk_target_x, &walk_target_y);
        walk_target_x = walk_target_x - 10;
        lcp_walk_to_destination();
        sprite_layer_flags[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sprite_update_slots();

        result = randomRange(0, 100);
        if (lcp.initiative_threshold < result) {
                action_interruptible_flag = YES;
                house_get_position_xy(POS_BTM_FRONT_DOOR,
                                      &walk_target_x, &walk_target_y);
                lcp_walk_to_destination();
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                action_open_close_front_door(1);
        }
        action_interruptible_flag = NO;
}

/* action_tidy_house: walk to filing cabinet, possibly close it.
   addr: action_tidy_house() */

void
action_tidy_house()
{
        short   result;

        house_get_position_xy(POS_TOP_FILING_CABINET,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        action_walk_to_and_turn();

        result = randomRange(0, 100);
        if (lcp.initiative_threshold < result ||
            intro_sequence_active != NO)
                action_open_close_filing_cabinet();
}

/* action_clean_up: sweep all open doors/cabinets and close them.
   Order matters -- upstairs first (filing cabinet, study door) so the
   dresser/closet close animation doesn't collide with the toilet-door
   sprite pipeline.
   addr: action_clean_up() */

void
action_clean_up()
{
        short   result;

        if (lcp_filing_cabinet_open != NO) {
                house_get_position_xy(POS_TOP_FILING_CABINET,
                                      &walk_target_x, &walk_target_y);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                action_open_close_filing_cabinet();
        }
        if (lcp_study_door_open != NO) {
                house_get_position_xy(POS_TOP_STUDY_DOOR,
                                      &walk_target_x, &walk_target_y);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                lcp_facing_direction = FACING_RIGHT;
                lcp_state            = STATE_STAND_FACING_SCREEN;
                lcp_wait_head_reach_target();
                lcp_facing_direction = FACING_LEFT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                object_draw(object_id_door_study_open_1, 178, 23);
                game_tick_and_animate(2);
                object_draw(object_id_door_study_closed,  178, 23);
                soundeffect_select(SFX_DOOR_CLOSE, 6L);
                game_tick_and_animate(2);
                lcp_study_door_open = NO;
                lcp_facing_direction = FACING_RIGHT;
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(0);
        }
        if (lcp_toilet_door_open != NO) {
                house_get_position_xy(POS_MID_TOILET_DOOR,
                                      &walk_target_x, &walk_target_y);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                action_close_toilet_door();
        }
        if (lcp_closet_door_open != NO) {
                house_get_position_xy(POS_MID_BEDROOM_CLOSET,
                                      &walk_target_x, &walk_target_y);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                action_close_closet_door();
        }
        if (lcp_dresser_open != NO) {
                house_get_position_xy(POS_MID_DRESSER,
                                      &walk_target_x, &walk_target_y);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                action_open_close_dresser(1);
        }
        if (lcp_cabinet_open != NO) {
                house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                                      &walk_target_x, &walk_target_y);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                action_open_close_cabinet(1);
        }
        if (lcp_front_door_open != NO) {
                walk_to_front_door();
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                action_open_close_front_door(1);
        }
}

/* action_open_close_bedroom_closet: 3-sprite dress-change sequence.
   Closet door swings open, then a 3-frame in-closet animation while
   the palette gets swapped to the newly-picked clothing/skin colours,
   then door swings back.  value=0 -> palette_apply_clothing_colors,
   value=1 -> palette_apply_skin_colors.
   addr: action_open_close_bedroom_closet() */

void
action_open_close_bedroom_closet(value)
short   value;
{
        short   result;
        short   saved_x;
        short   counter;

        house_get_position_xy(POS_MID_DRESSER,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        action_open_close_dresser(0);
        counter = randomRange(0, 100);
        if (lcp.initiative_threshold < counter)
                action_open_close_dresser(1);

        house_get_position_xy(POS_MID_BEDROOM_CLOSET,
                              &walk_target_x, &walk_target_y);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();
        action_interruptible_flag = NO;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        if (lcp_closet_door_open == NO) {
                lcp_facing_direction = FACING_LEFT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                object_draw(object_id_door_closet_closed, 75, 87);
                game_tick_and_animate(2);
                object_draw(object_id_door_closet_open_1, 75, 87);
                soundeffect_select(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                object_draw(object_id_door_closet_open_2, 75, 87);
                game_tick_and_animate(2);
                lcp_closet_door_open = YES;
        }

        /* Walk into the closet. */
        lcp_facing_direction = FACING_RIGHT;
        sprite_layer_flags[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_CLOSET_WIDE_OPEN);
        sprite_pending_x[sprite_slot_map[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        sprite_pending_y[sprite_slot_map[SPRITE_CLOSET_WIDE_OPEN]] = 87;

        house_get_position_xy(POS_MID_BEDROOM_CLOSET,
                              &walk_target_x, &walk_target_y);
        walk_target_y = walk_target_y - 3;
        walk_target_x = walk_target_x - 10;
        action_interruptible_flag = YES;
        lcp_walk_to_destination();
        saved_x = lcp_x;
        action_interruptible_flag = NO;

        /* Close door behind: wide -> ajar -> lcp-inside. */
        sprite_layer_flags[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_CLOSET_AJAR] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_CLOSET_AJAR);
        sprite_pending_x[sprite_slot_map[SPRITE_CLOSET_AJAR]] = 75;
        sprite_pending_y[sprite_slot_map[SPRITE_CLOSET_AJAR]] = 87;
        object_draw(object_id_door_closet_open_1, 75, 87);
        game_tick_and_animate(1);

        sprite_layer_flags[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_CLOSET_LCP_INSIDE] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_CLOSET_LCP_INSIDE);
        hide_lcp_sprites();
        sprite_pending_x[sprite_slot_map[SPRITE_CLOSET_LCP_INSIDE]] = 75;
        sprite_pending_y[sprite_slot_map[SPRITE_CLOSET_LCP_INSIDE]] = 87;
        object_draw(object_id_door_closet_closed, 75, 87);
        soundeffect_select(SFX_DOOR_CLOSE, 6L);
        game_tick_and_animate(1);

        counter = randomRange(45, 60);
        game_tick_and_animate(counter);
        if (intro_sequence_active == NO) {
                if (value == 0)
                        palette_apply_clothing_colors();
                else
                        palette_apply_skin_colors();
        }

        /* Open door back up + walk out. */
        sprite_layer_flags[SPRITE_CLOSET_LCP_INSIDE] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_CLOSET_AJAR] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_CLOSET_AJAR);
        show_lcp_sprites();
        sprite_pending_x[sprite_slot_map[SPRITE_CLOSET_AJAR]] = 75;
        sprite_pending_y[sprite_slot_map[SPRITE_CLOSET_AJAR]] = 87;
        object_draw(object_id_door_closet_open_1, 75, 87);
        soundeffect_select(SFX_DOOR_OPEN, 6L);
        game_tick_and_animate(1);

        sprite_layer_flags[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_CLOSET_WIDE_OPEN);
        sprite_pending_x[sprite_slot_map[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        sprite_pending_y[sprite_slot_map[SPRITE_CLOSET_WIDE_OPEN]] = 87;
        object_draw(object_id_door_closet_open_2, 75, 87);
        game_tick_and_animate(1);
        lcp_closet_door_open = YES;

        lcp_x = saved_x;
        house_get_position_xy(POS_MID_BEDROOM_CLOSET,
                              &walk_target_x, &walk_target_y);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();
        action_interruptible_flag = NO;

        if (lcp_closet_door_open != NO) {
                sprite_layer_flags[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
                sprite_update_slots();
                game_tick_and_animate(0);
        }

        counter = randomRange(0, 100);
        if (lcp.initiative_threshold < counter ||
            intro_sequence_active != NO)
                action_close_closet_door();
}

/* action_open_close_upstairs_closet: walk to the study door, open it
   if closed (3-frame sprite animation), then walk into the study.
   Chains into lcp_enter_study_and_save; the `value` argument selects
   whether the entry saves (value != 0 -> do_save=YES) or just plays
   the animation (value == 0 -> do_save=NO).  Both entries play the
   door SFX.
   addr: action_open_close_upstairs_closet() */

void
action_open_close_upstairs_closet(value)
short   value;
{
        short   result;

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        head_anim_mode         = HEAD_ANIM_DISABLED;
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        if (lcp_study_door_open == NO) {
                lcp_facing_direction = FACING_LEFT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                object_draw(object_id_door_study_closed,  178, 23);
                game_tick_and_animate(2);
                object_draw(object_id_door_study_open_1,  178, 23);
                soundeffect_select(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                object_draw(object_id_door_study_open_2,  178, 23);
                game_tick_and_animate(2);
                lcp_study_door_open = YES;
        }

        /* Walk into the study, ducking behind the wide-open door. */
        lcp_facing_direction = FACING_RIGHT;
        sprite_layer_flags[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_DOOR_STUDY_WIDE_OPEN);
        sprite_pending_x[sprite_slot_map[SPRITE_DOOR_STUDY_WIDE_OPEN]] = 178;
        sprite_pending_y[sprite_slot_map[SPRITE_DOOR_STUDY_WIDE_OPEN]] =  23;

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &walk_target_x, &walk_target_y);
        walk_target_y = walk_target_y - 3;
        walk_target_x = walk_target_x - 10;
        action_interruptible_flag = YES;
        lcp_walk_to_destination();
        action_interruptible_flag = NO;

        /* Swap wide-open sprite for ajar and hide the resident. */
        sprite_layer_flags[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_DOOR_STUDY_AJAR);
        sprite_pending_x[sprite_slot_map[SPRITE_DOOR_STUDY_AJAR]] = 178;
        sprite_pending_y[sprite_slot_map[SPRITE_DOOR_STUDY_AJAR]] =  23;
        object_draw(object_id_door_study_open_1, 178, 23);
        hide_lcp_sprites();
        game_tick_and_animate(1);
        sprite_layer_flags[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        sprite_update_slots();

        /* Continue into the study; value != 0 -> save HYBER. */
        if (value == 0)
                lcp_enter_study_and_save(NO,  YES);
        else
                lcp_enter_study_and_save(YES, YES);
}
