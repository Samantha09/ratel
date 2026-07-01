"""Tests for the strategy module (direct-play contract)."""

from unittest.mock import MagicMock, patch

import pytest

from dou_dizhu_agent.strategy import (
    MockStrategy,
    MinimaxAnthropicStrategy,
    OpenAICompatibleStrategy,
    _build_user_prompt,
    _parse_play,
    create_strategy,
)


class TestPromptBuilder:
    def test_follow_includes_context(self):
        prompt = _build_user_prompt(
            hand=["3", "4", "5"],
            role="landlord",
            last_play=["6"],
            other_counts={"p2": 17},
            card_counter_summary="3:3 4:4",
        )
        assert "landlord" in prompt
        assert "3 4 5" in prompt
        assert "Last play by opponent: 6" in prompt
        assert "Opponent card counts" in prompt
        assert "3:3 4:4" in prompt

    def test_lead_prompt(self):
        prompt = _build_user_prompt(
            hand=["3", "4"], role="peasant", last_play=[], other_counts={}, card_counter_summary=""
        )
        assert "first to play (lead" in prompt
        assert "peasant" in prompt

    def test_no_candidate_list_in_prompt(self):
        """关键回归:prompt 里绝不能出现候选枚举(那会诱发模型截断)。"""
        prompt = _build_user_prompt(
            hand=["3", "4", "5"], role="landlord", last_play=[], other_counts={}, card_counter_summary=""
        )
        assert "0:" not in prompt
        assert "candidates" not in prompt.lower()


class TestParsePlay:
    def test_pass(self):
        assert _parse_play("pass") == "pass"

    def test_pass_case_insensitive(self):
        assert _parse_play("Pass") == "pass"
        assert _parse_play("  PASS  ") == "pass"

    def test_single_card(self):
        assert _parse_play("9") == ["9"]

    def test_pair(self):
        assert _parse_play("9 9") == ["9", "9"]

    def test_straight_with_ten(self):
        assert _parse_play("6 7 8 9 10") == ["6", "7", "8", "9", "10"]

    def test_extracts_cards_from_prose(self):
        # 模型偶尔带说明;只抽合法牌 token。
        assert _parse_play("I play 10 J Q K A") == ["10", "J", "Q", "K", "A"]

    def test_empty_is_invalid(self):
        assert _parse_play("") == "invalid"

    def test_garbage_is_invalid(self):
        assert _parse_play("hello world") == "invalid"


class TestMockStrategy:
    def test_returns_play(self):
        strategy = MockStrategy(play=["3"])
        assert strategy.choose(["3"], "peasant", [], {}, "") == ["3"]

    def test_returns_pass(self):
        strategy = MockStrategy(play="pass")
        assert strategy.choose(["3"], "peasant", ["3"], {}, "") == "pass"

    def test_returns_invalid(self):
        strategy = MockStrategy(play="invalid")
        assert strategy.choose(["3"], "peasant", [], {}, "") == "invalid"


class TestOpenAICompatibleStrategy:
    def test_create_without_api_key_raises_on_use(self):
        strategy = OpenAICompatibleStrategy(api_key=None)
        with pytest.raises(RuntimeError):
            strategy._get_llm()

    def test_choose_returns_invalid_on_exception(self):
        strategy = OpenAICompatibleStrategy(api_key="fake")
        with patch.object(strategy, "_get_llm", side_effect=Exception("boom")):
            result = strategy.choose(["3"], "peasant", [], {}, "")
            assert result == "invalid"

    def test_choose_parses_llm_response(self):
        strategy = OpenAICompatibleStrategy(api_key="fake")
        mock_response = MagicMock()
        mock_response.content = "9 9"
        mock_llm = MagicMock()
        mock_llm.invoke.return_value = mock_response
        strategy._llm = mock_llm

        result = strategy.choose(["9", "9"], "peasant", ["8", "8"], {"p2": 17}, "3:3")
        assert result == ["9", "9"]
        mock_llm.invoke.assert_called_once()


class TestMinimaxAnthropicStrategy:
    def test_choose_returns_invalid_on_exception(self):
        strategy = MinimaxAnthropicStrategy(api_key="fake")
        with patch("dou_dizhu_agent.strategy.httpx.post", side_effect=Exception("boom")):
            result = strategy.choose(["3"], "peasant", [], {}, "")
            assert result == "invalid"

    def test_choose_parses_text_block_and_ignores_thinking(self):
        strategy = MinimaxAnthropicStrategy(api_key="fake")
        mock_resp = MagicMock()
        mock_resp.raise_for_status.return_value = None
        mock_resp.json.return_value = {
            "content": [
                {"type": "thinking", "text": "should be ignored"},
                {"type": "text", "text": "9 9"},
            ]
        }
        with patch("dou_dizhu_agent.strategy.httpx.post", return_value=mock_resp):
            result = strategy.choose(["9", "9"], "peasant", ["8", "8"], {"p2": 17}, "3:3")
            assert result == ["9", "9"]

    def test_choose_invalid_when_no_text_block(self):
        strategy = MinimaxAnthropicStrategy(api_key="fake")
        mock_resp = MagicMock()
        mock_resp.raise_for_status.return_value = None
        mock_resp.json.return_value = {"content": [{"type": "thinking", "text": "..."}]}
        with patch("dou_dizhu_agent.strategy.httpx.post", return_value=mock_resp):
            result = strategy.choose(["3"], "peasant", [], {}, "")
            assert result == "invalid"


class TestCreateStrategy:
    def test_minimax(self):
        strategy = create_strategy(provider="minimax")
        assert isinstance(strategy, MinimaxAnthropicStrategy)

    def test_openai(self):
        strategy = create_strategy(provider="openai")
        assert isinstance(strategy, OpenAICompatibleStrategy)

    def test_unsupported(self):
        with pytest.raises(ValueError):
            create_strategy(provider="unknown")


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
