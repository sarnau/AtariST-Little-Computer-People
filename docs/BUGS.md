# Little Computer People — Known Bugs

Every defect known in the 1985 Atari ST release, as found while porting it.
The C port in [`source/`](../source/README.md) compiles to a binary
byte-identical to the shipped `LCP.PRG`, so **all of these are kept on
purpose**: fixing one would change the binary.  Names are the port's; the
links point at the code that shows each bug.

Each entry is tagged with what the player notices:

- **Visible** / **Audible** -- the player can see or hear it.
- **Invisible** -- wrong, but nothing on screen or in the speaker changes.
- **Can crash** -- could hang or crash the machine in some circumstances.

Some entries may be design decisions rather than mistakes; those say so.

## The resident

### Sickness is never capped

**Invisible, mostly.**  When he gets worse past `SICKNESS_CRITICAL` (4),
`simStep` was meant to clamp the level, but the line is a comparison, not an
assignment (`resident.sicknessLevel == SICKNESS_CRITICAL;`), so the level goes
on to 5, 6, ...  Nothing looks at the level above 4; recovery just takes 5
more minutes per extra level.  It is rarely reached anyway, see the next
entry.  [`sim.c`](../source/sim.c)

### Falling sick again makes him less sick

**Visible (timing).**  `fallSick` sets the level to `SICKNESS_MILD` every time
a need runs out at severe -- also when he is already moderately, severely or
critically sick.  A neglected resident keeps being knocked back to mild, so he
can never become very sick.  Possibly intended.
[`health.c`](../source/health.c)

### Eating does not restart the hunger timer

**Visible (timing).**  Drinking sets thirst to satisfied **and** restarts the
thirst timer; eating sets hunger to satisfied but leaves the hunger timer
running, so after a meal he gets hungry again sooner than the timer's length.
[`parts/eatFromCabinet.c`](../source/parts/eatFromCabinet.c), compare
[`parts/drinkWater.c`](../source/parts/drinkWater.c)

### Drinking from an empty tank still quenches his thirst

**Visible.**  With the water tank empty he goes to the cooler, goes through
the motions, and his thirst is reset all the same: the reset sits outside the
`waterLevel != 0` test.  A healthy resident even chooses to drink with an
empty tank; only a sick one checks the water first.
[`parts/drinkWater.c`](../source/parts/drinkWater.c),
[`parts/chooseAction.c`](../source/parts/chooseAction.c)

### Weekends fall on the wrong days

**Visible (behaviour).**  On Sundays and Saturdays his active spells are
meant to become relaxed and moderate (`pickIdleAction`).  The weekday comes
from `calcWeekday`, which has two bugs:

- for every month already past it adds the length of the **current** month
  (`daysInMonth(t_mon, ...)` instead of `daysInMonth(i, ...)`);
- its year loop counts 1900 as a leap year, so every date after February 1900
  comes out one day late -- 1 January 1985 (a Tuesday) is computed as a
  Wednesday.

[`parts/calcWeekday.c`](../source/parts/calcWeekday.c),
[`airandom.c`](../source/airandom.c)

### Idle "nothing" entries

**Visible.**  Each of the three idle-activity tables contains
`ACTION_EVENT_PHONE_CALL` (the moderate one twice) -- an event id that
`runAction` has no case for.  Drawing it does nothing; a sleeping resident is
even woken up for it first.
[`dat_aitables.c`](../source/dat_aitables.c), [`actions.c`](../source/actions.c)

### A second phone call on a silent phone

**Visible.**  Ctrl-C and the random daytime call both check that he is not
already **on** the phone, but not that it is already **ringing**.  Pressing
Ctrl-C twice, or a random call arriving while it rings, queues a second call:
after the first conversation he walks back, picks up the silent phone and
talks again.  [`parts/handleKey.c`](../source/parts/handleKey.c),
[`sim.c`](../source/sim.c)

### Records that never stop

