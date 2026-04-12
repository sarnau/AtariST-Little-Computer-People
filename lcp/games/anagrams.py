"""
Anagram word game for Little Computer People (Atari ST).
Translated from Ghidra decompilation of anagram_main().

addr: anagram_main(), anagram_select_and_scramble_word(),
      anagram_match_result()

Rules:
  - A word is scrambled (10–20 random pair swaps) and displayed
  - Player has up to 8 guesses to unscramble it
  - F1 reveals one clue letter per turn (costs 1 guess)
  - Clue: swaps one letter into its correct position in the scrambled display
  - If all letters revealed by clues: "You took too many clues!"
  - Dictionary: 150 words from DATA/WORDS (compressed, 11 bytes each)
  - 3 random wrong-guess messages

Card/word encoding: words are stored as 11-byte null/space/period-terminated
strings in the WORDS data file.
"""

import random
from typing import Optional


# Wrong guess messages (3 entries, randomly selected)
# addr: anagram_wrong_guess_messages[3]
WRONG_GUESS_MESSAGES = [
    "Nope. Try again.",
    "Sorry, that's wrong.",
    "Not quite right.",
]


def scramble_word(word: str) -> str:
    """
    Scramble a word with 10–20 random pair swaps.
    Re-scrambles if result matches original.
    addr: anagram_select_and_scramble_word()
    """
    letters = list(word)
    word_len = len(letters)
    if word_len < 2:
        return word

    while True:
        # Check if already different from original
        if ''.join(letters) != word:
            break
        # Perform 10–20 random swaps
        num_swaps = random.randint(10, 20)
        for _ in range(num_swaps):
            a = random.randint(0, word_len - 1)
            b = random.randint(0, word_len - 1)
            letters[a], letters[b] = letters[b], letters[a]

    return ''.join(letters)


def strings_match(a: str, b: str) -> bool:
    """
    Case-sensitive string comparison.
    Returns True if identical, False if different.
    addr: anagram_match_result()
    """
    return a == b


class AnagramGame:
    """
    Anagram word game.
    addr: anagram_main()
    """

    def __init__(self, word_list: list[str]) -> None:
        # Words are stored lowercase, terminated by '.' or space in the original
        self.word_list = [w.strip().rstrip('.').lower() for w in word_list
                          if len(w.strip().rstrip('.')) >= 2]
        self.original: str = ''
        self.scrambled: str = ''
        self.word_length: int = 0
        self.guess_number: int = 1
        self.clue_count: int = 0
        self.all_clues_used: bool = False
        self.clue_used_this_round: bool = False
        self.message: str = ''
        self.round_over: bool = False

    def new_round(self) -> dict:
        """
        Pick a random word and scramble it.
        addr: anagram_select_and_scramble_word()
        """
        # Select random word (0–149 in original, index × 11 bytes)
        idx = random.randint(0, min(149, len(self.word_list) - 1))
        self.original = self.word_list[idx]
        self.word_length = len(self.original)
        self.scrambled = scramble_word(self.original)
        self.guess_number = 1
        self.clue_count = 0
        self.all_clues_used = False
        self.clue_used_this_round = False
        self.round_over = False
        self.message = ''

        return {
            'scrambled': self.scrambled,
            'length': self.word_length,
            'guess_number': self.guess_number,
        }

    def _start_turn(self) -> Optional[dict]:
        """
        Check if the round should end before accepting input.
        addr: anagram_main() top-of-loop exit check
        """
        self.clue_used_this_round = False
        # Exit condition: guess > 8 AND (guess > 9 OR not all_clues_used)
        if self.guess_number > 8 and (self.guess_number > 9 or not self.all_clues_used):
            self.round_over = True
            return {'outcome': 'round_over'}
        return None

    def guess(self, word: str) -> dict:
        """
        Submit a guess. Input is trimmed and lowercased.
        addr: anagram_main() guess evaluation
        """
        # Check turn validity
        turn_check = self._start_turn()
        if turn_check:
            return turn_check

        word = word.strip().lower()

        if strings_match(word, self.original):
            self.message = "YOU GOT IT!!!!!!"
            self.round_over = True
            return {
                'outcome': 'correct',
                'word': self.original,
                'message': self.message,
            }

        # Wrong guess
        if self.guess_number > 7:
            # 8th wrong guess — too many guesses
            self.guess_number += 1
            self.message = "Sorry, too many guesses!"
            self.round_over = True
            return {
                'outcome': 'too_many_guesses',
                'word': self.original,
                'message': self.message,
            }

        self.guess_number += 1
        self.message = random.choice(WRONG_GUESS_MESSAGES)
        return {
            'outcome': 'wrong',
            'guess_number': self.guess_number,
            'message': self.message,
        }

    def request_clue(self) -> dict:
        """
        Reveal one clue letter (F1). Costs 1 guess.
        Only 1 clue per turn. Swaps one letter into correct position.
        addr: anagram_main() F1 clue logic
        """
        if self.clue_used_this_round:
            return {'outcome': 'clue_already_used'}

        if self.round_over:
            return {'outcome': 'round_over'}

        # Check if scrambled already matches original (all revealed)
        if strings_match(self.original, self.scrambled):
            return {'outcome': 'all_revealed'}

        self.clue_count += 1
        self.guess_number += 1
        self.clue_used_this_round = True

        if self.guess_number == 9:
            self.all_clues_used = True

        # Find first mismatched position
        scrambled_list = list(self.scrambled)
        mismatch_pos = 0
        for i in range(self.word_length):
            if self.original[i] != scrambled_list[i]:
                mismatch_pos = i
                break

        # Find where the correct letter is in scrambled word (search from end)
        correct_char = self.original[mismatch_pos]
        swap_pos = self.word_length - 1
        while swap_pos > mismatch_pos:
            if scrambled_list[swap_pos] == correct_char:
                break
            swap_pos -= 1

        # Swap to unscramble one position
        scrambled_list[swap_pos], scrambled_list[mismatch_pos] = (
            scrambled_list[mismatch_pos], scrambled_list[swap_pos]
        )
        self.scrambled = ''.join(scrambled_list)

        # Check if all letters now revealed
        if strings_match(self.original, self.scrambled):
            self.message = "You took too many clues!"
            self.round_over = True
            return {
                'outcome': 'too_many_clues',
                'scrambled': self.scrambled,
                'word': self.original,
                'guess_number': self.guess_number,
                'message': self.message,
            }

        return {
            'outcome': 'clue',
            'scrambled': self.scrambled,
            'guess_number': self.guess_number,
            'clue_count': self.clue_count,
        }
