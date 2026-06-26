import { GameClient } from './game-client.js';
import { AgentState, ServerEvent, Card } from './types.js';
import { decideLandlord, decidePlay, LlmConfig } from './decision.js';
import { validSells, detectSell, sortHand } from './rules.js';

export interface AgentOptions {
  url: string;
  nickname: string;
  llmConfig: LlmConfig;
  onLog?: (msg: string) => void;
}

export class Agent {
  private client: GameClient;
  private state: AgentState;
  private readonly llmConfig: LlmConfig;
  private readonly onLog?: (msg: string) => void;

  constructor(opts: AgentOptions) {
    this.llmConfig = opts.llmConfig;
    this.onLog = opts.onLog;
    this.state = {
      clientId: -1,
      nickname: opts.nickname,
      phase: 'lobby',
      hand: [],
      seats: [],
      turnClientId: -1,
      lastPlay: null,
      consecutivePasses: 0,
      landlordId: null,
      myRole: null,
    };
    this.client = new GameClient({
      url: opts.url,
      nickname: opts.nickname,
      onEvent: (e) => this.handleEvent(e),
      onError: (err) => this.log(`WS error: ${err.message}`),
      onClose: () => this.log('WS closed'),
    });
  }

  async start(): Promise<void> {
    await this.client.connect();
    this.client.setNickname();
  }

  close(): void {
    this.client.close();
  }

  private log(msg: string): void {
    this.onLog?.(`[${this.state.nickname}] ${msg}`);
  }

  private handleEvent(event: ServerEvent): void {
    switch (event.event) {
      case 'idSet':
        this.state.clientId = event.data.clientId;
        break;
      case 'gameStarting':
        this.state.phase = 'bidding';
        if (event.data.pokers) {
          this.state.hand = sortHand(event.data.pokers);
        }
        if (event.data.nextClientId !== undefined) {
          this.state.turnClientId = event.data.nextClientId;
        }
        if (this.state.turnClientId === this.state.clientId) {
          void this.actBidding();
        }
        break;
      case 'landlordElect':
        this.state.turnClientId = event.data.nextClientId;
        if (event.data.nextClientId === this.state.clientId) {
          void this.actBidding();
        }
        break;
      case 'landlordConfirm':
        this.state.landlordId = event.data.landlordId;
        this.state.myRole = event.data.landlordId === this.state.clientId ? 'LANDLORD' : 'PEASANT';
        if (event.data.additionalPokers && this.state.myRole === 'LANDLORD') {
          this.state.hand = sortHand([...this.state.hand, ...event.data.additionalPokers]);
        }
        this.state.phase = 'playing';
        // 如果自己是地主，gateway 不会单独发 playRedirect，需主动出牌
        if (this.state.landlordId === this.state.clientId) {
          void this.actPlay();
        }
        break;
      case 'showPokers':
        if (event.data.clientId !== this.state.clientId) {
          this.state.lastPlay = {
            clientId: event.data.clientId,
            nickname: event.data.clientNickname ?? String(event.data.clientId),
            cards: event.data.pokers,
          };
          this.state.consecutivePasses = 0;
        }
        break;
      case 'playRedirect':
        this.state.turnClientId = event.data.sellClientId;
        if (event.data.pokers) this.state.hand = sortHand(event.data.pokers);
        if (event.data.clientInfos) this.state.seats = event.data.clientInfos;
        if (event.data.sellClientId === this.state.clientId) {
          void this.actPlay();
        }
        break;
      case 'playPass':
        this.state.turnClientId = event.data.nextClientId;
        this.state.consecutivePasses++;
        if (this.state.consecutivePasses >= 2) {
          this.state.lastPlay = null;
          this.state.consecutivePasses = 0;
        }
        if (event.data.nextClientId === this.state.clientId) {
          void this.actPlay();
        }
        break;
      case 'playError':
        this.log(`playError: ${event.data.code}`);
        if (this.state.phase === 'bidding') void this.actBidding();
        else if (this.state.phase === 'playing') void this.actPlay();
        break;
      case 'gameOver':
        this.state.phase = 'over';
        this.log(`game over, winner: ${event.data.winnerNickname} (${event.data.winnerType})`);
        break;
    }
  }

  private async actBidding(): Promise<void> {
    try {
      const decision = await decideLandlord({ nickname: this.state.nickname, hand: this.state.hand }, this.llmConfig);
      this.log(`bidding decision: ${decision.action}, ${decision.reason}`);
      this.client.sendLandlordElect(decision.action === 'grab');
    } catch (err) {
      this.log(`bidding error: ${(err as Error).message}`);
      this.client.sendLandlordElect(false);
    }
  }

  private async actPlay(): Promise<void> {
    try {
      const lastSell = this.state.lastPlay ? detectSell(this.state.lastPlay.cards) : null;
      const options = validSells(lastSell, this.state.hand);
      const decision = await decidePlay(
        {
          nickname: this.state.nickname,
          role: this.state.myRole,
          hand: this.state.hand,
          lastPlay: this.state.lastPlay,
          options,
        },
        this.llmConfig
      );
      this.log(`play decision: ${decision.action}, ${decision.reason}`);
      if (decision.action === 'play' && decision.cards) {
        this.client.sendPlay(decision.cards);
      } else {
        this.client.sendPass();
      }
    } catch (err) {
      this.log(`play error: ${(err as Error).message}`);
      this.client.sendPass();
    }
  }
}
