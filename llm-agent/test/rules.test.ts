import { describe, it, expect } from 'vitest';
import { detectSell, canBeat, validSells, SellType, cardsInHand, sortHand } from '../src/rules.js';
import { Card, PokerType } from '../src/types.js';

const c = (level: number, type: PokerType = 0): Card => ({ level, type });

describe('detectSell', () => {
  it('detects rocket', () => {
    expect(detectSell([c(13), c(14)])?.type).toBe(SellType.KING_BOMB);
  });
  it('detects bomb', () => {
    expect(detectSell([c(3), c(3), c(3), c(3)])?.type).toBe(SellType.BOMB);
  });
  it('detects single straight', () => {
    expect(detectSell([c(3), c(4), c(5), c(6), c(7)])?.type).toBe(SellType.SINGLE_STRAIGHT);
  });
  it('returns null for illegal', () => {
    expect(detectSell([c(3), c(5)])).toBeNull();
  });
});

describe('canBeat', () => {
  it('bomb beats single', () => {
    const bomb = detectSell([c(3), c(3), c(3), c(3)])!;
    const single = detectSell([c(4)])!;
    expect(canBeat(bomb, single)).toBe(true);
  });
  it('same type higher level wins', () => {
    const low = detectSell([c(3)])!;
    const high = detectSell([c(4)])!;
    expect(canBeat(high, low)).toBe(true);
    expect(canBeat(low, high)).toBe(false);
  });
});

describe('validSells', () => {
  it('returns all legal sells when no last play', () => {
    const hand = [c(3), c(4), c(5), c(6), c(7)];
    const sells = validSells(null, hand);
    expect(sells.length).toBeGreaterThan(0);
    expect(sells.some((s) => s.type === SellType.SINGLE_STRAIGHT)).toBe(true);
  });
  it('returns only beats when last play exists', () => {
    const last = detectSell([c(3)])!;
    const hand = [c(4), c(5), c(3)];
    const sells = validSells(last, hand);
    expect(sells.every((s) => s.coreLevel > 3)).toBe(true);
  });
});

describe('cardsInHand', () => {
  it('checks card membership', () => {
    const hand = [c(3, 1), c(4, 2)];
    expect(cardsInHand(hand, [c(3, 1)])).toBe(true);
    expect(cardsInHand(hand, [c(3, 2)])).toBe(false);
  });
});

describe('sortHand', () => {
  it('sorts by level then type', () => {
    const sorted = sortHand([c(5, 1), c(3, 2), c(4, 3)]);
    expect(sorted.map((c) => c.level)).toEqual([3, 4, 5]);
  });
});
