import { spawn } from 'child_process';
import WebSocket from 'ws';
import { setTimeout as sleep } from 'timers/promises';
import { validSells, detectSell, sortHand } from '../src/rules.js';
import { Card } from '../src/types.js';

const GW_PATH = '/home/san/VsCodeProject/ratel/ratel/gateway';
const AGENT_DIR = '/home/san/VsCodeProject/ratel/ratel/llm-agent';

async function main(): Promise<void> {
  const gw = spawn(GW_PATH, ['127.0.0.1', '8787'], { stdio: ['ignore', 'pipe', 'pipe'] });
  gw.stdout?.on('data', (d) => {});
  gw.stderr?.on('data', (d) => {});
  await sleep(500);

  const agents = spawn('npm', ['run', 'dev'], {
    cwd: AGENT_DIR,
    stdio: ['ignore', 'pipe', 'pipe'],
    env: { ...process.env, LLM_PROVIDER: 'mock' },
  });
  agents.stdout?.on('data', (d) => {});
  agents.stderr?.on('data', (d) => {});
  await sleep(1000);

  const ws = new WebSocket('ws://127.0.0.1:8787');
  let done = false;
  let lastEventAt = Date.now();

  ws.on('open', () => {
    console.log('[smoke] human connected');
  });

  let myClientId = -1;
  let myHand: Card[] = [];
  let lastPlay: Card[] | null = null;
  let consecutivePasses = 0;

  function sendPlay(cards: Card[]): void {
    ws.send(JSON.stringify({ event: 'play', data: { pokers: cards } }));
  }

  function sendPass(): void {
    ws.send(JSON.stringify({ event: 'pass', data: {} }));
  }

  function act(): void {
    const lastSell = lastPlay ? detectSell(lastPlay) : null;
    const options = validSells(lastSell, myHand);
    if (options.length > 0) {
      const choice = options[0];
      myHand = sortHand(myHand.filter((h) => !choice.cards.some((c) => c.level === h.level && c.type === h.type)));
      sendPlay(choice.cards);
    } else {
      sendPass();
    }
  }

  ws.on('message', (raw) => {
    lastEventAt = Date.now();
    const msg = JSON.parse(raw.toString());
    console.log(`[smoke] <- ${msg.event}`);

    if (msg.event === 'idSet') {
      myClientId = msg.data.clientId;
      ws.send(JSON.stringify({ event: 'setNickname', data: { nickname: 'human' } }));
      setTimeout(() => ws.send(JSON.stringify({ event: 'createRoomPve', data: {} })), 200);
    }

    if (msg.event === 'gameStarting') {
      myHand = sortHand(msg.data.pokers ?? []);
    }

    if (msg.event === 'landlordConfirm') {
      if (msg.data.landlordId === myClientId) {
        myHand = sortHand([...myHand, ...(msg.data.additionalPokers ?? [])]);
      }
      // If human is landlord, act immediately (gateway does not send playRedirect for landlord)
      if (msg.data.landlordId === myClientId) {
        lastPlay = null;
        act();
      }
    }

    if (msg.event === 'showPokers') {
      if (msg.data.clientId !== myClientId) {
        lastPlay = msg.data.pokers;
      }
      consecutivePasses = 0;
    }

    if (msg.event === 'playPass') {
      consecutivePasses++;
      if (consecutivePasses >= 2) {
        lastPlay = null;
        consecutivePasses = 0;
      }
      if (msg.data.nextClientId === myClientId) {
        act();
      }
    }

    if (msg.event === 'playRedirect' && msg.data.sellClientId === myClientId) {
      myHand = sortHand(msg.data.pokers ?? []);
      if (msg.data.lastSellPokers?.length) {
        lastPlay = msg.data.lastSellPokers;
      }
      act();
    }

    if (msg.event === 'playError') {
      // Retry with next valid option or pass
      if (msg.data.code === 'cantPass') {
        act();
      } else {
        sendPass();
      }
    }

    if (msg.event === 'gameOver') done = true;
  });

  const stall = setInterval(() => {
    if (done) {
      clearInterval(stall);
      ws.close();
      agents.kill();
      gw.kill();
      process.exit(0);
    }
    if (Date.now() - lastEventAt > 8000) {
      console.error('[smoke] STALLED');
      clearInterval(stall);
      ws.close();
      agents.kill();
      gw.kill();
      process.exit(1);
    }
  }, 1000);

  await sleep(30000);
  console.error('[smoke] TIMEOUT');
  ws.close();
  agents.kill();
  gw.kill();
  process.exit(1);
}

main();
