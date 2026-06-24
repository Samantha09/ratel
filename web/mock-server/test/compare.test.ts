import { describe, it, expect } from 'vitest';
import { detectSell } from '../src/cardtype.js';
import { canBeat } from '../src/compare.js';

const sell = (cards: [number, number][]) =>
  detectSell(cards.map(([l, t]) => ({ type: t as 0 | 1 | 2 | 3 | 4, level: l })))!;

describe('canBeat', () => {
  it('higher single beats lower single', () => {
    expect(canBeat(sell([[5, 1]]), sell([[3, 1]]))).toBe(true);
    expect(canBeat(sell([[3, 1]]), sell([[5, 1]]))).toBe(false);
  });

  it('requires same type + length for normal plays', () => {
    expect(canBeat(sell([[5, 1], [5, 2]]), sell([[3, 1]]))).toBe(false); // pair vs single
    expect(canBeat(sell([[5, 1]]), sell([[3, 1], [3, 2]]))).toBe(false); // single vs pair
  });

  it('straight must match length', () => {
    const s5 = sell([[0, 1], [1, 1], [2, 1], [3, 1], [4, 1]]);
    const s5b = sell([[1, 1], [2, 1], [3, 1], [4, 1], [5, 1]]);
    expect(canBeat(s5b, s5)).toBe(true);
  });

  it('bomb beats any non-bomb', () => {
    expect(canBeat(sell([[7, 1], [7, 2], [7, 3], [7, 4]]), sell([[11, 1]]))).toBe(true);
  });

  it('higher bomb beats lower bomb', () => {
    const low = sell([[3, 1], [3, 2], [3, 3], [3, 4]]);
    const high = sell([[9, 1], [9, 2], [9, 3], [9, 4]]);
    expect(canBeat(high, low)).toBe(true);
    expect(canBeat(low, high)).toBe(false);
  });

  it('rocket beats everything, nothing beats rocket', () => {
    const rocket = sell([[13, 0], [14, 0]]);
    const bomb = sell([[12, 1], [12, 2], [12, 3], [12, 4]]);
    expect(canBeat(rocket, bomb)).toBe(true);
    expect(canBeat(bomb, rocket)).toBe(false);
  });
});
