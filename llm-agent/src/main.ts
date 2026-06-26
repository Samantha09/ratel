import dotenv from 'dotenv';
import { existsSync } from 'fs';
import { Agent } from './agent.js';
import { LlmConfig } from './decision.js';

dotenv.config({ path: existsSync('.env.local') ? '.env.local' : '.env' });

function env(key: string, defaultValue?: string): string {
  const value = process.env[key];
  if (value === undefined && defaultValue === undefined) {
    throw new Error(`Missing environment variable: ${key}`);
  }
  return value ?? defaultValue!;
}

function main(): void {
  console.log('[main] starting with provider:', env('LLM_PROVIDER', 'mock'));
  const url = env('GATEWAY_URL', 'ws://127.0.0.1:8787');
  const provider = env('LLM_PROVIDER', 'mock') as LlmConfig['provider'];
  const count = parseInt(env('AGENT_COUNT', '2'), 10);
  const prefix = env('AGENT_NICKNAME_PREFIX', 'robot');

  console.log('[main] gateway url:', url, 'agents:', count);

  const llmConfig: LlmConfig = {
    provider,
    apiKey: process.env.MINIMAX_API_KEY,
    baseUrl: process.env.MINIMAX_BASE_URL,
    model: process.env.MINIMAX_MODEL ?? 'MiniMax-M2.7',
    // 出牌用高速变体,显著缩短推理延迟;可用 PLAY_MODEL 覆盖
    playModel: process.env.PLAY_MODEL ?? 'MiniMax-M2.7-highspeed',
    playTimeoutMs: parseInt(process.env.PLAY_TIMEOUT_MS ?? '20000', 10),
    temperature: parseFloat(process.env.LLM_TEMPERATURE ?? '0.3'),
  };

  const agents: Agent[] = [];
  for (let i = 1; i <= count; i++) {
    const nickname = `${prefix}_${i}`;
    const agent = new Agent({
      url,
      nickname,
      llmConfig,
      onLog: (msg) => console.log(msg),
    });
    agents.push(agent);
    void agent.start();
  }

  process.on('SIGINT', () => {
    console.log('Shutting down agents...');
    agents.forEach((a) => a.close());
    process.exit(0);
  });
}

main();
