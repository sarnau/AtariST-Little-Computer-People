# Little Computer People — Mini-Games

The LCP character can play five mini-games at the game table. The player selects a game
by pressing keys 1–5 when prompted:

```
What game do you want to play?
1. Anagrams   2. War  3. Poker
4. Blackjack  5. Word Puzzles
```

The LCP walks to the filing cabinet, retrieves a game box (SPRITE_GAME_BOX), carries it
to the kitchen table, sits down (STATE_EAT_BITE with +8y/+6x offset), and launches the
selected game. On exit, the LCP picks up the box and returns it to the cabinet.

All mini-games share a common framework:
- `mgSetup()` fills the top 77 rows with background, freezes text scroll
- `mgWaitKey()` handles input while processing urgent game events
  (alarm, bathroom, thirst, doorbell) — the LCP leaves the table, handles the event, returns
- Auto-quit after 7,200 frames (~15 min) of inactivity, sets `mgTimedOut`
- Card graphics loaded from `cards` data file (52 cards + card back, 53 MFDB blocks)

Screen resolution: 320×200 pixels, 16 colors, Atari ST low resolution.

---

## 1. Anagram Game

**Entry point:** `playAnagrams()` (0x181AE)
**Data file:** `words` — 150 words × 11 bytes each (compressed)

### Concept

The computer selects a random word from a 150-word dictionary, scrambles its letters,
and challenges the player to unscramble it. The player has up to 9 guesses and can
request letter clues at the cost of one guess each.

### Data Structures

| Variable | Type | Purpose |
|---|---|---|
| `anaDict` | char* | 10,000-byte buffer for decompressed dictionary |
| `anaAnswer` | char* | Pointer into dictionary for current word |
| `anaScrambled` | char[11] | Working copy with shuffled characters |
| `anaWordLen` | short | Length of current word (max 10) |
| `anaInput` | char[11] | Player's typed guess |
| `anaGuessNum` | short | Current guess attempt (1–9) |
| `anaNumClues` | short | Total clues used (cumulative) |
| `anaExtraGuess` | short | Flag: 1 when all letters revealed |
| `anaClueUsed` | short | Flag: prevents >1 clue per guess |

### Dictionary Format

Each word is stored as a fixed 11-byte record. Words are terminated by either a period
(`.`) or a space character. The file is loaded compressed via `unpackFile()`
and decompressed into a 10,000-byte heap buffer. Words are accessed by index:
`anaDict + (index * 11)`.

### Scrambling Algorithm (`anaPickWord`)

1. Pick random index 0–149
2. Copy word characters into `anaScrambled` (stop at `.` or space)
3. Store null terminator; also null-terminate the original (replacing delimiter)
4. Scramble loop:
   - Generate random swap count (10–20)
   - For each swap: pick two random positions, exchange characters
5. Compare scrambled with original via `anaStrMatch()`
6. If still identical → repeat scrambling (ensures puzzle is solvable)
7. Display in large green text via `anaShowWord()`

### Clue Algorithm (F1 key)

When the player presses F1 (once per guess, only if word not yet fully revealed):

1. Scan left-to-right for first position where `scrambled[i] != original[i]`
2. Search backwards from end of `scrambled` to find where `original[i]` currently sits
3. Swap those two positions in `scrambled`
4. Redisplay the (partially unscrambled) word
5. If `scrambled` now equals `original` → "You took too many clues!" and round ends

This progressively reveals the word from left to right, one letter per clue.

### Game Flow

```
1. Load dictionary, setup mini-game screen
2. Display intro text: "I am thinking of a word..."
3. LOOP (new word each iteration):
   a. Select and scramble random word
   b. LOOP (up to 9 guesses):
      - Display "Guess #N?" prompt
      - Input loop:
        * Type letters (a-z, converted to lowercase, max 10 chars)
        * Cursor-left = backspace
        * F1 = reveal one clue letter (costs one guess, once per turn)
        * F10 = quit game entirely
        * Enter = submit guess
      - Compare input with original word:
        * Match → "YOU GOT IT!!!!!!" → new word
        * Wrong → random taunt from 3 messages → next guess
      - After 8 wrong guesses: "Sorry, too many guesses!" → reveal answer → new word
```

