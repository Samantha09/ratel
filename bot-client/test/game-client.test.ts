import { describe, it, expect, beforeEach, afterEach } from 'vitest';
import { WebSocketServer } from 'ws';
import { GameClient } from '../src/game-client.js';
import { ServerEvent } from '../src/types.js';

describe('GameClient', () => {
  let wss: WebSocketServer;
  let url: string;

  beforeEach(async () => {
    wss = new WebSocketServer({ port: 0 });
    await new Promise<void>((resolve) => wss.once('listening', resolve));
    const addr = wss.address();
    if (addr === null || typeof addr === 'string') throw new Error('invalid server address');
    url = `ws://127.0.0.1:${addr.port}`;
  });

  afterEach(() => {
    wss.close();
  });

  it('connects and receives idSet', async () => {
    const events: ServerEvent[] = [];
    const client = new GameClient({
      url,
      nickname: 'robot_1',
      onEvent: (e) => events.push(e),
    });

    await client.connect();
    wss.clients.forEach((ws) => ws.send(JSON.stringify({ event: 'idSet', data: { clientId: 42 } })));

    await new Promise((r) => setTimeout(r, 50));
    expect(events).toHaveLength(1);
    expect(events[0]).toEqual({ event: 'idSet', data: { clientId: 42 } });
  });

  it('sends nickname on setNickname', async () => {
    let received: string | null = null;
    wss.on('connection', (ws) => {
      ws.on('message', (raw) => {
        received = raw.toString();
      });
    });

    const client = new GameClient({ url, nickname: 'robot_1', onEvent: () => {} });
    await client.connect();
    client.setNickname();

    await new Promise((r) => setTimeout(r, 50));
    expect(received).toEqual(JSON.stringify({ event: 'setNickname', data: { nickname: 'robot_1' } }));
  });
});
