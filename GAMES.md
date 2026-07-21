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
- `minigame_setup_screen()` fills the top 77 rows with background, freezes text scroll
- `minigame_wait_for_key_with_events()` handles input while processing urgent game events
  (alarm, bathroom, thirst, doorbell) — the LCP leaves the table, handles the event, returns
- Auto-quit after 7,200 frames (~15 min) of inactivity, sets `mg_tofl`
- Card graphics loaded from `cards` data file (52 cards + card back, 53 MFDB blocks)

Screen resolution: 320×200 pixels, 16 colors, Atari ST low resolution.

---

## 1. Anagram Game

**Entry point:** `anagram_main()` (0x181AE)
**Data file:** `words` — 150 words × 11 bytes each (compressed)

### Concept

The computer selects a random word from a 150-word dictionary, scrambles its letters,
and challenges the player to unscramble it. The player has up to 9 guesses and can
request letter clues at the cost of one guess each.

### Data Structures

| Variable | Type | Purpose |
|---|---|---|
| `g_agwb` | char* | 10,000-byte buffer for decompressed dictionary |
| `g_agorw` | char* | Pointer into dictionary for current word |
| `g_agscw` | char[11] | Working copy with shuffled characters |
| `g_agwol` | short | Length of current word (max 10) |
| `g_aginb` | char[11] | Player's typed guess |
| `g_aggun` | short | Current guess attempt (1–9) |
| `g_agclc` | short | Total clues used (cumulative) |
| `g_agacu` | short | Flag: 1 when all letters revealed |
| `_anagram_clue_used_this_round` | short | Flag: prevents >1 clue per guess |

### Dictionary Format

Each word is stored as a fixed 11-byte record. Words are terminated by either a period
(`.`) or a space character. The file is loaded compressed via `file_read_compressed()`
and decompressed into a 10,000-byte heap buffer. Words are accessed by index:
`anagram_words_buffer + (index * 11)`.

### Scrambling Algorithm (`anagram_select_and_scramble_word`)

1. Pick random index 0–149
2. Copy word characters into `g_agscw` (stop at `.` or space)
3. Store null terminator; also null-terminate the original (replacing delimiter)
4. Scramble loop:
   - Generate random swap count (10–20)
   - For each swap: pick two random positions, exchange characters
5. Compare scrambled with original via `anagram_strings_equal()`
6. If still identical → repeat scrambling (ensures puzzle is solvable)
7. Display in large green text via `anagram_display_word_large()`

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
| `anagram_select_and_scramble_word` | 0x18084 | Pick and scramble random word |
| `anagram_strings_equal` | 0x17538 | Case-sensitive string compare (1=match, 0=different) |
| `anagram_display_word_large` | 0x17E34 | Display word in 20px tall letters at (162,37) |
| `anagram_clear_word_display_area` | 0x17E9C | Clear right panel (162,10)-(319,49) |
| `anagram_show_intro_text` | 0x17EE4 | Print 5-line intro in left panel |
| `anagram_show_guess_prompt` | 0x17F2E | Show "Guess #N?" from lookup table |
| `anagram_clear_guess_prompt_area` | 0x17F4A | Clear prompt area (166,50)-(319,65) |
| `anagram_clear_status_bar` | 0x17F82 | Clear status bar (5,62)-(319,75) |

---

## 2. Card Games (War, Poker, Blackjack)

The three card games share a common infrastructure: card graphics (`cards` data file),
display routines, input handler, and money/pot tracking. Despite function names using
the `poker_` prefix throughout, each game is distinct.

### Shared Infrastructure

**Card representation:** Each card is a `CARD_TYPE` value 0–51:
- Rank = `card % 13` (0=2, 1=3, ..., 8=10, 9=J, 10=Q, 11=K, 12=Ace)
- Suit = `card / 13` (0=Hearts, 1=Diamonds, 2=Clubs, 3=Spades)
- `CARD_NONE` (0xFF) = empty slot; `CARD_BACK` = face-down display

