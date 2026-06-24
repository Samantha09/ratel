import { Card } from './types.js';

export function buildDeck(): Card[] {
  const cards: Card[] = [];
  for (let level = 0; level <= 12; level++) {
    for (let type = 1; type <= 4; type++) {
      cards.push({ type: type as Card['type'], level });
    }
  }
  cards.push({ type: 0, level: 13 }); // small joker
  cards.push({ type: 0, level: 14 }); // big joker
  return cards;
}

export function shuffle<T>(arr: T[], rng: () => number = Math.random): T[] {
  const a = [...arr];
  for (let i = a.length - 1; i > 0; i--) {
    const j = Math.floor(rng() * (i + 1));
    [a[i], a[j]] = [a[j], a[i]];
  }
  return a;
}

export function deal(deck: Card[], rng: () => number = Math.random): { hands: Card[][]; bottom: Card[] } {
  const shuffled = shuffle(deck, rng);
  const hands: Card[][] = [[], [], []];
  for (let i = 0; i < 51; i++) hands[i % 3].push(shuffled[i]);
  const bottom = shuffled.slice(51, 54);
  const sortFn = (a: Card, b: Card) => a.level - b.level || a.type - b.type;
  hands.forEach((h) => h.sort(sortFn));
  bottom.sort(sortFn);
  return { hands, bottom };
}
