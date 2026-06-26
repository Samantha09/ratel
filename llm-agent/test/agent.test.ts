import { describe, it, expect, beforeEach, afterEach } from 'vitest';
import { WebSocketServer } from 'ws';
import { Agent } from '../src/agent.js';
import { LlmConfig } from '../src/decision.js';

describe('Agent', () => {
  let wss: WebSocketServer;
  let url: string;
  let received: string[];
  const mockConfig: LlmConfig = { provider: 'mock' };

  beforeEach(async () => {
    received = [];
    wss = new WebSocketServer({ port: 0 });
    await new Promise<void>((resolve) => wss.once('listening', resolve));
    const addr = wss.address();
    if (addr === null || typeof addr === 'string') throw new Error('invalid server address');
    url = `ws://127.0.0.1:${addr.port}`;
    wss.on('connection', (ws) => {
      ws.send(JSON.stringify({ event: 'idSet', data: { clientId: 1 } }));
      ws.on('message', (raw) => received.push(raw.toString()));
    });
  });

  afterEach(() => wss.close());

  it('sends nickname on start', async () => {
    const agent = new Agent({ url, nickname: 'robot_1', llmConfig: mockConfig });
    await agent.start();
    await new Promise((r) => setTimeout(r, 50));
    expect(received).toContain(JSON.stringify({ event: 'setNickname', data: { nickname: 'robot_1' } }));
  });

  it('responds to landlordElect when it is turn', async () => {
    const agent = new Agent({ url, nickname: 'robot_1', llmConfig: mockConfig });
    await agent.start();
    await new Promise((r) => setTimeout(r, 50));
    received = [];

    wss.clients.forEach((ws) =>
      ws.send(JSON.stringify({ event: 'landlordElect', data: { nextClientId: 1, nextClientNickname: 'robot_1' } }))
    );

    await new Promise((r) => setTimeout(r, 100));
    expect(received.some((m) => m.includes('landlordElect'))).toBe(true);
  });
});