### Screen Layout

```
+---------------------------+-----------------------------+
|  ***ANAGRAMS***           | F1 Clue, F10 Quit           |  y=0-9
+---------------------------+-----------------------------+
|  I am thinking of         |                             |  y=10-49
|  a word. Here it          |   S C R A M B L E D        |  (large green text
|  is jumbled up...         |     (20px tall)             |   at x=162, y=37)
|  See if you can           |                             |
|  guess what it is.        |                             |
+---------------------------+-----------------------------+
|                           | Guess #1?                   |  y=50-65
|                           | [player input]              |
+-----------------------------------------------------------+
|  YOU GOT IT!!!!!!         (or wrong guess message)      |  y=62-75
+-----------------------------------------------------------+
```

### Helper Functions

| Function | Address | Purpose |
|---|---|---|
| `anaPickWord` | 0x18084 | Pick and scramble random word |
| `anaStrMatch` | 0x17538 | Case-sensitive string compare (1=match, 0=different) |
| `anaShowWord` | 0x18004 | Display word in 20px tall letters at (162,37) |
| `anaClrWord` | 0x17E9C | Clear right panel (162,10)-(319,49) |
| `anaIntroText` | 0x17F84 | Print 5-line intro in left panel |
| `anaDrawPrompt` | 0x18052 | Show "Guess #N?" from lookup table |
| `anaClrGuess` | 0x17F10 | Clear prompt area (166,50)-(319,65) |
| `anaClrBottom` | 0x17F4A | Clear status bar (5,62)-(319,75) |

---

## 2. Card Games (War, Poker, Blackjack)

The three card games share a common infrastructure: card graphics (`cards` data file),
display routines, input handler, and money/pot tracking.  The analysis this
document began as named all of them `poker_*`; each game is distinct.

### Shared Infrastructure

**Card representation:** Each card is a `CARD_TYPE` value 0–51:
- Rank = `card % 13` (0=2, 1=3, ..., 8=10, 9=J, 10=Q, 11=K, 12=Ace)
- Suit = `card / 13` (0=Hearts, 1=Diamonds, 2=Clubs, 3=Spades)
- `CARD_NONE` (0xFF) = empty slot; `CARD_BACK` = face-down display

**Card graphics:** 53 MFDB blocks loaded from `cards` data file (52 face cards + 1 back).
Two display rows: top row for computer (y positions from `cardYComp[]`), bottom row
for player (from `cardYPlyr[]`). Up to 5 cards per row.

**Shared globals:**

| Variable | Purpose |
|---|---|
| `plyrChips` | Player's chip count (poker/blackjack) or card count (war) |
| `compChips` | Computer's chip count or card count |
| `potChips` | Current pot (chips in play) |
| `bjBetSplit` / `bjBetMain` | Current bet amounts |
| `cardQuit` | Set to YES when game should end |
| `cardImages` | Heap-allocated buffer for card MFDB image data |

**Shared functions:**

| Function | Address | Purpose |
|---|---|---|
| `cardLoad` | 0x1AB04 | Load `cards` file, build 53 MFDB blocks |
| `cardDraw` | 0x1AA64 | Draw one card at (row, position) |
| `cardKeyInput` | 0x1AC92 | Wait for F-key input, map to action codes |
| `cardMessage` | 0x1B0AA | Print message in status area |
| `dispCompChips` | 0x1AD26 | Show computer's chip/card count |
| `dispPlyrChips` | 0x1ADE6 | Show player's chip/card count |
| `dispPot` | 0x1AEA6 | Show current pot amount |
| `bjShowBet` | 0x1D78E | Show bet amount with emphasis |
| `potToWinner` | 0x1A664 | Transfer pot to winner with animation |
| `pkrAddChips` | 0x1A840 | Move chips from player to pot |
| `panelErase` | 0x186E0 | Clear a screen rectangle |

---

### 2a. War

