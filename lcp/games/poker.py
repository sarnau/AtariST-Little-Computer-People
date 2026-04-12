"""
5-card draw Poker mini-game for Little Computer People (Atari ST).
Translated from Ghidra decompilation of poker_main(), poker_evaluate_hand(),
poker_computer_draw_cards(), poker_computer_decide_bluff(), poker_showdown().

addr: poker_main(), poker_evaluate_hand(), poker_computer_draw_cards(),
      poker_computer_decide_bluff(), poker_computer_decide_bet(),
      poker_showdown(), poker_ante_phase(), poker_evaluate_hands()

Rules:
  - 5-card draw poker
  - Both players start with 400 chips
  - Ante 1 chip each round
  - Bet up to 20 chips per betting round
  - Computer AI: evaluates hand, decides draw, optional bluff (1-in-15 chance)
  - Showdown: higher hand_rank wins; ties broken by kicker/high-card comparison

Hand ranks (poker_evaluate_hand):
  0 = high card
  1 = one pair
  2 = two pair
  3 = three of a kind
  4 = straight
  5 = flush
  6 = full house
  7 = four of a kind
  8 = straight flush
  9 = royal flush

Card encoding (shared with war.py / blackjack.py):
  card % 13 = rank (0=2, 1=3, ..., 8=10, 9=J, 10=Q, 11=K, 12=A)
  card // 13 = suit (0=hearts, 1=diamonds, 2=clubs, 3=spades)
"""

import random
from typing import Optional
from .war import shuffle_deck, card_rank, RANK_NAMES, SUIT_NAMES


STARTING_MONEY = 400
MAX_BET        = 20   # poker_computer_decide_bet caps raise at 20

# Sentinel for 'no card'
NO_CARD = 0xFF


# ---------------------------------------------------------------------------
# Hand evaluation
# addr: poker_evaluate_hand()
# ---------------------------------------------------------------------------

def _bubble_sort_hand(hand: list[int]) -> list[int]:
    """Bubble-sort a 5-card hand by rank (ascending). addr: inline sort in poker_evaluate_hand()"""
    s = hand[:]
    swapped = True
    while swapped:
        swapped = False
        for i in range(4):
            if s[i + 1] % 13 < s[i] % 13:
                s[i], s[i + 1] = s[i + 1], s[i]
                swapped = True
    return s


