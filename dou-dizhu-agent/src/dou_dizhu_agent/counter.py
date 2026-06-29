"""Card counter for dou-dizhu.

Tracks the distribution of cards still held by opponents based on visible
information: our hand, bottom cards, played history, and last play.
"""

from __future__ import annotations

from collections import Counter

from dou_dizhu_agent.rules import CARD_RANKS

# Standard dou-dizhu deck distribution.
STANDARD_COUNTS: dict[str, int] = {rank: 4 for rank in CARD_RANKS}
STANDARD_COUNTS["小王"] = 1
STANDARD_COUNTS["大王"] = 1


def _cards_from_history(history: list[dict] | None) -> list[str]:
    """Flatten history entries into a list of cards."""
    if not history:
        return []
    cards: list[str] = []
    for entry in history:
        if isinstance(entry, dict):
            cards.extend(entry.get("cards", []))
        elif isinstance(entry, list):
            cards.extend(entry)
    return cards


class CardCounter:
    """Maintains remaining card counts for all ranks.

    Visible cards are subtracted from the standard deck. Cards held by
    opponents are inferred from the remaining total and their reported counts.
    """

    def __init__(
        self,
        hand: list[str],
        bottom_cards: list[str] | None = None,
        history: list[dict] | None = None,
    ):
        self.remaining = Counter(STANDARD_COUNTS)
        self._subtract(hand)
        if bottom_cards:
            self._subtract(bottom_cards)
        self._subtract(_cards_from_history(history))

    def _subtract(self, cards: list[str]) -> None:
        for card in cards:
            if card in self.remaining:
                self.remaining[card] -= 1

    def update(
        self,
        hand: list[str],
        last_play: list[str] | None = None,
        history: list[dict] | None = None,
        bottom_cards: list[str] | None = None,
    ) -> None:
        """Reset and recompute remaining distribution from the latest state."""
        self.remaining = Counter(STANDARD_COUNTS)
        self._subtract(hand)
        if bottom_cards:
            self._subtract(bottom_cards)
        if history:
            self._subtract(_cards_from_history(history))
        if last_play:
            self._subtract(last_play)

    def summary(self) -> str:
        """Return a concise summary of remaining cards suitable for an LLM prompt."""
        parts = []
        for rank in CARD_RANKS:
            count = self.remaining.get(rank, 0)
            parts.append(f"{rank}:{count}")
        return " ".join(parts)

    def count(self, rank: str) -> int:
        return self.remaining.get(rank, 0)

    def total_remaining(self) -> int:
        return sum(self.remaining.values())
