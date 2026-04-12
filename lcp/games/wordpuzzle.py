"""
Word Puzzle mini-game for Little Computer People (Atari ST).
Translated from Ghidra decompilation of word_puzzle_main().

addr: word_puzzle_main()

Rules:
  - 33 fill-in-the-blank puzzles loaded from DATA/WORDPZ.TXT
  - File has 66 lines: even lines = template (with '@' marking blanks),
    odd lines = answer text
  - F1/next → advance puzzle, F2/prev → go back, F5/solve → enter solve phase
  - Player types their answer for each blank
"""

from typing import Optional


NUM_PUZZLES = 33   # 0x21 puzzles (indices 0–32)


def parse_wordpuzzle_data(raw: bytes) -> list[tuple[str, str]]:
    """
    Parse the decompressed wordpz.txt buffer into (template, answer) pairs.
    addr: word_puzzle_main() parse loop — 66 lines, even=template, odd=answer
    Lines are separated by any byte < 0x20 (control chars).
    """
    lines: list[str] = []
    start = 0
    i = 0
    data = raw if isinstance(raw, (bytes, bytearray)) else raw.encode()
    while i < len(data) and len(lines) < 66:
        if data[i] < 0x20:
            # End of line
            line = data[start:i].decode('latin-1', errors='replace')
            lines.append(line)
            # Skip all trailing control chars
            while i < len(data) and data[i] < 0x20:
                i += 1
            start = i
        else:
            i += 1
    # Flush last line if any
    if start < len(data) and len(lines) < 66:
        line = data[start:].decode('latin-1', errors='replace')
        if line:
            lines.append(line)

    puzzles: list[tuple[str, str]] = []
    for idx in range(NUM_PUZZLES):
        template = lines[idx * 2] if idx * 2 < len(lines) else ''
        answer   = lines[idx * 2 + 1] if idx * 2 + 1 < len(lines) else ''
        puzzles.append((template, answer))
    return puzzles


def get_blanks(template: str) -> list[str]:
    """
    Extract the answer characters that fill each '@' blank in the template.
    addr: word_puzzle_main() blank extraction loop —
    '@' is followed immediately by the answer character for that blank.
    """
    blanks: list[str] = []
    i = 0
    while i < len(template):
        if template[i] == '@':
            if i + 1 < len(template):
                blanks.append(template[i + 1])
            i += 2
        else:
            i += 1
    return blanks


def render_template(template: str, player_answers: list[str]) -> str:
    """
    Render the puzzle template, substituting player answers for '@x' slots.
    addr: word_puzzle_render_template_with_answers()
    """
    result = []
    blank_idx = 0
    i = 0
    while i < len(template):
        if template[i] == '@':
            # '@x' → replace with player answer (or '_' if not answered)
            if blank_idx < len(player_answers) and player_answers[blank_idx]:
                result.append(player_answers[blank_idx])
            else:
                result.append('_')
            blank_idx += 1
            i += 2   # skip '@' and the answer char
        else:
            result.append(template[i])
            i += 1
    return ''.join(result)


class WordPuzzleGame:
    """
    Word Puzzle mini-game.
    addr: word_puzzle_main()
    """

    def __init__(self, puzzles: list[tuple[str, str]]) -> None:
        """
        puzzles: list of (template, answer) tuples, as returned by parse_wordpuzzle_data().
        Templates use '@x' to mark blanks (x = the correct answer char).
        """
        self.puzzles          = puzzles
        self.current_index    = 0
        self.player_answers:  list[str] = []
        self.blank_count      = 0
        self.message          = ''
        self._load_puzzle(0)

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _load_puzzle(self, index: int) -> None:
        self.current_index   = index % NUM_PUZZLES
        template, _answer    = self.puzzles[self.current_index]
        self.blank_count     = template.count('@')
        self.player_answers  = [''] * self.blank_count
        self.message         = ''

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    @property
    def puzzle_number(self) -> int:
        """1-based puzzle number (matches on-screen display)."""
        return self.current_index + 1

    @property
    def template(self) -> str:
        return self.puzzles[self.current_index][0]

    @property
    def correct_answers(self) -> list[str]:
        """The correct fill-in characters extracted from the template."""
        return get_blanks(self.template)

    def rendered(self) -> str:
        """Current state of the puzzle with player answers substituted."""
        return render_template(self.template, self.player_answers)

    def next_puzzle(self) -> dict:
        """Advance to the next puzzle (wraps at 33)."""
        self._load_puzzle(self.current_index + 1)
        return self._state()

    def prev_puzzle(self) -> dict:
        """Go back to the previous puzzle (wraps at 0 → 32)."""
        idx = self.current_index - 1
        if idx < 0:
            idx = NUM_PUZZLES - 1
        self._load_puzzle(idx)
        return self._state()

    def goto_puzzle(self, index: int) -> dict:
        """Jump to puzzle by 0-based index."""
        self._load_puzzle(index)
        return self._state()

    def fill_blank(self, blank_index: int, char: str) -> dict:
        """
        Fill in a single blank slot (0-based) with a player-supplied character.
        Returns outcome dict.
        """
        if blank_index < 0 or blank_index >= self.blank_count:
            return {'outcome': 'invalid_blank'}
        self.player_answers[blank_index] = char[:1].upper() if char else ''
        return self._state()

    def solve(self) -> dict:
        """
        Reveal the correct answers for all blanks (F5 in original game).
        addr: word_puzzle_solve_phase()
        """
        self.player_answers = list(self.correct_answers)
        self.message = 'Solved!'
        return self._state(outcome='solved')

    def check(self) -> dict:
        """
        Check whether the player's current answers match the correct answers.
        Returns outcome='correct' or 'wrong' with details.
        """
        correct = self.correct_answers
        if self.player_answers == correct:
            self.message = 'Correct!'
            return self._state(outcome='correct')
        wrong_slots = [i for i, (p, c) in enumerate(zip(self.player_answers, correct)) if p != c]
        self.message = 'Wrong answer.'
        return self._state(outcome='wrong', wrong_slots=wrong_slots)

    # ------------------------------------------------------------------

    def _state(self, **extra) -> dict:
        state = {
            'puzzle_number':   self.puzzle_number,
            'rendered':        self.rendered(),
            'blank_count':     self.blank_count,
            'player_answers':  list(self.player_answers),
            'message':         self.message,
        }
        state.update(extra)
        return state