**Card graphics:** 53 MFDB blocks loaded from `cards` data file (52 face cards + 1 back).
Two display rows: top row for computer (y positions from `cards_y_pos_a[]`), bottom row
for player (from `cards_y_pos_b[]`). Up to 5 cards per row.

**Shared globals:**

| Variable | Purpose |
|---|---|
| `g_ppmon` | Player's chip count (poker/blackjack) or card count (war) |
| `g_pcmon` | Computer's chip count or card count |
| `g_ppppa` | Current pot (chips in play) |
| `g_ppbet` / `g_pcbet` | Current bet amounts |
| `pk_quit` | Set to YES when game should end |
| `crd_dat` | Heap-allocated buffer for card MFDB image data |

**Shared functions:**

| Function | Address | Purpose |
|---|---|---|
| `poker_load_card_graphics` | 0x1AB04 | Load `cards` file, build 53 MFDB blocks |
| `poker_draw_card_sprite` | 0x1AA64 | Draw one card at (row, position) |
| `poker_input_handler` | 0x1AC92 | Wait for F-key input, map to action codes |
| `poker_print_message` | 0x1B0AA | Print message in status area |
| `poker_display_computer_money` | 0x1AD26 | Show computer's chip/card count |
| `poker_display_player_money` | 0x1ADE6 | Show player's chip/card count |
| `poker_display_pot` | 0x1AEA6 | Show current pot amount |
| `poker_display_bet_with_highlight` | 0x1D78E | Show bet amount with emphasis |
| `poker_award_pot` | 0x1A664 | Transfer pot to winner with animation |
| `poker_add_to_pot` | 0x1A840 | Move chips from player to pot |
| `play_erase_rect` | 0x186E0 | Clear a screen rectangle |

---

### 2a. War

**Entry point:** `poker_war_main()` (0x1B15C)
**Starting cards:** 26 each (full 52-card deck split evenly)

#### Concept

A simple card game: both players flip their top card, higher rank wins both cards.
On tie, a "war" round is played with multiple face-down cards and a final face-up
card deciding the outcome. The game ends when one player runs out of cards.

#### Data Structures

| Variable | Purpose |
|---|---|
| `g_ppdrp` (0x3F712) | Player's card pile (up to 52 cards) |
| `g_pcdrp` (0x47E24) | Computer's card pile |
| `pk_pwc` (0x3C9DC) | Player's face-down war cards |
| `pk_cwc` (0x3CC78) | Computer's face-down war cards |
| `g_ppmon` | Number of cards remaining (starts at 26) |
| `g_pcmon` | Number of cards remaining (starts at 26) |

Note: `g_ppmon` and `g_pcmon` represent **card counts** in War
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
      * Tie → WAR sub-game (poker_blackjack_war_round)
   e. Check for game end:
      * Computer out of cards → "I'm out of cards! You're too good!"
      * Player out of cards → "No cards, huh? Better luck next time."
