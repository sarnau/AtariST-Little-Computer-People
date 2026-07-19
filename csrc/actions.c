/*
 * actions.c -- doAct() dispatcher (45 cases).
 *
 * Called from chk_actT() with a resolved ACTION_ID
 * already in g_trac.  Snapshots g_trac into lastAct
 * (used by the AI to avoid picking the same action twice in a row),
 * clears the trigger, waking the resident first if asleep, then
 * switches to the per-action handler.  All 45 handlers live in
 * separate .c files (or, until ported, astubs.c).
 *
 * addr: doAct()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "abathrm.h"
#include "actions.h"
#include "afood.h"
#include "agames.h"
#include "ahouse.h"
#include "ai.h"
#include "aidle.h"
#include "aleisure.h"
#include "aletter.h"
#include "asimple.h"
#include "globals.h"
/* Forward-declarations for every action_ handler.  Real ports live in
   action_*.c; unported ones share stub bodies in astubs.c. */

/* doAct: dispatch g_trac to its handler.
   addr: doAct() */

void
doAct()
{
        short   action_number;

        action_number = g_trac;
        lastAct   = g_trac;
        g_trac = ACTION_NONE;

        if (lcp.is_sleeping != NO)
                a_gioob();

        switch (action_number) {
        case ACTION_SIT_AND_EXERCISE:         a_sitae();          break;
        case ACTION_READ_NEWSPAPER:           a_readn();            break;
        case ACTION_PLAY_COMPUTER:            a_playc();             break;
        case ACTION_WASH_HANDS:               a_washh();                break;
        case ACTION_GET_IN_OUT_OF_BED:        a_gioob();         break;
        case ACTION_LISTEN_SONG:              a_lists();               break;
        case ACTION_PLAY_PIANO:               a_playp();                break;
        case ACTION_WRITE_LETTER:             a_writl();              break;
        case ACTION_DANCE:                    a_dance();                     break;
        case ACTION_YAWN_AND_STRETCH:         a_yawas();          break;
        case ACTION_PACE_NERVOUSLY:           a_pacen();            break;
        case ACTION_WANDER_IDLY:              a_wandi();               break;
        case ACTION_SLEEP:                    a_sleep(-1);                   break;
        case ACTION_DRINK:                    a_drink();                     break;
        case ACTION_NOD_HEAD:                 a_nodh();                  break;
        case ACTION_PEEK_AROUND:              a_peeka();               break;
        case ACTION_PLAY_A_GAME:              a_plaag();               break;
        case ACTION_BRUSH_TEETH:              a_brust();               break;
        case ACTION_KITCHEN_CABINET:          a_kitcc();           break;
        case ACTION_SIT_ON_COUCH_WITH_DOG:    a_socwd();     break;
        case ACTION_LIGHT_FIREPLACE:          a_lighf();           break;
        case ACTION_USE_TOILET:               a_uset();                break;
        case ACTION_TAKE_SHOWER:              a_takes();               break;
        case ACTION_FEED_DOG:                 a_feedd(0);                 break;
        case ACTION_HELLO:                    a_hello();                     break;
        case ACTION_EAT_MEAL:                 a_eatm();                  break;
        case ACTION_PLAY_WITH_RECORD:         a_plawr();          break;
        case ACTION_OPEN_UPSTAIRS_CLOSET:     a_opcuc(1); break;
        case ACTION_GET_SNACK_FROM_FRIDGE:    a_gesff();     break;
        case ACTION_OPEN_BEDROOM_CLOSET:      a_opcbc(); break;
        case ACTION_GET_DRESSED:              a_getd();               break;
        case ACTION_CLEAN_UP:                 a_cleau();                  break;
        case ACTION_TIDY_HOUSE:               a_tidyh();                break;
        case ACTION_CHECK_FRONT_DOOR:         a_chefd(40);        break;
        case ACTION_TOGGLE_TV:                a_toggt();                 break;
        case ACTION_CALL_DOG:                 a_calld();                  break;
        case ACTION_WAKE_FROM_ALARM:          a_wakfa();           break;
        case ACTION_PET_DOG:                  a_petd();                   break;
        case ACTION_WAKE_UP_MORNING:          a_wakum();           break;
        case ACTION_GO_TO_BED_NIGHT:          a_gotbn();           break;
        }
}
