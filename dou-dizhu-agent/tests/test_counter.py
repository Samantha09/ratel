"""Tests for the card counter."""

import pytest

from dou_dizhu_agent.counter import CardCounter


class TestCardCounter:
    def test_initial_full_deck(self):
        cc = CardCounter([])
        assert cc.total_remaining() == 54

    def test_with_hand(self):
        hand = ["3", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2", "小王", "大王"]
        cc = CardCounter(hand)
        assert cc.count("3") == 2  # 4 total - 2 in hand
        assert cc.count("大王") == 0
        assert cc.total_remaining() == 54 - len(hand)

    def test_with_bottom_cards(self):
        cc = CardCounter(["3", "4", "5"], bottom_cards=["大王", "小王", "A"])
        assert cc.count("大王") == 0
        assert cc.count("小王") == 0
        assert cc.count("A") == 3

    def test_with_history(self):
        history = [
            {"player": "p1", "cards": ["3", "3", "3"]},
            {"player": "p2", "cards": ["4", "4"]},
        ]
        cc = CardCounter(["5", "6"], history=history)
        assert cc.count("3") == 1
        assert cc.count("4") == 2
        assert cc.count("5") == 3

    def test_update_resets_state(self):
        cc = CardCounter(["3", "3", "3"])
        assert cc.count("3") == 1
        cc.update(["4", "4", "4"], last_play=["5", "5"])
        assert cc.count("3") == 4
        assert cc.count("4") == 1
        assert cc.count("5") == 2

    def test_summary(self):
        cc = CardCounter(["大王", "小王"])
        summary = cc.summary()
        assert "大王:0" in summary
        assert "小王:0" in summary
        assert "3:4" in summary

    def test_summary_no_remaining(self):
        # All 54 cards visible.
        hand = []
        for rank in ["3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2"]:
            hand.extend([rank] * 4)
        hand.extend(["小王", "大王"])
        cc = CardCounter(hand)
        assert "大王:0" in cc.summary()
        assert "3:0" in cc.summary()


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
