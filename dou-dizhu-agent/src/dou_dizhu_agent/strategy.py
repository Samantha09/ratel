"""Strategy module: ask an LLM to play dou-dizhu DIRECTLY (output cards).

设计要点(2026-07-01 实测):不再把合法候选清单塞进 prompt 让模型「选 index」——
那会诱发推理模型逐项深度比较,思考量爆 max_tokens → 截断 → 无答案 → 回退最笨规则牌。
改为只给手牌与局面,让模型直接出牌(自然语言最省思考);合法性由 api 层的候选集
成员校验 + 规则兜底保证。
"""

from __future__ import annotations

import logging
import os
from typing import Protocol

import httpx
from langchain_openai import ChatOpenAI

from dou_dizhu_agent.rules import VALID_CARDS

logger = logging.getLogger(__name__)

DEFAULT_SYSTEM_PROMPT = """你是经验丰富的斗地主高手,冷静、有策略,目标是用最小代价赢最多的局。

【输出格式 · 必须严格遵守】
只输出你要出的牌,空格分隔,点数用精确写法:3 4 5 6 7 8 9 10 J Q K A 2 小王 大王
例如 "9 9"、"6 7 8 9 10"、"4 4 4 6"。要过牌就只输出 "pass"。不要任何解释或多余文字。

【角色与目标】
清楚自己是地主还是农民(农民需与队友配合)。目标是尽快出完手牌。

【核心原则】
- 能走则走,不贪炸:有明确赢牌路线时优先走完,不为凑炸弹或炫技拖延。
- 控制节奏:地主要压场;农民要找机会让队友接牌或自己跑掉。
- 配合优先:农民之间互相送牌,不内耗、不抢队友要跑的牌。
- 关键牌是安全垫:炸弹(四张同点)和王炸(小王 大王)只在「对手剩余 ≤2 张须封堵」
  「衔接自己的收官赢牌」「打破关键控场」时使用;绝不用于领出,也不拿来管一张小牌。
- 出牌果断:局势不利保存实力,有利时不给对手机会。

【出牌策略】
当 landlord(地主):
- 优先打出能收回出牌权的牌型(对子、三带一、顺子),不要一上来就把大牌全甩出去,留一手兜底。
- 农民明显在互相送牌时,用大牌打断他们的节奏。
- 手牌单张多且没王时,先出单张消耗农民的大牌。
当 peasant(农民):
- 队友明显要跑时不要抢牌,让他走;队友走不掉时主动接管。
- 地主出小牌时,能用大牌顶就顶,不让他舒服过牌。
- 不与队友内耗。

【读牌与记牌】
- 依「剩余牌摘要」推断炸弹/王炸是否还在;小王/大王若显示为 0,说明已出或在某家手里。
- 注意某家长时间不出某类牌型,可能在憋顺子、飞机或炸弹。
- 观察对手最后几张牌,判断其牌型结构。
"""


def _build_user_prompt(
    hand: list[str],
    role: str,
    last_play: list[str],
    other_counts: dict[str, int],
    card_counter_summary: str,
) -> str:
    """Build the natural-language prompt sent to the LLM.

    手牌用牌名(模型训练里最熟的形式,最省思考)。不列候选清单,也不列底牌
    (底牌信息已被剩余牌摘要隐含)。
    """
    if last_play:
        last_line = (
            f"Last play by opponent: {' '.join(last_play)}   "
            "(beat it with the same pattern, or a bomb/rocket, or pass)"
        )
    else:
        last_line = "You are first to play (lead; you cannot pass)."
    return "\n".join(
        [
            f"Role: {role}",
            f"My hand ({len(hand)} cards): {' '.join(hand)}",
            last_line,
            f"Opponent card counts: {other_counts}",
            f"Remaining cards summary: {card_counter_summary}",
            "Output ONLY the cards you play (or 'pass').",
        ]
    )


def _parse_play(text: str) -> list[str] | str:
    """Parse LLM output into a card list, 'pass', or 'invalid'.

    Returns the list of valid card tokens, 'pass', or 'invalid' (empty/unparseable).
    """
    text = (text or "").strip()
    if text.lower() == "pass":
        return "pass"
    tokens = [t for t in text.replace(",", " ").split() if t in VALID_CARDS]
    return tokens if tokens else "invalid"


class StrategyModule(Protocol):
    """Protocol for strategy modules."""

    def choose(
        self,
        hand: list[str],
        role: str,
        last_play: list[str],
        other_counts: dict[str, int],
        card_counter_summary: str,
    ) -> list[str] | str:
        """Return a card list (the play), 'pass', or 'invalid'."""
        ...


