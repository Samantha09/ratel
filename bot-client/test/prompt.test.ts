import { describe, it, expect } from 'vitest';
import { buildPlayPrompt, buildLandlordPrompt, formatHand } from '../src/prompt.js';
import { validSells } from '../src/rules.js';
import { Card, PokerType } from '../src/types.js';

const c = (level: number, type: PokerType = 0): Card => ({ level, type });

describe('prompts', () => {
  it('formats hand', () => {
    expect(formatHand([c(0), c(1), c(2)])).toContain('3');
  });

  it('includes options in play prompt', () => {
    const hand = [c(3), c(4), c(5), c(6), c(7)];
    const options = validSells(null, hand);
    const prompt = buildPlayPrompt({
      nickname: 'robot_1',
      role: 'PEASANT',
      hand,
      lastPlay: null,
      options,
    });
    expect(prompt).toContain('robot_1');
    expect(prompt).toContain('顺子');
  });

  it('includes pass option when no options', () => {
    const prompt = buildPlayPrompt({
      nickname: 'robot_1',
      role: 'PEASANT',
      hand: [c(3)],
      lastPlay: { clientId: 2, nickname: 'robot_2', cards: [c(14), c(13)] },
      options: [],
    });
    expect(prompt).toContain('必须 pass');
  });

  it('includes hand in landlord prompt', () => {
    const prompt = buildLandlordPrompt({ nickname: 'robot_1', hand: [c(3), c(4)] });
    expect(prompt).toContain('robot_1');
    expect(prompt).toContain('抢地主');
  });
});
