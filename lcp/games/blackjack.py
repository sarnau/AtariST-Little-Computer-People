"""
Blackjack/21 mini-game for Little Computer People (Atari ST).
Translated from Ghidra decompilation of poker_blackjack_main().

addr: poker_blackjack_main(), poker_calculate_hand_score(),
      poker_check_natural_blackjack(), poker_blackjack_round()

Rules:
  - Standard Blackjack / 21
  - Dealer hits up to 3 times, stands on > 16
  - Natural blackjack (Ace + 10-value) wins immediately
  - Pair splitting supported (matching ranks)
  - Double-down supported (doubles bet, gets exactly 1 card)
  - Max 5 cards per hand, max bet of 20

Card encoding (from Ghidra decompilation):
  card % 13 = rank (0=2, 1=3, ..., 8=10, 9=J, 10=Q, 11=K, 12=A)
  card // 13 = suit (0=hearts, 1=diamonds, 2=clubs, 3=spades)
"""

import random
from typing import Optional
from .war import shuffle_deck, card_rank, card_name, RANK_NAMES, SUIT_NAMES


# Sentinel for empty card slots (matches Ghidra 0xFF)
CARD_EMPTY = 0xFF
MAX_HAND_SIZE = 5
MAX_BET = 20


def calculate_hand_score(cards: list[int], ace_mode: int) -> int:
    """
    Calculate hand score with two ace modes (matches Ghidra exactly).
    ace_mode=0: all aces count as 1
    ace_mode=1: first ace counts as 11, subsequent aces count as 1
    addr: poker_calculate_hand_score()
    """
    score = 0
    ace_high_used = False
    for card in cards:
        if card == CARD_EMPTY:
            break
        rank = card % 13
        if rank == 12:  # Ace
            if ace_mode == 0:
                score += 1
            elif ace_high_used:
                score += 1
            else:
                score += 11
                ace_high_used = True
        elif rank > 7:  # 10, J, Q, K (ranks 8-11)
            score += 10
        else:
            score += rank + 2  # ranks 0-7 → values 2-9
    return score


def best_hand_score(cards: list[int]) -> int:
    """
    Return the best (highest non-bust) score for a hand.
    Uses both ace modes and picks the best ≤ 21, or lowest if both bust.
    addr: poker_blackjack_main() showdown score logic
    """
    lo = calculate_hand_score(cards, 0)  # aces as 1
    hi = calculate_hand_score(cards, 1)  # first ace as 11
    if hi <= 21:
        return hi
    return lo


def is_natural_blackjack(cards: list[int]) -> bool:
    """
    Detect a natural blackjack (exactly 2 cards: Ace + 10-value card).
    addr: poker_check_natural_blackjack()
    """
    rank0 = cards[0] % 13
    rank1 = cards[1] % 13
    if rank0 == 12 and rank1 < 12 and rank1 > 7:
        return True
    if rank1 == 12 and rank0 < 12 and rank0 > 7:
        return True
    return False


def is_pair(cards: list[int]) -> bool:
    """Return True if first two cards are a pair (same rank), enabling splitting."""
    return cards[0] % 13 == cards[1] % 13


def hit_score(cards: list[int]) -> int:
    """
    Score used during hit rounds — aces always count as 1.
    addr: poker_blackjack_round() inline score calculation
    """
    return calculate_hand_score(cards, 0)