def evaluate_hand(hand: list[int]) -> tuple[int, list[int]]:
    """
    Evaluate a 5-card poker hand.
    Returns (hand_rank, rank_flags) where rank_flags[i]=1 marks cards
    belonging to the best combination (pair/trips/quads/full-house pair).
    addr: poker_evaluate_hand()

    Hand ranks: 0=high card, 1=one pair, 2=two pair, 3=three of a kind,
                4=straight, 5=flush, 6=full house, 7=four of a kind,
                8=straight flush, 9=royal flush
    """
    sorted_hand = _bubble_sort_hand(hand)
    rank_flags  = [0] * 5

    # --- Straight detection ---
    is_straight = True
    for i in range(3):
        if sorted_hand[i] % 13 != sorted_hand[i + 1] % 13 - 1:
            is_straight = False
            break
    # Special: A-2-3-4-5 (wheel): top card rank=12 (Ace), bottom rank=0
    is_wheel = (sorted_hand[4] % 13 == 12 and is_straight and sorted_hand[0] % 13 == 0)
    if not is_wheel and sorted_hand[3] % 13 != sorted_hand[4] % 13 - 1:
        is_straight = False

    # --- Flush detection ---
    is_flush = all(sorted_hand[i] // 13 == sorted_hand[i + 1] // 13 for i in range(4))

    hand_rank = 0
    if is_straight:
        hand_rank = 4
    if is_flush:
        hand_rank = 5
    if is_straight and is_flush:
        hand_rank = 8
    # Royal flush: straight flush with lowest rank = 8 (10)
    if hand_rank == 8 and sorted_hand[0] % 13 == 8:
        hand_rank = 9
    if hand_rank != 0:
        return hand_rank, rank_flags

    # --- Count matching ranks (pairs/trips/quads) ---
    # high_card_rank: best group size seen (1=pair, 3=trips, 7=quads sentinel)
    # best_pair_rank: second group
    high_card_rank = 0
    best_pair_rank = 0
    rank_counts = [0] * 5    # flags for first group
    suit_counts = [0] * 5    # flags for second group

    for target_rank in range(13):
        group_cards = [i for i in range(5) if hand[i] % 13 == target_rank]
        count = len(group_cards)

        if count == 4:
            high_card_rank = 7
            rank_flags = [1 if i in group_cards else 0 for i in range(5)]
            break

        if count == 3:
            if high_card_rank == 0:
                high_card_rank = 3
                rank_counts = [1 if i in group_cards else 0 for i in range(5)]
                rank_flags = rank_counts[:]
            elif best_pair_rank == 0:
                best_pair_rank = 3
                for i in group_cards:
                    rank_flags[i] = 1
                break   # full house — stop scanning

        if count == 2:
            if high_card_rank == 0:
                high_card_rank = 1
                rank_counts = [1 if i in group_cards else 0 for i in range(5)]
                rank_flags = rank_counts[:]
            elif best_pair_rank == 0:
                if high_card_rank == 1:
                    best_pair_rank = 1
                    for i in group_cards:
                        rank_flags[i] = 1   # mark second pair
                elif high_card_rank == 3:
                    best_pair_rank = 1      # full house: trips + pair

    combined = best_pair_rank + high_card_rank
    if combined == 7:
        hand_rank = 7   # four of a kind
    elif combined == 4:
        hand_rank = 6   # full house (3+1)
    elif combined == 3:
        hand_rank = 3   # three of a kind
    elif combined == 2:
        hand_rank = 2   # two pair
    elif combined == 1:
        hand_rank = 1   # one pair
    else:
        hand_rank = 0   # high card

    return hand_rank, rank_flags


# ---------------------------------------------------------------------------
# Showdown comparison
# addr: poker_showdown() tie-breaking logic
# ---------------------------------------------------------------------------

def compare_hands(
    computer_hand: list[int],
    player_hand: list[int],
) -> int:
    """
    Compare two evaluated hands. Returns:
      0 → computer wins
      1 → player wins
    addr: poker_showdown() comparison block
    """
    comp_rank,  comp_flags  = evaluate_hand(computer_hand)
    plyr_rank,  plyr_flags  = evaluate_hand(player_hand)

    if plyr_rank < comp_rank:
        return 0   # computer wins
    if comp_rank < plyr_rank:
        return 1   # player wins

    # Equal ranks — tie-breaking
    rank = comp_rank
    comp_sorted = _bubble_sort_hand(computer_hand)
    plyr_sorted = _bubble_sort_hand(player_hand)

    # Straight, flush, straight flush: compare highest card
    if rank in (4, 5, 8):
        if plyr_sorted[4] % 13 < comp_sorted[4] % 13:
            return 0
        return 1

    # Four of a kind, full house, three of a kind: compare the group card
    if rank in (7, 6, 3):
        comp_group = next((i for i in range(5) if comp_flags[i] == 1), 0)
        plyr_group = next((i for i in range(5) if plyr_flags[i] == 1), 0)
        if player_hand[plyr_group] % 13 < computer_hand[comp_group] % 13:
            return 0
        return 1

    # Two pair: compare higher pair, then lower pair, then kicker
    if rank == 2:
        # Find high pair and low pair for each
        comp_pair_ranks = sorted(set(
            computer_hand[i] % 13 for i in range(5) if comp_flags[i]
        ), reverse=True)
        plyr_pair_ranks = sorted(set(
            player_hand[i] % 13 for i in range(5) if plyr_flags[i]
        ), reverse=True)
        for cr, pr in zip(comp_pair_ranks, plyr_pair_ranks):
            if pr < cr:
                return 0
            if cr < pr:
                return 1
        # Kicker
        comp_kicker = next(
            (computer_hand[i] % 13 for i in range(5) if not comp_flags[i]), 0
        )
        plyr_kicker = next(
            (player_hand[i] % 13 for i in range(5) if not plyr_flags[i]), 0
        )
        return 0 if plyr_kicker < comp_kicker else 1

    # One pair: compare pair rank, then kicker cards high→low
    if rank == 1:
        comp_pair_rank = next(
            (computer_hand[i] % 13 for i in range(5) if comp_flags[i]), 0
        )
        plyr_pair_rank = next(
            (player_hand[i] % 13 for i in range(5) if plyr_flags[i]), 0
        )
        if plyr_pair_rank < comp_pair_rank:
            return 0
        if comp_pair_rank < plyr_pair_rank:
            return 1
        # Compare sorted hands high→low (kicker comparison)
        for i in range(4, -1, -1):
            if plyr_sorted[i] % 13 < comp_sorted[i] % 13:
                return 0
            if comp_sorted[i] % 13 < plyr_sorted[i] % 13:
                return 1
        return 1   # exact tie → player wins (original: poker_card_display_slot = 1)

    # High card: compare all cards high→low
    for i in range(4, -1, -1):
        if plyr_sorted[i] % 13 < comp_sorted[i] % 13:
            return 0
        if comp_sorted[i] % 13 < plyr_sorted[i] % 13:
            return 1
    return 1   # exact tie → player wins


# ---------------------------------------------------------------------------
# AI helpers
# ---------------------------------------------------------------------------

def _deal_hand(exclude: list[int]) -> int:
    """Deal one random card not already in use. addr: rejection-sample loop in poker_evaluate_hands()"""
    while True:
        card = random.randint(0, 51)
        if card not in exclude:
            return card


def computer_decide_bluff(hand_rank: int) -> bool:
    """
    1-in-15 bluff chance, only when hand_rank < 2 (pair or worse).
    addr: poker_computer_decide_bluff()
    """
    return random.randint(0, 14) == 0 and hand_rank < 2


def computer_draw_strategy(
    hand: list[int],
    bluffing: bool,
) -> tuple[list[int], int]:
    """
    Decide which card indices to replace.
    Returns (discard_indices, draw_count).
    addr: poker_computer_draw_cards()

    Draw strategy:
      hand_rank 0 (high card)    → keep highest card, draw 4
      hand_rank 1 (one pair)     → keep pair, draw 3
      hand_rank 2 (two pair)     → keep both pairs, draw 1
      hand_rank 3 (three of a kind) → keep trips, draw 2
      hand_rank ≥ 4 (straight+) → stay, draw 0
      bluffing                   → draw random 0-2 non-flagged cards
    """
    hand_rank, rank_flags = evaluate_hand(hand)

    if bluffing:
        draw_count = random.randint(0, 2)
        discards = []
        for i in range(5):
            if rank_flags[i] == 0 and len(discards) < draw_count:
                discards.append(i)
        return discards, draw_count

    if hand_rank >= 4:
        return [], 0

    if hand_rank == 3:
        return [i for i in range(5) if rank_flags[i] == 0], 2

    if hand_rank == 2:
        return [i for i in range(5) if rank_flags[i] == 0], 1

    if hand_rank == 1:
        return [i for i in range(5) if rank_flags[i] == 0], 3

    # High card: keep highest-rank card, discard rest
    best_idx = max(range(5), key=lambda i: hand[i] % 13)
    return [i for i in range(5) if i != best_idx], 4


def computer_raise_amount(money: int) -> int:
    """
    Compute raise size: money // 10, clamped to [1, 20].
    addr: poker_computer_decide_bet() raise path
    """
    amount = money // 10
    if amount == 0:
        amount = 1
    if amount > MAX_BET:
        amount = MAX_BET
    return amount


# ---------------------------------------------------------------------------
# Main game class
# ---------------------------------------------------------------------------

class PokerGame:
    """
    5-card draw Poker game.
    addr: poker_main()
    """

    def __init__(self) -> None:
        self.player_money:   int = STARTING_MONEY
        self.computer_money: int = STARTING_MONEY
        self.pot:            int = 0
        self.player_hand:    list[int] = []
        self.computer_hand:  list[int] = []
        self.discard_pile:   list[int] = []
        self.hand_rank:      int = 0         # computer hand rank after evaluate
        self.player_rank:    int = 0
        self.bluffing:       bool = False
        self.message:        str = ''
        self.round_count:    int = 0

    # ------------------------------------------------------------------

    def ante(self) -> dict:
        """
        Both players put 1 chip into the pot.
        addr: poker_ante_phase()
        """
        if self.player_money == 0:
            return {'outcome': 'player_broke', 'message': "Sorry, you're all out!!!"}
        if self.computer_money == 0:
            return {'outcome': 'computer_broke', 'message': "I'm all out!!!"}
        self.player_money   -= 1
        self.computer_money -= 1
        self.pot = 2
        self.message = 'Ante up to play.'
        return {'outcome': 'ok', 'pot': self.pot}

    def deal(self) -> dict:
        """
        Deal 5 cards to each player (rejection sampling, no duplicates).
        addr: poker_evaluate_hands() — despite the name, this is the deal function
        """
        used: list[int] = []
        self.computer_hand = []
        self.player_hand   = []
        self.discard_pile  = []

        for _ in range(5):
            c = _deal_hand(used); used.append(c); self.computer_hand.append(c)
            p = _deal_hand(used); used.append(p); self.player_hand.append(p)

        self.hand_rank, _  = evaluate_hand(self.computer_hand)
        self.bluffing      = computer_decide_bluff(self.hand_rank)
        return {
            'player_hand':    self.player_hand[:],
            'computer_hand':  ['?' ] * 5,   # face down
        }

    def player_draw(self, discard_indices: list[int]) -> dict:
        """
        Replace the player's chosen cards with fresh ones.
        addr: player draw loop in poker_main()
        """
        used = self.computer_hand + self.player_hand + self.discard_pile
        for i in discard_indices:
            self.discard_pile.append(self.player_hand[i])
            card = _deal_hand(used)
            used.append(card)
            self.player_hand[i] = card
        self.player_rank, _ = evaluate_hand(self.player_hand)
        return {'player_hand': self.player_hand[:], 'drew': len(discard_indices)}

    def computer_draw(self) -> dict:
        """
        Computer draws cards according to AI strategy.
        addr: poker_computer_draw_cards()
        """
        discards, draw_count = computer_draw_strategy(self.computer_hand, self.bluffing)
        used = self.computer_hand + self.player_hand + self.discard_pile
        for i in discards:
            self.discard_pile.append(self.computer_hand[i])
            card = _deal_hand(used)
            used.append(card)
            self.computer_hand[i] = card
        self.hand_rank, _ = evaluate_hand(self.computer_hand)
        msg = "I'll stay!" if draw_count == 0 else f"I'll take {draw_count} card{'s' if draw_count != 1 else ''}."
        self.message = msg
        return {'drew': draw_count, 'message': msg}

    def computer_bet(self) -> tuple[int, bool]:
        """
        Computer decides whether to bet/raise or pass.
        Returns (amount, passed).
        addr: poker_computer_decide_bet() + bluff raise logic in poker_main()
        """
        if self.computer_money == 0:
            return 0, True
        if not self.bluffing and self.hand_rank < 2:
            # No pair or worse and not bluffing → pass
            return 0, True
        amount = computer_raise_amount(self.computer_money)
        amount = min(amount, self.computer_money)
        return amount, False

    def place_computer_bet(self, amount: int) -> None:
        """Transfer chips from computer to pot."""
        amount = min(amount, self.computer_money)
        self.computer_money -= amount
        self.pot            += amount

    def place_player_bet(self, amount: int) -> dict:
        """Transfer chips from player to pot."""
        amount = min(amount, self.player_money)
        self.player_money -= amount
        self.pot          += amount
        return {'player_money': self.player_money, 'pot': self.pot}

    def showdown(self) -> dict:
        """
        Reveal hands and determine winner.
        addr: poker_showdown()
        """
        self.round_count += 1
        winner = compare_hands(self.computer_hand, self.player_hand)
        self.player_rank, _  = evaluate_hand(self.player_hand)
        self.hand_rank,   _  = evaluate_hand(self.computer_hand)

        if winner == 1:
            self.player_money   += self.pot
            self.message         = "You're so lucky!!!"
        else:
            self.computer_money += self.pot
            self.message         = 'I win!!!'
        self.pot = 0

        return {
            'winner':          'player' if winner == 1 else 'computer',
            'player_hand':     self.player_hand[:],
            'computer_hand':   self.computer_hand[:],
            'player_rank':     self.player_rank,
            'computer_rank':   self.hand_rank,
            'player_money':    self.player_money,
            'computer_money':  self.computer_money,
            'message':         self.message,
        }

    def award_pot_to_player(self) -> dict:
        """Player wins pot (computer folds). addr: poker_ante_and_new_round(1)"""
        self.player_money += self.pot
        self.pot = 0
        self.message = 'Your pot.'
        return {'player_money': self.player_money, 'message': self.message}

    def award_pot_to_computer(self) -> dict:
        """Computer wins pot (player folds). addr: poker_ante_and_new_round(0)"""
        self.computer_money += self.pot
        self.pot = 0
        self.message = 'My pot.'
        return {'computer_money': self.computer_money, 'message': self.message}

    def game_over(self) -> Optional[str]:
        if self.player_money <= 0:
            return 'computer'
        if self.computer_money <= 0:
            return 'player'
        return None

    def hand_name(self, rank: int) -> str:
        names = [
            'High Card', 'One Pair', 'Two Pair', 'Three of a Kind',
            'Straight', 'Flush', 'Full House', 'Four of a Kind',
            'Straight Flush', 'Royal Flush',
        ]
        return names[rank] if 0 <= rank <= 9 else 'Unknown'
