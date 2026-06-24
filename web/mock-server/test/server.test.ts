import { describe, it, expect, afterEach, beforeAll } from 'vitest';
import { WebSocket } from 'ws';
import { startServer } from '../src/server.js';
import type { WebSocketServer } from 'ws';

// Keep idle auto-resolution from racing the test's active input.
beforeAll(() => {
  process.env.IDLE_MS = '100000';
});

let server: WebSocketServer;
afterEach(() => server?.close());

/** Wait for the ephemeral-port server to be listening, then return its port. */
function listenPort(srv: WebSocketServer): Promise<number> {
  return new Promise((resolve, reject) => {
    const addr = srv.address();
    if (addr && typeof addr === 'object') return resolve(addr.port);
    srv.once('listening', () => resolve((srv.address() as any).port));
    srv.once('error', reject);
  });
}

// Per-socket buffer of received raw messages. Attached before 'open' resolves
// so the handshake messages (sent synchronously on 'connection') are never lost
// to a listener-registration race.
const buffers = new WeakMap<WebSocket, Buffer[]>();

function connect(port: number): Promise<WebSocket> {
  return new Promise((resolve) => {
    const ws = new WebSocket(`ws://127.0.0.1:${port}`);
    buffers.set(ws, []);
    ws.on('message', (raw: Buffer) => buffers.get(ws)!.push(raw));
    ws.on('open', () => resolve(ws));
  });
}

function recv(ws: WebSocket, predicate: (e: any) => boolean, timeoutMs = 2000): Promise<any> {
  const buf = buffers.get(ws)!;
  // Drain anything already buffered first.
  for (let i = 0; i < buf.length; i++) {
    const e = JSON.parse(buf[i].toString());
    if (predicate(e)) { buf.splice(i, 1); return Promise.resolve(e); }
  }
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error('timeout waiting for event')), timeoutMs);
    ws.on('message', (raw: Buffer) => {
      const e = JSON.parse(raw.toString());
      if (predicate(e)) {
        clearTimeout(timer);
        resolve(e);
      }
    });
  });
}

describe('server', () => {
  it('answers connect with connected + idSet(0)', async () => {
    server = startServer({ port: 0 });
    const port = await listenPort(server);
    const ws = await connect(port);
    const id = await recv(ws, (e) => e.event === 'idSet');
    expect(id.data.clientId).toBe(0);
    ws.close();
  });

  it('createRoomPve then landlordElect(grab) yields landlordConfirm + showPokers', async () => {
    server = startServer({ port: 0 });
    const port = await listenPort(server);
    const ws = await connect(port);

    ws.send(JSON.stringify({ event: 'setNickname', data: { nickname: 'san' } }));
    ws.send(JSON.stringify({ event: 'createRoomPve', data: {} }));

    // default bidding prompts human (client 0) first
    await recv(ws, (e) => e.event === 'landlordElect' && e.data.client === 0);
    ws.send(JSON.stringify({ event: 'landlordElect', data: { grab: true } }));

    const confirm = await recv(ws, (e) => e.event === 'landlordConfirm');
    expect(confirm.data.landlord).toBe(0);
    const pok = await recv(ws, (e) => e.event === 'showPokers');
    expect(pok.data.pokers.length).toBe(20);
    ws.close();
  });
});
