"""Dou-dizhu rule engine.

Provides card pattern recognition, comparison, and candidate generation.
"""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
from typing import Iterable

CARD_RANKS = ["3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2", "小王", "大王"]
RANK_INDEX = {rank: i for i, rank in enumerate(CARD_RANKS)}
VALID_CARDS = set(CARD_RANKS)

# Special indices for jokers.
BLACK_JOKER_INDEX = RANK_INDEX["小王"]
RED_JOKER_INDEX = RANK_INDEX["大王"]


@dataclass(frozen=True)
class HandPattern:
    """A parsed dou-dizhu card pattern.

    Attributes:
        pattern_type: The kind of pattern (e.g. "single", "pair", "bomb").
        main_rank: The rank index of the primary part of the pattern.
        length: Number of primary units for patterns like straights/airplanes.
        kicker_count: Number of side cards attached (e.g. 1 for triple+1, 2 for triple+2).
        kicker_type: "single" or "pair" for airplane side cards, None otherwise.
        card_count: Total number of cards in the pattern.
    """

    pattern_type: str
    main_rank: int
    length: int = 1
    kicker_count: int = 0
    kicker_type: str | None = None
    card_count: int = 1

    def is_bomb_family(self) -> bool:
        return self.pattern_type in {"bomb", "rocket"}


def _sort_cards(cards: Iterable[str]) -> list[str]:
    """Sort cards by rank order, duplicates kept."""
    return sorted((c for c in cards if c in RANK_INDEX), key=RANK_INDEX.get)


def _validate_cards(cards: list[str]) -> bool:
    """Return True if all cards are valid dou-dizhu ranks."""
    return bool(cards) and all(c in VALID_CARDS for c in cards)


