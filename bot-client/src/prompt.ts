import { Card, LastPlay } from './types.js';
import { Sell } from './rules.js';

function cardName(c: Card): string {
  const levels = ['3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A', '2', '小王', '大王'];
  const types = ['', '♦', '♣', '♠', '♥'];
  return `${types[c.type] ?? ''}${levels[c.level] ?? String(c.level)}`.trim();
}

function sortHand(hand: Card[]): Card[] {
  return [...hand].sort((a, b) => a.level - b.level || a.type - b.type);
}

export function formatHand(hand: Card[]): string {
  return sortHand(hand).map(cardName).join(', ');
}

export function formatSell(sell: Sell): string {
  const typeNames: Record<number, string> = {
    1: '炸弹',
    2: '王炸',
    3: '单张',
    4: '对子',
    5: '三张',
    6: '三带一',
    7: '三带二',
    10: '顺子',
    11: '连对',
  };
  return `${typeNames[sell.type] ?? '牌型'}(${sell.cards.map(cardName).join(' ')})`;
}

export interface PlayPromptInput {
  nickname: string;
  role: 'LANDLORD' | 'PEASANT' | null;
  hand: Card[];
  lastPlay: LastPlay | null;
  options: Sell[];
  /** 对手剩余牌数 {昵称或id: 张数};仅传给 Python play-agent 记牌,LLM prompt 不使用。 */
  otherCounts?: Record<string, number>;
  /** 已知底牌(仅地主可知);仅传给 Python play-agent,LLM prompt 不使用。 */
  bottomCards?: Card[];
}

export interface LandlordPromptInput {
  nickname: string;
  hand: Card[];
}

export function buildLandlordPrompt(input: LandlordPromptInput): string {
  return `你是斗地主玩家 ${input.nickname}。
当前手牌：${formatHand(input.hand)}。
请决定是否抢地主。抢到地主会获得 3 张底牌，但需独自对抗两名农民。
请返回 JSON：{"action": "grab" | "pass", "reason": "..."}`;
}

export function buildPlayPrompt(input: PlayPromptInput): string {
  const roleText = input.role === 'LANDLORD' ? '地主' : input.role === 'PEASANT' ? '农民' : '未知';
  const lastPlayText = input.lastPlay
    ? `上家出牌：${input.lastPlay.nickname} 出了 [${input.lastPlay.cards.map(cardName).join(', ')}]`
    : '你是首家，尚无场上出牌。';

  const optionLines = input.options.length
    ? input.options
        .map((sell, idx) => `${idx + 1}. [${sell.cards.map(cardName).join(', ')}] — ${formatSell(sell)}`)
        .join('\n')
    : '（无合法出牌，必须 pass）';

  return `你是斗地主玩家 ${input.nickname}，身份：${roleText}。
当前手牌：${formatHand(input.hand)}。
${lastPlayText}
轮到你出牌。只能从以下已通过规则校验的合法选项中选择一组（或 pass）：
${optionLines}
pass — 要不起

请返回 JSON：{"action": "play" | "pass", "cards": [{"level": number, "type": number}, ...], "reason": "..."}
注意：如果选择 play，cards 必须完全匹配上面某一组牌的 level 和 type。`;
}

export function formatDecisionForLog(decision: { action: string; cards?: Card[]; reason: string }): string {
  if (decision.action === 'pass') return 'pass';
  if (decision.action === 'grab') return '抢地主';
  return `出牌 [${decision.cards?.map(cardName).join(', ') ?? ''}]`;
}
