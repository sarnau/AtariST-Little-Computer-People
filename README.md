# Little Computer People — Atari ST Reverse Engineering

**LCP.PRG** is the main executable for *Little Computer People* on the Atari ST, a life-simulation game originally developed by Activision (Rich Gold, David Crane) and released in 1985. The player observes and interacts with a virtual character — the "Little Computer Person" — who lives inside a three-story house displayed on screen.

This repository documents a comprehensive reverse engineering effort of the Atari ST binary using Ghidra with MCP integration.

## Game Overview

Little Computer People is one of the earliest "virtual pet" or life-simulation games, predating The Sims by over a decade. A procedurally generated character moves into a furnished house and autonomously goes about daily life — eating, sleeping, showering, exercising, playing piano, writing letters, and more. The player can interact by typing natural-language commands, ringing the doorbell (Ctrl+A), ordering deliveries, and playing card and word games.

Each copy of the game generates a unique character with randomized appearance, name, personality, and daily schedule. The character's state persists across sessions via a 128-byte save file (`hyber`), creating a sense of a living, persistent digital companion.

## Technical Specifications

| Property | Value |
|---|---|
| Platform | Atari ST/STe |
| CPU | Motorola 68000, 32-bit big-endian |
| Resolution | 320×200, 16 colors (ST low) |
| Binary format | GEMDOS PRG executable |
| Compiler | Alcyon C (Digital Research CP/M-68K toolchain) |
| Integer size | 16-bit (`int` = `short` in Ghidra) |
| Code size | ~170 KB TEXT segment (0x10000–0x296DB) |
| Total functions | 395 identified and named |
| Total symbols | 3,516 labeled |
| Save file | `hyber`, 128 bytes (LCP struct) |

## Documentation

The reverse engineering analysis is organized into the following documents:

| Document | Contents |
|---|---|
| [ARCHITECTURE.md](ARCHITECTURE.md) | System architecture, memory layout, game loop, AI decision engine, action system, player interaction, house layout, copy protection |
| [PEOPLE.md](PEOPLE.md) | LCP character movement, pathfinding, walk cycles, head animation, body sprite assembly, player states |
| [DOG.md](DOG.md) | Dog AI, autonomous wandering, eating behavior, petting interaction, depth-sorted sprite rendering |
| [GAMES.md](GAMES.md) | Five mini-games: Anagram, War, Poker, Blackjack, Word Puzzle — rules, AI, card deck management |
| [SOUND.md](SOUND.md) | MIDI sequencer engine, PSG envelope processor, 23 sound effects, Music Studio .SNG/.ORG file format, song catalog |
| [IMAGEFORMAT.md](IMAGEFORMAT.md) | Pixel format, color palette, compressed screen images (.SCN), sprite/object files, playing cards, character body sprites (PE*.LCP) |

## Data Files

| File | Format | Description |
|---|---|---|
| `house.scn` | Nibble-dict compressed | House background (320×200), see [IMAGEFORMAT.md](IMAGEFORMAT.md) |
| `title.scn` | Nibble-dict compressed | Title screen, see [IMAGEFORMAT.md](IMAGEFORMAT.md) |
| `body.lcp` | Fixed-frame sprites | Body sprite sheet (66 states × 16×21), see [IMAGEFORMAT.md](IMAGEFORMAT.md) |
| `PE2–PE6.lcp` | Fixed-frame sprites | Character appearance variants, see [IMAGEFORMAT.md](IMAGEFORMAT.md) |
| `objects` | Sequential bitmaps | 56 static object graphics, see [IMAGEFORMAT.md](IMAGEFORMAT.md) |
| `sprites` | Sequential + index table | 50 overlay sprite definitions, see [IMAGEFORMAT.md](IMAGEFORMAT.md) |
| `cards` | Raw bitmap array | 53 playing cards (16×24), see [IMAGEFORMAT.md](IMAGEFORMAT.md) |
| `sounds.lcp` | DoSound sequences | 23 YM2149 sound effects, see [SOUND.md](SOUND.md) |
| `*.sng` | Music Studio format | 11 background music songs by Ed Bogas, see [SOUND.md](SOUND.md) |
| `*.org` | Music Studio format | 5 classical/traditional pieces for piano, see [SOUND.md](SOUND.md) |
| `words` | Nibble-compressed text | Anagram dictionary (150 words), see [GAMES.md](GAMES.md) |
| `wordpz.txt` | Nibble-compressed text | Word puzzle templates, see [GAMES.md](GAMES.md) |
| `names` | Plain text | Character name pool (266 × 10 bytes) |
| `letter.txt` | Nibble-compressed text | Letter writing templates |
| `hyber` | Raw struct dump | Save file (128-byte LCP struct), see [ARCHITECTURE.md](ARCHITECTURE.md) |

## Reverse Engineering Status

### Completed
- All 395 functions identified, named, and documented with plate comments
- All auto-generated variable names resolved (0 remaining)
- 128-byte LCP save file struct fully mapped (49 fields)
- 25+ custom enums applied (ACTION_ID, PLAYER_STATE, HOUSE_POS, WORD_ID, SOUND_EFFECT_ID, CLOTHING_COLOR_ID, SKIN_COLOR_ID, SICKNESS_LEVEL, HAPPINESS_LEVEL, etc.)
- 6+ custom structs defined (LCP, PSG_ENVELOPE, MFDB, FILE_IMG_DATA, CCB, _iobuf/FILE, fcbtab)
- Alcyon C runtime library fully identified (~60 functions matched to original source code)
- All sound/music data files fully documented with verified format specifications
- All image/sprite data files fully documented
- Complete sprite rendering pipeline documented
- AI decision engine and all 45 action handlers fully analyzed
- Five mini-games fully reverse-engineered with complete rule documentation

### Tools Used
- **Ghidra** with MCP (Model Context Protocol) integration for interactive analysis
- **Alcyon C compiler source code** (from Digital Research CP/M-68K) for runtime library identification
- Original game running in **Hatari** emulator for behavioral verification

## Running the Game

1. Use an Atari ST emulator (Hatari, Steem) configured for **ST low resolution** (320×200, 16 colors)
2. Place `LCP.PRG` and all data files on a virtual floppy or hard disk image
3. Launch `LCP.PRG` from the GEM desktop
4. The game will display an error alert if not in low resolution mode

## License

This is a reverse engineering documentation project. The original game is © 1985 Activision. All analysis is for educational and preservation purposes.
