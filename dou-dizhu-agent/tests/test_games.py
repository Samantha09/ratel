"""End-to-end scenario tests: the /play endpoint always returns a legal play."""

import pytest
from fastapi.testclient import TestClient

from dou_dizhu_agent.api import create_app
from dou_dizhu_agent.rules import can_beat, parse_pattern
from dou_dizhu_agent.strategy import MockStrategy


def _client(play="invalid"):
    app = create_app()
    app.state.strategy = MockStrategy(play=play)
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
    """无论 LLM 返回什么(合法/非法/pass),端点都必须给出合法动作。"""
    assert response.status_code == 200
    data = response.json()
    if data["action"] == "pass":
        return

    cards = data["cards"]
    # 出的牌必须都在手里。
    hand_counts: dict[str, int] = {}
    for c in hand:
        hand_counts[c] = hand_counts.get(c, 0) + 1
    for c in cards:
        assert c in hand_counts, f"played card {c!r} not in hand"
        hand_counts[c] -= 1
        assert hand_counts[c] >= 0, f"played too many {c!r}"

    # 必须是合法牌型。
    pattern = parse_pattern(cards)
    assert pattern is not None, f"invalid pattern: {cards}"

    # 跟牌时必须管住上家。
    if last_play:
        last_pattern = parse_pattern(last_play)
        assert can_beat(pattern, last_pattern), f"{cards} cannot beat {last_play}"


class TestGameScenarios:
    def test_landlord_first_play(self):
        hand = ["3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2", "小王", "大王"]
        client = _client(play=["3"])
        response = client.post("/play", json=_request(hand=hand))
        _assert_legal_play(response, hand, None)

    def test_peasant_follows_single(self):
        hand = ["3", "5", "8", "J", "Q", "K", "A", "2"]
        client = _client(play=["J"])
        response = client.post("/play", json=_request(hand=hand, last_play=["10"], role="peasant"))
        _assert_legal_play(response, hand, ["10"])

    def test_bomb_beat_straight(self):
        hand = ["3", "4", "5", "6", "7", "K", "K", "K", "K"]
        client = _client(play=["K", "K", "K", "K"])
        response = client.post("/play", json=_request(hand=hand, last_play=["8", "9", "10", "J", "Q"]))
        _assert_legal_play(response, hand, ["8", "9", "10", "J", "Q"])

    def test_must_pass_against_rocket(self):
        hand = ["3", "4", "5", "6", "7"]
        client = _client(play="pass")
        response = client.post("/play", json=_request(hand=hand, last_play=["小王", "大王"]))
        assert response.status_code == 200
        data = response.json()
        assert data["action"] == "pass"
        assert data["cards"] == []

    def test_airplane_cannot_beat_higher_airplane_passes(self):
        hand = ["3", "3", "3", "4", "4", "4", "5", "6", "7", "8"]
        client = _client(play="pass")
        response = client.post(
            "/play", json=_request(hand=hand, last_play=["9", "9", "9", "10", "10", "10", "J", "Q"])
        )
        _assert_legal_play(response, hand, ["9", "9", "9", "10", "10", "10", "J", "Q"])

    def test_illegal_output_still_legal(self):
        """LLM 出无效内容时,端点也必须回退到合法动作。"""
        hand = ["3", "4", "5", "6", "7"]
        client = _client(play="invalid")
        response = client.post("/play", json=_request(hand=hand, last_play=["6", "7", "8", "9", "10"]))
        _assert_legal_play(response, hand, ["6", "7", "8", "9", "10"])


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
