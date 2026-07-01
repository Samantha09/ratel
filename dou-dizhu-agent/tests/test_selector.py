"""Tests for the play resolver (raw LLM output → concrete legal play)."""

import pytest

from dou_dizhu_agent.selector import resolve


class TestResolveValidPlay:
    def test_play_in_candidates_is_adopted(self):
        candidates = [["3"], ["4"], ["5"]]
        action, cards = resolve(candidates, ["4"], ["3"])
        assert action == "play"
        assert cards == ["4"]

    def test_play_order_independent(self):
        # LLM 出牌顺序不影响成员校验。
        candidates = [["9", "10", "J"]]
        action, cards = resolve(candidates, ["J", "9", "10"], ["6", "7", "8"])
        assert action == "play"
        assert sorted(cards) == sorted(["9", "10", "J"])

    def test_play_pair(self):
        candidates = [["9", "9"], ["10", "10"]]
        action, cards = resolve(candidates, ["9", "9"], ["8", "8"])
        assert action == "play"
        assert cards == ["9", "9"]


class TestResolveIllegalFallback:
    def test_play_not_in_candidates_falls_back(self):
        # ["A"] 不在能管 ["2"] 的候选里(单张 A 管不上 2)→ 兜底。
        candidates = [["小王"]]
        action, cards = resolve(candidates, ["A"], ["2"])
        assert action == "play"
        assert cards == ["小王"]

    def test_play_with_unknown_card_falls_back(self):
        candidates = [["3"]]
        action, cards = resolve(candidates, ["3", "X"], [])
        assert action == "play"
        assert cards == ["3"]  # 最小非炸弹候选

    def test_invalid_falls_back_and_does_not_waste_rocket(self):
        # 王炸只有 2 张比顺子短;兜底应出最小普通牌,不甩王炸。
        candidates = [["4", "5", "6", "7", "8"], ["小王", "大王"]]
        action, cards = resolve(candidates, "invalid", ["3"])
        assert action == "play"
        assert cards == ["4", "5", "6", "7", "8"]

    def test_invalid_uses_bomb_only_when_forced(self):
        candidates = [["小王", "大王"]]
        action, cards = resolve(candidates, "invalid", ["3"])
        assert action == "play"
        assert cards == ["小王", "大王"]

    def test_smallest_by_length_then_rank(self):
        candidates = [["5", "6", "7", "8", "9"], ["3", "4", "5", "6", "7"]]
        action, cards = resolve(candidates, "invalid", ["3"])
        assert action == "play"
        assert cards == ["3", "4", "5", "6", "7"]


class TestResolvePass:
    def test_pass_when_following_is_legal(self):
        candidates = [["3"], ["4"]]
        action, cards = resolve(candidates, "pass", ["5"])
        assert action == "pass"
        assert cards == []

    def test_pass_when_leading_falls_back_to_play(self):
        # 领出不能过牌 → 兜底出牌。
        candidates = [["3"], ["4"]]
        action, cards = resolve(candidates, "pass", [])
        assert action == "play"
        assert cards == ["3"]

    def test_pass_none_last_play_falls_back(self):
        candidates = [["3"]]
        action, cards = resolve(candidates, "pass", None)
        assert action == "play"
        assert cards == ["3"]


class TestResolveOnlyPass:
    def test_only_pass_with_pass(self):
        candidates = [[]]
        action, cards = resolve(candidates, "pass", ["2"])
        assert action == "pass"
        assert cards == []

    def test_only_pass_with_play_falls_back_to_pass(self):
        # 无任何能管的候选 → 兜底也只能过牌。
        candidates = [[]]
        action, cards = resolve(candidates, ["3"], ["小王", "大王"])
        assert action == "pass"
        assert cards == []

    def test_only_pass_with_invalid(self):
        candidates = [[]]
        action, cards = resolve(candidates, "invalid", ["2"])
        assert action == "pass"
        assert cards == []


class TestResolveLeadBombGuardrail:
    """领出时 LLM 想甩炸弹/王炸、但还有普通牌可选 → 强制走非炸弹兜底。"""

    def test_lead_rocket_overridden_when_non_bomb_exists(self):
        candidates = [["3"], ["4"], ["小王", "大王"]]
        action, cards = resolve(candidates, ["小王", "大王"], [])
        assert action == "play"
        assert cards == ["3"]  # 不甩王炸,出最小普通牌

    def test_lead_bomb_overridden_when_non_bomb_exists(self):
        candidates = [["3"], ["4", "4", "4", "4"]]
        action, cards = resolve(candidates, ["4", "4", "4", "4"], [])
        assert action == "play"
        assert cards == ["3"]

    def test_lead_bomb_kept_when_only_bomb_available(self):
        # 手里只剩王炸 → 只能出王炸。
        candidates = [["小王", "大王"]]
        action, cards = resolve(candidates, ["小王", "大王"], [])
        assert action == "play"
        assert cards == ["小王", "大王"]

    def test_follow_bomb_not_overridden(self):
        # 跟牌用炸弹管牌是局势判断,不拦截。
        candidates = [["9", "9"], ["小王", "大王"]]
        action, cards = resolve(candidates, ["小王", "大王"], ["8", "8"])
        assert action == "play"
        assert cards == ["小王", "大王"]


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
