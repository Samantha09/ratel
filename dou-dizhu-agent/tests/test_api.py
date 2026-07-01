"""Integration tests for the FastAPI /play endpoint (direct-play contract)."""

import pytest
from fastapi.testclient import TestClient

from dou_dizhu_agent.api import create_app
from dou_dizhu_agent.strategy import MockStrategy


def _make_client(play="invalid"):
    app = create_app()
    app.state.strategy = MockStrategy(play=play)
    return TestClient(app)


class TestPlayEndpoint:
    def test_first_play_adopts_legal_play(self):
        client = _make_client(play=["3"])
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["3", "4", "5", "6", "7", "8", "9"],
                "role": "landlord",
                "is_my_turn": True,
                "last_play": [],
                "other_players_card_count": {"p2": 17, "p3": 16},
                "bottom_cards": ["3", "4", "5"],
            },
        )
        assert response.status_code == 200
        data = response.json()
        assert data["action"] == "play"
        assert data["cards"] == ["3"]

    def test_beat_single(self):
        client = _make_client(play=["A"])
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["5", "K", "A"],
                "role": "peasant",
                "is_my_turn": True,
                "last_play": ["10"],
                "last_play_player": "p2",
                "other_players_card_count": {"p2": 16, "p3": 16},
                "bottom_cards": ["3", "4", "5"],
            },
        )
        assert response.status_code == 200
        data = response.json()
        assert data["action"] == "play"
        assert data["cards"] == ["A"]

    def test_must_pass_when_cannot_beat(self):
        # 手牌 3/4/5 管不上单张 2 → 候选只有 pass。
        client = _make_client(play="pass")
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["3", "4", "5"],
                "role": "peasant",
                "is_my_turn": True,
                "last_play": ["2"],
                "other_players_card_count": {"p2": 16, "p3": 16},
                "bottom_cards": ["3", "4", "5"],
            },
        )
        assert response.status_code == 200
        data = response.json()
        assert data["action"] == "pass"
        assert data["cards"] == []

    def test_pass_when_following(self):
        client = _make_client(play="pass")
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["3", "4", "5", "6", "7"],
                "role": "peasant",
                "is_my_turn": True,
                "last_play": ["3"],
                "other_players_card_count": {"p2": 16, "p3": 16},
                "bottom_cards": ["3", "4", "5"],
            },
        )
        assert response.status_code == 200
        data = response.json()
        assert data["action"] == "pass"
        assert data["cards"] == []

    def test_leading_pass_falls_back_to_play(self):
        # 领出时 LLM 说 pass → 不能过牌 → 规则兜底出牌。
        client = _make_client(play="pass")
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["3", "4", "5", "6", "7"],
                "role": "landlord",
                "is_my_turn": True,
                "last_play": [],
                "other_players_card_count": {"p2": 17, "p3": 16},
                "bottom_cards": ["3", "4", "5"],
            },
        )
        assert response.status_code == 200
        data = response.json()
        assert data["action"] == "play"
        assert data["cards"] == ["3"]

    def test_illegal_play_falls_back(self):
        # LLM 出了手里没有的牌 → 非法 → 兜底。
        client = _make_client(play=["2", "2"])
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["3", "4", "5", "6", "7"],
                "role": "landlord",
                "is_my_turn": True,
                "last_play": [],
                "other_players_card_count": {"p2": 17, "p3": 16},
                "bottom_cards": ["3", "4", "5"],
            },
        )
        assert response.status_code == 200
        data = response.json()
        assert data["action"] == "play"
        assert data["cards"] == ["3"]  # 最小单张兜底

    def test_invalid_output_falls_back(self):
        client = _make_client(play="invalid")
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["3", "4", "5", "6", "7"],
                "role": "landlord",
                "is_my_turn": True,
                "last_play": [],
                "other_players_card_count": {"p2": 17, "p3": 16},
                "bottom_cards": ["3", "4", "5"],
            },
        )
        assert response.status_code == 200
        data = response.json()
        assert data["action"] == "play"
        assert data["cards"] == ["3"]

    def test_invalid_role(self):
        client = _make_client()
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["3", "4", "5"],
                "role": "farmer",
                "is_my_turn": True,
                "last_play": [],
                "other_players_card_count": {"p2": 17},
                "bottom_cards": ["3", "4", "5"],
            },
        )
        assert response.status_code == 422

    def test_invalid_card(self):
        client = _make_client()
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["3", "X"],
                "role": "peasant",
                "is_my_turn": True,
                "last_play": [],
                "other_players_card_count": {"p2": 17},
                "bottom_cards": ["3", "4", "5"],
            },
        )
        assert response.status_code == 422


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