def parse_pattern(cards: list[str]) -> HandPattern | None:
    """Parse a list of cards into a HandPattern, or None if invalid."""
    if not cards or not _validate_cards(cards):
        return None

    sorted_cards = _sort_cards(cards)
    counts = Counter(sorted_cards)
    n = len(sorted_cards)

    # Rocket (both jokers).
    if n == 2 and set(sorted_cards) == {"小王", "大王"}:
        return HandPattern(pattern_type="rocket", main_rank=RED_JOKER_INDEX, card_count=2)

    # Bomb (four of a kind, non-joker).
    if n == 4 and len(counts) == 1 and sorted_cards[0] not in {"小王", "大王"}:
        return HandPattern(
            pattern_type="bomb",
            main_rank=RANK_INDEX[sorted_cards[0]],
            card_count=4,
        )

    rank_counts = Counter(RANK_INDEX[c] for c in sorted_cards)

    # Single.
    if n == 1:
        return HandPattern(pattern_type="single", main_rank=RANK_INDEX[sorted_cards[0]], card_count=1)

    # Pair.
    if n == 2 and len(counts) == 1 and sorted_cards[0] not in {"小王", "大王"}:
        return HandPattern(pattern_type="pair", main_rank=RANK_INDEX[sorted_cards[0]], card_count=2)

    # Triple.
    if n == 3 and len(counts) == 1 and sorted_cards[0] not in {"小王", "大王"}:
        return HandPattern(pattern_type="triple", main_rank=RANK_INDEX[sorted_cards[0]], card_count=3)

    # Triple + single.
    if n == 4 and sorted(rank_counts.values()) == [1, 3]:
        triple_rank = next(r for r, cnt in rank_counts.items() if cnt == 3)
        return HandPattern(
            pattern_type="triple_single",
            main_rank=triple_rank,
            kicker_count=1,
            card_count=4,
        )

    # Triple + pair.
    if n == 5 and sorted(rank_counts.values()) == [2, 3]:
        triple_rank = next(r for r, cnt in rank_counts.items() if cnt == 3)
        return HandPattern(
            pattern_type="triple_pair",
            main_rank=triple_rank,
            kicker_count=2,
            card_count=5,
        )

    # Straight (5+ consecutive singles, no 2 or jokers).
    if n >= 5 and all(cnt == 1 for cnt in rank_counts.values()):
        ranks = sorted(rank_counts)
        if ranks[-1] <= RANK_INDEX["A"] and ranks == list(range(ranks[0], ranks[0] + n)):
            return HandPattern(
                pattern_type="straight",
                main_rank=ranks[0],
                length=n,
                card_count=n,
            )

    # Consecutive pairs (3+ consecutive pairs, no 2 or jokers).
    if n >= 6 and n % 2 == 0 and all(cnt == 2 for cnt in rank_counts.values()):
        ranks = sorted(rank_counts)
        pair_count = n // 2
        if ranks[-1] <= RANK_INDEX["A"] and ranks == list(range(ranks[0], ranks[0] + pair_count)):
            return HandPattern(
                pattern_type="consecutive_pairs",
                main_rank=ranks[0],
                length=pair_count,
                card_count=n,
            )

    # Airplane: consecutive triples.
    triple_ranks = sorted(r for r, cnt in rank_counts.items() if cnt == 3)
    other_ranks = sorted(r for r, cnt in rank_counts.items() if cnt != 3)

    if len(triple_ranks) >= 2 and triple_ranks[-1] <= RANK_INDEX["A"]:
        # Find the longest consecutive run of triples.
        runs = []
        current_start = triple_ranks[0]
        current_len = 1
        for i in range(1, len(triple_ranks)):
            if triple_ranks[i] == triple_ranks[i - 1] + 1:
                current_len += 1
            else:
                runs.append((current_start, current_len))
                current_start = triple_ranks[i]
                current_len = 1
        runs.append((current_start, current_len))

        # Pick the longest run; if tie, pick the one starting lowest (standard play).
        best_start, best_len = max(runs, key=lambda x: (x[1], x[0]))
        expected_others = best_len
        other_counts = Counter(rank_counts[r] for r in other_ranks)

        # Airplane + single.
        if len(other_ranks) == expected_others and all(rank_counts[r] == 1 for r in other_ranks):
            return HandPattern(
                pattern_type="airplane_single",
                main_rank=best_start,
                length=best_len,
                kicker_count=best_len,
                kicker_type="single",
                card_count=n,
            )

        # Airplane + pair.
        if len(other_ranks) == expected_others and all(rank_counts[r] == 2 for r in other_ranks):
            return HandPattern(
                pattern_type="airplane_pair",
                main_rank=best_start,
                length=best_len,
                kicker_count=best_len,
                kicker_type="pair",
                card_count=n,
            )

        # Pure airplane (no kickers).
        if not other_ranks:
            return HandPattern(
                pattern_type="airplane",
                main_rank=best_start,
                length=best_len,
                card_count=n,
            )

    return None


def can_beat(attacker: HandPattern, defender: HandPattern) -> bool:
    """Return True if attacker can beat defender.

    Rules:
    - Rocket beats everything.
    - Bomb beats any non-bomb pattern.
    - Same pattern type with higher main_rank wins.
    - For same pattern type, length must match.
    """
    if attacker.pattern_type == "rocket":
        return True
    if defender.pattern_type == "rocket":
        return False
    if attacker.pattern_type == "bomb":
        return defender.pattern_type != "bomb" or attacker.main_rank > defender.main_rank
    if defender.pattern_type == "bomb":
        return False

    if attacker.pattern_type != defender.pattern_type:
        return False
    if attacker.length != defender.length:
        return False
    if attacker.kicker_count != defender.kicker_count:
        return False
    if attacker.kicker_type != defender.kicker_type:
        return False
    if attacker.card_count != defender.card_count:
        return False

    return attacker.main_rank > defender.main_rank


def _combinations_of_ranks(hand_counts: Counter[int], ranks: list[int], count_per_rank: int) -> list[tuple[int, ...]]:
    """Return all ways to choose `count_per_rank` cards from each of `ranks` in hand."""
    if not ranks:
        return [()]
    result = []
    first = ranks[0]
    rest = ranks[1:]
    if hand_counts[first] < count_per_rank:
        return []
    for tail in _combinations_of_ranks(hand_counts, rest, count_per_rank):
        result.append((first,) + tail)
    return result