```

#### War Sub-Game (on tie)

When both players flip the same rank, a war round triggers:

1. Both sides place 3 cards face-down, then 1 card face-up
2. Higher face-up card wins all 8 cards (plus any pot)
3. If face-up cards tie again → another war round (recursive)
4. Cards are drawn via `poker_remove_top_card()` and won cards returned
   via `poker_append_card_to_pile()`

#### Screen Layout

```
+------+------+------+------+------+---------+-------------------+
| Computer's card (face-up)        | $nnn    | Status messages    |
|  row 0: cards_y_pos_a[]          | (cards) |                    |
+------+------+------+------+------+---------+-------------------+
|                                  | Pot:nnn |                    |
+------+------+------+------+------+---------+-------------------+
| Player's card (face-down→up)     | $nnn    | F1  Show           |
|  row 1: cards_y_pos_b[]          | (cards) | F10 Quit           |
+------+------+------+------+------+---------+-------------------+
|  "Show me your card, Ace."                                      |
+------------------------------------------------------------------+
```

---

### 2b. Five-Card Draw Poker

**Entry point:** `poker_main()` (0x18D10)
**Starting chips:** 400 each

#### Concept

Standard 5-card draw poker with computer AI. Players ante, receive 5 cards, bet,
optionally discard and draw new cards, bet again, then compare hands at showdown.
The computer has hand evaluation, bluff logic, and draw strategy.

#### Data Structures

| Variable | Purpose |
|---|---|
| `poker_player_hand[5]` (0x3CCF0) | Player's 5-card hand |
| `poker_computer_hand[5]` (0x3CD06) | Computer's 5-card hand |
| `poker_hand_rank_flags[5]` | Per-card flags: 1=part of scoring combo (computer) |
| `poker_player_hand_rank_flags[5]` | Same for player |
| `poker_hand_suit_flags[5]` | Sorted hand by rank (computer) |
| `poker_player_hand_suit_flags[5]` | Same for player |
| `pk_chrk` | Computer's hand rank (0–8) |
| `poker_card_selected[5]` | Cards selected for discard (1=selected) |
| `poker_discard_pile[]` | Discarded cards (prevents re-dealing) |
| `pk_disc` | Number of discarded cards |
| `pk_bluff` | YES if computer is bluffing this round |

#### Hand Ranks (`poker_evaluate_hand`, 0x18804)

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

**Bluff decision** (`poker_computer_decide_bluff`): 1/15 chance of bluffing when hand rank < 2.

**Opening check** (`poker_computer_check_opening`): Passes if no pair and not bluffing.
Otherwise finds highest card; opens only with ace or better.

**Bet decision** (`poker_computer_decide_bet`): Returns 99 (call) if weak hand and not
bluffing. Otherwise calculates raise = money/10 (capped at 1–20). Returns 114 (raise).

**Draw strategy** (`poker_computer_draw_cards`):
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
   a. ANTE PHASE (poker_ante_phase):
      - Prompt F1=Ante, F10=Quit
      - Each player puts 1 chip in pot
   b. DEAL (poker_deal_initial_hands):
      - Deal 5 random unique cards to each player
      - Computer cards face-down, player cards face-up
   c. INITIAL BET (poker_player_bet_input):
      - Player: F1=Bet (+1 chip), F3=Enter (confirm), F5=Pass/Clear
      - Max bet: 20 chips per round
      - Computer responds: call, raise, or fold
   d. DRAW PHASE:
      - Player selects cards to discard (click positions 1-5, F1=Draw, F3=Stay)
      - Selected cards shown as empty; new cards dealt from unused deck
      - Computer draws via AI strategy (poker_computer_draw_cards)
      - Computer announces: "I'll take N cards" or "I'll stay!"
   e. FINAL BET:
      - Another betting round (same as initial)
   f. SHOWDOWN (poker_showdown, 0x19A3A):
      - Evaluate both hands via poker_evaluate_hand
      - Compare ranks; tie-break by kicker cards
      - Winner takes pot via poker_settle_bet
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

**Entry point:** `poker_blackjack_main()` (0x1BC72, 623 lines)
**Starting chips:** 400 each

#### Concept

Standard blackjack rules: get as close to 21 as possible without going over.
Aces can count as 1 or 11. Face cards (10/J/Q/K) count as 10.
Supports pair splitting. Ties trigger a War sub-game.

#### Card Values (`poker_calculate_hand_score`, 0x1D1B4)

| Card Rank | Value |
|---|---|
| 2–7 | Face value (rank + 2) |
| 8, 9, 10, J, Q | 10 |
| Ace (rank 12) | 1 or 11 (ace_mode parameter) |

The `ace_mode` parameter controls ace handling:
- 0 = all aces count as 1
- 1 = first ace counts as 11, subsequent aces count as 1

#### Data Structures

| Variable | Purpose |
|---|---|
| `poker_player_hand[5]` | Player's main hand (up to 5 hit cards) |
| `poker_player_split_hand[5]` (0x3F6D2) | Player's split hand (after pair split) |
| `poker_computer_hand[5]` | Computer/dealer's hand |
| `pk_pcc` (0x501A6) | Cards dealt to player hand |
| `pk_pscc` (0x50240) | Cards dealt to split hand |
| `pk_ccc` (0x480D2) | Cards dealt to computer hand |
| `_poker_war_round` | Flag: non-zero during war resolution |
| `_poker_war_computer_score` | Computer's war status |
| `poker_blackjack_flag` (0x3D114) | Split game active flag |

#### Natural Blackjack Detection (`poker_check_natural_blackjack`, 0x1D608)

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
      - Deal 2 cards to each (poker_deal_card_to_hand)
      - Check for natural blackjack on both hands
      - Natural blackjack → immediate win (1.5x payout)
   c. SPLIT OPTION (if player's 2 cards have same rank):
      - Prompt "Do you wish to split?" F1=Split, F3=No split
      - If split: move second card to poker_player_split_hand
      - Play first hand fully, then second hand
      - Each hand gets its own bet (matched from player's chips)
   d. HIT/STAND ROUNDS (poker_blackjack_round, 0x1D294):
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
      - Tie → WAR sub-game (poker_blackjack_war_round)
      - Winner awarded pot via poker_settle_bet
   g. Check for game end
```

