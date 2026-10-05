# Little Computer People — The Resident

The resident (the "Little Computer Person", `resident` in the source) lives in
the three-floor house on his own schedule.  This document explains what he
is, how his day runs, how he decides what to do next, every way the player
can interact with him, and how he moves around the house.

Everything here was read from the C port in `source/`, which compiles to a
binary byte-identical to the 1985 release; file references are to that
source.  How he is drawn is in [RENDERING.md](RENDERING.md), the sounds and
music in [SOUND.md](SOUND.md).  Names are the port's (see [NAMEMAP.md](NAMEMAP.md) for the Ghidra
names the older analysis documents use).

## Time

The game runs at **8 frames per second**: `renderFrame`
([`parts/renderFrame.c`](../source/parts/renderFrame.c)) returns early until
25 ticks of TOS's 200 Hz clock (125 ms) have passed.  Code waits in whole
frames: `gameTick(n)` ([`tick.c`](../source/tick.c)) lasts n + 1 frames, and
this document calls one frame a **tick**.

Each tick, `gameTick` also runs `simStep` ([`sim.c`](../source/sim.c)), which
advances the clock one second every 8 ticks.  **The house clock therefore runs
in real time**: a game minute is a real minute, a game day a real day.  The
calendar starts from the date and time typed on the title screen.

## Who the resident is

A new resident is rolled by `rollResident`
([`parts/rollResident.c`](../source/parts/rollResident.c)); a returning one is
read back from the 128-byte save file `HYBER` into the `PLAYER` struct
([`include/structs.h`](../source/include/structs.h)).

| Field | Rolled as | Effect |
|---|---|---|
| `characterName` | one of 266 names in `NAMES` | signs his letters |
| `characterSpriteId` | 2..6 | which head file, `PE2.LCP`..`PE6.LCP` |
| `clothingColor` | 0..15 | his usual clothes (palette slots 1 and 2) |
| `skinColor` | 0..7 | his usual skin tone |
| `bedtimeHour` | 22, 23 or 0 | the nightly routine starts at this hour |
| `wakeHour` | bedtime + 6, so 4, 5 or 6 | the morning routine starts at this hour |
| `lunchHour`, `dinnerHour` | 11..13, 17..19 | scheduled meals |
| `activityLevel` | 0..7 | picks his idle-activity rhythm (see below) |
| `initiativeThreshold` | 20..80 | how often he leaves doors open (see below) |
| `personalityType` | 0..3 | saved, but nothing in the game reads it |
| `moodDuration[3]` | happy 6..24, content 6..24, sad 6..12 hours | length of each mood spell |
| `thirstTimerMax` | 45..75 minutes | thirst rises one level per period |
| `hungerTimerMax` | 75..120 minutes | hunger rises one level per period |
| `bathroomTimerMax` | 20..40 minutes | time from a meal to needing the toilet |

He starts content, healthy, with a water tank at 7 of 10, a full food cabinet
(4 packs) and a collection of 4 records.

**Initiative.**  Whenever he has opened something -- the front door, the
kitchen cabinet, the toilet door, the closet, the filing cabinet -- he closes
it again only if a 0..100 roll beats `initiativeThreshold`.  A resident with a
low threshold usually tidies up behind himself; one with a high threshold
leaves things open, which gives `cleanUp` (`ACTION_CLEAN_UP`) work to do.

## Body and mind

`simStep` updates his needs once per game minute and his mood once per game
hour.

### Thirst and hunger

Each has a level, `NEED_SATISFIED` (0) to `NEED_SEVERE` (3), and a timer that
counts minutes down from its maximum.  When the timer runs out it restarts and
the level rises by one; when it runs out while the level is already severe,
he **falls sick** (`fallSick`, [`health.c`](../source/health.c)).

- **Drinking** (`drinkWater`) sets thirst to satisfied and restarts the
  thirst timer.  It draws 3 units from the water tank -- but thirst is reset
  even when the tank is empty and he only goes through the motions.
- **Eating** (`eatFromCabinet`, also the end of `cookMeal`) takes one pack
  from the kitchen cabinet and sets hunger to satisfied.  With the cabinet
  empty he opens it, finds nothing and stays hungry.  Note that eating does
  not restart the hunger timer.

### Bathroom

Eating restarts the bathroom timer; when it runs out, `bathroomNeed` is set
and he goes to the toilet at the next decision (`useToilet`), which clears it.
So he needs the toilet 20..40 minutes after each meal.

