import { describe, it, expect } from 'vitest';
import { Card } from '../types';
import { evaluate, beats, legalPlays, ordinal, isBomb, isKingBomb } from './pokerEngine';

const c = (level: number, type = 1): Card => ({ type: type as Card['type'], level });
// 小王 level 13 / 大王 level 14 (type 0 = BLANK)
const small = (): Card => c(13, 0);
const big = (): Card => c(14, 0);

describe('pokerEngine · ordinal', () => {
  it('maps ranks 3..2 then 小王/大王 in strict order', () => {
    expect(ordinal(c(0))).toBe(3); // 3
    expect(ordinal(c(11))).toBe(14); // A
    expect(ordinal(c(12))).toBe(15); // 2
    expect(ordinal(small())).toBe(16); // 小王
    expect(ordinal(big())).toBe(17); // 大王
  });
});

describe('pokerEngine · evaluate', () => {
  it('single / pair / triple', () => {
    expect(evaluate([c(0)])?.kind).toBe('single');
    expect(evaluate([c(0), c(0, 2)])?.kind).toBe('pair');
    expect(evaluate([c(5), c(5, 2), c(5, 3)])?.kind).toBe('triple');
  });

  it('trio with single / pair wings', () => {
    expect(evaluate([c(5), c(5, 2), c(5, 3), c(0)])?.kind).toBe('trio_single');
    expect(evaluate([c(5), c(5, 2), c(5, 3), c(0), c(0, 2)])?.kind).toBe('trio_pair');
  });

  it('straight of 5+ up to A', () => {
    expect(evaluate([c(0), c(1), c(2), c(3), c(4)])?.kind).toBe('straight'); // 3-7
    expect(evaluate([c(7), c(8), c(9), c(10), c(11)])?.kind).toBe('straight'); // 10,J,Q,K,A
  });

  it('straight cannot include 2 or jokers', () => {
    expect(evaluate([c(8), c(9), c(10), c(11), c(12)])).toBeNull(); // J,Q,K,A,2
  });

  it('consecutive pairs run (>=3 pairs)', () => {
    expect(evaluate([c(0), c(0, 2), c(1), c(1, 2), c(2), c(2, 2)])?.kind).toBe('pairs_run');
  });

  it('bomb (4 of a kind) and rocket (both jokers)', () => {
    const bomb = evaluate([c(5), c(5, 2), c(5, 3), c(5, 4)])!;
    expect(bomb.kind).toBe('bomb');
    expect(isBomb(bomb)).toBe(true);
    expect(isKingBomb(bomb)).toBe(false);

    const rocket = evaluate([small(), big()])!;
    expect(rocket.kind).toBe('rocket');
    expect(isKingBomb(rocket)).toBe(true);
    expect(isBomb(rocket)).toBe(false);
  });

  it('airplane — two consecutive triples', () => {
    // 555 666
    expect(evaluate([c(2), c(2, 2), c(2, 3), c(3), c(3, 2), c(3, 3)])?.kind).toBe('airplane');
  });

  it('rejects illegal combinations', () => {
    expect(evaluate([])).toBeNull();
    expect(evaluate([c(0), c(2)])).toBeNull(); // two unrelated singles
    expect(evaluate([c(0), c(1), c(2)])).toBeNull(); // 3-card "straight" too short
  });
});

describe('pokerEngine · beats', () => {
  it('leading (null) is always beatable', () => {
    expect(beats(evaluate([c(0)])!, null)).toBe(true);
  });

  it('rocket beats a bomb', () => {
    expect(beats(evaluate([small(), big()])!, evaluate([c(5), c(5, 2), c(5, 3), c(5, 4)])!)).toBe(true);
  });

  it('bomb beats any non-bomb', () => {
    expect(beats(evaluate([c(5), c(5, 2), c(5, 3), c(5, 4)])!, evaluate([c(11)])!)).toBe(true);
  });

  it('higher single beats lower; mismatched type does not beat', () => {
    expect(beats(evaluate([c(6)])!, evaluate([c(5)])!)).toBe(true);
    expect(beats(evaluate([c(5), c(5, 2)])!, evaluate([c(6)])!)).toBe(false);
  });
});

describe('pokerEngine · legalPlays', () => {
  it('when leading, returns a spread of valid options including the straight', () => {
    const hand = [c(0), c(1), c(2), c(3), c(4)]; // 3,4,5,6,7
    const plays = legalPlays(hand, null);
    expect(plays.length).toBeGreaterThan(0);
    for (const p of plays) expect(evaluate(p)).not.toBeNull();
    expect(plays.some((p) => evaluate(p)?.kind === 'straight')).toBe(true);
  });

  it('when following a single, returns only singles that beat it', () => {
    const hand = [c(5), c(6)]; // 8, 9
    const last = evaluate([c(4)])!; // single 5
    const plays = legalPlays(hand, last);
    expect(plays).toHaveLength(2);
    for (const p of plays) expect(evaluate(p)?.kind).toBe('single');
  });

  it('offers a bomb override when no normal play beats the lead', () => {
    const hand = [c(2), c(2, 2), c(2, 3), c(2, 4)]; // bomb of 5s
    const last = evaluate([c(11)])!; // single A
    const plays = legalPlays(hand, last);
    expect(plays.some((p) => evaluate(p)?.kind === 'bomb')).toBe(true);
  });
});