**Visible.**  `recordPlaying` is cleared only by `stopRecord`, not when the
song ends.  After a record has played out, the record player's needle keeps
sweeping in silence, he refuses to put on another record ("already playing"),
and DANCE starts no music, so he dances for no time at all -- until he
happens to pick STOP RECORD.  The flag is saved, so a game saved while a
record played starts with a silent, spinning record player.
[`parts/playRecord.c`](../source/parts/playRecord.c),
[`parts/danceToMusic.c`](../source/parts/danceToMusic.c),
[`parts/stopRecord.c`](../source/parts/stopRecord.c)

### More records than songs

**Audible.**  The record picked is the n-th `.SNG` file on the disk, found by
calling `Fsnext` n-1 times.  With more records in his collection than song
files (every record delivery adds one), the search runs off the end, the
directory entry keeps the last name, and every extra pick plays the last song
on the disk.  [`parts/playRecord.c`](../source/parts/playRecord.c)

### Small slips in his activities

- **Audible:** closing the fridge after putting food away plays the door-open
  sound. [`parts/putInFridge.c`](../source/parts/putInFridge.c)
- **Visible:** before closing the study door, `cleanUp` computes
  `headTarget - 12` and throws the result away (`-=` was surely meant), so the
  head turn uses the old target. [`parts/cleanUp.c`](../source/parts/cleanUp.c)
- **Invisible:** `readNewspaper` adds 0 to his x position, a lost offset.
  [`parts/readNewspaper.c`](../source/parts/readNewspaper.c)
- **Invisible:** `washAtSink` and `washHands` compare against `lastPick`
  before ever setting it, and `danceToMusic` steps an uninitialised `i`; only
  the first pose of each is affected.
  [`parts/washAtSink.c`](../source/parts/washAtSink.c),
  [`parts/danceToMusic.c`](../source/parts/danceToMusic.c)

## Typed requests

### Two requests can never work

**Visible.**  Row 0 of `phraseTable` (HELLO, EXCUSE ME, ...: wave hello)
needs a bit no word sets.  Row 6 (MESSY ... IS ... HOUSE: clean up) needs the
bit of the **second** IS in the vocabulary, but the word lookup always finds
the first.  START and LIKE are listed twice as well; their second entries are
never found.  [`dat_parser.c`](../source/dat_parser.c)

### FEED THE DOG does nothing

**Visible.**  FEED THE DOG, FILL THE BOWL and OPEN A CAN are accepted and
queued, but the rows name `ACTION_EVENT_DOG_FOOD` -- the delivery event --
instead of `ACTION_FEED_DOG`, and `runAction` has no case for it.
[`dat_parser.c`](../source/dat_parser.c)

### Refused requests fill the queue for good

**Visible.**  A request whose priority is below 4 is dropped by shifting the
queue, but `queueCount` is not lowered.  Each refusal makes the queue look one
entry longer; stale entries behind the real ones are then treated as
requests, and after ten refusals in one session every new request is ignored
until the game is restarted.
[`parts/chooseAction.c`](../source/parts/chooseAction.c),
[`parts/submitCommand.c`](../source/parts/submitCommand.c)

### PLEASE works by accident

**Visible, harmless.**  `matchCommand` treats word index 0 as "unknown", but
index 0 is PLEASE (unknown words are -1).  PLEASE therefore sets no bit and
instead adds 4 to the priority -- every PLEASE in the line.  It reads like a
feature, but it rests on the sentinel clash.
[`parts/matchCommand.c`](../source/parts/matchCommand.c)

### Words he knows but never acts on

**Visible.**  19 vocabulary words set a bit that no row tests: DO, LIKE,
ENJOY, WILL, WOULD, RELAX, ON, PROGRAM, UTILITIES, MATH, HOMEWORK, ADD,
SUBTRACT, MULTIPLY, DIVIDE, GET, IF, TV, CHAIR.  So WATCH TV, RELAX or DO
YOUR HOMEWORK can never be requested -- remains of cut requests.  The one
row with arithmetic words pairs them with allergy words (DUST ... ADDITION:
nod), which looks like two rows merged.
[`dat_parser.c`](../source/dat_parser.c)

### REFRIDGERATOR

**Visible.**  The vocabulary spells it REFRIDGERATOR, so typing
REFRIGERATOR is not understood (FRIDGE works).
[`dat_parser.c`](../source/dat_parser.c)

