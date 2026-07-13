/*
 * actions.c -- do_action() dispatcher (45 cases).
 *
 * Called from check_for_any_action_triggers() with a resolved ACTION_ID
 * already in trigger_action.  Snapshots trigger_action into last_action
 * (used by the AI to avoid picking the same action twice in a row),
 * clears the trigger, waking the resident first if asleep, then
 * switches to the per-action handler.  All 45 handlers live in
 * separate .c files (or, until ported, action_stubs.c).
 *
 * addr: do_action()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

/* Forward-declarations for every action_ handler.  Real ports live in
   action_*.c; unported ones share stub bodies in action_stubs.c. */
extern void     action_sit_and_exercise();
extern void     action_read_newspaper();
extern void     action_play_computer();
extern void     action_wash_hands();
extern void     action_get_in_out_of_bed();
extern void     action_listen_song();
extern void     action_play_piano();
extern void     action_write_letter();
extern void     action_dance();
extern void     action_yawn_and_stretch();
extern void     action_pace_nervously();
extern void     action_wander_idly();
extern void     action_sleep();
extern void     action_drink();
extern void     action_nod_head();
extern void     action_peek_around();
extern void     action_play_a_game();
extern void     action_brush_teeth();
extern void     action_kitchen_cabinet();
extern void     action_sit_on_couch_with_dog();
extern void     action_light_fireplace();
extern void     action_use_toilet();
extern void     action_take_shower();
extern void     action_feed_dog();
extern void     action_hello();
extern void     action_eat_meal();
extern void     action_play_with_record();
extern void     action_open_close_upstairs_closet();
extern void     action_get_snack_from_fridge();
extern void     action_open_close_bedroom_closet();
extern void     action_get_dressed();
extern void     action_clean_up();
extern void     action_tidy_house();
extern void     action_check_front_door();
extern void     action_toggle_tv();
extern void     action_call_dog();
extern void     action_wake_from_alarm();
extern void     action_pet_dog();
extern void     action_wake_up_morning();
extern void     action_go_to_bed_night();

/* do_action: dispatch trigger_action to its handler.
   addr: do_action() */

void
do_action()
{
        short   action_number;

        action_number = trigger_action;
        last_action   = trigger_action;
        trigger_action = ACTION_NONE;

        if (lcp.is_sleeping != NO)
                action_get_in_out_of_bed();

        switch (action_number) {
        case ACTION_SIT_AND_EXERCISE:         action_sit_and_exercise();          break;
        case ACTION_READ_NEWSPAPER:           action_read_newspaper();            break;
        case ACTION_PLAY_COMPUTER:            action_play_computer();             break;
        case ACTION_WASH_HANDS:               action_wash_hands();                break;
        case ACTION_GET_IN_OUT_OF_BED:        action_get_in_out_of_bed();         break;
        case ACTION_LISTEN_SONG:              action_listen_song();               break;
        case ACTION_PLAY_PIANO:               action_play_piano();                break;
        case ACTION_WRITE_LETTER:             action_write_letter();              break;
        case ACTION_DANCE:                    action_dance();                     break;
        case ACTION_YAWN_AND_STRETCH:         action_yawn_and_stretch();          break;
        case ACTION_PACE_NERVOUSLY:           action_pace_nervously();            break;
        case ACTION_WANDER_IDLY:              action_wander_idly();               break;
        case ACTION_SLEEP:                    action_sleep(-1);                   break;
        case ACTION_DRINK:                    action_drink();                     break;
        case ACTION_NOD_HEAD:                 action_nod_head();                  break;
        case ACTION_PEEK_AROUND:              action_peek_around();               break;
        case ACTION_PLAY_A_GAME:              action_play_a_game();               break;
        case ACTION_BRUSH_TEETH:              action_brush_teeth();               break;
        case ACTION_KITCHEN_CABINET:          action_kitchen_cabinet();           break;
        case ACTION_SIT_ON_COUCH_WITH_DOG:    action_sit_on_couch_with_dog();     break;
        case ACTION_LIGHT_FIREPLACE:          action_light_fireplace();           break;
        case ACTION_USE_TOILET:               action_use_toilet();                break;
        case ACTION_TAKE_SHOWER:              action_take_shower();               break;
        case ACTION_FEED_DOG:                 action_feed_dog(0);                 break;
        case ACTION_HELLO:                    action_hello();                     break;
        case ACTION_EAT_MEAL:                 action_eat_meal();                  break;
        case ACTION_PLAY_WITH_RECORD:         action_play_with_record();          break;
        case ACTION_OPEN_UPSTAIRS_CLOSET:     action_open_close_upstairs_closet(1); break;
        case ACTION_GET_SNACK_FROM_FRIDGE:    action_get_snack_from_fridge();     break;
        case ACTION_OPEN_BEDROOM_CLOSET:      action_open_close_bedroom_closet(); break;
        case ACTION_GET_DRESSED:              action_get_dressed();               break;
        case ACTION_CLEAN_UP:                 action_clean_up();                  break;
        case ACTION_TIDY_HOUSE:               action_tidy_house();                break;
        case ACTION_CHECK_FRONT_DOOR:         action_check_front_door(40);        break;
        case ACTION_TOGGLE_TV:                action_toggle_tv();                 break;
        case ACTION_CALL_DOG:                 action_call_dog();                  break;
        case ACTION_WAKE_FROM_ALARM:          action_wake_from_alarm();           break;
        case ACTION_PET_DOG:                  action_pet_dog();                   break;
        case ACTION_WAKE_UP_MORNING:          action_wake_up_morning();           break;
        case ACTION_GO_TO_BED_NIGHT:          action_go_to_bed_night();           break;
        }
}
