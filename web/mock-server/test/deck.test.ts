import { describe, it, expect } from 'vitest';
import { buildDeck, shuffle, deal } from '../src/deck.js';

describe('deck', () => {
  it('builds a 54-card deck', () => {
    expect(buildDeck().length).toBe(54);
  });

  it('has 4 of each rank 3..2 and two jokers', () => {
    const d = buildDeck();
    for (let lvl = 0; lvl <= 12; lvl++) {
      expect(d.filter((c) => c.level === lvl).length).toBe(4);
    }
    expect(d.filter((c) => c.level === 13).length).toBe(1); // small joker
    expect(d.filter((c) => c.level === 14).length).toBe(1); // big joker
  });

  it('shuffle preserves contents and length', () => {
    const d = buildDeck();
    const s = shuffle(d);
    expect(s.length).toBe(54);
    expect([...s].sort((a, b) => a.level - b.level || a.type - b.type))
      .toEqual([...d].sort((a, b) => a.level - b.level || a.type - b.type));
  });

  it('deal gives three 17-card hands and a 3-card bottom', () => {
    const { hands, bottom } = deal(buildDeck());
    expect(hands.length).toBe(3);
    expect(hands.every((h) => h.length === 17)).toBe(true);
    expect(bottom.length).toBe(3);
  });
});