**Entry point:** `playWar()` (0x1B15C)
**Starting cards:** 26 each (full 52-card deck split evenly)

#### Concept

A simple card game: both players flip their top card, higher rank wins both cards.
On tie, a "war" round is played with multiple face-down cards and a final face-up
card deciding the outcome. The game ends when one player runs out of cards.

#### Data Structures

| Variable | Purpose |
|---|---|
| `plyrPile` (0x3F712) | Player's card pile (up to 52 cards) |
| `compPile` (0x47E24) | Computer's card pile |
| `plyrWarCards` (0x3C9DC) | Player's face-down war cards |
| `compWarCards` (0x3CC78) | Computer's face-down war cards |
| `plyrChips` | Number of cards remaining (starts at 26) |
| `compChips` | Number of cards remaining (starts at 26) |

Note: `plyrChips` and `compChips` represent **card counts** in War
(not money), since the shared globals are reused across all three card games.

#### Deck Initialization

1. Fill array with cards 0–51 in order
2. Shuffle by performing 400 random swaps
3. Deal alternating: even indices → computer pile, odd indices → player pile
4. Each player starts with 26 cards

#### Game Flow

```
1. Shuffle deck, split 26/26
2. ROUND LOOP:
   a. Player draws top card (face-down), computer draws top card (face-up)
   b. Prompt: "Show me your card, Ace." with F1=Show, F10=Quit
   c. Player reveals card
   d. Compare ranks (card % 13):
      * Player wins → pot to player, both cards added to player's pile
        - Computer says random taunt: "Dog-gone it.", "Arrghh!", etc.
        - LCP peeks around (action_peek_around animation)
      * Computer wins → pot to computer, both cards added to computer's pile
        - Computer says random boast: "That was easy!", "Beat you by a mile.", etc.
      * Tie → WAR sub-game (warRound)
   e. Check for game end:
      * Computer out of cards → "I'm out of cards! You're too good!"
      * Player out of cards → "No cards, huh? Better luck next time."
```

#### War Sub-Game (on tie)

When both players flip the same rank, a war round triggers:

1. Both sides place 3 cards face-down, then 1 card face-up
2. Higher face-up card wins all 8 cards (plus any pot)
3. If face-up cards tie again → another war round (recursive)
4. Cards are drawn via `popCard()` and won cards returned
   via `pushCard()`

#### Screen Layout

```
+------+------+------+------+------+---------+-------------------+
| Computer's card (face-up)        | $nnn    | Status messages    |
|  row 0: cardYComp[]          | (cards) |                    |
+------+------+------+------+------+---------+-------------------+
|                                  | Pot:nnn |                    |
+------+------+------+------+------+---------+-------------------+
| Player's card (face-down→up)     | $nnn    | F1  Show           |
|  row 1: cardYPlyr[]          | (cards) | F10 Quit           |
+------+------+------+------+------+---------+-------------------+
|  "Show me your card, Ace."                                      |
+------------------------------------------------------------------+
```

---

### 2b. Five-Card Draw Poker

**Entry point:** `playPoker()` (0x18D10)
**Starting chips:** 400 each

#### Concept

Standard 5-card draw poker with computer AI. Players ante, receive 5 cards, bet,
optionally discard and draw new cards, bet again, then compare hands at showdown.
The computer has hand evaluation, bluff logic, and draw strategy.

#### Data Structures

| Variable | Purpose |
|---|---|
| `plyrHand[5]` (0x3CCF0) | Player's 5-card hand |
| `compHand[5]` (0x3D11E) | Computer's 5-card hand |
| `compScoring[5]` | Per-card flags: 1=part of scoring combo (computer) |
| `plyrScoring[5]` | Same for player |
| `compSorted[5]` | Sorted hand by rank (computer) |
| `plyrSorted[5]` | Same for player |
| `compRank` | Computer's hand rank (0–8) |
| `pkrSelected[5]` | Cards selected for discard (1=selected) |
| `pkrDiscPile[]` | Discarded cards (prevents re-dealing) |
| `pkrNumDisc` | Number of discarded cards |
| `pkrBluffing` | YES if computer is bluffing this round |

