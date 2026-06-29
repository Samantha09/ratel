"""Strategy module for choosing a play candidate using an LLM."""

from __future__ import annotations

import logging
import os
import re
from typing import Protocol

import httpx
from langchain_openai import ChatOpenAI

logger = logging.getLogger(__name__)

DEFAULT_SYSTEM_PROMPT = """You are a dou-dizhu (Chinese card game) player.
Your job is to choose the best legal play from the provided candidates.
Respond with ONLY the candidate index (0-based integer) or the word "pass".
Do not output any cards directly."""


def _build_user_prompt(
    candidates: list[list[str]],
    hand: list[str],
    role: str,
    last_play: list[str],
    other_counts: dict[str, int],
    bottom_cards: list[str],
    card_counter_summary: str,
) -> str:
    """Build the prompt sent to the LLM."""
    lines = [
        f"Role: {role}",
        f"My hand ({len(hand)} cards): {' '.join(hand)}",
        f"Last play: {' '.join(last_play) if last_play else 'none (you are first to play)'}",
        f"Opponent card counts: {other_counts}",
        f"Bottom cards: {' '.join(bottom_cards)}",
        f"Remaining cards summary: {card_counter_summary}",
        "",
        "Legal candidates (respond with the index or 'pass'):",
    ]
    for i, candidate in enumerate(candidates):
        lines.append(f"{i}: {' '.join(candidate) if candidate else 'pass'}")
    return "\n".join(lines)


def _parse_choice(text: str, num_candidates: int) -> int | str:
    """Parse LLM output into an index or 'pass'.

    Returns the integer index, 'pass', or 'invalid'.
    """
    text = text.strip().lower()
    if text == "pass":
        return "pass"

    # Try to extract a leading integer.
    match = re.search(r"\d+", text)
    if match:
        idx = int(match.group())
        if 0 <= idx < num_candidates:
            return idx

    # Some models may say "candidate 2" or "option 1".
    for word in ["candidate", "option", "choice", "index"]:
        if word in text:
            match = re.search(rf"{word}\s*(\d+)", text)
            if match:
                idx = int(match.group(1))
                if 0 <= idx < num_candidates:
                    return idx

    return "invalid"


class StrategyModule(Protocol):
    """Protocol for strategy modules."""

    def choose(
        self,
        candidates: list[list[str]],
        hand: list[str],
        role: str,
        last_play: list[str],
        other_counts: dict[str, int],
        bottom_cards: list[str],
        card_counter_summary: str,
    ) -> int | str:
        """Return a candidate index, 'pass', or 'invalid'."""
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
        candidates: list[list[str]],
        hand: list[str],
        role: str,
        last_play: list[str],
        other_counts: dict[str, int],
        bottom_cards: list[str],
        card_counter_summary: str,
    ) -> int | str:
        """Ask the LLM to choose a candidate and parse the response."""
        if not candidates:
            return "pass"

        try:
            llm = self._get_llm()
            messages = [
                ("system", DEFAULT_SYSTEM_PROMPT),
                (
                    "human",
                    _build_user_prompt(
                        candidates,
                        hand,
                        role,
                        last_play,
                        other_counts,
                        bottom_cards,
                        card_counter_summary,
                    ),
                ),
            ]
            response = llm.invoke(messages)
            text = response.content
            if not isinstance(text, str):
                text = str(text)
            logger.debug("LLM response: %s", text)
            return _parse_choice(text, len(candidates))
        except Exception:
            logger.exception("LLM strategy call failed")
            return "invalid"


class MockStrategy:
    """Deterministic strategy for tests."""

    def __init__(self, choice: int | str = 0):
        self.choice = choice

    def choose(
        self,
        candidates: list[list[str]],
        hand: list[str],
        role: str,
        last_play: list[str],
        other_counts: dict[str, int],
        bottom_cards: list[str],
        card_counter_summary: str,
    ) -> int | str:
        if self.choice == "invalid":
            return "invalid"
        if self.choice == "pass":
            return "pass"
        if isinstance(self.choice, int) and 0 <= self.choice < len(candidates):
            return self.choice
        return 0 if candidates else "pass"


class MinimaxAnthropicStrategy:
    """MiniMax via its Anthropic-compatible endpoint (POST {base}/v1/messages).

    M2.7 是重度推理模型,且会忽略 thinking/enable_thinking 等关闭参数(社区已知问题),
    真实牌局会先生成上千 token 的(脱敏)思考、再给答案,单次约 20-40s。因此:
      - 走 /anthropic 端点:思考与最终答案分属不同 content 块,解析只取 text 块;
      - max_tokens 给足(默认 4096)让思考+答案放得下,否则思考截断→无 text 块→回退;
      - 单次超时默认 90s、不做重试(避免把超时放大成数分钟)。

    慢是模型本性;要快就换非推理模型(如 MiniMax-Text-01)。
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
        candidates: list[list[str]],
        hand: list[str],
        role: str,
        last_play: list[str],
        other_counts: dict[str, int],
        bottom_cards: list[str],
        card_counter_summary: str,
    ) -> int | str:
        """Ask the model to choose a candidate and parse the text-block response."""
        if not candidates:
            return "pass"
        if not self.api_key:
            raise RuntimeError(
                "LLM API key not configured. Set LLM_API_KEY or MINIMAX_API_KEY environment variable."
            )
        try:
            prompt = _build_user_prompt(
                candidates,
                hand,
                role,
                last_play,
                other_counts,
                bottom_cards,
                card_counter_summary,
            )
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
            # thinking 已关闭;即便残留 thinking 块也忽略,只取文本块拼接。
            text = "".join(
                block.get("text", "")
                for block in data.get("content", [])
                if block.get("type") == "text"
            )
            if not text.strip():
                logger.warning("LLM returned no text block: %s", data)
                return "invalid"
            logger.debug("LLM text response: %s", text)
            return _parse_choice(text, len(candidates))
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
      - 'minimax': MiniMax via Anthropic-compatible endpoint (thinking disabled).
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
