import { Card } from './types.js';

/**
 * HTTP 桥接:把出牌决策委托给 Python dou-dizhu-agent 的 `POST /play`。
 *
 * Python agent 是无状态的"出牌大脑"——只认字符牌(rank string)、不带花色,
 * 也不连 gateway。本模块负责:
 *  1. 把 TS 的 {type,level} 手牌/上家牌转成 Python 的字符牌;
 *  2. 调用 /play 拿到 {action, cards:string[]};
 *  3. 把返回的字符牌按 level 贪心映射回手里**真实存在**的 {type,level} 卡
 *     (Python 丢了花色,这里从手牌补回,保证 gateway 的 checkPokerIndex 通过)。
 *
 * level 与 Python RANK_INDEX 顺序完全一致(见 prompt.ts cardName / rules.py CARD_RANKS):
 *   0='3' … 12='2', 13='小王', 14='大王'。
 */
const CARD_RANKS = ['3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A', '2', '小王', '大王'];
const RANK_TO_LEVEL = new Map(CARD_RANKS.map((r, i) => [r, i]));

export function levelToRank(level: number): string {
  return CARD_RANKS[level] ?? String(level);
}

export function cardsToRanks(cards: Card[]): string[] {
  return cards.map((c) => levelToRank(c.level));
}

/**
 * 把字符牌按 level 贪心映射回手牌里真实的 {type,level} 卡。
 * 任一字符牌无法在(剩余)手牌中找到对应 level → 返回 null(由调用方回退规则牌)。
 */
export function ranksToCards(ranks: string[], hand: Card[]): Card[] | null {
  const pool = [...hand];
  const out: Card[] = [];
  for (const r of ranks) {
    const level = RANK_TO_LEVEL.get(r);
    if (level === undefined) return null;
    const idx = pool.findIndex((c) => c.level === level);
    if (idx < 0) return null;
    out.push(pool[idx]);
    pool.splice(idx, 1);
  }
  return out;
}

/** POST /play 的请求体,字段名与 Python PlayRequest(models.py)一一对应。 */
export interface PlayAgentRequest {
  player_id: string;
  hand: string[];
  role: 'landlord' | 'peasant';
  is_my_turn: boolean;
  last_play: string[];
  last_play_player?: string | null;
  other_players_card_count: Record<string, number>;
  bottom_cards: string[];
}

export interface PlayAgentResponse {
  action: 'play' | 'pass';
  cards: string[];
}

export interface PlayAgentInput {
  nickname: string;
  role: 'LANDLORD' | 'PEASANT' | null;
  hand: Card[];
  lastPlay: { nickname: string; cards: Card[] } | null;
  /** 对手剩余牌数:{昵称或id: 张数};Python 仅用于记牌摘要,可空。 */
  otherCounts?: Record<string, number>;
  /** 已知底牌(仅地主可知),未知传空。 */
  bottomCards?: Card[];
}

/** 兜底超时:即使调用方漏传也绝不让 HTTP 调用无界等待。 */
const DEFAULT_PLAY_AGENT_TIMEOUT_MS = 20000;

export function buildPlayAgentRequest(input: PlayAgentInput): PlayAgentRequest {
  return {
    player_id: input.nickname,
    hand: cardsToRanks(input.hand),
    role: input.role === 'LANDLORD' ? 'landlord' : 'peasant',
    is_my_turn: true,
    last_play: input.lastPlay ? cardsToRanks(input.lastPlay.cards) : [],
    last_play_player: input.lastPlay?.nickname ?? null,
    other_players_card_count: input.otherCounts ?? {},
    bottom_cards: input.bottomCards ? cardsToRanks(input.bottomCards) : [],
  };
}

/**
 * 调用 Python /play。返回映射回手牌的具体出牌,或 pass。
 * 任何网络/超时/解析/映射失败都会抛错,由 decidePlay 回退到规则牌。
 */
export async function callPlayAgent(
  input: PlayAgentInput,
  opts: { url: string; timeoutMs?: number }
): Promise<{ action: 'play' | 'pass'; cards?: Card[] }> {
  const timeoutMs =
    typeof opts.timeoutMs === 'number' && Number.isFinite(opts.timeoutMs) && opts.timeoutMs > 0
      ? opts.timeoutMs
      : DEFAULT_PLAY_AGENT_TIMEOUT_MS;
  const endpoint = `${opts.url.replace(/\/+$/, '')}/play`;
  const body = buildPlayAgentRequest(input);

  const res = await fetch(endpoint, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body),
    signal: AbortSignal.timeout(timeoutMs),
  });
  if (!res.ok) {
    throw new Error(`play-agent ${endpoint} responded ${res.status}`);
  }
  const data = (await res.json()) as PlayAgentResponse;

  if (data.action === 'pass') return { action: 'pass' };

  const cards = ranksToCards(data.cards ?? [], input.hand);
  if (!cards || cards.length === 0) {
    throw new Error(`play-agent returned cards not mappable to hand: ${JSON.stringify(data.cards)}`);
  }
  return { action: 'play', cards };
}
