"""Tests for the dou-dizhu rule engine."""

import pytest

from dou_dizhu_agent.rules import (
    CARD_RANKS,
    RANK_INDEX,
    can_beat,
    generate_candidates,
    parse_pattern,
)


def test_constants():
    assert CARD_RANKS[0] == "3"
    assert CARD_RANKS[-1] == "大王"
    assert RANK_INDEX["10"] == 7
    assert RANK_INDEX["小王"] == 13
    assert RANK_INDEX["大王"] == 14


class TestParsePattern:
    def test_single(self):
        p = parse_pattern(["A"])
        assert p.pattern_type == "single"
        assert p.main_rank == RANK_INDEX["A"]

    def test_pair(self):
        p = parse_pattern(["5", "5"])
        assert p.pattern_type == "pair"
        assert p.main_rank == RANK_INDEX["5"]

    def test_triple(self):
        p = parse_pattern(["K", "K", "K"])
        assert p.pattern_type == "triple"
        assert p.main_rank == RANK_INDEX["K"]

    def test_triple_single(self):
        p = parse_pattern(["7", "7", "7", "3"])
        assert p.pattern_type == "triple_single"
        assert p.main_rank == RANK_INDEX["7"]
        assert p.kicker_count == 1

    def test_triple_pair(self):
        p = parse_pattern(["9", "9", "9", "4", "4"])
        assert p.pattern_type == "triple_pair"
        assert p.main_rank == RANK_INDEX["9"]
        assert p.kicker_count == 2

    def test_straight(self):
        p = parse_pattern(["3", "4", "5", "6", "7"])
        assert p.pattern_type == "straight"
        assert p.main_rank == RANK_INDEX["3"]
        assert p.length == 5

    def test_straight_long(self):
        p = parse_pattern(["3", "4", "5", "6", "7", "8", "9", "10", "J"])
        assert p.pattern_type == "straight"
        assert p.length == 9

    def test_straight_invalid_with_2(self):
        assert parse_pattern(["J", "Q", "K", "A", "2"]) is None

    def test_straight_invalid_too_short(self):
        assert parse_pattern(["3", "4", "5", "6"]) is None

    def test_consecutive_pairs(self):
        p = parse_pattern(["3", "3", "4", "4", "5", "5"])
        assert p.pattern_type == "consecutive_pairs"
        assert p.main_rank == RANK_INDEX["3"]
        assert p.length == 3

    def test_consecutive_pairs_invalid(self):
        assert parse_pattern(["3", "3", "4", "4"]) is None

    def test_bomb(self):
        p = parse_pattern(["A", "A", "A", "A"])
        assert p.pattern_type == "bomb"
        assert p.main_rank == RANK_INDEX["A"]

    def test_rocket(self):
        p = parse_pattern(["小王", "大王"])
        assert p.pattern_type == "rocket"

    def test_airplane_pure(self):
        p = parse_pattern(["3", "3", "3", "4", "4", "4"])
        assert p.pattern_type == "airplane"
        assert p.main_rank == RANK_INDEX["3"]
        assert p.length == 2

    def test_airplane_single(self):
        p = parse_pattern(["3", "3", "3", "4", "4", "4", "5", "6"])
        assert p.pattern_type == "airplane_single"
        assert p.length == 2
        assert p.kicker_count == 2

    def test_airplane_pair(self):
        p = parse_pattern(["3", "3", "3", "4", "4", "4", "5", "5", "6", "6"])
        assert p.pattern_type == "airplane_pair"
        assert p.length == 2
        assert p.kicker_type == "pair"

    def test_invalid_mixed(self):
        assert parse_pattern(["3", "4", "4"]) is None

    def test_invalid_card(self):
        assert parse_pattern(["3", "X"]) is None

    def test_empty(self):
        assert parse_pattern([]) is None


