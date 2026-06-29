"""Tests for Pydantic API models."""

import pytest
from pydantic import ValidationError

from dou_dizhu_agent.models import PlayRequest, PlayResponse


class TestPlayRequest:
    def test_valid_request(self):
        req = PlayRequest(
            player_id="p1",
            hand=["3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2", "小王", "大王"],
            role="landlord",
            is_my_turn=True,
            last_play=["3", "4", "5", "6", "7"],
            last_play_player="p2",
            other_players_card_count={"p2": 17, "p3": 16},
            bottom_cards=["3", "4", "5"],
        )
        assert req.player_id == "p1"
        assert req.role == "landlord"

    def test_default_last_play(self):
        req = PlayRequest(
            player_id="p1",
            hand=["3", "4", "5"],
            role="peasant",
            is_my_turn=True,
            other_players_card_count={"p2": 17},
            bottom_cards=["3", "4", "5"],
        )
        assert req.last_play == []

    def test_invalid_role(self):
        with pytest.raises(ValidationError):
            PlayRequest(
                player_id="p1",
                hand=["3", "4", "5"],
                role="farmer",
                is_my_turn=True,
                other_players_card_count={"p2": 17},
                bottom_cards=["3", "4", "5"],
            )

    def test_invalid_card_in_hand(self):
        with pytest.raises(ValidationError):
            PlayRequest(
                player_id="p1",
                hand=["3", "X"],
                role="peasant",
                is_my_turn=True,
                other_players_card_count={"p2": 17},
                bottom_cards=["3", "4", "5"],
            )

    def test_empty_hand(self):
        with pytest.raises(ValidationError):
            PlayRequest(
                player_id="p1",
                hand=[],
                role="peasant",
                is_my_turn=True,
                other_players_card_count={"p2": 17},
                bottom_cards=["3", "4", "5"],
            )

    def test_negative_card_count(self):
        with pytest.raises(ValidationError):
            PlayRequest(
                player_id="p1",
                hand=["3", "4", "5"],
                role="peasant",
                is_my_turn=True,
                other_players_card_count={"p2": -1},
                bottom_cards=["3", "4", "5"],
            )

    def test_invalid_bottom_card(self):
        with pytest.raises(ValidationError):
            PlayRequest(
                player_id="p1",
                hand=["3", "4", "5"],
                role="peasant",
                is_my_turn=True,
                other_players_card_count={"p2": 17},
                bottom_cards=["3", "X"],
            )


class TestPlayResponse:
    def test_play(self):
        resp = PlayResponse(action="play", cards=["4", "5", "6", "7", "8"])
        assert resp.action == "play"
        assert resp.cards == ["4", "5", "6", "7", "8"]

    def test_pass(self):
        resp = PlayResponse(action="pass", cards=[])
        assert resp.action == "pass"
        assert resp.cards == []

    def test_pass_with_cards_invalid(self):
        with pytest.raises(ValidationError):
            PlayResponse(action="pass", cards=["3"])

    def test_invalid_action(self):
        with pytest.raises(ValidationError):
            PlayResponse(action="fold", cards=[])

    def test_invalid_card(self):
        with pytest.raises(ValidationError):
            PlayResponse(action="play", cards=["X"])


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
