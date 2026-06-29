"""Play selector: maps an LLM choice to concrete cards."""

from __future__ import annotations

from dou_dizhu_agent.rules import RANK_INDEX, generate_candidates


def _is_bomb(cards: list[str]) -> bool:
    """火箭(双王)或四张相同点子(炸弹)——这类牌很珍贵,兜底时不应轻易出。"""
    if len(cards) == 2 and set(cards) == {"小王", "大王"}:
        return True
    if len(cards) == 4 and len(set(cards)) == 1:
        return True
    return False


def _default_fallback(candidates: list[list[str]]) -> tuple[str, list[str]]:
    """Fallback strategy: play the smallest non-bomb candidate; avoid wasting bombs.

    LLM 返回无效时兜底。炸弹/王炸很珍贵,只在别无选择时才出;
    否则出最小的普通牌型;无候选则过牌。

    旧实现按 (len, ranks) 取最小,会优先选张数最少的牌——而王炸只有 2 张、
    比任何顺子都短,导致每次 LLM 无效就白白甩出王炸。这里把炸弹排除后再选最小。
    """
    if not candidates or candidates == [[]]:
        return "pass", []
    non_bombs = [c for c in candidates if c and not _is_bomb(c)]
    pool = non_bombs if non_bombs else [c for c in candidates if c]
    if not pool:
        return "pass", []
    smallest = min(pool, key=lambda c: (len(c), [RANK_INDEX[card] for card in c]))
    return "play", smallest


def select(
    candidates: list[list[str]],
    choice: int | str,
) -> tuple[str, list[str]]:
    """Map the LLM's choice to a concrete play.

    Args:
        candidates: List of legal candidate plays; [[]] means only pass.
        choice: Candidate index (int), "pass", or "invalid".

    Returns:
        ("play", cards) or ("pass", []).
    """
    if choice == "pass":
        return "pass", []

    if choice == "invalid":
        return _default_fallback(candidates)

    if isinstance(choice, int):
        if 0 <= choice < len(candidates):
            selected = candidates[choice]
            if not selected:
                return "pass", []
            return "play", selected

    return _default_fallback(candidates)


def select_from_request(
    hand: list[str],
    last_play: list[str] | None,
    choice: int | str,
) -> tuple[str, list[str]]:
    """Convenience wrapper that generates candidates then selects.

    Useful for callers that want a single entry point.
    """
    candidates = generate_candidates(hand, last_play)
    return select(candidates, choice)
