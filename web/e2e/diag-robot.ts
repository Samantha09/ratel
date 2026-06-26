import { spawn } from 'child_process';
import WebSocket from 'ws';

// 在单个 Node 进程内 spawn gateway（子进程，非 shell &）+ 驱动对局 + 打印两层证据
const GW_PATH = '/home/san/VsCodeProject/ratel/ratel/gateway';
// stdio inherit：gateway 日志直接到终端（无缓冲），便于看 onHandshake 是否触发
const gw = spawn(GW_PATH, ['127.0.0.1', '8787'], { stdio: ['ignore', 'inherit', 'inherit'] });
gw.on('exit', (code, sig) => console.log(`[GW exit] code=${code} sig=${sig}`));

const sleep = (ms: number) => new Promise((r) => setTimeout(r, ms));
function connect(): Promise<WebSocket> {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket('ws://127.0.0.1:8787');
    ws.on('open', () => resolve(ws));
    ws.on('error', reject);
  });
}

async function main() {
  let ws: WebSocket | null = null;
  for (let i = 0; i < 60 && !ws; i++) {
    try {
      ws = await connect();
    } catch {
      await sleep(200);
    }
  }
  if (!ws) {
    console.error('!无法连接 gateway');
    gw.kill();
    process.exit(1);
  }
  console.log('=== 已连接，开始驱动 ===');

  let lastEventAt = Date.now();
  let done = false;
  ws.on('message', (raw) => {
    lastEventAt = Date.now();
    let msg: { event: string; data: any };
    try {
      msg = JSON.parse(raw.toString());
    } catch {
      return;
    }
    console.log(`<- ${msg.event} ${JSON.stringify(msg.data).slice(0, 110)}`);
    if (msg.event === 'idSet') {
      ws!.send(JSON.stringify({ event: 'setNickname', data: { nickname: 'san' } }));
      ws!.send(JSON.stringify({ event: 'createRoomPve', data: {} }));
      return;
    }
    if (msg.event === 'landlordElect') {
      ws!.send(JSON.stringify({ event: 'landlordElect', data: { grab: true } }));
      return;
    }
    if (msg.event === 'playRedirect' || msg.event === 'playTurn') {
      const hand: { level: number; type: number }[] = msg.data?.pokers ?? [];
      if (hand.length)
        ws!.send(JSON.stringify({ event: 'play', data: { pokers: [{ level: hand[0].level, type: hand[0].type }] } }));
      else ws!.send(JSON.stringify({ event: 'pass', data: {} }));
      return;
    }
    if (msg.event === 'gameOver') done = true;
  });

  // 卡住检测：5 秒无事件 → 判定 robot stall
  const stall = setInterval(() => {
    if (done) {
      clearInterval(stall);
      return;
    }
    if (Date.now() - lastEventAt > 5000) {
      console.log('!!! STALLED: 5 秒无事件（疑似 robot 不出牌）');
      clearInterval(stall);
      ws!.close();
      gw.kill();
      process.exit(2);
    }
  }, 1000);

  await sleep(25000);
  console.log('=== 整体超时 ===');
  ws.close();
  gw.kill();
  process.exit(done ? 0 : 3);
}

main();
