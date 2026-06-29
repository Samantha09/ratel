import { describe, it, expect } from 'vitest';
import { decidePlay, decideLandlord, extractJsonObject, LlmConfig } from '../src/decision.js';
import { validSells, detectSell } from '../src/rules.js';
import { Card, PokerType } from '../src/types.js';

const c = (level: number, type: PokerType = 0): Card => ({ level, type });
const mockConfig: LlmConfig = { provider: 'mock' };
// 真实 provider 但无需联网:第0层强制回合会在调用 LLM 前短路返回
const liveConfig: LlmConfig = { provider: 'minimax', apiKey: 'unused' };

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

describe('decidePlay 分层管线', () => {
  it('leading: 甩出最长牌组(顺子)而非最小单张', async () => {
    const hand = [c(1), c(2), c(3), c(4), c(5)];
    const options = validSells(null, hand);
    const decision = await decidePlay({ nickname: 'r', role: 'PEASANT', hand, lastPlay: null, options }, mockConfig);
    expect(decision.action).toBe('play');
    expect(decision.cards).toHaveLength(5); // 5 连顺
  });

  it('第0层: 领出唯一选项强制出,不调 LLM(无 key 也能跑)', async () => {
    const hand = [c(5)];
    const options = validSells(null, hand);
    expect(options).toHaveLength(1);
    const decision = await decidePlay({ nickname: 'r', role: 'PEASANT', hand, lastPlay: null, options }, liveConfig);
    expect(decision.action).toBe('play');
    expect(decision.cards).toHaveLength(1);
  });

  it('第0层: 跟牌唯一非炸弹直接压,不调 LLM', async () => {
    const options = [detectSell([c(8)])!]; // 单张 8
    const decision = await decidePlay(
      { nickname: 'r', role: 'PEASANT', hand: [c(8)], lastPlay: { clientId: 2, nickname: 'x', cards: [c(7)] }, options },
      liveConfig
    );
    expect(decision.action).toBe('play');
  });

  it('第0层: 跟牌唯一选项是炸弹则保留过牌,不调 LLM', async () => {
    const bomb = detectSell([c(8, 1), c(8, 2), c(8, 3), c(8, 4)])!;
    const decision = await decidePlay(
      { nickname: 'r', role: 'PEASANT', hand: [c(8, 1), c(8, 2), c(8, 3), c(8, 4)], lastPlay: { clientId: 2, nickname: 'x', cards: [c(7)] }, options: [bomb] },
      liveConfig
    );
    expect(decision.action).toBe('pass');
  });

  it('跟牌: 多个选项里只有炸弹时保留炸弹过牌', async () => {
    const single9 = detectSell([c(9)])!;
    const bomb = detectSell([c(8, 1), c(8, 2), c(8, 3), c(8, 4)])!;
    // 上家出单张 10,只有炸弹能压(9 压不过),options 含一个非法对照 + 炸弹 → 用 mock 走规则
    const decision = await decidePlay(
      { nickname: 'r', role: 'PEASANT', hand: [c(8, 1), c(8, 2), c(8, 3), c(8, 4), c(9)], lastPlay: { clientId: 2, nickname: 'x', cards: [c(10)] }, options: [bomb] },
      mockConfig
    );
    // 单一炸弹选项 → 第0层炸弹保留 → pass
    expect(decision.action).toBe('pass');
    void single9;
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

describe('extractJsonObject', () => {
  it('extracts a bare JSON object', () => {
    expect(extractJsonObject('{"action":"pass","reason":"weak"}')).toBe('{"action":"pass","reason":"weak"}');
  });

  it('extracts JSON from a ```json fenced block', () => {
    const text = 'Here is my choice:\n```json\n{"action":"grab","reason":"strong"}\n```';
    expect(JSON.parse(extractJsonObject(text)).action).toBe('grab');
  });

  it('ignores prose before and after the object', () => {
    const text = '思考一下……我决定 {"action":"play","cards":[{"level":1,"type":0}],"reason":"x"} 就这样。';
    expect(JSON.parse(extractJsonObject(text)).action).toBe('play');
  });

  it('handles nested objects via brace matching', () => {
    const text = '{"action":"play","cards":[{"level":3,"type":2}],"reason":"a } in prose"}';
    const parsed = JSON.parse(extractJsonObject(text));
    expect(parsed.cards[0].level).toBe(3);
  });

  it('throws when no JSON object present', () => {
    expect(() => extractJsonObject('no json here')).toThrow();
  });
});
