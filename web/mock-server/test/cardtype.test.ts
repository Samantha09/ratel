import { describe, it, expect } from 'vitest';
import { detectSell } from '../src/cardtype.js';
import { SellType, Card } from '../src/types.js';

const c = (level: number, type = 1): Card => ({ type: type as Card['type'], level });

describe('detectSell', () => {
  it('rejects empty', () => {
    expect(detectSell([])).toBeNull();
  });

  it('detects single', () => {
    const s = detectSell([c(3)])!;
    expect(s.type).toBe(SellType.SINGLE);
    expect(s.coreLevel).toBe(3);
  });

  it('detects pair', () => {
    const s = detectSell([c(5, 1), c(5, 2)])!;
    expect(s.type).toBe(SellType.DOUBLE);
    expect(s.coreLevel).toBe(5);
  });

  it('detects triple', () => {
    const s = detectSell([c(7, 1), c(7, 2), c(7, 3)])!;
    expect(s.type).toBe(SellType.THREE);
    expect(s.coreLevel).toBe(7);
  });

  it('detects bomb (4 same)', () => {
    const s = detectSell([c(9, 1), c(9, 2), c(9, 3), c(9, 4)])!;
    expect(s.type).toBe(SellType.BOMB);
    expect(s.coreLevel).toBe(9);
  });

  it('detects rocket (both jokers)', () => {
    const s = detectSell([c(13, 0), c(14, 0)])!;
    expect(s.type).toBe(SellType.KING_BOMB);
  });

  it('detects triple + single', () => {
    const s = detectSell([c(6, 1), c(6, 2), c(6, 3), c(11, 1)])!;
    expect(s.type).toBe(SellType.THREE_ZONES_SINGLE);
    expect(s.coreLevel).toBe(6);
  });

  it('detects triple + pair', () => {
    const s = detectSell([c(6, 1), c(6, 2), c(6, 3), c(8, 1), c(8, 2)])!;
    expect(s.type).toBe(SellType.THREE_ZONES_DOUBLE);
    expect(s.coreLevel).toBe(6);
  });

  it('detects single straight (len 5, 3..7)', () => {
    const s = detectSell([c(0), c(1), c(2), c(3), c(4)])!;
    expect(s.type).toBe(SellType.SINGLE_STRAIGHT);
    expect(s.coreLevel).toBe(0);
    expect(s.length).toBe(5);
  });

  it('rejects 4-card straight (min is 5)', () => {
    expect(detectSell([c(0), c(1), c(2), c(3)])).toBeNull();
  });

  it('rejects straight containing a 2 (level 12) or joker', () => {
    expect(detectSell([c(9), c(10), c(11), c(12), c(13)])).toBeNull();
  });

  it('detects pair straight (3 pairs, 3..5)', () => {
    const s = detectSell([c(0, 1), c(0, 2), c(1, 1), c(1, 2), c(2, 1), c(2, 2)])!;
    expect(s.type).toBe(SellType.DOUBLE_STRAIGHT);
    expect(s.length).toBe(3);
  });

  it('returns null for illegal mixes', () => {
    expect(detectSell([c(0), c(2)])).toBeNull(); // two different singles
    expect(detectSell([c(0), c(1), c(3)])).toBeNull(); // not consecutive
  });
});