class OpenAICompatibleStrategy:
    """Strategy backed by an OpenAI-compatible chat model (default: MiniMax)."""

    def __init__(
        self,
        api_key: str | None = None,
        base_url: str | None = None,
        model: str | None = None,
        temperature: float = 0.3,
        timeout: float = 10.0,
    ):
        self.api_key = api_key or os.getenv("LLM_API_KEY") or os.getenv("MINIMAX_API_KEY")
        self.base_url = base_url or os.getenv("LLM_BASE_URL") or os.getenv("MINIMAX_BASE_URL")
        self.model = model or os.getenv("LLM_MODEL") or os.getenv("MINIMAX_MODEL") or "minimax-text-01"
        self.temperature = temperature
        self.timeout = timeout
        self._llm: ChatOpenAI | None = None

    def _get_llm(self) -> ChatOpenAI:
        if self._llm is None:
            if not self.api_key:
                raise RuntimeError(
                    "LLM API key not configured. Set LLM_API_KEY or MINIMAX_API_KEY environment variable."
                )
            self._llm = ChatOpenAI(
                api_key=self.api_key,
                base_url=self.base_url,
                model=self.model,
                temperature=self.temperature,
                timeout=self.timeout,
            )
        return self._llm

    def choose(
        self,
        hand: list[str],
        role: str,
        last_play: list[str],
        other_counts: dict[str, int],
        card_counter_summary: str,
    ) -> list[str] | str:
        """Ask the model to play directly and parse the cards from the response."""
        if not hand:
            return "pass"
        try:
            llm = self._get_llm()
            messages = [
                ("system", DEFAULT_SYSTEM_PROMPT),
                (
                    "human",
                    _build_user_prompt(hand, role, last_play, other_counts, card_counter_summary),
                ),
            ]
            response = llm.invoke(messages)
            text = response.content
            if not isinstance(text, str):
                text = str(text)
            logger.debug("LLM response: %s", text)
            return _parse_play(text)
        except Exception:
            logger.exception("LLM strategy call failed")
            return "invalid"


class MockStrategy:
    """Deterministic strategy for tests: returns a fixed play / 'pass' / 'invalid'."""

    def __init__(self, play: list[str] | str = "invalid"):
        self.play = play

    def choose(
        self,
        hand: list[str],
        role: str,
        last_play: list[str],
        other_counts: dict[str, int],
        card_counter_summary: str,
    ) -> list[str] | str:
        return self.play


class MinimaxAnthropicStrategy:
    """MiniMax via its Anthropic-compatible endpoint (POST {base}/v1/messages).

    M2.7 是重度推理模型,thinking 关不掉也卡不长(实测 thinking:disabled 与
    budget_tokens 均被忽略);因此不再把候选清单塞进 prompt(那会诱发逐项比较 →
    思考爆 max_tokens → 截断),而是给手牌让模型直接出牌。思考与最终答案分属不同
    content 块,解析只取 text 块;max_tokens 给足(默认 4096)让思考+答案放得下。
    """

    def __init__(
        self,
        api_key: str | None = None,
        base_url: str | None = None,
        model: str | None = None,
        temperature: float = 0.3,
        timeout: float = 90.0,
        max_tokens: int = 4096,
    ):
        self.api_key = api_key or os.getenv("LLM_API_KEY") or os.getenv("MINIMAX_API_KEY")
        self.base_url = (
            base_url
            or os.getenv("LLM_BASE_URL")
            or os.getenv("MINIMAX_BASE_URL")
            or "https://api.minimaxi.com/anthropic"
        ).rstrip("/")
        self.model = (
            model
            or os.getenv("LLM_MODEL")
            or os.getenv("MINIMAX_MODEL")
            or "MiniMax-M2.7-highspeed"
        )
        self.temperature = temperature
        self.timeout = timeout
        self.max_tokens = max_tokens

    def choose(
        self,
        hand: list[str],
        role: str,
        last_play: list[str],
        other_counts: dict[str, int],
        card_counter_summary: str,
    ) -> list[str] | str:
        """Ask the model to play directly and parse the text-block response."""
        if not hand:
            return "pass"
        if not self.api_key:
            raise RuntimeError(
                "LLM API key not configured. Set LLM_API_KEY or MINIMAX_API_KEY environment variable."
            )
        try:
            prompt = _build_user_prompt(hand, role, last_play, other_counts, card_counter_summary)
            resp = httpx.post(
                f"{self.base_url}/v1/messages",
                headers={
                    "x-api-key": self.api_key,
                    "anthropic-version": "2023-06-01",
                    "content-type": "application/json",
                },
                json={
                    "model": self.model,
                    "max_tokens": self.max_tokens,
                    "temperature": self.temperature,
                    "system": DEFAULT_SYSTEM_PROMPT,
                    "messages": [{"role": "user", "content": prompt}],
                },
                timeout=self.timeout,
            )
            resp.raise_for_status()
            data = resp.json()
            # 即便残留 thinking 块也忽略,只取文本块拼接。
            text = "".join(
                block.get("text", "")
                for block in data.get("content", [])
                if block.get("type") == "text"
            )
            if not text.strip():
                logger.warning("LLM returned no text block: %s", data)
                return "invalid"
            logger.debug("LLM text response: %s", text)
            return _parse_play(text)
        except Exception:
            logger.exception("LLM (anthropic) strategy call failed")
            return "invalid"


def create_strategy(
    provider: str = "minimax",
    api_key: str | None = None,
    base_url: str | None = None,
    model: str | None = None,
    temperature: float = 0.3,
    timeout: float = 90.0,
    max_tokens: int = 4096,
) -> StrategyModule:
    """Factory for creating a strategy module.

    Supported providers:
      - 'minimax': MiniMax via Anthropic-compatible endpoint (direct play).
      - 'openai': any OpenAI-compatible endpoint (langchain).
    """
    if provider == "minimax":
        return MinimaxAnthropicStrategy(
            api_key=api_key,
            base_url=base_url,
            model=model,
            temperature=temperature,
            timeout=timeout,
            max_tokens=max_tokens,
        )
    if provider == "openai":
        return OpenAICompatibleStrategy(
            api_key=api_key,
            base_url=base_url,
            model=model,
            temperature=temperature,
        )
    raise ValueError(f"Unsupported strategy provider: {provider!r}")
