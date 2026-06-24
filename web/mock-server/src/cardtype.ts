import { Card, SellType, SellTypeValue } from './types.js';

export interface Sell {
  type: SellTypeValue;
  coreLevel: number; // comparison key (level of the main body)
  length: number;    // number of cards for non-straights; unit count for straights
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

  // Rocket
  if (n === 2 && counts.has(13) && counts.has(14)) return mk(SellType.KING_BOMB, 100, 2);
  // Bomb
  if (n === 4 && distinct.length === 1) return mk(SellType.BOMB, distinct[0], 4);
  // Single
  if (n === 1) return mk(SellType.SINGLE, levels[0], 1);
  // Pair
  if (n === 2 && distinct.length === 1) return mk(SellType.DOUBLE, distinct[0], 2);
  // Triple
  if (n === 3 && distinct.length === 1) return mk(SellType.THREE, distinct[0], 3);
  // Triple + single
  if (n === 4 && distinct.length === 2) {
    const triple = [...counts.entries()].find(([, cnt]) => cnt === 3);
    if (triple) return mk(SellType.THREE_ZONES_SINGLE, triple[0], 4);
  }
  // Triple + pair
  if (n === 5 && distinct.length === 2) {
    const vals = [...counts.values()];
    if (vals.includes(3) && vals.includes(2)) {
      const triple = [...counts.entries()].find(([, cnt]) => cnt === 3)!;
      return mk(SellType.THREE_ZONES_DOUBLE, triple[0], 5);
    }
  }
  // Single straight: >=5 distinct consecutive, all <= 11 (A)
  if (n >= 5 && distinct.length === n && distinct.every((l) => l <= 11) && isConsecutive(distinct)) {
    return mk(SellType.SINGLE_STRAIGHT, distinct[0], n);
  }
  // Pair straight: >=3 pairs (>=6 cards), all counts 2, consecutive, all <= 11
  if (n >= 6 && n % 2 === 0 && [...counts.values()].every((cnt) => cnt === 2)
      && distinct.every((l) => l <= 11) && isConsecutive(distinct)) {
    return mk(SellType.DOUBLE_STRAIGHT, distinct[0], distinct.length);
  }
  return null;
}