#### Hand Ranks (`pkrEvalHand`, 0x18804)

| Rank | Name | Description |
|---|---|---|
| 0 | High Card | No combination |
| 1 | One Pair | Two cards of same rank |
| 2 | Two Pair | Two different pairs |
| 3 | Three of a Kind | Three cards of same rank |
| 4 | Straight | Five sequential ranks |
| 5 | Flush | Five cards of same suit |
| 6 | Full House | Three of a kind + pair |
| 7 | Four of a Kind | Four cards of same rank |
| 8 | Straight Flush | Sequential ranks, same suit |

The evaluator sorts cards by rank (bubble sort), then checks for flush (all same suit),
straight (sequential ranks, with A-2-3-4-5 wrap), and counts rank groups for pairs/trips/quads.

#### Computer AI

**Bluff decision** (`pkrDecideBluff`): 1/15 chance of bluffing when hand rank < 2.

**Opening check** (`pkrOpenRank`): Passes if no pair and not bluffing.
Otherwise finds highest card; opens only with ace or better.

**Bet decision** (`pkrCallOrRaise`): Returns 99 (call) if weak hand and not
bluffing. Otherwise calculates raise = money/10 (capped at 1–20). Returns 114 (raise).

**Draw strategy** (`pkrCompDraw`):
- Four of a kind: keep all, draw 0
- Full house: keep all, draw 0
- Flush/straight: keep all, draw 0
- Three of a kind: keep trips, draw 2
- Two pair: keep both pairs, draw 1
- One pair: keep pair, draw 3
- High card: keep highest, draw 4
- Bluffing: draw random 0–3 cards regardless of hand

#### Game Flow

```
1. Allocate card graphics memory, setup screen
2. Both players start with 400 chips
3. ROUND LOOP:
   a. ANTE PHASE (pkrAnte):
      - Prompt F1=Ante, F10=Quit
      - Each player puts 1 chip in pot
   b. DEAL (pkrDealHands):
      - Deal 5 random unique cards to each player
      - Computer cards face-down, player cards face-up
   c. INITIAL BET (pkrPlyrBet):
      - Player: F1=Bet (+1 chip), F3=Enter (confirm), F5=Pass/Clear
      - Max bet: 20 chips per round
      - Computer responds: call, raise, or fold
   d. DRAW PHASE:
      - Player selects cards to discard (click positions 1-5, F1=Draw, F3=Stay)
      - Selected cards shown as empty; new cards dealt from unused deck
      - Computer draws via AI strategy (pkrCompDraw)
      - Computer announces: "I'll take N cards" or "I'll stay!"
   e. FINAL BET:
      - Another betting round (same as initial)
   f. SHOWDOWN (pkrShowdown, 0x19A3A):
      - Evaluate both hands via pkrEvalHand
      - Compare ranks; tie-break by kicker cards
      - Winner takes pot via bjSettle
      - Display hand rank names and result messages
   g. Check for game end (either player at 0 chips)
```

#### Screen Layout

```
+------+------+------+------+------+---------+-------------------+
| [##] | [##] | [##] | [##] | [##] | $nnn    | F1 Bet             |
| Computer's hand (face-down)      | (comp)  | F3 Enter           |
+------+------+------+------+------+---------+ F5 Pass/Clr        |
|                                  | Pot:nnn | F10 Quit           |
+------+------+------+------+------+---------+-------------------+
| [5♠] | [K♥] | [K♦] | [3♣] | [7♠] | $nnn    |                    |
| Player's hand (face-up)          | (player)|                    |
+------+------+------+------+------+---------+-------------------+
|  "Do you feel lucky today?"                                     |
+------------------------------------------------------------------+
```

---

### 2c. Blackjack (21)

**Entry point:** `playBlackjack()` (0x1BC72, 623 lines)
**Starting chips:** 400 each

#### Concept

Standard blackjack rules: get as close to 21 as possible without going over.
Aces can count as 1 or 11. Face cards (10/J/Q/K) count as 10.
Supports pair splitting. Ties trigger a War sub-game.

