"""
War card game for Little Computer People (Atari ST).
Translated from Ghidra decompilation of poker_war_main().

addr: poker_war_main()

Rules: 52-card deck split equally. Both players flip one card.
Higher rank wins both cards. Ties → War: deal 3 face-down + 1 face-up, compare.
First player to accumulate all 52 cards wins.

Card encoding (from Ghidra decompilation):
  card % 13 = rank (0=2, 1=3, ..., 8=10, 9=J, 10=Q, 11=K, 12=A)
  card // 13 = suit (0=hearts, 1=diamonds, 2=clubs, 3=spades)
"""

import random
from typing import Optional


CARD_VALUES = {i: i + 2 for i in range(9)}   # ranks 0-8 → values 2-10
CARD_VALUES.update({9: 11, 10: 12, 11: 13, 12: 14})   # J=11, Q=12, K=13, A=14

RANK_NAMES = ['2','3','4','5','6','7','8','9','10','J','Q','K','A']
SUIT_NAMES = ['♥','♦','♣','♠']


def card_rank(card: int) -> int:
    return card % 13

def card_suit(card: int) -> int:
    return card // 13

def card_name(card: int) -> str:
    return RANK_NAMES[card_rank(card)] + SUIT_NAMES[card_suit(card)]

def card_value(card: int) -> int:
    return CARD_VALUES[card_rank(card)]


def shuffle_deck() -> list[int]:
    """
    Create and shuffle a 52-card deck.
    Original: 400 random swap passes.
    addr: poker_war_main() deck init
    """
    deck = list(range(52))
    for _ in range(400):
        i = random.randint(0, 51)
        j = random.randint(0, 51)
        deck[i], deck[j] = deck[j], deck[i]
    return deck


class WarGame:
    """
    5-card War card game.
    addr: poker_war_main()
    """

    def __init__(self) -> None:
        deck = shuffle_deck()
        # Alternate deal: even indices → computer, odd → player (matches Ghidra)
        self.computer_pile: list[int] = [deck[i] for i in range(0, 52, 2)]
        self.player_pile: list[int] = [deck[i] for i in range(1, 52, 2)]
        self.message: str = ''

    @property
    def player_count(self) -> int:
        return len(self.player_pile)

    @property
    def computer_count(self) -> int:
        return len(self.computer_pile)

    def winner(self) -> Optional[str]:
        if not self.player_pile:
            return 'computer'
        if not self.computer_pile:
            return 'player'
        return None

    def play_round(self) -> dict:
        """
        Play one round of War.  Returns a result dict with:
          player_card, computer_card, outcome ('player'|'computer'|'war'),
          war_cards (list of cards committed to war pot, if any)
        addr: poker_war_main() round logic
        """
        if not self.player_pile or not self.computer_pile:
            return {'outcome': 'game_over'}

        p_card = self.player_pile.pop(0)
        c_card = self.computer_pile.pop(0)
        war_pot = [p_card, c_card]

        result = {
            'player_card': p_card,
            'computer_card': c_card,
            'war_cards': [],
            'outcome': '',
        }

        p_val = card_value(p_card)
        c_val = card_value(c_card)

        if p_val > c_val:
            # Player wins — cards go to player's pile
            self.player_pile.extend(war_pot)
            result['outcome'] = 'player'
            self.message = self._player_win_message(p_card)
        elif c_val > p_val:
            # Computer wins — cards go to computer's pile
            self.computer_pile.extend(war_pot)
            result['outcome'] = 'computer'
            self.message = self._computer_win_message(c_card, p_card)
        else:
            # War — commit 3 face-down + 1 face-up from each side
            war_result = self._war_round(war_pot)
            result['war_cards'] = war_result.get('war_cards', [])
            result['outcome'] = war_result['outcome']

        return result

    def _war_round(self, existing_pot: list[int]) -> dict:
        """
        Resolve a tie with a War sub-round.
        addr: poker_war_main() war tie-breaking
        3 face-down + 1 face-up from each side, higher face-up wins all.
        """
        war_cards = list(existing_pot)
        # Each side contributes 3 face-down + 1 face-up (4 cards total)
        needed = 4
        for _ in range(needed):
            if self.player_pile:
                war_cards.append(self.player_pile.pop(0))
            if self.computer_pile:
                war_cards.append(self.computer_pile.pop(0))

        # Compare the last player card vs last computer card (face-up)
        # Odd indices in war_cards are computer, even are player (simplified)
        if len(war_cards) < 4:
            # Not enough cards — whoever has more wins
            outcome = 'player' if self.player_count > self.computer_count else 'computer'
        else:
            # Last two cards added are player_face_up and computer_face_up
            p_face = war_cards[-2] if len(war_cards) >= 2 else war_cards[-1]
            c_face = war_cards[-1]
            if card_value(p_face) >= card_value(c_face):
                random.shuffle(war_cards)
                self.player_pile.extend(war_cards)
                outcome = 'player'
                self.message = 'You win the war!'
            else:
                random.shuffle(war_cards)
                self.computer_pile.extend(war_cards)
                outcome = 'computer'
                self.message = 'Computer wins the war!'

        return {'outcome': outcome, 'war_cards': war_cards}

    def _player_win_message(self, p_card: int) -> str:
        """Computer's reaction when player wins. addr: poker_war_main() player-win branch"""
        if card_rank(p_card) == 12:   # Ace
            return "Ace? I don't believe it!"
        msgs = [
            "You're awfully lucky!",
            "Arrghh!",
            "You're tough.",
            "I'll get you next time.",
            "Dog-gone it.",
            "All right. Slow down.",
        ]
        return random.choice(msgs)

    def _computer_win_message(self, c_card: int, p_card: int) -> str:
        """Computer's reaction when computer wins. addr: poker_war_main() computer-win branch"""
        c_rank = card_rank(c_card)
        p_rank = card_rank(p_card)
        diff = c_rank - p_rank

        if c_rank == 12:   # Ace
            return "Ace takes it!"
        elif diff < 3:
            return random.choice([
                "Hmm... That's not too bad!",
                "Whew! That was too close.",
            ])
        elif diff < 7:
            if p_rank < 4:
                return random.choice([
                    "Not a very high card, but I'll take it.",
                    "That's an easy card to beat.",
                ])
            elif p_rank < 9:
                return random.choice([
                    "Alright. I win!",
                    "Better luck next time.",
                    "Hey... look at that!",
                ])
            else:
                return "Great, a face card, and it's mine now!"
        else:
            return random.choice([
                "No contest. You lose!",
                "Beat you by a mile.",
                "That was easy!",
            ])
