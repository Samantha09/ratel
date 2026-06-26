import { generateText } from 'ai';
import { createAnthropic } from '@ai-sdk/anthropic';
import { createOpenAI } from '@ai-sdk/openai';
import { z } from 'zod';
import { Card, Decision } from './types.js';
import { buildPlayPrompt, PlayPromptInput, LandlordPromptInput } from './prompt.js';
import { Sell, SellType, cardsInHand, sortHand } from './rules.js';

export interface LlmConfig {
  provider: 'minimax' | 'anthropic' | 'openai' | 'ollama' | 'mock';
  apiKey?: string;
  baseUrl?: string;
  model?: string;
  /** 出牌阶段专用模型；默认走高速变体以缩短推理延迟。 */
  playModel?: string;
  /** 出牌 LLM 调用的硬超时(毫秒);超时即回退规则牌,避免回合卡死。 */
  playTimeoutMs?: number;
  temperature?: number;
}

const decisionSchema = z.object({
  action: z.enum(['play', 'pass', 'grab']),
  cards: z.array(z.object({ level: z.number(), type: z.number() })).optional(),
  reason: z.string(),
});

type DecisionObject = z.infer<typeof decisionSchema>;

/** 兜底超时:即使调用方漏传 timeoutMs,也绝不让 LLM 调用无界等待。 */
const DEFAULT_DECISION_TIMEOUT_MS = 30000;

/**
 * 从模型文本里提取首个完整的 JSON 对象。
 * MiniMax-M2.7 等推理模型不支持 OpenAI 的结构化输出（json_schema/tool 模式），
 * 但能在普通文本里返回干净 JSON，可能带 ```json 代码块或前置说明，需自行解析。
 */
export function extractJsonObject(text: string): string {
  const fenced = text.match(/```(?:json)?\s*([\s\S]*?)```/i);
  const source = fenced ? fenced[1] : text;
  const start = source.indexOf('{');
  if (start === -1) throw new Error(`no JSON object in model output: ${text.slice(0, 120)}`);
  let depth = 0;
  let inString = false;
  let escaped = false;
  for (let i = start; i < source.length; i++) {
    const ch = source[i];
    if (escaped) {
      escaped = false;
      continue;
    }
    if (ch === '\\') {
      escaped = true;
      continue;
    }
    if (ch === '"') inString = !inString;
    if (inString) continue;
    if (ch === '{') depth++;
    else if (ch === '}') {
      depth--;
      if (depth === 0) return source.slice(start, i + 1);
    }
  }
  throw new Error(`unterminated JSON object in model output: ${text.slice(0, 120)}`);
}

async function generateDecision(
  config: LlmConfig,
  prompt: string,
  opts: { model?: string; timeoutMs?: number } = {}
): Promise<DecisionObject> {
  const provider = createProvider(config);
  // 永远应用一个超时:有效正数用调用方的值,否则用兜底值
  const timeoutMs =
    typeof opts.timeoutMs === 'number' && Number.isFinite(opts.timeoutMs) && opts.timeoutMs > 0
      ? opts.timeoutMs
      : DEFAULT_DECISION_TIMEOUT_MS;
  const { text } = await generateText({
    model: provider(opts.model ?? config.model ?? 'MiniMax-M2.7'),
    prompt,
    temperature: config.temperature ?? 0.3,
    // 硬超时:网络挂死或推理过久时主动取消请求,由调用方回退规则牌
    abortSignal: AbortSignal.timeout(timeoutMs),
  });
  return decisionSchema.parse(JSON.parse(extractJsonObject(text)));
}

function createProvider(config: LlmConfig) {
  if (config.provider === 'minimax' || config.provider === 'openai') {
    return createOpenAI({
      apiKey: config.apiKey,
      baseURL: config.baseUrl ?? 'https://api.minimaxi.com/v1',
    });
  }
  if (config.provider === 'anthropic') {
    return createAnthropic({
      apiKey: config.apiKey,
      baseURL: config.baseUrl,
    });
  }
  throw new Error(`Provider ${config.provider} not yet implemented; use minimax, openai, anthropic or mock`);
}

/**
 * 抢地主用瞬时启发式,不调 LLM。
 * 抢地主只是 grab/pass 的二元判断,不值得用推理模型等 30~57 秒;
 * 出牌阶段才保留 LLM 推理(见 decidePlay)。
 */