#### Card Values (`bjScore`, 0x1D1B4)

| Card Rank | Value |
|---|---|
| 2–7 | Face value (rank + 2) |
| 8, 9, 10, J, Q | 10 |
| Ace (rank 12) | 1 or 11 (aceMode parameter) |

The `aceMode` parameter controls ace handling:
- 0 = all aces count as 1
- 1 = first ace counts as 11, subsequent aces count as 1

#### Data Structures

| Variable | Purpose |
|---|---|
| `plyrHand[5]` | the player's main hand |
| `bjSplitHand[5]` | the player's split hand, after splitting a pair |
| `compHand[5]` | the dealer's (the resident's) hand |
| `bjHitsMain`, `bjHitsSplit`, `bjHitsDealer` | cards dealt to each hand |
| `bjBetMain`, `bjBetSplit` | chips bet on each of the player's hands |
| `bjDidSplit` | YES once the player has split |
| `bjNatMain`, `bjNatSplit` | a hand is a natural blackjack |
| `bjBustMain`, `bjBustSplit` | a hand has bust |
| `bjDblMain`, `bjDblSplit` | a hand has doubled down |
| `bjDealerScore`, `bjPlyrScore` | final scores |

#### Natural Blackjack Detection (`bjIsNatural`, 0x1D608)

Checks if the initial 2-card deal contains an ace (rank 12) paired with a face card
(rank 8–11, i.e., 10/J/Q/K). Returns 1 if natural blackjack, 0 otherwise.

#### Game Flow

```
1. Allocate card graphics, setup screen, 400 chips each
2. ROUND LOOP:
   a. BET PHASE:
      - Prompt F1=Bet, F10=Quit
      - Player places bet (F1 increments, F3 confirms, F5 clears)
      - Max 20 chips per bet
      - Computer matches bet automatically
   b. DEAL:
      - Deal 2 cards to each (bjDealCard)
      - Check for natural blackjack on both hands
      - Natural blackjack → immediate win (1.5x payout)
   c. SPLIT OPTION (if player's 2 cards have same rank):
      - Prompt "Do you wish to split?" F1=Split, F3=No split
      - If split: move second card to bjSplitHand
      - Play first hand fully, then second hand
      - Each hand gets its own bet (matched from player's chips)
   d. HIT/STAND ROUNDS (bjPlayHand, 0x1D294):
      - For each hand (main, then split if applicable):
        * Display hand face-up
        * F1=Hit (deal another card), F3=Stand
        * Bust (score > 21) → immediate loss, returns -1
        * Stand or 5-card limit → returns 0
   e. DEALER PLAYS:
      - Computer hits on 16 or less, stands on 17+
      - Same bust/stand logic
   f. COMPARE SCORES:
      - Higher score wins (without busting)
      - Tie → WAR sub-game (warRound)
      - Winner awarded pot via bjSettle
   g. Check for game end
```

#### Pair Splitting

When the player's initial two cards have the same rank (e.g., two Kings):

1. Second card moved to `bjSplitHand`
2. First hand played fully (hit/stand)
3. Then second hand played with its own bet
4. Each hand checked independently for blackjack and bust
5. Both hands compared against dealer's hand separately

#### Screen Layout

```
+------+------+------+------+------+---------+-------------------+
| [##] | [##] |      |      |      | $nnn    |                    |
| Dealer's hand (face-down)        | (dealer)|                    |
+------+------+------+------+------+---------+-------------------+
|                                  | Pot:nnn |                    |
+------+------+------+------+------+---------+-------------------+
| [A♠] | [K♥] |      |      |      | $nnn    | F1 Hit             |
| Player's hand (face-up)          | (player)| F3 Stand           |
+------+------+------+------+------+---------+-------------------+
|  "Your turn."                                                   |
+------------------------------------------------------------------+
```

---

## 3. Word Puzzle Game

**Entry point:** `playWordPuzzle()` (0x176F8)
**Data file:** `wordpz.txt` — 33 fill-in-the-blank puzzles (compressed)

