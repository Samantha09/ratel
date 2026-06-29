import { describe, it, expect, vi, afterEach } from 'vitest';
import { Card } from '../src/types.js';
import {
  levelToRank,
  cardsToRanks,
  ranksToCards,
  buildPlayAgentRequest,
  callPlayAgent,
} from '../src/play-agent.js';

const c = (type: number, level: number): Card => ({ type: type as Card['type'], level });

describe('card <-> rank conversion', () => {
  it('maps levels to Python rank strings (identical ordering)', () => {
    expect(levelToRank(0)).toBe('3');
    expect(levelToRank(7)).toBe('10');
    expect(levelToRank(12)).toBe('2');
    expect(levelToRank(13)).toBe('小王');
    expect(levelToRank(14)).toBe('大王');
  });

  it('cardsToRanks drops suit, keeps rank', () => {
    expect(cardsToRanks([c(3, 0), c(4, 0), c(1, 7)])).toEqual(['3', '3', '10']);
  });

  it('ranksToCards picks real cards from hand by level', () => {
    const hand = [c(3, 0), c(4, 0), c(2, 5), c(1, 12)];
    const out = ranksToCards(['3', '3'], hand);
    expect(out).not.toBeNull();
    expect(out!.map((x) => x.level)).toEqual([0, 0]);
    // distinct concrete cards (different suits) were chosen
    expect(out![0]).not.toBe(out![1]);
  });

  it('ranksToCards returns null when a rank is not in hand', () => {
    expect(ranksToCards(['2'], [c(3, 0)])).toBeNull();
    expect(ranksToCards(['3', '3'], [c(3, 0)])).toBeNull(); // only one available
    expect(ranksToCards(['??'], [c(3, 0)])).toBeNull(); // unknown rank
  });
});

describe('buildPlayAgentRequest', () => {
  it('serialises hand/last_play/role/other_counts to the Python schema', () => {
    const req = buildPlayAgentRequest({
      nickname: 'robot_1',
      role: 'LANDLORD',
      hand: [c(3, 0), c(1, 13), c(1, 14)],
      lastPlay: { nickname: 'robot_2', cards: [c(2, 5)] },
      otherCounts: { robot_2: 16, you: 17 },
    });
    expect(req).toEqual({
      player_id: 'robot_1',
      hand: ['3', '小王', '大王'],
      role: 'landlord',
      is_my_turn: true,
      last_play: ['8'],
      last_play_player: 'robot_2',
      other_players_card_count: { robot_2: 16, you: 17 },
      bottom_cards: [],
    });
  });

  it('peasant + leading (no last play) yields empty last_play', () => {
    const req = buildPlayAgentRequest({
      nickname: 'robot_1',
      role: 'PEASANT',
      hand: [c(3, 0)],
      lastPlay: null,
    });
    expect(req.role).toBe('peasant');
    expect(req.last_play).toEqual([]);
    expect(req.last_play_player).toBeNull();
    expect(req.other_players_card_count).toEqual({});
  });
});

describe('callPlayAgent', () => {
  afterEach(() => {
    vi.restoreAllMocks();
  });

  it('maps a play response back to concrete hand cards', async () => {
    const hand = [c(3, 0), c(4, 0), c(2, 5)];
    vi.stubGlobal(
      'fetch',
      vi.fn(async () => new Response(JSON.stringify({ action: 'play', cards: ['3', '3'] }), { status: 200 }))
    );
    const result = await callPlayAgent(
      { nickname: 'r', role: 'PEASANT', hand, lastPlay: null },
      { url: 'http://127.0.0.1:8000' }
    );
    expect(result.action).toBe('play');
    expect(result.cards!.map((x) => x.level)).toEqual([0, 0]);
  });

  it('passes through a pass response', async () => {
    vi.stubGlobal(
      'fetch',
      vi.fn(async () => new Response(JSON.stringify({ action: 'pass', cards: [] }), { status: 200 }))
    );
    const result = await callPlayAgent(
      { nickname: 'r', role: 'PEASANT', hand: [c(3, 0)], lastPlay: { nickname: 'x', cards: [c(2, 5)] } },
      { url: 'http://127.0.0.1:8000' }
    );
    expect(result.action).toBe('pass');
  });

  it('throws on non-2xx so the caller can fall back', async () => {
    vi.stubGlobal('fetch', vi.fn(async () => new Response('boom', { status: 500 })));
    await expect(
      callPlayAgent({ nickname: 'r', role: 'PEASANT', hand: [c(3, 0)], lastPlay: null }, { url: 'http://x' })
    ).rejects.toThrow();
  });

  it('throws when returned cards cannot be mapped to the hand', async () => {
    vi.stubGlobal(
      'fetch',
      vi.fn(async () => new Response(JSON.stringify({ action: 'play', cards: ['2'] }), { status: 200 }))
    );
    await expect(
      callPlayAgent({ nickname: 'r', role: 'PEASANT', hand: [c(3, 0)], lastPlay: null }, { url: 'http://x' })
    ).rejects.toThrow();
  });
});
