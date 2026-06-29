"""End-to-end scenario tests for known dou-dizhu rounds."""

import pytest
from fastapi.testclient import TestClient

from dou_dizhu_agent.api import create_app
from dou_dizhu_agent.rules import parse_pattern
from dou_dizhu_agent.strategy import MockStrategy


def _client(choice: int | str):
    app = create_app()
    app.state.strategy = MockStrategy(choice=choice)
    return TestClient(app)


def _request(
    hand: list[str],
    last_play: list[str] | None = None,
    role: str = "landlord",
    other_counts: dict[str, int] | None = None,
    bottom_cards: list[str] | None = None,
):
    return {
        "player_id": "p1",
        "hand": hand,
        "role": role,
        "is_my_turn": True,
        "last_play": last_play or [],
        "other_players_card_count": other_counts or {"p2": 17, "p3": 17},
        "bottom_cards": bottom_cards or ["3", "4", "5"],
    }


def _assert_legal_play(response, hand: list[str], last_play: list[str] | None):
    assert response.status_code == 200
    data = response.json()
    if data["action"] == "pass":
        return

    cards = data["cards"]
    # All played cards must come from hand.
    hand_counts = {}
    for c in hand:
        hand_counts[c] = hand_counts.get(c, 0) + 1
    for c in cards:
        assert c in hand_counts, f"played card {c!r} not in hand"
        hand_counts[c] -= 1
        assert hand_counts[c] >= 0, f"played too many {c!r}"

    # Must be a valid pattern.
    pattern = parse_pattern(cards)
    assert pattern is not None, f"invalid pattern: {cards}"

    # Must beat last play (unless first play).
    if last_play:
        last_pattern = parse_pattern(last_play)
        from dou_dizhu_agent.rules import can_beat

        assert can_beat(pattern, last_pattern), f"{cards} cannot beat {last_play}"


class TestGameScenarios:
    def test_landlord_first_play(self):
        hand = ["3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2", "小王", "大王"]
        client = _client(choice=0)
        response = client.post("/play", json=_request(hand=hand))
        _assert_legal_play(response, hand, None)

    def test_peasant_follows_single(self):
        hand = ["3", "5", "8", "J", "Q", "K", "A", "2"]
        last_play = ["10"]
        client = _client(choice=1)
        response = client.post("/play", json=_request(hand=hand, last_play=last_play))
        _assert_legal_play(response, hand, last_play)

    def test_bomb_beat_straight(self):
        hand = ["3", "4", "5", "6", "7", "K", "K", "K", "K"]
        last_play = ["8", "9", "10", "J", "Q"]
        client = _client(choice=-1)  # last candidate is bomb
        response = client.post("/play", json=_request(hand=hand, last_play=last_play))
        _assert_legal_play(response, hand, last_play)

    def test_must_pass_against_rocket(self):
        hand = ["3", "4", "5", "6", "7"]
        last_play = ["小王", "大王"]
        client = _client(choice=0)
        response = client.post("/play", json=_request(hand=hand, last_play=last_play))
        assert response.status_code == 200
        data = response.json()
        assert data["action"] == "pass"
        assert data["cards"] == []

    def test_airplane_scenario(self):
        hand = ["3", "3", "3", "4", "4", "4", "5", "6", "7", "8"]
        last_play = ["9", "9", "9", "10", "10", "10", "J", "Q"]
        client = _client(choice=0)
        response = client.post("/play", json=_request(hand=hand, last_play=last_play))
        _assert_legal_play(response, hand, last_play)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
