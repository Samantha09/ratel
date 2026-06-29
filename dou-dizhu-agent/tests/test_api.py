"""Integration tests for the FastAPI /play endpoint."""

import pytest
from fastapi.testclient import TestClient

from dou_dizhu_agent.api import create_app
from dou_dizhu_agent.strategy import MockStrategy


def _make_client(strategy_choice: int | str = 0):
    app = create_app()
    app.state.strategy = MockStrategy(choice=strategy_choice)
    return TestClient(app)


class TestPlayEndpoint:
    def test_first_play(self):
        client = _make_client(strategy_choice=0)
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
        client = _make_client(strategy_choice=1)
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

    def test_must_pass(self):
        client = _make_client(strategy_choice=0)
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

    def test_invalid_choice_falls_back(self):
        client = _make_client(strategy_choice="invalid")
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

    def test_pass_choice(self):
        client = _make_client(strategy_choice="pass")
        response = client.post(
            "/play",
            json={
                "player_id": "p1",
                "hand": ["3", "4", "5", "6", "7"],
                "role": "landlord",
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


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
