import { generateObject } from 'ai';
import { createAnthropic } from '@ai-sdk/anthropic';
import { z } from 'zod';
import { Card, Decision } from './types.js';
import { buildPlayPrompt, buildLandlordPrompt, PlayPromptInput, LandlordPromptInput } from './prompt.js';
import { Sell, validSells, detectSell, cardsInHand, sortHand } from './rules.js';

export interface LlmConfig {
  provider: 'minimax' | 'anthropic' | 'openai' | 'ollama' | 'mock';
  apiKey?: string;
  baseUrl?: string;
  model?: string;
  temperature?: number;
}

const decisionSchema = z.object({
  action: z.enum(['play', 'pass', 'grab']),
  cards: z.array(z.object({ level: z.number(), type: z.number() })).optional(),
  reason: z.string(),
});

function createProvider(config: LlmConfig) {
  if (config.provider === 'minimax') {
    return createAnthropic({
      apiKey: config.apiKey,
      baseURL: config.baseUrl ?? 'https://api.minimaxi.com/anthropic',
    });
  }
  throw new Error(`Provider ${config.provider} not yet implemented; use minimax or mock`);
}

export async function decideLandlord(input: LandlordPromptInput, config: LlmConfig): Promise<Decision> {
  if (config.provider === 'mock') {
    const hasBomb = input.hand.some((c) => input.hand.filter((x) => x.level === c.level).length === 4);
    const hasJoker = input.hand.some((c) => c.level === 13 || c.level === 14);
    return { action: hasBomb || hasJoker ? 'grab' : 'pass', reason: 'mock strategy' };
  }
  const provider = createProvider(config);
  const { object } = await generateObject({
    model: provider(config.model ?? 'MiniMax-M2.7'),
    schema: decisionSchema,
    prompt: buildLandlordPrompt(input),
    temperature: config.temperature ?? 0.3,
  });
  return { action: object.action === 'grab' ? 'grab' : 'pass', reason: object.reason };
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

function chooseFallback(options: Sell[]): Decision {
  if (options.length === 0) return { action: 'pass', reason: 'no valid sells (fallback)' };
  const sorted = [...options].sort((a, b) => a.coreLevel - b.coreLevel || a.cards.length - b.cards.length);
  return { action: 'play', cards: sorted[0].cards, reason: 'fallback smallest beat' };
}

export async function decidePlay(input: PlayPromptInput, config: LlmConfig): Promise<Decision> {
  if (config.provider === 'mock') {
    return chooseFallback(input.options);
  }
  const provider = createProvider(config);
  const { object } = await generateObject({
    model: provider(config.model ?? 'MiniMax-M2.7'),
    schema: decisionSchema,
    prompt: buildPlayPrompt(input),
    temperature: config.temperature ?? 0.3,
  });

  if (object.action === 'pass') {
    return { action: 'pass', reason: object.reason };
  }

  const chosen = object.cards ?? [];
  if (!cardsInHand(input.hand, chosen)) {
    return chooseFallback(input.options);
  }
  const matched = findMatchingOption(chosen, input.options);
  if (!matched) {
    return chooseFallback(input.options);
  }
  return { action: 'play', cards: matched.cards, reason: object.reason };
}