def _generate_bombs(hand_counts: Counter[int]) -> list[list[str]]:
    """Generate all bomb patterns from hand counts."""
    bombs = []
    for rank_idx in range(RANK_INDEX["3"], RANK_INDEX["2"] + 1):
        if hand_counts[rank_idx] >= 4:
            rank_name = CARD_RANKS[rank_idx]
            bombs.append([rank_name] * 4)
    return bombs


def _generate_rocket(hand_counts: Counter[int]) -> list[list[str]]:
    if hand_counts[BLACK_JOKER_INDEX] >= 1 and hand_counts[RED_JOKER_INDEX] >= 1:
        return [["小王", "大王"]]
    return []


def _generate_straights(hand_counts: Counter[int]) -> list[list[str]]:
    """Generate all valid straights (5+ consecutive single cards, up to A)."""
    straights = []
    max_start = RANK_INDEX["A"]  # straights cannot include 2 or jokers
    for start in range(RANK_INDEX["3"], max_start + 1):
        for length in range(5, max_start - start + 2):
            ranks = list(range(start, start + length))
            if all(hand_counts[r] >= 1 for r in ranks):
                straights.append([CARD_RANKS[r] for r in ranks])
    return straights


def _generate_consecutive_pairs(hand_counts: Counter[int]) -> list[list[str]]:
    """Generate all valid consecutive pairs (3+ consecutive pairs, up to A)."""
    result = []
    max_start = RANK_INDEX["A"]
    for start in range(RANK_INDEX["3"], max_start + 1):
        for pair_count in range(3, max_start - start + 2):
            ranks = list(range(start, start + pair_count))
            if all(hand_counts[r] >= 2 for r in ranks):
                result.append([CARD_RANKS[r] for r in ranks for _ in range(2)])
    return result


def _generate_triples(hand_counts: Counter[int]) -> list[list[str]]:
    triples = []
    for rank_idx in range(RANK_INDEX["3"], RANK_INDEX["大王"] + 1):
        if hand_counts[rank_idx] >= 3:
            triples.append([CARD_RANKS[rank_idx]] * 3)
    return triples


def _generate_pairs(hand_counts: Counter[int]) -> list[list[str]]:
    pairs = []
    for rank_idx in range(RANK_INDEX["3"], RANK_INDEX["大王"] + 1):
        if hand_counts[rank_idx] >= 2:
            pairs.append([CARD_RANKS[rank_idx]] * 2)
    return pairs


def _generate_singles(hand_counts: Counter[int]) -> list[list[str]]:
    singles = []
    for rank_idx in range(RANK_INDEX["3"], RANK_INDEX["大王"] + 1):
        if hand_counts[rank_idx] >= 1:
            singles.append([CARD_RANKS[rank_idx]])
    return singles