function heuristicLandlord(hand: Card[]): Decision {
  const countByLevel = new Map<number, number>();
  for (const c of hand) countByLevel.set(c.level, (countByLevel.get(c.level) ?? 0) + 1);

  const hasBomb = [...countByLevel.values()].some((n) => n >= 4);
  const hasSmallJoker = countByLevel.has(13);
  const hasBigJoker = countByLevel.has(14);
  const hasRocket = hasSmallJoker && hasBigJoker;
  const twos = countByLevel.get(12) ?? 0;
  const jokers = (hasSmallJoker ? 1 : 0) + (hasBigJoker ? 1 : 0);

  // 炸弹/王炸/大王 直接抢;否则手握 ≥2 张高牌(2 或王)也抢
  const grab = hasBomb || hasRocket || hasBigJoker || twos + jokers >= 2;
  const reasons: string[] = [];
  if (hasBomb) reasons.push('有炸弹');
  if (hasRocket) reasons.push('有王炸');
  else if (hasBigJoker) reasons.push('有大王');
  if (twos) reasons.push(`${twos} 张 2`);
  if (jokers && !hasRocket) reasons.push(`${jokers} 张王`);
  return {
    action: grab ? 'grab' : 'pass',
    reason: `启发式:${reasons.length ? reasons.join('、') : '无强牌'} → ${grab ? '抢' : '不抢'}`,
  };
}

export async function decideLandlord(input: LandlordPromptInput, _config: LlmConfig): Promise<Decision> {
  return heuristicLandlord(input.hand);
}

function findMatchingOption(cards: Card[], options: Sell[]): Sell | null {
  const sortedCards = sortHand(cards);
  return (
    options.find((sell) => {
      const sortedSell = sortHand(sell.cards);
      if (sortedSell.length !== sortedCards.length) return false;
      return sortedSell.every((c, i) => c.level === sortedCards[i].level && c.type === sortedCards[i].type);
    }) ?? null
  );
}

const BOMB_TYPES: number[] = [SellType.BOMB, SellType.KING_BOMB];
const isBomb = (s: Sell): boolean => BOMB_TYPES.includes(s.type);

/**
 * 瞬时规则出牌:不调 LLM,作为分层管线的安全网与超时兜底。
 * - 跟牌(有上家牌):优先用最小的非炸弹牌压制;若只能用炸弹/王炸则过牌保留炸弹。
 * - 领出(无上家牌):优先甩出最长的牌组(顺子/连对)快速减手,否则出最小单牌;不主动拆炸弹。
 */
function ruleBasedPlay(input: PlayPromptInput): Decision {
  const { options, lastPlay } = input;
  if (options.length === 0) return { action: 'pass', reason: '无合法出牌' };

  if (lastPlay) {
    const nonBomb = options.filter((s) => !isBomb(s));
    if (nonBomb.length === 0) {
      return { action: 'pass', reason: '只能用炸弹压,保留炸弹过牌' };
    }
    const best = [...nonBomb].sort((a, b) => a.coreLevel - b.coreLevel || a.cards.length - b.cards.length)[0];
    return { action: 'play', cards: best.cards, reason: '规则:最小非炸弹压制' };
  }

  // 领出:优先甩最长牌组减手,长度相同选最小;炸弹放最后
  const lead = [...options].sort((a, b) => {
    if (isBomb(a) !== isBomb(b)) return isBomb(a) ? 1 : -1;
    if (a.cards.length !== b.cards.length) return b.cards.length - a.cards.length;
    return a.coreLevel - b.coreLevel;
  })[0];
  return { action: 'play', cards: lead.cards, reason: '规则:领出甩最长牌组' };
}

/**
 * 分层出牌决策,目标是又快又不卡:
 *  第0层(瞬时,不调 LLM):0 个选项→pass;1 个选项→强制出。
 *  第1层(瞬时):算出规则牌作为安全网。
 *  第2层(LLM+硬超时):多选策略回合调 LLM,超时/出错/解析失败→回退第1层规则牌。
 */
export async function decidePlay(input: PlayPromptInput, config: LlmConfig): Promise<Decision> {
  const { options, lastPlay } = input;

  // 第0层:强制/平凡回合,瞬时返回,完全不调 LLM
  if (options.length === 0) return { action: 'pass', reason: '无合法出牌' };

  // 第1层:规则牌兜底(也用于第0层的炸弹保留判断)
  const fallback = ruleBasedPlay(input);

  if (options.length === 1) {
    // 领出唯一牌必须出;跟牌唯一非炸弹直接压;跟牌唯一炸弹交规则(默认保留过牌)
    if (!lastPlay || !isBomb(options[0])) {
      return { action: 'play', cards: options[0].cards, reason: '唯一合法牌,直接出' };
    }
    return fallback;
  }

  if (config.provider === 'mock') return fallback;

  // 第2层:多选策略回合调 LLM,加硬超时;超时/出错/解析失败→回退规则牌
  let object: DecisionObject;
  try {
    object = await generateDecision(config, buildPlayPrompt(input), {
      model: config.playModel,
      timeoutMs: config.playTimeoutMs,
    });
  } catch {
    return fallback;
  }

  if (object.action === 'pass') {
    return { action: 'pass', reason: object.reason };
  }

  const chosen = (object.cards as Card[] | undefined) ?? [];
  if (!cardsInHand(input.hand, chosen)) {
    return fallback;
  }
  const matched = findMatchingOption(chosen, options);
  if (!matched) {
    return fallback;
  }
  return { action: 'play', cards: matched.cards, reason: object.reason };
}
