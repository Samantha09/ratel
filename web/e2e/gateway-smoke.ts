import WebSocket from 'ws';

const URL = process.env.GATEWAY_URL ?? 'ws://127.0.0.1:8787';

type Msg = { event: string; data: Record<string, unknown> };

const seen: string[] = [];
let ws: WebSocket;
let resolveDone: () => void;
const done = new Promise<void>((r) => (resolveDone = r));

function send(event: string, data: Record<string, unknown> = {}) {
  const msg = JSON.stringify({ event, data });
  console.log('->', event, JSON.stringify(data).slice(0, 80));
  ws.send(msg);
}

function start() {
  ws = new WebSocket(URL);
  ws.on('message', (raw) => {
    let msg: Msg;
    try { msg = JSON.parse(raw.toString()); } catch { return; }
    seen.push(msg.event);
    console.log('<-', msg.event, JSON.stringify(msg.data).slice(0, 120));
    if (msg.event === 'unknown') {
      console.log('FULL UNKNOWN MESSAGE:', raw.toString());
    }

    if (msg.event === 'idSet') {
      // PRE-FLIGHT FIX: send setNickname and createRoomPve consecutively
      // Do NOT wait for showOptions/nicknameSet event (it won't arrive due to JsonMapHelper mapping)
      console.log('Sending setNickname and createRoomPve');
      send('setNickname', { nickname: 'san' });
      send('createRoomPve', { choose: 1 });  // difficulty: 1 = easy
      return;
    }
    if (msg.event === 'landlordElect') { send('landlordElect', { grab: true }); return; }
    if (msg.event === 'playRedirect' || msg.event === 'playTurn') {
      // Check if it's our turn (we're clientId 1)
      const nextClientId = msg.data.nextClientId as number;
      if (nextClientId !== 1) {
        console.log('Not our turn, waiting for', nextClientId);
        return;  // Not our turn
      }
      // Play the lowest single card we hold, else pass.
      const hand = (msg.data.pokers as { level: number; type: number }[]) ?? [];
      if (hand.length) send('play', { pokers: [{ level: hand[0].level, type: hand[0].type }] });
      else send('pass');
      return;
    }
    if (msg.event === 'gameOver') { resolveDone(); }
  });
  ws.on('error', (e) => { console.error('ws error', e); process.exit(1); });
}

start();
const timer = setTimeout(() => { console.error('TIMEOUT'); process.exit(1); }, 10000);  // 10s timeout
(async () => {
  await done;
  clearTimeout(timer);
  ws.close();
  const want = ['idSet', 'gameStarting', 'landlordConfirm', 'gameOver'];
  const ok = want.every((w) => seen.includes(w));
  console.log('\nseen:', seen.join(', '));
  console.log(ok ? 'SMOKE PASS' : 'SMOKE FAIL');
  process.exit(ok ? 0 : 1);
})();
