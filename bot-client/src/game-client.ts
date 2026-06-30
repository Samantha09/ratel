import WebSocket from 'ws';
import { Card, ClientEvent, ServerEvent } from './types.js';

export interface GameClientOptions {
  url: string;
  nickname: string;
  onEvent: (event: ServerEvent) => void;
  onError?: (err: Error) => void;
  onClose?: () => void;
}

export class GameClient {
  private ws: WebSocket | null = null;
  private readonly url: string;
  private readonly nickname: string;
  private readonly onEvent: (event: ServerEvent) => void;
  private readonly onError?: (err: Error) => void;
  private readonly onClose?: () => void;
  private openPromise: Promise<void> | null = null;

  constructor(opts: GameClientOptions) {
    this.url = opts.url;
    this.nickname = opts.nickname;
    this.onEvent = opts.onEvent;
    this.onError = opts.onError;
    this.onClose = opts.onClose;
  }

  connect(): Promise<void> {
    if (this.openPromise) return this.openPromise;
    this.openPromise = new Promise((resolve, reject) => {
      this.ws = new WebSocket(this.url);
      this.ws.on('open', () => resolve());
      this.ws.on('error', (err) => {
        this.onError?.(err);
        reject(err);
      });
      this.ws.on('message', (raw) => {
        try {
          const parsed = JSON.parse(raw.toString()) as ServerEvent;
          this.onEvent(parsed);
        } catch (e) {
          this.onError?.(new Error(`Failed to parse message: ${raw}`));
        }
      });
      this.ws.on('close', () => this.onClose?.());
    });
    return this.openPromise;
  }

  send(event: ClientEvent): void {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      throw new Error('WebSocket is not open');
    }
    this.ws.send(JSON.stringify(event));
  }

  setNickname(): void {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      // Socket not open (e.g. gateway closed it after gameOver). The reconnect
      // path re-sends setNickname on a fresh socket, so skipping here is safe.
      return;
    }
    this.send({ event: 'setNickname', data: { nickname: this.nickname } });
  }

  sendLandlordElect(grab: boolean): void {
    this.send({ event: 'landlordElect', data: { grab } });
  }

  sendPlay(cards: Card[]): void {
    this.send({ event: 'play', data: { pokers: cards } });
  }

  sendPass(): void {
    this.send({ event: 'pass', data: {} });
  }

  close(): void {
    this.ws?.close();
  }
}