### Sickness

`sicknessLevel` runs `SICKNESS_HEALTHY` (0) to `SICKNESS_CRITICAL` (4) and
moves one step every 60 minutes while worsening, every 5 while improving.

- `fallSick` sets him to `SICKNESS_MILD`, worsening, and makes him one mood
  step sadder.  It does this every time a need runs out at severe -- even
  when he is already sicker, which knocks the level back down to mild.
- Recovery (`startRecovery`) starts only when hunger **and** thirst are both
  satisfied, after a meal or a drink.  Back at healthy, his skin turns normal.
- The intended clamp at `SICKNESS_CRITICAL` has a typo (`==` for `=`), so a
  resident left untreated keeps getting worse past level 4.

While sick: his skin (palette slot 6) turns green, he walks at half speed,
and he will only drink when the tank has water and only eat when the cabinet
has food.  From `SICKNESS_MODERATE` on he is forced sad and spends all his
free time asleep (see the idle picker).  Nothing in the game kills him.

### Mood

`happiness` is `MOOD_HAPPY` (0), `MOOD_CONTENT` (1) or `MOOD_SAD` (2).  Every
game hour the current spell counts down; when it ends the mood moves one step
in `happinessDirection` and turns around at either end, so it cycles
happy -> content -> sad -> content -> happy, each spell lasting its
`moodDuration`.  The cycle pauses while he is sick and sad.

Being patted (Ctrl-P) makes him happy at once and starts a fresh happy spell;
falling sick makes him sadder.

Mood changes **how** he does things, not **what** he does: it selects his
facial expressions (each head file holds a content, a sad and a happy set),
the paragraphs of his letters, and how readily he accepts typed requests
(see below).  The choice of activity never looks at it.

## How he decides

After the start-up, the game is a loop of `gameTick(0)` and `chooseAction`
([`parts/gameLoop.c`](../source/parts/gameLoop.c)).  Most activities take many
ticks, so `chooseAction` runs once per finished activity.

### The decision ladder

`chooseAction` ([`parts/chooseAction.c`](../source/parts/chooseAction.c))
takes the first rung that applies:

1. **An outside event is waiting** (`eventQueue`): run it through `runEvent`
   ([`ai.c`](../source/ai.c)) -- a delivery, a phone call.
2. **The alarm is ringing**: `wakeFromAlarm` -- walk to the alarm clock and
   switch it off.
3. **He needs the toilet**: `useToilet`.
4. **Thirsty** (any level above satisfied): drink.  A healthy resident only
   does so one time in three (a roll over 66 out of 100); a sick one always,
   provided the tank has water.
5. **Hungry**: eat from the cabinet, with the same one-in-three chance when
   healthy -- and never twice in a row -- and always when sick, provided
   there is food.
6. **Lunch hour**, 7. **dinner hour**: `cookMeal`, once a day each.
8. **Wake-up hour**: `morningRoutine`, once a day.
9. **Bedtime hour**: `nightRoutine`, once a day.
10. **A typed request is waiting**: see "Typed requests".
11. **Otherwise** an idle activity from `pickIdleAction`.

The once-a-day flags are cleared at midnight (`resetDailyFlags`).  An
activity is started through `runAction` ([`actions.c`](../source/actions.c)),
which first gets him out of bed if he is asleep -- any activity wakes him.

### Idle activities

`pickIdleAction` ([`airandom.c`](../source/airandom.c)) counts the hours since
his wake-up hour:

- **18 hours or more, or moderately sick:** the sleep tier.  If he is awake
  he goes to bed (`ACTION_GET_IN_OUT_OF_BED`); if he is asleep, nothing
  happens and he sleeps on.
- **Otherwise** the hours are grouped in two-hour slots that repeat every six
  hours, and `scheduleTiers[slot][activityLevel]`
  ([`dat_aitables.c`](../source/dat_aitables.c)) gives a tier:

  | activity level | hours 0-1, 6-7, 12-13 | 2-3, 8-9, 14-15 | 4-5, 10-11, 16-17 |
  |---|---|---|---|
  | 0 | active | relaxed | moderate |
  | 1 | active | moderate | relaxed |
  | 2 | relaxed | active | moderate |
  | 3 | relaxed | moderate | active |
  | 4 | moderate | relaxed | active |
  | 5 | moderate | active | relaxed |
  | 6 | active | relaxed | moderate |
  | 7 | moderate | active | relaxed |

  At weekends he takes it easier: on Sunday an active slot becomes relaxed,
  on Saturday moderate.