#### Pair Splitting

When the player's initial two cards have the same rank (e.g., two Kings):

1. Second card moved to `pk_psh`
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

**Entry point:** `word_puzzle_main()` (0x176F8)
**Data file:** `wordpz.txt` — 33 fill-in-the-blank puzzles (compressed)

### Concept

Fill-in-the-blank word puzzles. The game displays a sentence with missing words
(marked by `@`), and the player types in guesses for each blank. After all blanks
are filled, the answers are compared against the solution.

### Data Structures

| Variable | Type | Purpose |
|---|---|---|
| `g_wpdb` | char* | 2,000-byte buffer for decompressed puzzle data |
| `g_wpci` | short | Current puzzle number (0–32) |
| `wp_blk` | short | Number of `@` blanks in current puzzle |
| `word_puzzle_player_answers[][12]` | char[][] | Player's typed answers (up to 10 chars each) |
| `letter_line_ptr[66]` | char*[] | Parsed line pointers (2 lines per puzzle) |
| `word_puzzle_prompt_messages[9]` | char*[] | Prompts: "OK, what's the first word?", etc. |
| `word_puzzle_success_messages[6]` | char*[] | Success messages |
| `word_puzzle_failure_messages[6]` | char*[] | Failure messages (currently named `poker_miss_message`) |

### Puzzle File Format (`wordpz.txt`)

The file contains 33 puzzles stored as 66 lines (2 lines per puzzle):

- **Even lines** (0, 2, 4, ...): Template text with `@X` markers
  - Literal text is displayed as-is
  - `@` followed by a placeholder character marks a fill-in blank
  - The character after `@` is used as the initial display hint
- **Odd lines** (1, 3, 5, ...): Solution words separated by whitespace

Lines are delimited by control characters (ASCII < 32). The file is compressed and
decompressed into a 2,000-byte buffer. After loading, all 66 line start pointers are
stored in `g_ltlp[]`.

### Template Rendering (`word_puzzle_render_template_with_answers`, 0x17CAC)

The renderer walks the template string character by character:

1. **Space**: adds spacing (collapsed if at start of line)
2. **Literal text**: accumulates word characters, measures word length for word-wrap,
   prints each character individually in blue via `print_char()`
3. **`@` marker**: reads placeholder character, substitutes with player's answer from
   `word_puzzle_player_answers[answer_index][]`, prints in blue
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
`lcp_toupper()`, so solution words in the file must also be uppercase.

### Game Flow

