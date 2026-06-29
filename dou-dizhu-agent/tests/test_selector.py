"""Tests for the play selector."""

import pytest

from dou_dizhu_agent.selector import select, select_from_request


class TestSelect:
    def test_select_valid_index(self):
        candidates = [["3"], ["4"], ["5"]]
        action, cards = select(candidates, 1)
        assert action == "play"
        assert cards == ["4"]

    def test_select_pass_choice(self):
        candidates = [["3"], ["4"], []]
        action, cards = select(candidates, "pass")
        assert action == "pass"
        assert cards == []

    def test_select_pass_index(self):
        candidates = [["3"], []]
        action, cards = select(candidates, 1)
        assert action == "pass"
        assert cards == []

    def test_select_invalid_index_fallback(self):
        candidates = [["3"], ["4"]]
        action, cards = select(candidates, 10)
        assert action == "play"
        assert cards == ["3"]

    def test_select_invalid_choice_fallback(self):
        candidates = [["5", "6", "7", "8", "9"], ["3", "4", "5", "6", "7"]]
        action, cards = select(candidates, "invalid")
        assert action == "play"
        # Should pick smallest by length then rank.
        assert cards == ["3", "4", "5", "6", "7"]

    def test_select_invalid_does_not_waste_rocket(self):
        # 王炸(2 张)比顺子短;旧兜底会优先选它白白甩出。修复后应出最小普通牌。
        candidates = [["4", "5", "6", "7", "8"], ["小王", "大王"]]
        action, cards = select(candidates, "invalid")
        assert action == "play"
        assert cards == ["4", "5", "6", "7", "8"]

    def test_select_invalid_uses_bomb_only_when_forced(self):
        candidates = [["小王", "大王"]]
        action, cards = select(candidates, "invalid")
        assert action == "play"
        assert cards == ["小王", "大王"]

    def test_select_only_pass(self):
        candidates = [[]]
        action, cards = select(candidates, 0)
        assert action == "pass"
        assert cards == []

    def test_select_only_pass_invalid_choice(self):
        candidates = [[]]
        action, cards = select(candidates, "invalid")
        assert action == "pass"
        assert cards == []


class TestSelectFromRequest:
    def test_first_play(self):
        action, cards = select_from_request(["3", "4", "5", "6", "7"], None, 0)
        assert action == "play"
        assert cards == ["3"]

    def test_beat_single(self):
        action, cards = select_from_request(["5", "K", "A"], ["10"], 1)
        assert action == "play"
        assert cards == ["A"]

    def test_must_pass(self):
        action, cards = select_from_request(["3", "4", "5"], ["2"], 0)
        assert action == "pass"
        assert cards == []


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