Each tier is a table of 16 activities; one is drawn at random, re-drawn if it
is the same as the last activity.  Duplicates make some more likely:

| Tier | Activities (count out of 16) |
|---|---|
| active (`activeActions`) | computer 3, clean up 2, tidy house 2, read in the armchair 2, study 1, write a letter 1, feed the dog 1, wave hello 1, exercise 1, check the front door 1, *nothing* 1 |
| moderate (`moderateActions`) | wave hello 2, dance 2, TV on/off 2, *nothing* 2, check the front door 1, play a record 1, play the organ 1, read the newspaper 1, pace 1, play a game 1, study 1, exercise 1 |
| relaxed (`relaxedActions`) | read the newspaper 2, wait to be patted 2, read in the armchair 2, TV on/off 2, light the fire 1, play a record 1, study 1, *nothing* 1, wave hello 1, doze off 1, check the front door 1, stop the record 1 |

*Nothing*: each table contains `ACTION_EVENT_PHONE_CALL`, an event id that
`runAction` has no case for.  Drawing it does nothing; the picker simply runs
again a tick later (and cannot draw it twice in a row).

### Interruptions

Only **outside events** interrupt him.  `walkToTarget`
([`walk.c`](../source/walk.c)) abandons a walk when an event is queued --
unless he is carrying something, on the stairs, inside an event already, in
the move-in, or in an activity that has set `noPreempt` -- and the activity
then gives up.  Many activities with a duration (exercising, reading,
typing on the computer, washing, dancing, reading in the armchair, waiting
to be patted, dozing) also end early when an event arrives.  Typed requests,
needs and the schedule never interrupt; they wait for the next decision.

## A typical day

With bedtime 23:00 (so wake-up 05:00), lunch at 12:00 and dinner at 18:00:

- **05:00, morning routine** (`morningRoutine`): the alarm rings for 5..12
  seconds, he gets out of bed and switches it off, showers, brushes his teeth,
  dresses -- half the time in his usual clothes, otherwise in random colours
  -- and cooks breakfast at the stove, eating it at the kitchen table.  None
  of these walks can be interrupted.
- **During the day:** idle activities in the rhythm of his activity level,
  broken up by drinks, snacks, toilet visits, deliveries and phone calls.
  Between 08:00 and 21:59 the phone rings on its own with a 2% chance per
  game minute (about once an hour).
- **12:00 and 18:00:** he cooks and eats a meal (`cookMeal`).
- **23:00, night routine** (`nightRoutine`): shower; undress (his clothes take
  on a skin tone); a snack from the cabinet; brush teeth; into bed.
- **23:00-05:00:** 18 hours after waking, the idle picker only ever sends
  him to bed, and while he is asleep it does nothing.  Needs, events and
  requests still wake him; afterwards he goes back to bed.

When he is moderately sick or worse, the sleep tier applies all day: he
stays in bed except to drink, eat, use the toilet, answer the door, follow
his daily schedule or carry out a request.

## Interacting with him

The player never controls the resident directly.  Everything goes through the
keyboard, read every tick by `gameTick` and dispatched by `handleKey`
([`parts/handleKey.c`](../source/parts/handleKey.c)).  While he writes a letter
or plays a game the keyboard belongs to that activity (`keysBlocked`); during
the move-in it is ignored.

### Control keys

| Key | What happens |
|---|---|
| **Ctrl-A** | The alarm clock rings (`alarmRinging`).  He walks to it and switches it off at his next decision (rung 2). |
| **Ctrl-B** | Doorbell; a **book** arrives.  He fetches it from the front door and puts it on the bookshelf by the computer (`bookDelivery`). |
| **Ctrl-C** | The **phone** rings (ignored while he is on the phone).  He goes to the armchair beside it, picks up the receiver and chats for 40..50 rounds (`answerPhone`). |
| **Ctrl-D** | Doorbell; **dog food** arrives.  If the dog's bowl is empty he fills it, otherwise he stores the package in the fridge (`dogFoodDelivery`). |
| **Ctrl-F** | Doorbell; **food** arrives and he restocks the kitchen cabinet to 4 packs (`foodDelivery`).  Refused, silently, while the cabinet is full. |
| **Ctrl-P** | **Pat him.**  Only while he crouches or sits at the armchair by the phone (`patAllowed`): a hand reaches in and pats him, and he becomes happy. |
| **Ctrl-R** | Doorbell; a **record** arrives.  He takes it to the record player; his collection grows by one (`recordDelivery`).  Ignored while he has walked away from a game. |
| **Ctrl-W** | One unit of **water** is added to the tank (up to 10), with the tap sound. |
| Return | Submits the typed line (see below). |
| Backspace, cursor-left | Erase the last typed character. |
| F1..F10 | Used by the minigames ([GAMES.md](GAMES.md)). |

