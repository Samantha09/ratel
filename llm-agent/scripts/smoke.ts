import { spawn } from 'child_process';
import WebSocket from 'ws';
import { setTimeout as sleep } from 'timers/promises';

const GW_PATH = '/home/san/VsCodeProject/ratel/ratel/gateway';
const AGENT_DIR = '/home/san/VsCodeProject/ratel/ratel/llm-agent';

async function main(): Promise<void> {
  const gw = spawn(GW_PATH, ['127.0.0.1', '8787'], { stdio: 'inherit' });
  await sleep(500);

  const agents = spawn('npm', ['run', 'dev'], {
    cwd: AGENT_DIR,
    stdio: 'inherit',
    env: { ...process.env, LLM_PROVIDER: 'mock' },
  });
  await sleep(1000);

  const ws = new WebSocket('ws://127.0.0.1:8787');
  let done = false;
  let lastEventAt = Date.now();

  ws.on('open', () => {
    console.log('[smoke] human connected');
  });

  ws.on('message', (raw) => {
    lastEventAt = Date.now();
    const msg = JSON.parse(raw.toString());
    console.log(`[smoke] <- ${msg.event}`);
    if (msg.event === 'idSet') {
      ws.send(JSON.stringify({ event: 'setNickname', data: { nickname: 'human' } }));
      setTimeout(() => ws.send(JSON.stringify({ event: 'createRoomPve', data: {} })), 200);
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
