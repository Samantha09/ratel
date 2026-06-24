import { Card, SellType } from './types.js';
import { Sell, detectSell } from './cardtype.js';
import { canBeat } from './compare.js';

export type BotAction = { kind: 'play'; cards: Card[] } | { kind: 'pass' };

function groupByLevel(hand: Card[]): Map<number, Card[]> {
  const m = new Map<number, Card[]>();
  for (const card of hand) {
    if (!m.has(card.level)) m.set(card.level, []);
    m.get(card.level)!.push(card);
  }
  return m;
}

function findSmallestBeat(hand: Card[], last: Sell): Sell | null {
  const grouped = groupByLevel(hand);
  const levelsAsc = [...grouped.keys()].sort((a, b) => a - b);

  switch (last.type) {
    case SellType.SINGLE:
      for (const lvl of levelsAsc) if (lvl > last.coreLevel) return detectSell([grouped.get(lvl)![0]]);
      break;
    case SellType.DOUBLE:
      for (const lvl of levelsAsc) if (lvl > last.coreLevel && grouped.get(lvl)!.length >= 2)
        return detectSell(grouped.get(lvl)!.slice(0, 2));
      break;
    case SellType.THREE:
      for (const lvl of levelsAsc) if (lvl > last.coreLevel && grouped.get(lvl)!.length >= 3)
        return detectSell(grouped.get(lvl)!.slice(0, 3));
      break;
    case SellType.SINGLE_STRAIGHT: {
      const len = last.length;
      for (let start = last.coreLevel + 1; start + len - 1 <= 11; start++) {
        let ok = true;
        for (let i = 0; i < len; i++) if (!grouped.has(start + i)) { ok = false; break; }
        if (ok) {
          const cards: Card[] = [];
          for (let i = 0; i < len; i++) cards.push(grouped.get(start + i)![0]);
          return detectSell(cards);
        }
      }
      break;
    }
    case SellType.DOUBLE_STRAIGHT: {
      const len = last.length; // pair count
      for (let start = last.coreLevel + 1; start + len - 1 <= 11; start++) {
        let ok = true;
        for (let i = 0; i < len; i++) if ((grouped.get(start + i)?.length ?? 0) < 2) { ok = false; break; }
        if (ok) {
          const cards: Card[] = [];
          for (let i = 0; i < len; i++) cards.push(...grouped.get(start + i)!.slice(0, 2));
          return detectSell(cards);
        }
      }
      break;
    }
    default:
      // attachment types (3+1, 3+2) — bot passes on these in the mock
      break;
  }
  return null;
}

function findBomb(hand: Card[]): Sell | null {
  const grouped = groupByLevel(hand);
  for (const [, cards] of grouped) if (cards.length === 4) return detectSell(cards);
  if (grouped.has(13) && grouped.has(14)) return detectSell([grouped.get(13)![0], grouped.get(14)![0]]);
  return null;
}

export function botPlay(hand: Card[], last: Sell | null): BotAction {
  if (hand.length === 0) return { kind: 'pass' };

  if (!last) {
    const sorted = [...hand].sort((a, b) => a.level - b.level || a.type - b.type);
    return { kind: 'play', cards: [sorted[0]] };
  }

  const beat = findSmallestBeat(hand, last);
  if (beat) return { kind: 'play', cards: beat.cards };

  const bomb = findBomb(hand);
  if (bomb && canBeat(bomb, last)) return { kind: 'play', cards: bomb.cards };

  return { kind: 'pass' };
}