class TestCanBeat:
    def test_same_type_higher_wins(self):
        high = parse_pattern(["A"])
        low = parse_pattern(["K"])
        assert can_beat(high, low)
        assert not can_beat(low, high)

    def test_same_type_same_rank(self):
        a = parse_pattern(["A"])
        b = parse_pattern(["A"])
        assert not can_beat(a, b)

    def test_bomb_beats_normal(self):
        bomb = parse_pattern(["5", "5", "5", "5"])
        straight = parse_pattern(["3", "4", "5", "6", "7"])
        assert can_beat(bomb, straight)
        assert not can_beat(straight, bomb)

    def test_bomb_beats_bomb(self):
        big_bomb = parse_pattern(["A", "A", "A", "A"])
        small_bomb = parse_pattern(["5", "5", "5", "5"])
        assert can_beat(big_bomb, small_bomb)
        assert not can_beat(small_bomb, big_bomb)

    def test_rocket_beats_all(self):
        rocket = parse_pattern(["小王", "大王"])
        bomb = parse_pattern(["2", "2", "2", "2"])
        assert can_beat(rocket, bomb)
        assert not can_beat(bomb, rocket)

    def test_different_types_do_not_beat(self):
        single = parse_pattern(["A"])
        pair = parse_pattern(["K", "K"])
        assert not can_beat(single, pair)

    def test_straight_length_must_match(self):
        short = parse_pattern(["3", "4", "5", "6", "7"])
        long = parse_pattern(["4", "5", "6", "7", "8", "9"])
        assert not can_beat(long, short)


class TestGenerateCandidates:
    def test_first_play(self):
        hand = ["3", "4", "5", "6", "7", "8", "9"]
        candidates = generate_candidates(hand, None)
        assert any(c == ["3", "4", "5", "6", "7"] for c in candidates)
        assert any(c == ["3"] for c in candidates)

    def test_beat_single(self):
        hand = ["3", "5", "8", "K", "A"]
        candidates = generate_candidates(hand, ["10"])
        assert ["K"] in candidates
        assert ["A"] in candidates
        assert ["3"] not in candidates

    def test_cannot_beat(self):
        hand = ["3", "4", "5"]
        candidates = generate_candidates(hand, ["2"])
        assert candidates == [[]]

    def test_beat_with_bomb(self):
        hand = ["3", "4", "5", "6", "2", "2", "2", "2"]
        candidates = generate_candidates(hand, ["A", "A", "A", "A"])
        assert ["2", "2", "2", "2"] in candidates

    def test_pass_only_when_no_candidate(self):
        hand = ["3", "3", "3"]
        candidates = generate_candidates(hand, ["2", "2", "2", "2"])
        assert candidates == [[]]

    def test_rocket_candidate(self):
        hand = ["3", "小王", "大王"]
        candidates = generate_candidates(hand, ["2", "2", "2", "2"])
        assert ["小王", "大王"] in candidates

    def test_pair_beat_pair(self):
        hand = ["3", "3", "5", "5", "8", "8"]
        candidates = generate_candidates(hand, ["4", "4"])
        assert ["5", "5"] in candidates
        assert ["8", "8"] in candidates

    def test_triple_single_beat(self):
        hand = ["5", "5", "5", "6", "7", "8"]
        candidates = generate_candidates(hand, ["3", "3", "3", "4"])
        assert ["5", "5", "5", "6"] in candidates

    def test_airplane_candidates(self):
        hand = ["10", "10", "10", "J", "J", "J", "3", "4"]
        candidates = generate_candidates(hand, ["9", "9", "9", "10", "10", "10", "5", "6"])
        assert ["10", "10", "10", "J", "J", "J", "3", "4"] in candidates

    def test_candidates_use_hand_cards(self):
        hand = ["3", "4", "5", "6", "7"]
        candidates = generate_candidates(hand, ["3"])
        for c in candidates:
            if c:  # skip pass
                for card in c:
                    assert card in hand


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