## The dog

### Eating permission that outlives its trip

**Visible, minor.**  The dog may eat only after picking the spot beside the
bowl (`dogMayEat`).  If the bowl is empty when it gets there, the permission
is kept, and the dog eats the next time it stands still in the bowl corner
with food in the bowl -- also at the bowl spot itself, which is meant not to
grant it.  [`parts/renderFrame.c`](../source/parts/renderFrame.c)

### One meal always empties the bowl

**Visible, possibly intended.**  A meal lowers the bowl four times (at 60, 30
and 4 frames left and at the end) for only two levels, so even a full bowl is
always emptied.  [`parts/renderFrame.c`](../source/parts/renderFrame.c)

### It sometimes faces the wrong way

**Visible.**  When the dog moves only vertically on a floor, `moveDog` never
sets its mirror flag and draws the frame with whatever the stack held, so it
can flip direction at random.  [`parts/moveDog.c`](../source/parts/moveDog.c)

## The minigames

### Every word game and letter leaks memory

**Can crash (eventually).**  `unpackFile` frees the compressed buffer through
the pointer it has already advanced; TOS rejects that, so every Anagrams or
Word Puzzle game and every letter loses 1..8 KB.  A very long session could
end in "out of memory".  It also decodes past the end of the data it read
(WORDS is asked for 10 000 bytes and holds 2344), which is harmless on an ST.
[`parts/unpackFile.c`](../source/parts/unpackFile.c)

### Anagrams

- **Visible:** only the first 150 of the 213 words in `WORDS` are ever used
  (`rndRng(0, 149)`); nothing after PERUSE comes up.  The first unused word,
  PHAROAH, is misspelt.
- **Visible:** a clue taken on the last guess shows "Guess #9?", but there is
  no ninth guess; `anaExtraGuess` never takes effect.

[`games.c`](../source/games.c), [GAMES.md](GAMES.md)

### War

- **Visible:** there is no way to quit during a war (a tie); only the idle
  timeout gets out.
- **Invisible:** `dispPlyrChips;` at the top of each round lacks its
  parentheses, so the call never happens; the count is redrawn elsewhere
  before it matters.

[`games.c`](../source/games.c)

### Poker

- **Visible, rare:** in a tie between two straights (or straight flushes) the
  highest card decides, and the ace of A-2-3-4-5 sorts last, so the wheel
  beats a 6-high straight.
- **Visible, rare:** the two-pair tie-break finds the low pair only when the
  high pair comes first in the hand; otherwise it compares against a 2.
- **Visible:** the computer bets on the hand he held **before** drawing.
- **Visible:** "Sorry, I,m all out." -- a comma for the apostrophe.  The
  all-out exits also leave without settling the pot, and the raise counter
  keeps counting presses past the 20-chip cap but misses the press that
  spends the player's last chip.
- **Invisible:** when the computer draws a card that is already out, he
  pushes his old card onto the discard pile again before retrying; with
  enough retries the pile can run past its 13 slots into `letterLines`.

[`games.c`](../source/games.c), [GAMES.md](GAMES.md)

### Blackjack

- **Visible, possibly intended:** the dealer stops after three hits whatever
  his total, and a dealer who goes broke ends the game only on the natural
  blackjack path; otherwise a win he cannot cover is paid in part and play
  goes on.
- **Invisible:** the F5 Clear flag lives in poker's discard pile
  (`pkrDiscPile[10]`).

[`games.c`](../source/games.c)

### Word Puzzle

- **Visible:** puzzle "101 @D." expects DALMATIONS; the correct DALMATIANS is
  marked wrong.
- **Visible:** any key code from 'A' up is taken as a letter, so F1..F9 put
  stray characters into an answer.

[`games.c`](../source/games.c), `DATA/WORDPZ.TXT`

## Sound and music

### The sound-effect buffer overrun