def _generate_triple_with_kickers(
    hand_counts: Counter[int],
    kicker_count: int,
    kicker_count_per_rank: int,
) -> list[list[str]]:
    """Generate triple+1 and triple+2 patterns."""
    result = []
    for triple_rank in range(RANK_INDEX["3"], RANK_INDEX["大王"] + 1):
        if hand_counts[triple_rank] < 3:
            continue
        available = [r for r in range(RANK_INDEX["3"], RANK_INDEX["大王"] + 1) if r != triple_rank and hand_counts[r] >= kicker_count_per_rank]
        for combo in _choose_kicker_ranks(available, kicker_count // kicker_count_per_rank):
            cards = [CARD_RANKS[triple_rank]] * 3
            for kicker_rank in combo:
                cards.extend([CARD_RANKS[kicker_rank]] * kicker_count_per_rank)
            result.append(cards)
    return result


def _choose_kicker_ranks(available: list[int], k: int) -> list[tuple[int, ...]]:
    """Return all k-combinations of available ranks."""
    if k == 0:
        return [()]
    if k > len(available):
        return []
    result = []
    for i in range(len(available) - k + 1):
        for tail in _choose_kicker_ranks(available[i + 1 :], k - 1):
            result.append((available[i],) + tail)
    return result


def _generate_airplanes(
    hand_counts: Counter[int],
    kicker_type: str | None = None,
) -> list[list[str]]:
    """Generate airplane patterns (pure, +single, +pair)."""
    result = []
    max_start = RANK_INDEX["A"]
    for start in range(RANK_INDEX["3"], max_start + 1):
        for length in range(2, max_start - start + 2):
            ranks = list(range(start, start + length))
            if not all(hand_counts[r] >= 3 for r in ranks):
                continue

            remaining = Counter(hand_counts)
            for r in ranks:
                remaining[r] -= 3

            if kicker_type is None:
                result.append([CARD_RANKS[r] for r in ranks for _ in range(3)])
            elif kicker_type == "single":
                available = [r for r in range(RANK_INDEX["3"], RANK_INDEX["大王"] + 1) if remaining[r] >= 1]
                if len(available) >= length:
                    for combo in _choose_kicker_ranks(available, length):
                        cards = [CARD_RANKS[r] for r in ranks for _ in range(3)]
                        for kicker_rank in combo:
                            cards.append(CARD_RANKS[kicker_rank])
                        result.append(cards)
            elif kicker_type == "pair":
                available = [r for r in range(RANK_INDEX["3"], RANK_INDEX["大王"] + 1) if remaining[r] >= 2]
                if len(available) >= length:
                    for combo in _choose_kicker_ranks(available, length):
                        cards = [CARD_RANKS[r] for r in ranks for _ in range(3)]
                        for kicker_rank in combo:
                            cards.extend([CARD_RANKS[kicker_rank]] * 2)
                        result.append(cards)
    return result


def _all_first_plays(hand_counts: Counter[int]) -> list[list[str]]:
    """Generate all valid first-play patterns from a hand."""
    candidates: list[list[str]] = []
    candidates.extend(_generate_singles(hand_counts))
    candidates.extend(_generate_pairs(hand_counts))
    candidates.extend(_generate_triples(hand_counts))
    candidates.extend(_generate_triple_with_kickers(hand_counts, 1, 1))
    candidates.extend(_generate_triple_with_kickers(hand_counts, 2, 2))
    candidates.extend(_generate_straights(hand_counts))
    candidates.extend(_generate_consecutive_pairs(hand_counts))
    candidates.extend(_generate_airplanes(hand_counts, None))
    candidates.extend(_generate_airplanes(hand_counts, "single"))
    candidates.extend(_generate_airplanes(hand_counts, "pair"))
    candidates.extend(_generate_bombs(hand_counts))
    candidates.extend(_generate_rocket(hand_counts))
    return candidates


def generate_candidates(hand: list[str], last_play: list[str] | None) -> list[list[str]]:
    """Generate all legal plays from hand given last_play.

    If last_play is empty/None, return all valid first plays.
    If no play can beat last_play, return [[]] (PASS).
    """
    if not hand:
        return [[]]

    hand_counts = Counter(RANK_INDEX[c] for c in hand if c in RANK_INDEX)

    if not last_play:
        plays = _all_first_plays(hand_counts)
        return plays if plays else [[]]

    last_pattern = parse_pattern(last_play)
    if last_pattern is None:
        # If last play is somehow invalid, treat as first play.
        plays = _all_first_plays(hand_counts)
        return plays if plays else [[]]

    candidates: list[list[str]] = []
    for play in _all_first_plays(hand_counts):
        play_pattern = parse_pattern(play)
        if play_pattern and can_beat(play_pattern, last_pattern):
            candidates.append(play)

    return candidates if candidates else [[]]


def pattern_name(pattern: HandPattern) -> str:
    """Human-readable pattern name."""
    return pattern.pattern_type


def format_cards(cards: list[str]) -> str:
    """Format a list of cards as a string."""
    return " ".join(cards)
