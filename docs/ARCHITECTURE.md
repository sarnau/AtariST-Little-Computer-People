# Little Computer People — Architecture

*Little Computer People* (Activision, 1985; design by David Crane and Rich
Gold) is one of the first life simulations: a small person moves into a
three-floor house on the screen and lives there, on a schedule of his own,
with a dog.  The player watches, sends deliveries, calls him, pats him, types
requests to him, and plays card and word games with him.

This document is the overview.  The details are in:

| Document | Contents |
|---|---|
| [PEOPLE.md](PEOPLE.md) | the resident: needs, mood, sickness, how he decides, his day, every interaction, his activities, his movement |
| [DOG.md](DOG.md) | the dog: wandering, eating, how it is fed, its movement |
| [GAMES.md](GAMES.md) | the five minigames |
| [SOUND.md](SOUND.md) | the MIDI sequencer, PSG envelopes, sound effects, the song files |
| [RENDERING.md](RENDERING.md) | how the screen is drawn: the house picture, the compositor, sprites, the resident's and the dog's sprites, colours |
| [IMAGEFORMAT.md](IMAGEFORMAT.md) | the picture, sprite, object and card file formats |
| [NAMEMAP.md](NAMEMAP.md) | the port's names next to the Ghidra names older notes use |

All of it is read from the C port in [`source/`](../source/README.md), which
compiles under Alcyon C 4.14 to a binary byte-identical to the shipped
`LCP.PRG`; names are the port's.

## The program

| Property | Value |
|---|---|
| Platform | Atari ST, ST low resolution (320x200, 16 colours) |
| CPU | Motorola 68000 |
| Compiler | Alcyon C (Digital Research), 16-bit `int` |
| Executable | `LCP.PRG`, 123 352 bytes: text 104 156, data 12 260, bss 187 450 |
| Save file | `HYBER`, 128 bytes (the `PLAYER` struct) |

In the Ghidra project the program is loaded at 0x10000: text 0x10000-0x296DB,
data 0x296DC-0x2C6BF, bss 0x2C6C0-0x5A2F9.  Ghidra address minus 0x10000 is
the offset in the text segment.

### How the source is organised

The original was built from about seven large objects.  The port reproduces
each as a unity translation unit (`stx_u1.c`..`stx_u4.c`, `games.c`,
`vdistx.c`, and `midi_seq.c` via `globals.c`) whose `#include` order is the
original function order; most function bodies live one per file in
`source/parts/`.  Hand-written assembly: the copy protection (`cp_asm.s`), the
Timer-A interrupt (`mq_tick.s`), the PSG and MIDI pokes (`psg_asm.s`), a block
copy (`blkcp_a.s`) and the VDI dispatcher (`vdistx_a.s`).  See
[`source/README.md`](../source/README.md).

## Start-up and the main loop

`main` ([`parts/main.c`](../source/parts/main.c)) installs the Timer-A
interrupt, sets up AES, VDI and the screen buffers, and reads the save file if
there is one (`loadSavedGame`).  The title screen (`titleScreen`) asks for the
player's name, the date and the time.  Then it loads the house picture
(`HOUSE.SCN`), the resident's body frames, rolls a new resident if there was no
save (`rollResident`), loads his head frames, the object and sprite graphics
and the sound effects, places the dog and draws the house in its saved state.
Last come the copy-protection check and, for a new resident, the move-in
(`moveInScene`).

From then on `gameLoop` ([`parts/gameLoop.c`](../source/parts/gameLoop.c))
alternates `gameTick(0)` and `chooseAction` for ever:

- **`gameTick`** ([`tick.c`](../source/tick.c)) waits for the next frame
  (`renderFrame`), steps the clock and the resident's needs (`simStep`),
  animates the house -- clock pendulum, fire, phone, alarm, record player,
  TV, the dog's bowl -- updates the resident's sprites and reads the keyboard.
  Every activity waits through `gameTick`, so the house keeps living while
  the resident does something.
- **`chooseAction`** decides what the resident does next and runs it to the
  end ([PEOPLE.md](PEOPLE.md), "How he decides").

The game runs at 8 frames a second, and the house clock in real time: one
game second per 8 frames.

## The resident in brief

See [PEOPLE.md](PEOPLE.md).  In short:

- He has thirst, hunger, a bathroom need, a mood that cycles over the hours,
  and can fall sick when thirst or hunger go untreated.
