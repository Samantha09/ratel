import { describe, it, expect } from 'vitest';
import { decidePlay, decideLandlord, LlmConfig } from '../src/decision.js';
import { validSells } from '../src/rules.js';
import { Card } from '../src/types.js';

const c = (level: number, type = 0): Card => ({ level, type });
const mockConfig: LlmConfig = { provider: 'mock' };

describe('decideLandlord mock', () => {
  it('grabs when has bomb', async () => {
    const hand = [c(0), c(0), c(0), c(0), c(1)];
    const decision = await decideLandlord({ nickname: 'r', hand }, mockConfig);
    expect(decision.action).toBe('grab');
  });
  it('passes with weak hand', async () => {
    const hand = [c(0), c(1), c(2)];
    const decision = await decideLandlord({ nickname: 'r', hand }, mockConfig);
    expect(decision.action).toBe('pass');
  });
});

describe('decidePlay mock', () => {
  it('plays smallest valid sell', async () => {
    const hand = [c(1), c(2), c(3), c(4), c(5)];
    const options = validSells(null, hand);
    const decision = await decidePlay({ nickname: 'r', role: 'PEASANT', hand, lastPlay: null, options }, mockConfig);
    expect(decision.action).toBe('play');
    expect(decision.cards).toBeDefined();
  });

  it('passes when no options', async () => {
    const hand = [c(0)];
    const decision = await decidePlay(
      { nickname: 'r', role: 'PEASANT', hand, lastPlay: { clientId: 2, nickname: 'x', cards: [c(14), c(13)] }, options: [] },
      mockConfig
    );
    expect(decision.action).toBe('pass');
  });
});
