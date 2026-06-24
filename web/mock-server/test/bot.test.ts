import { describe, it, expect } from 'vitest';
import { botPlay } from '../src/bot.js';
import { detectSell } from '../src/cardtype.js';
import { Card } from '../src/types.js';

const c = (level: number, type = 1): Card => ({ type: type as Card['type'], level });

describe('botPlay', () => {
  it('leads with the smallest single when no last play', () => {
    const hand = [c(9), c(4), c(7)];
    const action = botPlay(hand, null);
    expect(action.kind).toBe('play');
    if (action.kind === 'play') expect(action.cards[0].level).toBe(4);
  });

  it('beats a single with the smallest higher single', () => {
    const hand = [c(5), c(8), c(11)];
    const last = detectSell([c(4)])!;
    const action = botPlay(hand, last);
    expect(action.kind).toBe('play');
    if (action.kind === 'play') expect(action.cards[0].level).toBe(5);
  });

  it('passes when it cannot beat', () => {
    const hand = [c(2), c(3)];
    const last = detectSell([c(11)])!; // Ace
    expect(botPlay(hand, last).kind).toBe('pass');
  });

  it('uses a bomb when it cannot beat otherwise', () => {
    const hand = [c(2), c(3), c(7, 1), c(7, 2), c(7, 3), c(7, 4)];
    const last = detectSell([c(11)])!; // Ace single
    const action = botPlay(hand, last);
    expect(action.kind).toBe('play');
    if (action.kind === 'play') expect(action.cards.length).toBe(4);
  });
});
