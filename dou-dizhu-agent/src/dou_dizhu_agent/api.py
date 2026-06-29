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
from dou_dizhu_agent.selector import select
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
            "No LLM_API_KEY or MINIMAX_API_KEY configured; using deterministic mock strategy."
        )
        return MockStrategy(choice=0)

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

        # Generate all legal candidates.
        candidates = generate_candidates(request.hand, request.last_play)

        # Ask strategy module to choose.
        choice = strategy.choose(
            candidates=candidates,
            hand=request.hand,
            role=request.role,
            last_play=request.last_play,
            other_counts=request.other_players_card_count,
            bottom_cards=request.bottom_cards,
            card_counter_summary=card_counter.summary(),
        )

        logger.info(
            "Player %s chose %r from %d candidates",
            request.player_id,
            choice,
            len(candidates),
        )

        # Map choice to concrete play, with fallback on invalid choices.
        action, cards = select(candidates, choice)

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
