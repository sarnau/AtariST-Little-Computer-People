# Little Computer People — Atari ST

**LCP.PRG** is the main executable for *Little Computer People* on the Atari ST, a life-simulation game originally developed by Activision and released in the mid-1980s. The player observes and interacts with a virtual resident who lives inside a multi-story house displayed on screen.

## Overview

Little Computer People is one of the earliest "virtual pet" or life-simulation games. A procedurally generated character moves into a house and goes about daily life — eating, sleeping, showering, exercising, playing piano, writing letters, and more. The player can interact by typing commands, ringing the doorbell, sending deliveries, and playing card and word games with the resident.

The Atari ST version is a native Motorola 68000 binary running in low-resolution mode (320×200, 16 colors). It uses the GEM VDI/AES windowing system for graphics, the Atari's YM2149 PSG chip for sound effects, and MIDI output for music playback.

## Technical Details

| Property             | Value                                       |
|----------------------|---------------------------------------------|
| **Platform**         | Atari ST/STe                                |
| **CPU**              | Motorola 68000, 32-bit big-endian           |
| **Resolution**       | 320×200, 16-color (ST low)                  |
| **Binary format**    | Raw PRG (GEMDOS executable)                 |
| **Compiler**         | CP/M-68K C compiler (Digital Research)       |
| **Code size**        | ~105 KB (TEXT segment)                      |
| **Total functions**  | 398 identified                              |
| **Memory segments**  | TEXT, DATA, BSS, plus LOWMEM_VARS and I/O   |

## Features

### Life Simulation
The resident autonomously performs daily routines driven by an internal AI system. A priority-based decision engine evaluates needs (hunger, thirst, bathroom, sleep) and mood, then selects appropriate actions. The character has configurable wake/lunch/dinner/bedtime hours, a happiness system that cycles over time, and a sickness mechanic triggered by neglecting basic needs.

### Player Interaction
The player communicates with the resident through typed natural language commands. A command parser recognizes keywords for actions like "play piano", "light fire", "feed dog", "write letter", "dance", and many more. Special keyboard shortcuts (Ctrl+B for book delivery, Ctrl+F for food delivery, Ctrl+D for dog food, Ctrl+R for record delivery, Ctrl+C for phone call, Ctrl+A for alarm) trigger in-world events.

### Mini-Games
The resident can play five different games with the player, selected from a menu:

- **Anagrams** — Guess a scrambled word within 9 attempts, with optional letter-reveal clues (F1)
- **War** — Classic card game with high-card showdowns
- **Poker** — 5-card draw poker with betting, raising, bluffing AI, and full hand evaluation
- **Blackjack** — Full casino rules including splits, double-down, and dealer AI
- **Word Puzzles** — Template-based crossword-style puzzles loaded from external data files

### Sound and Music
The game features a complete MIDI music engine that plays songs through the Atari's MIDI port or the YM2149 PSG chip. It parses custom song format headers, supports per-channel volume configuration, and uses Timer B interrupts for precise playback timing. A separate sound effects system handles ambient sounds (footsteps, doorbell, typewriter clicks, speech, TV clicks, etc.).

### Persistent State
The game saves and loads the resident's complete state — physical appearance, personality traits, need levels, furniture positions, food supplies, door states, and in-game time — to disk using the "hyber" save file format.

## Data Files

The game loads several external data files from a `data/` subdirectory:

| File           | Purpose                                          |
|----------------|--------------------------------------------------|
| `house.scn`    | Compressed house background scene graphics        |
| `title.scn`    | Title screen scene                                |
| `body.lcp`     | Character body sprite animation frames            |
| `pex.lcp`      | Character-specific sprite data (varies per resident) |
| `objects`       | Furniture and interactive object graphics         |
| `sprites`       | Sprite animation frames (50 sprites)              |
| `sounds.lcp`   | Sound effects sample data                         |
| `cards`         | Card game graphics (poker/blackjack/war)          |
| `words`         | Anagram dictionary (150 words, compressed)         |
| `wordpz.txt`   | Word puzzle template definitions                  |
| `names`         | Character name pool for random generation         |
| `letter.txt`    | Letter writing template text                      |
| `hyber`         | Save/load game state file                         |
| `*.sng`         | Music song files                                  |
| `*.org`         | Organ/instrument configuration files               |

## Copy Protection

The binary includes a floppy-disk-based copy protection system (`PROT_CHECK` and related functions). It reads specific tracks via direct FDC (Floppy Disk Controller) hardware access, decrypts verification data, and compares results. If the check fails, the game loop degrades to calling `action_sleep(-1)` in an infinite loop, making the character permanently sleep instead of performing any autonomous actions.

## Building / Running

This is a pre-compiled Atari ST binary. To run it:

1. Use an Atari ST emulator (e.g., Hatari, Steem) configured for ST low resolution (320×200)
2. Place `LCP.PRG` and the `data/` folder with all required data files on a virtual floppy or hard disk image
3. Launch `LCP.PRG` from the GEM desktop

The program requires low resolution mode and will display an alert if launched in medium or high resolution.

## Reverse Engineering

This project's analysis was performed using Ghidra. The binary has been extensively annotated with 398 named functions, custom data types (including `MFDB`, `AESPB`, `CARD_TYPE`, `FILE_IMG_DATA`, `MIDI_Note_Struct`, `action_id`, `color_enum`, `keycode_enum`), and detailed comments throughout the decompiled code.
