import { WebSocketServer, WebSocket } from 'ws';
import { Game } from './game.js';
import { ClientEvent, ServerEvent } from './types.js';

export interface ServerOpts {
  port?: number;
  host?: string;
  idleMs?: number;
}

export function startServer(opts: ServerOpts = {}): WebSocketServer {
  const idleMs = opts.idleMs ?? Number(process.env.IDLE_MS ?? 24 * 60 * 60 * 1000);
  const wss = new WebSocketServer({ port: opts.port ?? 0, host: opts.host ?? '127.0.0.1' });

  wss.on('connection', (ws: WebSocket) => {
    let game: Game | null = null;
    let nickname = 'san';
    let idleTimer: NodeJS.Timeout | null = null;

    const send = (e: ServerEvent) => {
      if (ws.readyState === WebSocket.OPEN) ws.send(JSON.stringify(e));
    };

    const clearIdle = () => {
      if (idleTimer) { clearTimeout(idleTimer); idleTimer = null; }
    };

    const armIdle = (onFire: () => void) => {
      clearIdle();
      idleTimer = setTimeout(() => {
        idleTimer = null;
        if (game) onFire();
      }, idleMs);
    };

    const emit = (e: ServerEvent) => {
      send(e);
      if (e.event === 'landlordElect') {
        armIdle(() => game!.onHumanLandlordElect(game!.suggestBid()));
      } else if (e.event === 'playTurn') {
        armIdle(() => {
          const a = game!.suggestPlay();
          if (a.kind === 'play') game!.onHumanPlay(a.cards);
          else game!.onHumanPass();
        });
      } else if (e.event === 'gameOver') {
        clearIdle();
      }
    };

    // connect handshake
    send({ event: 'connected', data: {} });
    send({ event: 'idSet', data: { clientId: 0 } });

    const startGame = () => {
      game = new Game({ humanNickname: nickname, emit });
      game.start();
    };

    ws.on('message', (raw) => {
      let msg: ClientEvent;
      try {
        msg = JSON.parse(raw.toString());
      } catch {
        return;
      }
      if (!game) {
        if (msg.event === 'setNickname') nickname = msg.data.nickname;
        if (msg.event === 'createRoomPve' || msg.event === 'joinRoom') startGame();
        return;
      }
      // Any inbound human message clears the pending idle timer.
      clearIdle();
      switch (msg.event) {
        case 'setNickname':
          nickname = msg.data.nickname;
          game.setNickname(nickname);
          break;
        case 'createRoomPve':
        case 'joinRoom':
          // game already started; accept as no-op
          break;
        case 'landlordElect':
          game.onHumanLandlordElect(msg.data.grab);
          break;
        case 'play':
          game.onHumanPlay(msg.data.pokers);
          break;
        case 'pass':
          game.onHumanPass();
          break;
        case 'exit':
          ws.close();
          break;
      }
    });

    ws.on('close', () => clearIdle());
  });

  return wss;
}