**Invisible in the shipped game; can crash in any other memory layout.**
`startSfx` copies each effect into a 56-byte buffer, but three effects are
longer (head nod and toilet refill 148 bytes, water tap 60), overrunning it by
up to 92 bytes.  In the 1985 binary the overrun lands on two variables nothing
reads and then an unused gap, which is why it shipped; in the port's test
layout it overwrote the object height table and crashed in the VDI.  Whether
the original buffer really was 56 bytes, or one 400-byte block with no
overrun, cannot be told from the binary.
[`globals.c`](../source/globals.c), [`sfx_irq.c`](../source/sfx_irq.c)

### The organ's anthem turns the music down

**Audible.**  `parseSongHeader` turns a song's velocity into a PSG volume.
Its last step, `defVelocity < 0x80`, compares a signed byte with -128 and is
never true, so velocities 0x67..0x7F set no volume and keep the previous
song's.  Every song on the disk uses 120 or 127 except STARSPAN.ORG (91,
volume 13): once he has played it on the organ, every later song plays at 13
instead of 15 until the game restarts.
[`parts/parseSongHeader.c`](../source/parts/parseSongHeader.c)

### A song header it cannot read hangs it

**Can crash (bad data only).**  `parseSongHeader` has no default case: an
unknown header byte leaves the pointer where it is and the loop never ends.
The shipped songs are fine.  [`parts/parseSongHeader.c`](../source/parts/parseSongHeader.c)

### GEMDOS runs out of folder buffers

**Can crash (long sessions, unconfirmed).**  Every record starts with
`Fsfirst("*.sng")`.  In the port's tests, a build whose songs never finish
made him retry the record over and over, and after a few hundred searches
TOS halted with "OUT OF INTERNAL MEMORY".  Those runs used Hatari's
emulated hard-disk folder; whether a real floppy behaves the same, and
whether normal play ever gets that far, is untested.
[`parts/playRecord.c`](../source/parts/playRecord.c), [history.md](history.md)

## The screen

### Out-of-bounds reads and writes that happen to work

- **Invisible:** the stroking-hand animation (Ctrl-P) reads `patSprites` one
  past its end; it gets `patLastSprite`, the next variable, which is the
  frame it needs. [`tick.c`](../source/tick.c),
  [`dat_anim.c`](../source/dat_anim.c)
- **Invisible:** a carried object that is momentarily hidden sits in slot 9
  of 8-entry arrays, so `pendY[9]` writes land in `drawnWidth[1]`, which is
  harmless. [`include/sprglobs.h`](../source/include/sprglobs.h)
- **Invisible:** the left-edge clamp for a carried object indexes `pendX` by
  the sprite id instead of its slot, so the object itself is never clamped
  and an unrelated value is zeroed if it happens to be negative.
  [`tick.c`](../source/tick.c)

### Pieces drawn one pixel off

**Invisible.**  The house picture has the toilet door, closet door, stove,
dog bowl and fireplace one pixel away from where the game paints them; the
start-up paints over them, so the difference never shows.
[RENDERING.md](RENDERING.md)

## Copy protection

**Visible, by design.**  When the check fails the resident sleeps for ever.
It also fails under every emulator, even with the original disk image, and
it only selects a drive correctly when the game was started from A: or B:.
[`cp_asm.s`](../source/cp_asm.s), [ARCHITECTURE.md](ARCHITECTURE.md)

## Dates

**Visible after 1999.**  The year is entered as two digits and taken as
19xx, and every year divisible by four counts as a leap year, 1900 included.
[`parts/titleScreen.c`](../source/parts/titleScreen.c),
[`parts/daysInMonth.c`](../source/parts/daysInMonth.c)

## Unused content

Not bugs, but cut or forgotten parts of the game:

- activities no one chooses: `washHands`, `yawnAndStretch`;
- `personalityType` is rolled and saved but never read;
- 15 of the 48 house positions are never used ([PEOPLE.md](PEOPLE.md));
- 10 of the 56 objects are never drawn: a bathroom sink animation, the
  medicine cabinet, a mirror patch and the typewriter
  ([RENDERING.md](RENDERING.md));
- sound effects `FOOTSTEP_3`..`5` are never played;
- `stopSequencer`, `unhookTimerA`, `gameCleanup` and `parseNumber` are never
  called.
