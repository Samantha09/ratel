"""FastAPI application for the dou-dizhu agent."""

from __future__ import annotations

import logging
import os
from contextlib import asynccontextmanager

from fastapi import FastAPI, status
from fastapi.responses import JSONResponse

from dou_dizhu_agent.counter import CardCounter
from dou_dizhu_agent.models import PlayRequest, PlayResponse
from dou_dizhu_agent.rules import generate_candidates
from dou_dizhu_agent.selector import resolve
from dou_dizhu_agent.strategy import MockStrategy, StrategyModule, create_strategy

logger = logging.getLogger(__name__)


def _load_strategy() -> StrategyModule:
    """Load the configured strategy module.

    If LLM_API_KEY or MINIMAX_API_KEY is set, use the OpenAI-compatible
    endpoint. Otherwise, fall back to a deterministic mock strategy that
    always selects the first candidate.
    """
    provider = os.getenv("STRATEGY_PROVIDER", "minimax").lower()
    api_key = os.getenv("LLM_API_KEY") or os.getenv("MINIMAX_API_KEY")
    base_url = os.getenv("LLM_BASE_URL") or os.getenv("MINIMAX_BASE_URL")
    model = os.getenv("LLM_MODEL") or os.getenv("MINIMAX_MODEL")

    if not api_key:
        logger.warning(
            "No LLM_API_KEY or MINIMAX_API_KEY configured; using deterministic rule fallback."
        )
        return MockStrategy(play="invalid")

    return create_strategy(
        provider=provider,
        api_key=api_key,
        base_url=base_url,
        model=model,
        temperature=float(os.getenv("LLM_TEMPERATURE", "0.3")),
        timeout=float(os.getenv("LLM_TIMEOUT", "90")),
        max_tokens=int(os.getenv("LLM_MAX_TOKENS", "4096")),
    )


@asynccontextmanager
async def lifespan(app: FastAPI):
    """Application lifespan handler."""
    app.state.strategy = _load_strategy()
    yield


def create_app() -> FastAPI:
    """Create and configure the FastAPI application."""
    app = FastAPI(
        title="Dou-Dizhu Agent",
        description="An AI agent that plays dou-dizhu as one player.",
        version="0.1.0",
        lifespan=lifespan,
    )

    @app.post("/play", response_model=PlayResponse)
    async def play(request: PlayRequest) -> PlayResponse:
        """Receive game state and return the agent's chosen play."""
        strategy: StrategyModule = app.state.strategy

        # Update card counter with latest visible information.
        card_counter = CardCounter(
            hand=request.hand,
            bottom_cards=request.bottom_cards,
            history=request.history,
        )
        card_counter.update(
            hand=request.hand,
            last_play=request.last_play,
            bottom_cards=request.bottom_cards,
            history=request.history,
        )

        # Generate all legal candidates (used for legality check + fallback only;
        # 不再喂给 LLM——实测枚举清单会诱发模型逐项比较 → 思考爆 max_tokens → 截断)。
        candidates = generate_candidates(request.hand, request.last_play)

        # Ask the LLM to play directly (returns cards / 'pass' / 'invalid').
        raw = strategy.choose(
            hand=request.hand,
            role=request.role,
            last_play=request.last_play,
            other_counts=request.other_players_card_count,
            card_counter_summary=card_counter.summary(),
        )

        logger.info(
            "Player %s raw=%r candidates=%d", request.player_id, raw, len(candidates)
        )

        # Validate against the candidate set; illegal/empty/pass-as-lead → rule fallback.
        action, cards = resolve(candidates, raw, request.last_play)

        return PlayResponse(action=action, cards=cards)

    @app.exception_handler(ValueError)
    async def value_error_handler(request, exc):
        logger.warning("Value error: %s", exc)
        return JSONResponse(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            content={"detail": str(exc)},
        )

    return app


app = create_app()