The deliveries and the phone go into the event queue (10 entries), so they
come first at his next decision and can cut short what he is doing.  When he
is asleep, an event gets him out of bed first.

The armchair by the phone is where he can be patted: `crouchForPat` walks him
there and crouches (it is also the first step of answering the phone), and he
stays pattable while crouching there (`ACTION_WAIT_FOR_PAT`, `waitForPat`) or
sitting there reading (`ACTION_READ_IN_ARMCHAIR`, `readInArmchair`).  The dog
plays no part in any of this (see [DOG.md](DOG.md)).

### Typed requests

Printable keys build a line of up to 38 characters at the top of the screen.
It disappears 20 seconds after the last key, and the next key then starts a
new line.  Return hands it to
`matchCommand` ([`parts/matchCommand.c`](../source/parts/matchCommand.c)):

1. Every word is looked up in the 161-word `vocabulary`
   ([`dat_parser.c`](../source/dat_parser.c)).  A known word sets one bit in a
   10-byte mask; words the game does not know are ignored.
2. The 33 rows of `phraseTable` are tried in order.  A row matches when the
   mask contains all the bits it needs -- one word from each of its groups,
   in any order, among any other words.  The first match wins.
3. The request gets a **priority**: 0..3 at random, plus 3 if he is happy,
   1 if content, 0 if sad, plus the row's own bonus.

The request then waits in a 10-entry queue (`queueActions`) and is looked at
when his decision ladder reaches rung 10:

- **priority 8 or more:** he does it.  Before playing a game or the organ he
  nods in agreement.
- **4 to 7:** not yet.  He does an idle activity instead, and the priority
  rises by one -- so it is done after 1 to 4 more activities.
- **below 4:** refused and dropped.

A happy resident is therefore much more obliging than a sad one.

