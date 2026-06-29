"""Tests for the strategy module."""

from unittest.mock import MagicMock, patch

import pytest

from dou_dizhu_agent.strategy import (
    MockStrategy,
    MinimaxAnthropicStrategy,
    OpenAICompatibleStrategy,
    _build_user_prompt,
    _parse_choice,
    create_strategy,
)


class TestPromptBuilder:
    def test_includes_all_context(self):
        prompt = _build_user_prompt(
            candidates=[["3"], ["4"]],
            hand=["3", "4", "5"],
            role="landlord",
            last_play=["3"],
            other_counts={"p2": 17},
            bottom_cards=["6", "7", "8"],
            card_counter_summary="3:3 4:4",
        )
        assert "landlord" in prompt
        assert "3 4 5" in prompt
        assert "Opponent card counts" in prompt
        assert "0: 3" in prompt
        assert "1: 4" in prompt


class TestParseChoice:
    def test_pass(self):
        assert _parse_choice("pass", 3) == "pass"

    def test_index(self):
        assert _parse_choice("2", 3) == 2

    def test_index_with_noise(self):
        assert _parse_choice("Candidate 1", 3) == 1

    def test_out_of_range(self):
        assert _parse_choice("5", 3) == "invalid"

    def test_invalid_text(self):
        assert _parse_choice("I choose the first one", 3) == "invalid"


class TestMockStrategy:
    def test_returns_choice(self):
        strategy = MockStrategy(choice=1)
        result = strategy.choose([["3"], ["4"]], [], "peasant", [], {}, [], "")
        assert result == 1

    def test_returns_pass(self):
        strategy = MockStrategy(choice="pass")
        result = strategy.choose([["3"], ["4"]], [], "peasant", [], {}, [], "")
        assert result == "pass"

    def test_returns_invalid(self):
        strategy = MockStrategy(choice="invalid")
        result = strategy.choose([["3"], ["4"]], [], "peasant", [], {}, [], "")
        assert result == "invalid"


class TestOpenAICompatibleStrategy:
    def test_create_without_api_key_raises_on_use(self):
        strategy = OpenAICompatibleStrategy(api_key=None)
        with pytest.raises(RuntimeError):
            strategy._get_llm()

    def test_choose_returns_invalid_on_exception(self):
        strategy = OpenAICompatibleStrategy(api_key="fake")
        with patch.object(strategy, "_get_llm", side_effect=Exception("boom")):
            result = strategy.choose([["3"]], [], "peasant", [], {}, [], "")
            assert result == "invalid"

    def test_choose_parses_llm_response(self):
        strategy = OpenAICompatibleStrategy(api_key="fake")
        mock_response = MagicMock()
        mock_response.content = "1"
        mock_llm = MagicMock()
        mock_llm.invoke.return_value = mock_response
        strategy._llm = mock_llm

        result = strategy.choose([["3"], ["4"]], ["3", "4"], "peasant", [], {"p2": 17}, ["5"], "3:3")
        assert result == 1
        mock_llm.invoke.assert_called_once()


class TestMinimaxAnthropicStrategy:
    def test_choose_returns_invalid_on_exception(self):
        strategy = MinimaxAnthropicStrategy(api_key="fake")
        with patch("dou_dizhu_agent.strategy.httpx.post", side_effect=Exception("boom")):
            result = strategy.choose([["3"]], [], "peasant", [], {}, [], "")
            assert result == "invalid"

    def test_choose_parses_text_block_and_ignores_thinking(self):
        strategy = MinimaxAnthropicStrategy(api_key="fake")
        mock_resp = MagicMock()
        mock_resp.raise_for_status.return_value = None
        mock_resp.json.return_value = {
            "content": [
                {"type": "thinking", "text": "should be ignored"},
                {"type": "text", "text": "1"},
            ]
        }
        with patch("dou_dizhu_agent.strategy.httpx.post", return_value=mock_resp):
            result = strategy.choose(
                [["3"], ["4"]], ["3", "4"], "peasant", [], {"p2": 17}, ["5"], "3:3"
            )
            assert result == 1

    def test_choose_invalid_when_no_text_block(self):
        strategy = MinimaxAnthropicStrategy(api_key="fake")
        mock_resp = MagicMock()
        mock_resp.raise_for_status.return_value = None
        mock_resp.json.return_value = {"content": [{"type": "thinking", "text": "..."}]}
        with patch("dou_dizhu_agent.strategy.httpx.post", return_value=mock_resp):
            result = strategy.choose([["3"], ["4"]], ["3", "4"], "peasant", [], {}, [], "")
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