### Concept

Fill-in-the-blank word puzzles. The game displays a sentence with missing words
(marked by `@`), and the player types in guesses for each blank. After all blanks
are filled, the answers are compared against the solution.

### Data Structures

| Variable | Type | Purpose |
|---|---|---|
| `wpzText` | char* | 2,000-byte buffer for decompressed puzzle data |
| `wpzIndex` | short | Current puzzle number (0–32) |
| `wpzBlanks` | short | Number of `@` blanks in current puzzle |
| `wpzAnswers[][12]` | char[][] | Player's typed answers (up to 10 chars each) |
| `letterLines[66]` | char*[] | Parsed line pointers (2 lines per puzzle) |
| `wpzPrompts[9]` | char*[] | Prompts: "OK, what's the first word?", etc. |
| `wpzRightMsgs[6]` | char*[] | Success messages |
| `wpzWrongMsgs[6]` | char*[] | Failure messages |

### Puzzle File Format (`wordpz.txt`)

The file contains 33 puzzles stored as 66 lines (2 lines per puzzle):

- **Even lines** (0, 2, 4, ...): Template text with `@X` markers
  - Literal text is displayed as-is
  - `@` followed by a placeholder character marks a fill-in blank
  - The character after `@` is used as the initial display hint
- **Odd lines** (1, 3, 5, ...): Solution words separated by whitespace

Lines are delimited by control characters (ASCII < 32). The file is compressed and
decompressed into a 2,000-byte buffer. After loading, all 66 line start pointers are
stored in `letterLines[]`.

### Template Rendering (`wpzRender`, 0x17CAC)

The renderer walks the template string character by character:

1. **Space**: adds spacing (collapsed if at start of line)
2. **Literal text**: accumulates word characters, measures word length for word-wrap,
   prints each character individually in blue via `printChar()`
3. **`@` marker**: reads placeholder character, substitutes with player's answer from
   `wpzAnswers[answer_index][]`, prints in blue
4. **After answer**: checks the character 2 positions after `@` for trailing punctuation
   (period, comma, etc.) and renders it inline
5. **Word wrap**: if next word would exceed column 38 (x position > 0x26), wraps to
   next line (y += 8). Two display lines available (y=40 and y=48).

### Answer Comparison

After the player enters all answers, the solve phase compares each answer against
the solution line from `wordpz.txt`:

1. Walk the solution line, skip whitespace (ASCII < `!`) to find each solution word
2. Compare character-by-character with the player's answer
3. If all characters match and player answer is fully consumed → word is correct
4. All words correct → random success message from 6 options
5. Any word wrong → random failure message from 6 options

Comparison is **case-sensitive**: player input is converted to uppercase via
`toUpper()`, so solution words in the file must also be uppercase.

### Game Flow

```
1. Load wordpz.txt, parse into 66 line pointers
2. Start at puzzle #1
3. BROWSE LOOP:
   a. Display puzzle number: "**WORD PUZZLE # NN **"
   b. Parse template line, count @ blanks (wpzBlanks)
   c. Render template with current answers (initially placeholder chars)
   d. Wait for input:
      * F1 → next puzzle (wraps 33→1)
      * F2 → previous puzzle (wraps 1→33)
      * F5 → enter SOLVE mode
      * F10 → quit
4. SOLVE MODE (wpzSolve):
   a. For each blank (0 to wpzBlanks-1):
      - Display prompt:
        * First blank: random from 5 options ("OK, what's the first word?")
        * Subsequent: "Next word?", "And the next?", etc.
      - Input loop:
        * Type letters (converted to uppercase, max 10 chars)
        * Cursor-left = backspace
        * Enter = confirm answer
        * F10 = cancel and return to browse mode
      - Store answer in wpzAnswers[i][]
   b. Compare all answers against solution line
   c. Display result:
      * All correct: render template with answers, random success message
      * Any wrong: render template with (wrong) answers, random failure message
   d. Wait 40 frames, return to browse mode
```

### Screen Layout