| Example | Needs one word from each group | Activity | Bonus |
|---|---|---|---|
| LIGHT THE FIRE | {LIGHT, START, MAKE, BURN, IGNITE, BUILD} {FIRE, FIREPLACE, LOG} | light the fire | 4 |
| YOU LOOK COLD | {YOU} {SEEM, LOOK, APPEAR} {CHILLY, COLD} | light the fire | 2 |
| USE THE FIREPLACE | {PLAY, PERFORM, USE, TRY, PLAYING} {FIRE, FIREPLACE, LOG} | light the fire | 4 |
| PUT ON A RECORD | {HEAR, LISTEN, PUT, SPIN} {STEREO, TURNTABLE, MUSIC, RECORD, PLATTER} | play a record | 4 |
| YOU SHOULD CLEAN UP | {CLEAN, TIDY, PICK} {UP} {SHOULD, OUGHT} | clean up | 8 |
| PLAY THE PIANO | {PLAY, PERFORM, USE, TRY, PLAYING} {PIANO, ORGAN} | play the organ | 4 |
| PLAY A SONG | {PLAY, ...} {SONG, TUNE, SONATA, FUGUE, SERENADE, JAZZ, BOOGIE} | play the organ | 4 |
| TICKLE THE IVORIES | {TICKLE} {IVORIES} | play the organ | 4 |
| WRITE A LETTER | {TYPE, TELL, WRITE, CONFIDE} {PROBLEM, PROBLEMS, TROUBLES, MATTER, LETTER, NOTE} | write a letter | 8 |
| WHAT IS THE MATTER | {LOOKS, IS, SEEMS, APPEARS} {PROBLEM, ..., NOTE} | write a letter | 6 |
| BRUSH YOUR TEETH | {BRUSH, FLOSS} {TEETH, HYGIENE} | brush teeth | 2 |
| MESSY TEETH | {SLOPPY, MESSY, UNTIDY} {TEETH, HYGIENE} | brush teeth | 2 |
| DRINK SOME WATER | {DRINK, IMBIBE} {WATER, LIQUID, LIQUIDS, FLUID, FLUIDS} | drink | 2 |
| YOU SEEM TO NEED A GLASS | {SEEM, LOOK, APPEAR} {GLASS, COOLER} | drink | 4 |
| FEED THE DOG | {FEED} {DOG, PET, MUTT, POOCH} | *nothing* | 8 |
| FILL THE BOWL | {FILL} {BOWL, DISH, CAN} | *nothing* | 8 |
| OPEN A CAN | {OPEN} {BOWL, DISH, CAN} | *nothing* | 8 |
| DANCE | {DANCE, MOON, SHOW} | dance | 2 |
| I'M TIRED OF THE MUSIC | {TIRED, BORED, APATHETIC} {STEREO, ..., PLATTER} | stop the record | 8 |
| I HATE THIS MUSIC | {HATE, AWFUL} {STEREO, ..., PLATTER} | stop the record | 8 |
| PLAY CARDS | {PLAY, ...} {GAME, CARDS, POKER, WAR, CARD, ANAGRAMS, BLACKJACK} | play a game | 8 |
| DUST ADDITION | {ALLERGY, ALLERGIC, FEVER, DUST, POLLEN, HANKY} {ADDITION, SUBTRACTION, MULTIPLICATION, DIVISION} | nod | 6 |
| USE THE COMPUTER | {COMPUTER, ATARI} | use the computer | 6 |
| WHAT IS IN THE UPSTAIRS CLOSET | {WHAT, WHAT'S} {IN, INSIDE, STORED, KEEP} {UPSTAIRS} {CLOSET} | go into the study | 6 |
| WHAT IS IN THE BEDROOM CLOSET | ... {BEDROOM} {CLOSET} | change clothes | 6 |
| WHAT IS IN THE KITCHEN CABINET | ... {KITCHEN} {CABINET} | eat | 6 |
| WHAT IS IN THE FILING CABINET | ... {FILING} {CABINET} | play a game | 6 |
| WHAT IS IN THE FREEZER | ... {FREEZER} | eat | 6 |
| WHAT IS IN THE FRIDGE | ... {REFRIDGERATOR, FRIDGE} | eat | 6 |
| WHAT IS IN THE DRESSER | ... {DRESSER} | change clothes | 6 |
| WHAT IS IN THE NIGHTSTAND | ... {NIGHTSTAND} | change clothes | 6 |

Quirks, all in the 1985 data and code:

- **Please helps.**  `matchCommand` treats the word with index 0 as
  "unknown" -- and that word is PLEASE, which instead of setting a bit adds 4
  to the priority.  Every PLEASE in the line adds 4.
- **FEED THE DOG does nothing.**  Its three rows ask for
  `ACTION_EVENT_DOG_FOOD`, an event id; the request is accepted, waits its
  turn and is then ignored by `runAction`.
- **Two rows can never match.**  Row 0 (`ACTION_HELLO`) needs a bit no word
  supplies, and row 6 (MESSY ... HOUSE, clean up) needs one only the duplicate
  spelling of IS would supply -- the earlier IS always wins the lookup.  START,
  LIKE and IS are listed twice; only their first entries are ever used.
- **Refused requests leak queue slots.**  Dropping a low-priority request
  shifts the queue but does not lower `queueCount`.  Each refusal leaves the
  queue looking one entry longer, stale entries behind the real ones are then
  examined as if they were requests, and after ten refusals in one session the
  queue counts as full and new requests are ignored until the game restarts.

### Saving

There is no save command.  Whenever he goes into the study
(`ACTION_OPEN_UPSTAIRS_CLOSET`: an idle activity in all three tiers, or the
UPSTAIRS CLOSET request) he writes `HYBER` while he is inside
(`enterStudy(1)` -> `studyVisit`).  A new resident is saved at the end of the
move-in.  When the game starts with a `HYBER` present, he simply comes out of
the study.

### The move-in

A new resident arrives in a cutscene (`moveInScene`,
[`parts/moveInScene.c`](../source/parts/moveInScene.c)): after an empty
house and two doorbells he walks in through the front door, tours the house
-- kitchen cabinet, sink, fridge, TV, study, alarm clock, dresser, closet,
toilet, bathroom sink, computer, filing cabinet -- fetches his suitcase from
the front step and unpacks it in the dresser.  Then the dog is let in, he
changes, and he goes into the study, which saves the game.  Events, typing
and the random phone are all off until it ends (`movingIn`).

## What he does: the activities

`runAction` dispatches on the action id (`ACTION_*`,
[`include/enums.h`](../source/include/enums.h)).  "Idle" means he picks it
himself from the tier tables; "request" that a typed line can ask for it.

| Id | Action | Routine | What he does | Comes from |
|---|---|---|---|---|
| 0 | `SIT_AND_EXERCISE` | `exercise` | arm exercises on the bedroom rug, 8..127 steps | idle |
| 1 | `READ_NEWSPAPER` | `readNewspaper` | switches the TV on, reads the paper in the blue armchair upstairs, switches it off | idle |
| 2 | `PLAY_COMPUTER` | `useComputer` | types at the computer; now and then its screen shows an animation | idle, request |
| 3 | `WASH_HANDS` | `washHands` | washes his hands at the bathroom sink | never |
| 4 | `GET_IN_OUT_OF_BED` | `getInOutOfBed` | goes to bed, or gets up | idle (sleep tier), routines |
| 5 | `LISTEN_SONG` | `playRecord` | puts a random record from his collection on the record player | idle, request |
| 6 | `STOP_RECORD` | `stopRecord` | stops the record | idle, request |
| 7 | `WRITE_LETTER` | `writeLetter` | types a letter to the player at the typewriter upstairs (see below) | idle, request |
| 8 | `DANCE` | `danceToMusic` | dances by the record player while music plays, starting a record if none is on | idle, request |
| 9 | `YAWN_AND_STRETCH` | `yawnAndStretch` | yawns and stretches | never |
| 10 | `PACE_NERVOUSLY` | `paceNervously` | paces on the spot | idle |
| 11 | `WANDER_IDLY` | `idleShrug` | shrugs on the spot | -- (games, move-in) |
| 12 | `SLEEP` | `dozeOff` | walks to the middle of the floor and dozes off, snoring, for 7..15 rounds | idle |
| 13 | `DRINK` | `drinkWater` | takes a glass from the kitchen to the water cooler and drinks | need, request |
| 14 | `NOD_HEAD` | `nodHead` | nods | request |
| 15 | `PEEK_AROUND` | `peekAround` | glances aside | -- (War) |
| 16 | `PLAY_A_GAME` | `playGame` | offers the five-game menu ([GAMES.md](GAMES.md)) | idle, request |
| 17 | `BRUSH_TEETH` | `brushTeeth` | brushes his teeth at the bathroom sink | routines, request |
| 18 | `KITCHEN_CABINET` | `eatFromCabinet` | eats a pack from the cabinet at the kitchen table | need, request |
| 19 | `SIT_ON_COUCH_WITH_DOG` | `readInArmchair` | reads a book in the armchair by the phone, 30..50 rounds; pattable | idle |
| 20 | `LIGHT_FIREPLACE` | `lightFire` | fetches wood from outside the front door and lights the fire, which burns for 2500..5000 ticks | idle, request |
| 21 | `USE_TOILET` | `useToilet` | uses the toilet | need |
| 22 | `TAKE_SHOWER` | `takeShower` | showers | routines |
| 23 | `FEED_DOG` | `feedDog(0)` | takes dog food from the fridge, fills the bowl, puts the package back | idle |
| 24 | `HELLO` | `sayHello` | waves and talks to the player | idle |
| 25 | `EAT_MEAL` | `cookMeal` | cooks a meal on the stove and eats it | schedule |
| 26 | `PLAY_ORGAN` | `playOrgan` | plays a random `.ORG` piece on the organ upstairs | idle, request |
| 27 | `OPEN_UPSTAIRS_CLOSET` | `enterStudy(1)` | goes into the study and saves the game | idle, request |
| 28-32 | `EVENT_*` | -- | outside events, run through `runEvent`; `runAction` ignores them | events |
| 33 | `GET_SNACK_FROM_FRIDGE` | `goToFridge` | puts something into the fridge | -- (dog food, move-in) |
| 34 | `OPEN_BEDROOM_CLOSET` | `changeClothes(0)` | changes his clothes in the bedroom closet | request, routines |
| 35 | `NOD_OK` | `nodOk` | nods in agreement | -- (requests, move-in) |
| 36 | `CLEAN_UP` | `cleanUp` | closes every door and cabinet left open | idle, request |
| 37 | `TIDY_HOUSE` | `tidyHouse` | rummages in the filing cabinet | idle |
| 38 | `CHECK_FRONT_DOOR` | `checkFrontDoor(40)` | steps out of the front door for a while | idle |
| 39 | `TOGGLE_TV` | `toggleTv` | switches the TV on or off | idle |
| 40 | `CALL_DOG` | `crouchForPat` | crouches by the armchair; pattable | -- (phone, reading, waiting to be patted) |
| 41 | `WAKE_FROM_ALARM` | `wakeFromAlarm` | switches the alarm clock off | alarm |
| 42 | `PET_DOG` | `waitForPat` | crouches by the armchair and waits 100..200 ticks to be patted | idle |
| 43 | `WAKE_UP_MORNING` | `morningRoutine` | the morning routine | schedule |
| 44 | `GO_TO_BED_NIGHT` | `nightRoutine` | the night routine | schedule |

"--": nothing chooses the action itself, but its routine runs as a step of
the activities named.  "never": the routine is reachable only through its
action id, and nothing ever asks for it.

**Letters.**  `writeLetter` loads `LETTER.TXT` and types the date, "Dear
<owner>,", two to four paragraphs from its four sections in shuffled order --
the wording picked by his mood, or a sick variant when he is ill -- a random
sign-off and his name, one character at a time at the typewriter.

**Events** (`runEvent`): `bookDelivery`, `recordDelivery`, `foodDelivery`
(dropped if the cabinet is full), `answerPhone`, `dogFoodDelivery`.

## Older names

The first Ghidra analysis named several of these things after what it
guessed: the armchair activities were "call the dog", "pet the dog" and "sit
on the couch with the dog", the record collection was a food supply, and
several desk positions were fireplace spots.  They were renamed on 2026-10-05
(`callDog` -> `crouchForPat`, `petDog` -> `waitForPat`, `sitWithDog` ->
`readInArmchair`, `foodSupply` -> `recordCount`, and the `ACTION_*`, `STATE_*`,
`SPRITE_*` and `POS_*` names with them); older notes and the Ghidra analysis
documents may still use the old ones.  `source/tools/renames.tsv` maps every
old name to the current one.

## Moving around

### The house

| Floor | `floorOfY` | Y range | Contents, left to right |
|---|---|---|---|
| Top | `FLOOR_TOP` (3) | y <= 77 | TV, record player, blue armchair, organ, study door, writing desk with typewriter, filing cabinet |
| Middle | `FLOOR_MIDDLE` (2) | 78..140 | bedroom (bed, alarm clock, closet, dresser), stairs up, bathroom (sink, toilet door, bathtub with shower), computer corner (bookshelf, clock, calendar, computer) |
| Bottom | `FLOOR_BOTTOM` (1) | y > 140 | kitchen (dog bowl, stove, fridge, food cabinet, sink, table, water cooler), stairs up, living room (phone, red armchair, fireplace, front door) |

Walking happens at a floor's walking line, `floorWalkY` = 198, 135 and 71.
Staircases connect the bottom floor to the middle (foot at (170,185)) and the
middle floor to the top (arriving at (182,72)); `stairWaypts`, `xLanding` and
`yLanding` ([`dat_world.c`](../source/dat_world.c)) hold their ends.

The house has 48 named spots, 16 per floor.  `posToXY`
([`movement.c`](../source/movement.c)) turns one into screen coordinates: x is
`posXHalf[i] * 2`, y the floor's base line (77, 140, 202) minus
`posYOffset[i]`.  The spots actually used:

| Spot | Where | Used for |
|---|---|---|
| `POS_TOP_LIVING_ROOM` (0) | in front of the TV | switching the TV on and off |
| `POS_TOP_DANCE_FLOOR` (1) | in front of the record player | records, dancing |
| `POS_TOP_ARMCHAIR` (2) | the blue armchair | reading the newspaper |
| `POS_TOP_ORGAN` (6) | the organ | playing the organ |
| `POS_TOP_STUDY_DOOR` (7) | the study door | the study, saving |
| `POS_TOP_DESK_CHAIR` (10) | the writing desk | letters |
| `POS_TOP_FILING_CABINET` (12) | the filing cabinet | games, paper, tidying |
| `POS_MID_RUG` (17) | the bedroom rug | exercising |
| `POS_MID_BED` (18) | the bed | sleeping |
| `POS_MID_BEDROOM_WALK` (19) | by the alarm clock | the alarm |
| `POS_MID_BEDROOM_CLOSET` (20), `POS_MID_DRESSER` (21) | closet, dresser | changing |
| `POS_MID_BATHROOM_SINK` (22) | the bathroom sink | washing, teeth |
| `POS_MID_TOILET_DOOR` (23) | the toilet door | the toilet |
| `POS_MID_SHOWER_DOOR` (25), `POS_MID_SHOWER_INSIDE` (24) | the bathtub with shower | showering |
| `POS_MID_BOOKSHELF` (27) | the bookshelf | putting books away |
| `POS_MID_COMPUTER_DESK` (29) | the computer | the computer |
| `POS_BTM_DOG_BOWL` (33) | the dog bowl | feeding the dog |
| `POS_BTM_STOVE` (34), `POS_BTM_FRIDGE` (35) | stove, fridge | cooking, the fridge |
| `POS_BTM_KITCHEN_SINK` (36), `POS_BTM_KITCHEN_CABINET` (37) | sink, food cabinet | the glass, food |
| `POS_BTM_TABLE_LEFT` (38), `POS_BTM_TABLE_RIGHT` (39) | the kitchen table | meals, games |
| `POS_BTM_WATER_TAP` (41) | the water cooler | drinking |
| `POS_BTM_ARMCHAIR` (43) | the red armchair by the phone | the phone, reading, being patted |
| `POS_BTM_FIREPLACE_LOGS` (45) | the fireplace | lighting the fire |
| `POS_BTM_FRONT_DOOR` (46) | the front door | deliveries, firewood, going out |

The dog uses a few more (see [DOG.md](DOG.md)); 3, 4, 8, 9, 13..16, 26, 28,
30, 31, 40, 42 and 44 are used by nothing.

### Walking

An activity sets `walkXTarget`/`walkYTarget` (usually with `posToXY`) and calls
`walkToTarget` ([`walk.c`](../source/walk.c)), which repeats `walkStep`
([`parts/walkStep.c`](../source/parts/walkStep.c)) until he arrives (returning
0) or an event interrupts the walk (returning -1, see "Interruptions").

- `nextWaypoint` ([`parts/nextWaypoint.c`](../source/parts/nextWaypoint.c))
  routes him: on the same floor straight to the target, otherwise to the foot
  or head of the staircase first, one flight at a time.
- On a floor he moves one pixel per tick horizontally and keeps to the floor's
  walking line, only turning towards the target's y when he is within 8
  pixels of it horizontally.  The walk cycle runs through states 0..7.
- On the stairs he moves diagonally through the climb (9..12), top-landing
  (13..16), descent (17..20) and bottom-landing (21..24) states.
- His footsteps sound different on carpet, wooden floor and stairs
  (`playFootstep`, see [SOUND.md](SOUND.md)).
- While he is sick every step takes an extra tick: half speed.

## Source reference

| Topic | Source |
|---|---|
| Main loop | [`parts/gameLoop.c`](../source/parts/gameLoop.c) |
| Decision ladder | [`parts/chooseAction.c`](../source/parts/chooseAction.c) |
| Action and event dispatch | [`actions.c`](../source/actions.c), [`ai.c`](../source/ai.c) |
| Idle picker and its tables | [`airandom.c`](../source/airandom.c), [`dat_aitables.c`](../source/dat_aitables.c) |
| Clock, needs, mood | [`sim.c`](../source/sim.c), [`health.c`](../source/health.c) |
| A new resident | [`parts/rollResident.c`](../source/parts/rollResident.c) |
| Keyboard | [`tick.c`](../source/tick.c), [`parts/handleKey.c`](../source/parts/handleKey.c), [`parts/getKey.c`](../source/parts/getKey.c) |
| Event queue | [`parts/queueEvent.c`](../source/parts/queueEvent.c), [`parts/nextEvent.c`](../source/parts/nextEvent.c) |
| Typed requests | [`parts/submitCommand.c`](../source/parts/submitCommand.c), [`parts/matchCommand.c`](../source/parts/matchCommand.c), [`parts/lookupWord.c`](../source/parts/lookupWord.c), [`dat_parser.c`](../source/dat_parser.c) |
| Activities | one file each in [`parts/`](../source/parts/), named after the routine |
| Walking | [`walk.c`](../source/walk.c), [`parts/walkStep.c`](../source/parts/walkStep.c), [`parts/nextWaypoint.c`](../source/parts/nextWaypoint.c), [`movement.c`](../source/movement.c) |
| Save file | [`parts/studyVisit.c`](../source/parts/studyVisit.c), [`parts/loadSavedGame.c`](../source/parts/loadSavedGame.c), [`parts/saveFile.c`](../source/parts/saveFile.c) |
