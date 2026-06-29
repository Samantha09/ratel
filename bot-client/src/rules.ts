import { Card } from './types.js';

export const SellType = {
  ILLEGAL: 0,
  BOMB: 1,
  KING_BOMB: 2,
  SINGLE: 3,
  DOUBLE: 4,
  THREE: 5,
  THREE_ZONES_SINGLE: 6,
  THREE_ZONES_DOUBLE: 7,
  SINGLE_STRAIGHT: 10,
  DOUBLE_STRAIGHT: 11,
  VOID_SELL: 18,
} as const;

export type SellTypeValue = (typeof SellType)[keyof typeof SellType];

export interface Sell {
  type: SellTypeValue;
  coreLevel: number;
  length: number;
  cards: Card[];
}

function countByLevel(cards: Card[]): Map<number, number> {
  const m = new Map<number, number>();
  for (const card of cards) m.set(card.level, (m.get(card.level) ?? 0) + 1);
  return m;
}

function isConsecutive(levels: number[]): boolean {
  for (let i = 1; i < levels.length; i++) {
    if (levels[i] !== levels[i - 1] + 1) return false;
  }
  return true;
}

export function detectSell(cards: Card[]): Sell | null {
  const n = cards.length;
  if (n === 0) return null;
  const counts = countByLevel(cards);
  const distinct = [...counts.keys()].sort((a, b) => a - b);
  const levels = cards.map((c) => c.level).sort((a, b) => a - b);
  const mk = (type: SellTypeValue, coreLevel: number, length: number): Sell => ({ type, coreLevel, length, cards });

  if (n === 2 && counts.has(13) && counts.has(14)) return mk(SellType.KING_BOMB, 100, 2);
  if (n === 4 && distinct.length === 1) return mk(SellType.BOMB, distinct[0], 4);
  if (n === 1) return mk(SellType.SINGLE, levels[0], 1);
  if (n === 2 && distinct.length === 1) return mk(SellType.DOUBLE, distinct[0], 2);
  if (n === 3 && distinct.length === 1) return mk(SellType.THREE, distinct[0], 3);
  if (n === 4 && distinct.length === 2) {
    const triple = [...counts.entries()].find(([, cnt]) => cnt === 3);
    if (triple) return mk(SellType.THREE_ZONES_SINGLE, triple[0], 4);
  }
  if (n === 5 && distinct.length === 2) {
    const vals = [...counts.values()];
    if (vals.includes(3) && vals.includes(2)) {
      const triple = [...counts.entries()].find(([, cnt]) => cnt === 3)!;
      return mk(SellType.THREE_ZONES_DOUBLE, triple[0], 5);
    }
  }
  if (n >= 5 && distinct.length === n && distinct.every((l) => l <= 11) && isConsecutive(distinct)) {
    return mk(SellType.SINGLE_STRAIGHT, distinct[0], n);
  }
  if (n >= 6 && n % 2 === 0 && [...counts.values()].every((cnt) => cnt === 2)
      && distinct.every((l) => l <= 11) && isConsecutive(distinct)) {
    return mk(SellType.DOUBLE_STRAIGHT, distinct[0], distinct.length);
  }
  return null;
}

export function canBeat(candidate: Sell, last: Sell): boolean {
  if (candidate.type === SellType.KING_BOMB) return true;
  if (last.type === SellType.KING_BOMB) return false;
  if (candidate.type === SellType.BOMB && last.type !== SellType.BOMB) return true;
  if (last.type === SellType.BOMB && candidate.type !== SellType.BOMB) return false;
  if (candidate.type === SellType.BOMB && last.type === SellType.BOMB) {
    return candidate.coreLevel > last.coreLevel;
  }
  if (candidate.type === last.type && candidate.length === last.length) {
    return candidate.coreLevel > last.coreLevel;
  }
  return false;
}

function combinations<T>(arr: T[], k: number): T[][] {
  if (k === 0) return [[]];
  if (arr.length < k) return [];
  if (arr.length === k) return [arr];
  const [first, ...rest] = arr;
  return combinations(rest, k - 1).map((c) => [first, ...c]).concat(combinations(rest, k));
}

export function validSells(lastPlay: Sell | null, hand: Card[]): Sell[] {
  if (!lastPlay) {
    const result: Sell[] = [];
    for (let k = 1; k <= hand.length; k++) {
      for (const combo of combinations(hand, k)) {
        const sell = detectSell(combo);
        if (sell && sell.type !== SellType.ILLEGAL) result.push(sell);
      }
    }
    return result;
  }
  const result: Sell[] = [];
  for (let k = 1; k <= hand.length; k++) {
    for (const combo of combinations(hand, k)) {
      const sell = detectSell(combo);
      if (sell && sell.type !== SellType.ILLEGAL && canBeat(sell, lastPlay)) {
        result.push(sell);
      }
    }
  }
  return result;
}

export function cardsInHand(hand: Card[], cards: Card[]): boolean {
  const pool = [...hand];
  for (const c of cards) {
    const idx = pool.findIndex((h) => h.level === c.level && h.type === c.type);
    if (idx < 0) return false;
    pool.splice(idx, 1);
  }
  return true;
}

export function removeCards(hand: Card[], cards: Card[]): Card[] {
  const pool = [...hand];
  for (const c of cards) {
    const idx = pool.findIndex((h) => h.level === c.level && h.type === c.type);
    if (idx >= 0) pool.splice(idx, 1);
  }
  return pool;
}

export function sortHand(hand: Card[]): Card[] {
  return [...hand].sort((a, b) => a.level - b.level || a.type - b.type);
}