- He decides by a fixed ladder: outside events, the alarm, the toilet,
  thirst, hunger, his daily schedule (wake-up, lunch, dinner, bedtime),
  typed requests, and otherwise a random activity from one of three tables
  chosen by the time of day and his activity level.
- The player interacts through ten keys (deliveries, the phone, the alarm,
  water, patting him) and typed requests, which he accepts more readily when
  he is happy.
- Whenever he goes into the study, he saves the game.

## The house

| Floor | Y range | Left to right |
|---|---|---|
| Top | y <= 77 | TV, record player, blue armchair, organ, study door, writing desk with typewriter, filing cabinet |
| Middle | 78..140 | bedroom (bed, alarm clock, closet, dresser), stairs up, bathroom (sink, toilet door, bathtub with shower), computer corner (bookshelf, clock, calendar, computer) |
| Bottom | y > 140 | kitchen (dog bowl, stove, fridge, food cabinet, sink, table, water cooler), stairs up, living room (phone, red armchair, fireplace, front door) |

48 named spots, 16 per floor, are what the resident walks to (`POS_*`,
`posToXY` in [`movement.c`](../source/movement.c)); [PEOPLE.md](PEOPLE.md)
lists which are used and for what.

## Graphics

See [RENDERING.md](RENDERING.md).  The house picture (`HOUSE.SCN`) is decoded
once into an off-screen buffer, and everything that changes in the house --
doors, the stove, the fire, the clock hands, the water tank -- is painted into
it.  Each frame `renderFrame` copies that picture into one of two screens,
draws eight hardware sprite slots over it with masked blits (the resident's
body and head, the dog, and whatever is in front of or behind him), and shows
the screen at the next vertical blank.

## Sound

See [SOUND.md](SOUND.md).  Songs (`.SNG`, and `.ORG` for the organ) are
Activision Music Studio files, played by a sequencer driven by the MFP Timer A
at 960 Hz (`timerAIsr` in `mq_tick.s`; `seqAdvance` and `stepEnvelopes` in
[`midi_seq.c`](../source/midi_seq.c)), on the YM2149 with software envelopes
and on MIDI out.  The 23 sound effects in `SOUNDS.LCP` are XBIOS `Dosound`
scripts, requested by priority through `sfxSelect` and started by the
compositor (`startSfx`).

## Copy protection

`checkCopyProt` ([`cp_asm.s`](../source/cp_asm.s), hand assembly shared with
Activision's *The Music Studio*) programs the 1772 floppy controller directly:
it seeks to track 79, reads it raw with READ TRACK and counts the `$FF` gap
bytes before the first sector header.  The original disk must give fewer than
15 on one read and 80 or more on another, within eleven reads.  The
measuring code is stored encrypted (each word plus `$1567`) and decrypted
only while it runs.  When the check fails, the resident sleeps for ever.  The
check never passes under an emulator, even with the original disk image; the
port's test builds skip it (`-DSKIP_COPYPROT`).

## Data files

| File | Contents |
|---|---|
| `HOUSE.SCN`, `TITLE.SCN` | the house and the title screen, nibble-compressed 320x200 pictures |
| `OBJECTS` | 56 object graphics drawn into the house picture |
| `SPRITES` | 50 sprite graphics, mapped to sprite ids by `spriteFileId` |
| `BODY.LCP` | 98 body frames of the resident |
| `PE2.LCP`..`PE6.LCP` | five head sets, 66 frames each (22 per mood) |
| `CARDS` | 53 card images |
| `SOUNDS.LCP` | 23 sound effects |
| `*.SNG`, `*.ORG` | records and organ pieces (Music Studio format) |
| `LETTER.TXT` | the letter templates, nibble-compressed |
| `WORDS`, `WORDPZ.TXT` | the anagram dictionary and the word puzzles |
| `NAMES` | 266 names, 10 bytes each |
| `HYBER` | the save file: the resident and the state of the house |

Formats: [IMAGEFORMAT.md](IMAGEFORMAT.md), [SOUND.md](SOUND.md),
[GAMES.md](GAMES.md).

## Running it

Copy the contents of `DATA/` to a floppy or a folder, use ST low resolution,
and start `LCP.PRG` directly from the desktop.  Under an emulator use a build
with `-DSKIP_COPYPROT=1`; see [`source/README.md`](../source/README.md).
