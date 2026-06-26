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

  // 把 agent 推进到"跟牌出牌"回合
  async function driveToFollowingPlay(agent: Agent): Promise<void> {
    await agent.start();
    await new Promise((r) => setTimeout(r, 50));
    const send = (e: unknown) => wss.clients.forEach((ws) => ws.send(JSON.stringify(e)));
    send({ event: 'gameStarting', data: { pokers: [{ level: 7, type: 0 }, { level: 9, type: 0 }] } });
    send({ event: 'landlordConfirm', data: { landlordId: 2 } });
    send({ event: 'showPokers', data: { clientId: 2, pokers: [{ level: 5, type: 0 }] } });
    await new Promise((r) => setTimeout(r, 30));
    received = [];
    send({ event: 'playRedirect', data: { sellClientId: 1, pokers: [{ level: 7, type: 0 }, { level: 9, type: 0 }] } });
    await new Promise((r) => setTimeout(r, 50));
  }

  it('跟牌被 gateway 拒绝后改为过牌,不再重复同一手牌(防卡死)', async () => {
    const agent = new Agent({ url, nickname: 'robot_1', llmConfig: mockConfig });
    await driveToFollowingPlay(agent);
    // 首次应出牌
    expect(received.some((m) => JSON.parse(m).event === 'play')).toBe(true);

    // gateway 拒绝该手牌(太小),turn 不前进,回到 robot
    received = [];
    wss.clients.forEach((ws) => ws.send(JSON.stringify({ event: 'playError', data: { code: 'less' } })));
    await new Promise((r) => setTimeout(r, 50));

    // 修复后:跟牌被拒应改为过牌(必合法),而非重发同一手牌导致无限循环
    const events = received.map((m) => JSON.parse(m).event);
    expect(events).toContain('pass');
    expect(events).not.toContain('play');
  });

  it('连续 playError 不会无限重试,最终安全过牌', async () => {
    const agent = new Agent({ url, nickname: 'robot_1', llmConfig: mockConfig });
    await driveToFollowingPlay(agent);
    received = [];
    // 连发 10 次 playError,模拟 gateway 持续拒绝
    for (let i = 0; i < 10; i++) {
      wss.clients.forEach((ws) => ws.send(JSON.stringify({ event: 'playError', data: { code: 'less' } })));
      await new Promise((r) => setTimeout(r, 10));
    }
    await new Promise((r) => setTimeout(r, 30));
    // 不应产生大量出牌尝试(无限循环),plays 应被控制在很小的数量内
    const plays = received.filter((m) => JSON.parse(m).event === 'play').length;
    expect(plays).toBeLessThanOrEqual(1);
  });

  it('出牌后从自己的 showPokers 同步手牌,下回合不重复已出的牌(防静默丢弃卡死)', async () => {
    const agent = new Agent({ url, nickname: 'robot_1', llmConfig: mockConfig });
    await agent.start();
    await new Promise((r) => setTimeout(r, 50));
    const send = (e: unknown) => wss.clients.forEach((ws) => ws.send(JSON.stringify(e)));
    // 地主,手牌 [3,5];landlordConfirm 后自动领出
    send({ event: 'gameStarting', data: { pokers: [{ level: 3, type: 0 }, { level: 5, type: 0 }] } });
    received = [];
    send({ event: 'landlordConfirm', data: { landlordId: 1 } });
    await new Promise((r) => setTimeout(r, 40));
    const firstPlay = received.map((m) => JSON.parse(m)).find((e) => e.event === 'play');
    expect(firstPlay).toBeDefined();
    const firstCard = firstPlay.data.pokers[0];

    // gateway 回传"自己出牌"的 showPokers → agent 应把该牌从手牌移除
    send({ event: 'showPokers', data: { clientId: 1, pokers: [firstCard] } });
    await new Promise((r) => setTimeout(r, 20));

    // 通过 playPass 再次轮到自己领出(playPass 不带手牌,只能靠已同步的手牌)
    received = [];
    send({ event: 'playPass', data: { nextClientId: 1 } });
    await new Promise((r) => setTimeout(r, 40));
    const secondPlay = received.map((m) => JSON.parse(m)).find((e) => e.event === 'play');
    expect(secondPlay).toBeDefined();
    // 第二次出的牌不能是已经出过的那张(否则 gateway 静默丢弃 → 卡死)
    const secondCard = secondPlay.data.pokers[0];
    expect(secondCard.level).not.toBe(firstCard.level);
  });
});
