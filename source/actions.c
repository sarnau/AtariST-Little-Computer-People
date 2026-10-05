/*
 * actions.c -- runAction() dispatcher.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "protos.h"
#include "globals.h"

/* Run the action chooseAction chose.  Consumes nextAction (copying it into
   lastAction so the random picker avoids repeating it, then clearing it
   to ACTION_NONE), gets the resident out of bed first if he is
   asleep, and dispatches to the matching a_* action routine.  Unknown
   action numbers fall through and do nothing. */
void
runAction()
{
        short   actionNumber;

        actionNumber = nextAction;
        lastAction = nextAction;
        nextAction = ACTION_NONE;

        if (resident.is_sleeping != NO)
                getInOutOfBed();

        switch (actionNumber) {
        case ACTION_SIT_AND_EXERCISE:         exercise();          break;
        case ACTION_READ_NEWSPAPER:           readNewspaper();            break;
        case ACTION_PLAY_COMPUTER:            useComputer();             break;
        case ACTION_WASH_HANDS:               washHands();                break;
        case ACTION_GET_IN_OUT_OF_BED:        getInOutOfBed();         break;
        case ACTION_LISTEN_SONG:              playRecord();               break;
        case ACTION_STOP_RECORD:              stopRecord();                break;
        case ACTION_WRITE_LETTER:             writeLetter();              break;
        case ACTION_DANCE:                    danceToMusic();                     break;
        case ACTION_YAWN_AND_STRETCH:         yawnAndStretch();          break;
        case ACTION_PACE_NERVOUSLY:           paceNervously();            break;
        case ACTION_WANDER_IDLY:              idleShrug();               break;
        case ACTION_SLEEP:                    dozeOff(SLEEP_RANDOM);                   break;
        case ACTION_DRINK:                    drinkWater();                     break;
        case ACTION_NOD_HEAD:                 nodHead();                  break;
        case ACTION_PEEK_AROUND:              peekAround();               break;
        case ACTION_PLAY_A_GAME:              playGame();               break;
        case ACTION_BRUSH_TEETH:              brushTeeth();               break;
        case ACTION_KITCHEN_CABINET:          eatFromCabinet();           break;
        case ACTION_SIT_ON_COUCH_WITH_DOG:    sitWithDog();     break;
        case ACTION_LIGHT_FIREPLACE:          lightFire();           break;
        case ACTION_USE_TOILET:               useToilet();                break;
        case ACTION_TAKE_SHOWER:              takeShower();               break;
        case ACTION_FEED_DOG:                 feedDog(0);                 break;
        case ACTION_HELLO:                    sayHello();                     break;
        case ACTION_EAT_MEAL:                 cookMeal();                  break;
        case ACTION_PLAY_ORGAN:               playOrgan();          break;
        case ACTION_OPEN_UPSTAIRS_CLOSET:     enterStudy(1); break;
        case ACTION_GET_SNACK_FROM_FRIDGE:    goToFridge();     break;
        case ACTION_OPEN_BEDROOM_CLOSET:      changeClothes(); break;
        case ACTION_NOD_OK:              nodOk();               break;
        case ACTION_CLEAN_UP:                 cleanUp();                  break;
        case ACTION_TIDY_HOUSE:               tidyHouse();                break;
        case ACTION_CHECK_FRONT_DOOR:         checkFrontDoor(40);        break;
        case ACTION_TOGGLE_TV:                toggleTv();                 break;
        case ACTION_CALL_DOG:                 callDog();                  break;
        case ACTION_WAKE_FROM_ALARM:          wakeFromAlarm();           break;
        case ACTION_PET_DOG:                  petDog();                   break;
        case ACTION_WAKE_UP_MORNING:          morningRoutine();           break;
        case ACTION_GO_TO_BED_NIGHT:          nightRoutine();           break;
        }
}