```
1. Load wordpz.txt, parse into 66 line pointers
2. Start at puzzle #1
3. BROWSE LOOP:
   a. Display puzzle number: "**WORD PUZZLE # NN **"
   b. Parse template line, count @ blanks (word_puzzle_blank_count)
   c. Render template with current answers (initially placeholder chars)
   d. Wait for input:
      * F1 → next puzzle (wraps 33→1)
      * F2 → previous puzzle (wraps 1→33)
      * F5 → enter SOLVE mode
      * F10 → quit
4. SOLVE MODE (word_puzzle_solve_phase):
   a. For each blank (0 to word_puzzle_blank_count-1):
      - Display prompt:
        * First blank: random from 5 options ("OK, what's the first word?")
        * Subsequent: "Next word?", "And the next?", etc.
      - Input loop:
        * Type letters (converted to uppercase, max 10 chars)
        * Cursor-left = backspace
        * Enter = confirm answer
        * F10 = cancel and return to browse mode
      - Store answer in word_puzzle_player_answers[i][]
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
| `word_puzzle_solve_phase` | 0x1799E | Interactive answer entry and comparison |
| `word_puzzle_render_template_with_answers` | 0x17CAC | Render template with filled answers |
| `word_puzzle_show_status_message` | 0x17C80 | Display message in bottom prompt area |
| `lcp_toupper` | 0x17510 | Convert ASCII character to uppercase |

---

## Function Cross-Reference (All Mini-Game Functions)

### Anagram (9 functions)
| Address | Function |
|---|---|
| 0x181AE | `ag_main` |
| 0x18084 | `anagram_select_and_scramble_word` |
| 0x17538 | `anagram_strings_equal` |
| 0x17E34 | `anagram_display_word_large` |
| 0x17E9C | `anagram_clear_word_display_area` |
| 0x17EE4 | `anagram_show_intro_text` |
| 0x17F2E | `anagram_show_guess_prompt` |
| 0x17F4A | `anagram_clear_guess_prompt_area` |
| 0x17F82 | `anagram_clear_status_bar` |

### Card Games — Shared (11 functions)
| Address | Function |
|---|---|
| 0x1AB04 | `poker_load_card_graphics` |
| 0x1AA64 | `poker_draw_card_sprite` |
| 0x1AC92 | `poker_input_handler` |
| 0x1B0AA | `poker_print_message` |
| 0x1AD26 | `poker_display_computer_money` |
| 0x1ADE6 | `poker_display_player_money` |
| 0x1AEA6 | `poker_display_pot` |
| 0x1D78E | `poker_display_bet_with_highlight` |
| 0x1A664 | `poker_award_pot` |
| 0x1A840 | `poker_add_to_pot` |
| 0x186E0 | `play_erase_rect` |

### War (4 functions)
| Address | Function |
|---|---|
| 0x1B15C | `poker_war_main` |
| 0x1B784 | `poker_blackjack_war_round` |
| 0x1B0E0 | `poker_remove_top_card` |
| 0x1B138 | `poker_append_card_to_pile` |

### Poker (10 functions)
| Address | Function |
|---|---|
| 0x18D10 | `poker_main` |
| 0x18804 | `poker_evaluate_hand` |
| 0x1A8C2 | `poker_deal_initial_hands` |
| 0x1AF66 | `poker_ante_phase` |
| 0x1A6B8 | `poker_player_bet_input` |
| 0x187A0 | `poker_computer_decide_bet` |
| 0x1A1BC | `poker_computer_check_opening` |
| 0x1A27A | `poker_computer_draw_cards` |
| 0x18438 | `poker_computer_decide_bluff` |
| 0x19A3A | `poker_showdown` |

### Blackjack (5 functions)
| Address | Function |
|---|---|
| 0x1BC72 | `poker_blackjack_main` |
| 0x1D294 | `poker_blackjack_round` |
| 0x1D608 | `poker_check_natural_blackjack` |
| 0x1D67C | `poker_deal_card_to_hand` |
| 0x1D1B4 | `poker_calculate_hand_score` |

### Settling (1 function)
| Address | Function |
|---|---|
| 0x1D864 | `poker_settle_bet` |

### Word Puzzle (5 functions)
| Address | Function |
|---|---|
| 0x176F8 | `wp_main` |
| 0x1799E | `word_puzzle_solve_phase` |
| 0x17CAC | `word_puzzle_render_template_with_answers` |
| 0x17C80 | `word_puzzle_show_status_message` |
| 0x17510 | `lcp_toupper` |

### Game Selection
| Address | Function |
|---|---|
| 0x21860 | `a_plaag` |
| 0x1759C | `minigame_setup_screen` |
| 0x173E8 | `minigame_wait_for_key_with_events` |
