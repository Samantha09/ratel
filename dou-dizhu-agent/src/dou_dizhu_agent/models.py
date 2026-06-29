"""Pydantic models for the dou-dizhu agent API."""

from __future__ import annotations

from pydantic import BaseModel, Field, field_validator

from dou_dizhu_agent.rules import VALID_CARDS

VALID_ROLES = {"landlord", "peasant"}


class PlayRequest(BaseModel):
    """Request body for POST /play."""

    player_id: str = Field(..., description="Unique identifier for the player.")
    hand: list[str] = Field(..., description="Cards currently held by the player.")
    role: str = Field(..., description="Player role: 'landlord' or 'peasant'.")
    is_my_turn: bool = Field(..., description="Whether it is this player's turn to act.")
    last_play: list[str] = Field(default_factory=list, description="Cards played by the previous player; empty means first play.")
    last_play_player: str | None = Field(default=None, description="Player ID who made last_play.")
    other_players_card_count: dict[str, int] = Field(
        ..., description="Mapping from opponent player IDs to their remaining card counts."
    )
    bottom_cards: list[str] = Field(..., description="The three bottom cards in this game.")
    history: list[dict] | None = Field(default=None, description="Optional history of previous plays.")

    @field_validator("hand")
    @classmethod
    def validate_hand(cls, value: list[str]) -> list[str]:
        if not value:
            raise ValueError("hand must contain at least one card")
        for card in value:
            if card not in VALID_CARDS:
                raise ValueError(f"invalid card in hand: {card!r}")
        return value

    @field_validator("role")
    @classmethod
    def validate_role(cls, value: str) -> str:
        if value not in VALID_ROLES:
            raise ValueError(f"role must be one of {VALID_ROLES}, got {value!r}")
        return value

    @field_validator("last_play", "bottom_cards")
    @classmethod
    def validate_cards_field(cls, value: list[str]) -> list[str]:
        for card in value:
            if card not in VALID_CARDS:
                raise ValueError(f"invalid card: {card!r}")
        return value

    @field_validator("other_players_card_count")
    @classmethod
    def validate_other_counts(cls, value: dict[str, int]) -> dict[str, int]:
        for player_id, count in value.items():
            if not isinstance(count, int) or count < 0:
                raise ValueError(f"card count for {player_id!r} must be a non-negative integer, got {count!r}")
        return value


class PlayResponse(BaseModel):
    """Response body for POST /play."""

    action: str = Field(..., description="'play' or 'pass'.")
    cards: list[str] = Field(default_factory=list, description="Cards to play; empty when action is 'pass'.")

    @field_validator("action")
    @classmethod
    def validate_action(cls, value: str) -> str:
        if value not in {"play", "pass"}:
            raise ValueError(f"action must be 'play' or 'pass', got {value!r}")
        return value

    @field_validator("cards")
    @classmethod
    def validate_cards(cls, value: list[str], info) -> list[str]:
        action = info.data.get("action")
        if action == "pass" and value:
            raise ValueError("cards must be empty when action is 'pass'")
        for card in value:
            if card not in VALID_CARDS:
                raise ValueError(f"invalid card in response: {card!r}")
        return value