class BlackjackGame:
    """
    Blackjack / 21 game.
    addr: poker_blackjack_main()
    """

    STARTING_MONEY = 400  # addr: poker_blackjack_main() init

    def __init__(self) -> None:
        self.deck: list[int] = []
        self.player_hand: list[int] = []
        self.split_hand: list[int] = []     # split hand (poker_player_split_hand)
        self.dealer_hand: list[int] = []    # poker_computer_hand
        self.player_money: int = self.STARTING_MONEY
        self.computer_money: int = self.STARTING_MONEY
        self.main_bet: int = 0              # poker_computer_bet
        self.split_bet: int = 0             # poker_player_bet
        self.split_active: bool = False     # poker_game_phase != 0
        self.double_down_main: bool = False
        self.double_down_split: bool = False
        self.message: str = ''
        self._refill_deck()

    def _refill_deck(self) -> None:
        """Shuffle a fresh 52-card deck."""
        self.deck = shuffle_deck()

    def _deal(self) -> int:
        if not self.deck:
            self._refill_deck()
        return self.deck.pop(0)

    def _hand_count(self, hand: list[int]) -> int:
        """Count non-empty cards in hand."""
        return sum(1 for c in hand if c != CARD_EMPTY)

    def place_bet(self, amount: int) -> dict:
        """
        Place a bet (1–20, capped by player money).
        addr: poker_blackjack_main() betting phase
        """
        amount = max(1, min(amount, MAX_BET, self.player_money))
        self.main_bet = amount
        self.player_money -= amount
        return {
            'bet': self.main_bet,
            'player_money': self.player_money,
        }

    def deal_initial(self) -> dict:
        """
        Deal initial 4 cards: player, dealer(down), player, dealer(up).
        addr: poker_blackjack_main() deal phase
        """
        self._refill_deck()
        self.split_active = False
        self.split_hand = []
        self.split_bet = 0
        self.double_down_main = False
        self.double_down_split = False

        # Initialize hands with 0xFF sentinels
        self.player_hand = [CARD_EMPTY] * MAX_HAND_SIZE
        self.dealer_hand = [CARD_EMPTY] * MAX_HAND_SIZE
        self.split_hand = [CARD_EMPTY] * MAX_HAND_SIZE

        # Deal: player face-up, dealer face-down, player face-up, dealer face-up
        self.player_hand[0] = self._deal()
        self.dealer_hand[0] = self._deal()   # face down initially
        self.player_hand[1] = self._deal()
        self.dealer_hand[1] = self._deal()   # face up

        p_natural = is_natural_blackjack(self.player_hand)
        d_natural = is_natural_blackjack(self.dealer_hand)

        result: dict = {
            'player_hand': self._visible_cards(self.player_hand),
            'dealer_up_card': self.dealer_hand[1],  # second card is face-up
            'player_score': best_hand_score(self._visible_cards(self.player_hand)),
            'can_split': is_pair(self.player_hand),
            'player_natural': p_natural,
            'dealer_natural': d_natural,
            'outcome': None,
        }

        if p_natural and d_natural:
            # Both have blackjack — push
            self.message = "You have BLACKJACK...but so do I !!"
            self._settle_push()
            result['outcome'] = 'push'
            result['message'] = self.message
        elif p_natural:
            self.message = "You have BLACKJACK!!"
            self._settle_player_wins()
            result['outcome'] = 'player_blackjack'
            result['message'] = self.message
        elif d_natural:
            self.message = "I have BLACKJACK!!"
            self._settle_computer_wins()
            result['outcome'] = 'dealer_blackjack'
            result['message'] = self.message

        return result

    def _visible_cards(self, hand: list[int]) -> list[int]:
        """Return non-empty cards from a hand."""
        return [c for c in hand if c != CARD_EMPTY]

    def _next_slot(self, hand: list[int]) -> int:
        """Find next empty slot in hand."""
        for i in range(MAX_HAND_SIZE):
            if hand[i] == CARD_EMPTY:
                return i
        return -1

    def can_split(self) -> bool:
        """Check if split is available."""
        return (not self.split_active and
                self._hand_count(self.player_hand) == 2 and
                is_pair(self.player_hand) and
                self.player_money >= self.main_bet)

    def player_split(self) -> dict:
        """
        Split a pair into two separate hands.
        addr: poker_blackjack_main() split logic
        """
        if not self.can_split():
            return {'error': 'cannot split'}

        self.split_active = True
        # Move second card to split hand
        self.split_hand[0] = self.player_hand[1]
        self.player_hand[1] = CARD_EMPTY

        # Deal one card to each hand
        self.player_hand[1] = self._deal()
        self.split_hand[1] = self._deal()

        # Match the bet on the split hand
        self.split_bet = 0
        bet_to_match = self.main_bet
        for _ in range(bet_to_match):
            if self.player_money == 0:
                break
            self.player_money -= 1
            self.split_bet += 1

        result: dict = {
            'hand1': self._visible_cards(self.player_hand),
            'hand2': self._visible_cards(self.split_hand),
            'score1': best_hand_score(self._visible_cards(self.player_hand)),
            'score2': best_hand_score(self._visible_cards(self.split_hand)),
            'split_bet': self.split_bet,
            'player_money': self.player_money,
        }

        # Check for natural blackjack on either split hand
        if is_natural_blackjack(self.player_hand):
            result['hand1_blackjack'] = True
            self.message = "You have BLACKJACK!!"
            self._settle_player_wins_bet('main')

        if is_natural_blackjack(self.split_hand):
            result['hand2_blackjack'] = True
            self.message = "You have BLACKJACK!!"
            self._settle_player_wins_bet('split')

        return result

    def can_double_down(self, hand: str = 'main') -> bool:
        """Check if double-down is available for a hand."""
        if hand == 'main':
            return self.player_money >= self.main_bet and not self.double_down_main
        else:
            return self.player_money >= self.split_bet and not self.double_down_split

    def player_double_down(self, hand: str = 'main') -> dict:
        """
        Double-down: double the bet, receive exactly one more card.
        addr: poker_blackjack_main() double-down logic
        """
        if hand == 'main':
            extra = self.main_bet
            for _ in range(extra):
                if self.player_money == 0:
                    break
                self.player_money -= 1
                self.main_bet += 1
            self.double_down_main = True
            target = self.player_hand
        else:
            extra = self.split_bet
            for _ in range(extra):
                if self.player_money == 0:
                    break
                self.player_money -= 1
                self.split_bet += 1
            self.double_down_split = True
            target = self.split_hand

        # Deal exactly one card
        slot = self._next_slot(target)
        if slot >= 0:
            target[slot] = self._deal()

        cards = self._visible_cards(target)
        score = hit_score(cards)
        bust = score > 21

        self.message = "Here's your card."
        return {
            'hand': cards,
            'score': best_hand_score(cards),
            'hit_score': score,
            'bust': bust,
            'bet': self.main_bet if hand == 'main' else self.split_bet,
            'player_money': self.player_money,
        }

    def player_hit(self, hand: str = 'main') -> dict:
        """
        Player takes another card (F1=Hit).
        addr: poker_blackjack_round() hit logic
        """
        target = self.player_hand if hand == 'main' else self.split_hand
        slot = self._next_slot(target)
        if slot < 0:
            self.message = "You cannot take any more cards."
            return {
                'hand': self._visible_cards(target),
                'score': best_hand_score(self._visible_cards(target)),
                'bust': False,
                'max_cards': True,
            }

        target[slot] = self._deal()
        cards = self._visible_cards(target)
        score = hit_score(cards)  # aces=1 for bust check
        bust = score > 21

        # Check if hand is full (5 cards)
        at_max = self._next_slot(target) < 0
        if at_max and not bust:
            self.message = "You cannot take any more cards."

        return {
            'hand': cards,
            'score': best_hand_score(cards),
            'hit_score': score,
            'bust': bust,
            'max_cards': at_max,
        }

    def player_stand(self, hand: str = 'main') -> dict:
        """Player stands (F3=Stand). addr: poker_blackjack_round() stand"""
        target = self.player_hand if hand == 'main' else self.split_hand
        cards = self._visible_cards(target)
        return {
            'hand': cards,
            'score': best_hand_score(cards),
        }

    def dealer_play(self) -> dict:
        """
        Dealer plays: hits up to 3 times, stands on > 16.
        Uses both ace modes to find best score.
        addr: poker_blackjack_main() dealer AI
        """
        cards = self._visible_cards(self.dealer_hand)
        hit_messages = [
            "I'll take a hit.",
            "I'll take another hit.",
            "I'll take one more.",
        ]
        dealer_busted = False
        hits_taken = 0

        for i in range(3):
            lo = calculate_hand_score(cards, 0)  # aces as 1
            hi = calculate_hand_score(cards, 1)  # first ace as 11

            if lo > 21 and hi > 21:
                dealer_busted = True
                break

            best = hi if hi <= 21 else lo
            if best > 16:
                self.message = "I'll stand."
                break

            self.message = hit_messages[i]
            slot = self._next_slot(self.dealer_hand)
            if slot >= 0:
                self.dealer_hand[slot] = self._deal()
                cards = self._visible_cards(self.dealer_hand)
                hits_taken += 1
        else:
            # Took all 3 hits — final check
            lo = calculate_hand_score(cards, 0)
            hi = calculate_hand_score(cards, 1)
            if lo > 21 and hi > 21:
                dealer_busted = True
            else:
                best = hi if hi <= 21 else lo
                self.message = "I'll stand."

        if dealer_busted:
            self.message = "I've busted !!"

        dealer_score = best_hand_score(cards)
        return {
            'dealer_hand': cards,
            'dealer_score': dealer_score,
            'dealer_busted': dealer_busted,
            'hits_taken': hits_taken,
        }

    def resolve(self, hand: str = 'main') -> dict:
        """
        Compare player hand vs dealer and settle bets.
        addr: poker_blackjack_main() showdown
        """
        target = self.player_hand if hand == 'main' else self.split_hand
        p_cards = self._visible_cards(target)
        p_score = best_hand_score(p_cards)

        dealer_info = self.dealer_play()
        d_score = dealer_info['dealer_score']
        dealer_busted = dealer_info['dealer_busted']

        hand_label = ''
        if self.split_active:
            hand_label = 'first' if hand == 'main' else 'second'

        if p_score > 21:
            # Player already busted (shouldn't reach here normally)
            outcome = 'dealer_wins'
            if hand_label:
                self.message = f"Your {hand_label} hand is busted !!"
            else:
                self.message = "You've busted!!!"
            self._settle_computer_wins_bet(hand)
        elif dealer_busted:
            outcome = 'player_wins'
            if hand_label:
                self.message = f"You win with your {hand_label} hand."
            else:
                self.message = "You win."
            self._settle_player_wins_bet(hand)
        elif p_score > d_score:
            outcome = 'player_wins'
            if hand_label:
                self.message = f"You win with your {hand_label} hand."
            else:
                self.message = "You win."
            self._settle_player_wins_bet(hand)
        elif d_score > p_score:
            outcome = 'dealer_wins'
            if hand_label:
                self.message = f"Your {hand_label} hand loses."
            else:
                self.message = "I win."
            self._settle_computer_wins_bet(hand)
        else:
            outcome = 'push'
            if hand_label:
                self.message = f"{hand_label.capitalize()} hand ties, nobody wins."
            else:
                self.message = "It's a tie and nobody wins."
            self._settle_push_bet(hand)

        return {
            'player_score': p_score,
            'dealer_score': d_score,
            'dealer_hand': dealer_info['dealer_hand'],
            'dealer_busted': dealer_busted,
            'outcome': outcome,
            'message': self.message,
            'player_money': self.player_money,
            'computer_money': self.computer_money,
        }

    # --- Bet settlement helpers ---
    # addr: poker_settle_bet()

    def _settle_player_wins(self) -> None:
        """Player wins main bet — return bet + win from computer."""
        amount = self.main_bet
        self.player_money += amount  # return bet
        if self.computer_money >= amount:
            self.computer_money -= amount
            self.player_money += amount  # winnings
        else:
            self.player_money += self.computer_money
            self.computer_money = 0
        self.main_bet = 0

    def _settle_computer_wins(self) -> None:
        """Computer wins main bet — take bet to computer."""
        self.computer_money += self.main_bet
        self.main_bet = 0

    def _settle_push(self) -> None:
        """Push — return bet to player."""
        self.player_money += self.main_bet
        self.main_bet = 0

    def _settle_player_wins_bet(self, hand: str) -> None:
        """Player wins specified hand's bet."""
        if hand == 'main':
            amount = self.main_bet
            self.player_money += amount
            if self.computer_money >= amount:
                self.computer_money -= amount
                self.player_money += amount
            else:
                self.player_money += self.computer_money
                self.computer_money = 0
            self.main_bet = 0
        else:
            amount = self.split_bet
            self.player_money += amount
            if self.computer_money >= amount:
                self.computer_money -= amount
                self.player_money += amount
            else:
                self.player_money += self.computer_money
                self.computer_money = 0
            self.split_bet = 0

    def _settle_computer_wins_bet(self, hand: str) -> None:
        """Computer wins specified hand's bet."""
        if hand == 'main':
            self.computer_money += self.main_bet
            self.main_bet = 0
        else:
            self.computer_money += self.split_bet
            self.split_bet = 0

    def _settle_push_bet(self, hand: str) -> None:
        """Push — return bet to player."""
        if hand == 'main':
            self.player_money += self.main_bet
            self.main_bet = 0
        else:
            self.player_money += self.split_bet
            self.split_bet = 0

    def game_over(self) -> Optional[str]:
        """Check if either player is out of money."""
        if self.player_money <= 0:
            self.message = "Game's over. I win."
            return 'computer'
        if self.computer_money <= 0:
            self.message = "I'm all out!!"
            return 'player'
        return None
