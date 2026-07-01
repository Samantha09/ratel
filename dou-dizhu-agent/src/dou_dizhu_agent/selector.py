"""Play resolver: map raw LLM output to a concrete, legal play.

LLM 直接出牌(不再选 index)。合法性由候选集成员校验保证:LLM 给出的牌必须是
generate_candidates 产出的某个合法候选的成员才采纳,否则回退到规则兜底。因此
永不出错牌、永不卡。
"""

from __future__ import annotations

import logging

from dou_dizhu_agent.rules import RANK_INDEX, VALID_CARDS

logger = logging.getLogger(__name__)


def _is_bomb(cards: list[str]) -> bool:
    """火箭(双王)或四张相同点子(炸弹)——这类牌很珍贵,兜底时不应轻易出。"""
    if len(cards) == 2 and set(cards) == {"小王", "大王"}:
        return True
    if len(cards) == 4 and len(set(cards)) == 1:
        return True
    return False


def _default_fallback(candidates: list[list[str]]) -> tuple[str, list[str]]:
    """Fallback strategy: play the smallest non-bomb candidate; avoid wasting bombs.

    LLM 返回无效/非法时兜底。炸弹/王炸很珍贵,只在别无选择时才出;
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


def _key(cards: list[str]) -> tuple[int, ...]:
    """排序键(按点数),用于候选成员比较,忽略出牌顺序。"""
    return tuple(sorted(RANK_INDEX[c] for c in cards))


def resolve(
    candidates: list[list[str]],
    raw: list[str] | str,
    last_play: list[str] | None,
) -> tuple[str, list[str]]:
    """Map raw LLM output to a concrete play with a legality guarantee.

    Args:
        candidates: Legal candidate plays from generate_candidates; [[]] means only pass.
        raw: The LLM's output — a list of card strings, "pass", or "invalid".
        last_play: The cards played by the previous player; empty/None means leading
            (cannot pass).

    Returns:
        ("play", cards) or ("pass", []).
    """
    following = bool(last_play)

    # 跟牌时过牌始终合法;领出不能过牌 → 兜底出牌。
    if raw == "pass":
        return ("pass", []) if following else _default_fallback(candidates)

    # 非法/空/含未知牌 → 兜底。
    if not isinstance(raw, list) or not raw or any(c not in VALID_CARDS for c in raw):
        return _default_fallback(candidates)

    cand_keys = {_key(c) for c in candidates if c}
    raw_key = _key(raw)
    if raw_key in cand_keys:
        # 领出护栏:开局满手牌时领出炸弹/王炸几乎从不是正着(留作王牌永远更优),
        # 而 M2.7 非确定性、prompt 无法保证它不这么干 → 用硬规则兜底。
        # 仅拦「领出 + 王牌 + 存在非炸弹候选」;跟牌用炸弹是局势判断,不拦。
        if not following and _is_bomb(raw) and any(not _is_bomb(c) for c in candidates if c):
            logger.warning("overrode bomb-on-lead %r → non-bomb fallback", raw)
            return _default_fallback(candidates)
        # 合法 → 采纳 LLM 决策,按点数规范排序返回(保留重复点数)。
        return "play", _expand(raw_key)

    return _default_fallback(candidates)


def _expand(key: tuple[int, ...]) -> list[str]:
    """Rebuild a sorted card list from a rank-index key (handles duplicate ranks)."""
    from dou_dizhu_agent.rules import CARD_RANKS

    return [CARD_RANKS[idx] for idx in key]