```
+---------------------------+-----------------------------+
| **WORD PUZZLE # 12 **     | F1 Next, F5 Solve           |  y=0-9
+---------------------------+ F2 Last, F10 Quit           |
| Choose the puzzle         |                             |  y=10-26
| you wish to solve.        |                             |
+-----------------------------------------------------------+
|                                                           |  y=31-49
|  The quick brown [_____] jumped over the lazy [_____].    |  (template with
|                                                           |   blanks, blue text,
|                                                           |   word-wrapped)
+-----------------------------------------------------------+
|  OK, what's the first word?                               |  y=50-59 (prompt)
+-----------------------------------------------------------+
|  [player typing here]                                     |  y=60-69 (input)
+-----------------------------------------------------------+
```

### Helper Functions

| Function | Address | Purpose |
|---|---|---|
| `wpzSolve` | 0x1799E | Interactive answer entry and comparison |
| `wpzRender` | 0x17CAC | Render template with filled answers |
| `wpzMessage` | 0x17C78 | Display message in bottom prompt area |
| `toUpper` | 0x272E8 | Convert ASCII character to uppercase |

---

## Function Cross-Reference (All Mini-Game Functions)

### Anagram (9 functions)
| Address | Function |
|---|---|
| 0x181AE | `playAnagrams` |
| 0x18084 | `anaPickWord` |
| 0x17538 | `anaStrMatch` |
| 0x18004 | `anaShowWord` |
| 0x17E9C | `anaClrWord` |
| 0x17F84 | `anaIntroText` |
| 0x18052 | `anaDrawPrompt` |
| 0x17F10 | `anaClrGuess` |
| 0x17F4A | `anaClrBottom` |

### Card Games — Shared (11 functions)
| Address | Function |
|---|---|
| 0x1AB04 | `cardLoad` |
| 0x1AA64 | `cardDraw` |
| 0x1AC92 | `cardKeyInput` |
| 0x1B0AA | `cardMessage` |
| 0x1AD26 | `dispCompChips` |
| 0x1ADE6 | `dispPlyrChips` |
| 0x1AEA6 | `dispPot` |
| 0x1D78E | `bjShowBet` |
| 0x1A664 | `potToWinner` |
| 0x1A840 | `pkrAddChips` |
| 0x186E0 | `panelErase` |

### War (4 functions)
| Address | Function |
|---|---|
| 0x1B15C | `playWar` |
| 0x1B784 | `warRound` |
| 0x1B0E0 | `popCard` |
| 0x1B138 | `pushCard` |

### Poker (10 functions)
| Address | Function |
|---|---|
| 0x18D10 | `playPoker` |
| 0x18804 | `pkrEvalHand` |
| 0x1A8C2 | `pkrDealHands` |
| 0x1AF66 | `pkrAnte` |
| 0x1A6B8 | `pkrPlyrBet` |
| 0x187A0 | `pkrCallOrRaise` |
| 0x1A1BC | `pkrOpenRank` |
| 0x1A27A | `pkrCompDraw` |
| 0x1A24A | `pkrDecideBluff` |
| 0x19A3A | `pkrShowdown` |

### Blackjack (5 functions)
| Address | Function |
|---|---|
| 0x1BC72 | `playBlackjack` |
| 0x1D294 | `bjPlayHand` |
| 0x1D608 | `bjIsNatural` |
| 0x1D67C | `bjDealCard` |
| 0x1D1B4 | `bjScore` |

### Settling (1 function)
| Address | Function |
|---|---|
| 0x1D864 | `bjSettle` |

### Word Puzzle (5 functions)
| Address | Function |
|---|---|
| 0x176F8 | `playWordPuzzle` |
| 0x1799E | `wpzSolve` |
| 0x17CAC | `wpzRender` |
| 0x17C78 | `wpzMessage` |
| 0x272E8 | `toUpper` |

### Game Selection
| Address | Function |
|---|---|
| 0x21860 | `playGame` |
| 0x1759C | `mgSetup` |
| 0x173E8 | `mgWaitKey` |
