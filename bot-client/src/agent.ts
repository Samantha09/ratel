import { GameClient } from './game-client.js';
import { AgentState, ServerEvent } from './types.js';
import { decideLandlord, decidePlay, LlmConfig } from './decision.js';
import { validSells, detectSell, sortHand, removeCards } from './rules.js';

export interface AgentOptions {
  url: string;
  nickname: string;
  llmConfig: LlmConfig;
  onLog?: (msg: string) => void;
}

/** 单回合内 playError 的最大重试次数,超过则安全过牌,杜绝无限循环。 */
const MAX_PLAY_RETRIES = 3;

export class Agent {
  private client: GameClient;
  private state: AgentState;
  private readonly llmConfig: LlmConfig;
  private readonly onLog?: (msg: string) => void;
  private readonly url: string;
  /** 当前回合已发生的 playError 次数;每次正常出牌回合开始时清零。 */
  private playRetries = 0;
  /** close() 后置 true,阻止 onConnectionClosed 继续重连(干净退出/不崩溃)。 */
  private stopped = false;

  constructor(opts: AgentOptions) {
    this.llmConfig = opts.llmConfig;
    this.onLog = opts.onLog;
    this.url = opts.url;
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
      onClose: () => this.onConnectionClosed(),
    });
  }

  async start(): Promise<void> {
    try {
      await this.client.connect();
    } catch (err) {
      // gateway 暂不可用(ECONNREFUSED 等):不要让 rejection 冒泡崩溃进程。
      // 连接失败时 ws 也会触发 'close' → onConnectionClosed 会重建客户端并重试。
      this.log(`connect failed: ${(err as Error).message}`);
      return;
    }
    this.client.setNickname();
  }

  close(): void {
    this.stopped = true;
    this.client.close();
  }

  private onConnectionClosed(): void {
    if (this.stopped) return; // 主动 close,不再重连
    this.log('WS closed, reconnecting in 1s...');
    this.state = {
      clientId: -1,
      nickname: this.state.nickname,
      phase: 'lobby',
      hand: [],
      seats: [],
      turnClientId: -1,
      lastPlay: null,
      consecutivePasses: 0,
      landlordId: null,
      myRole: null,
    };
    this.playRetries = 0;
    // Recreate client so a fresh socket can be opened.
    this.client = new GameClient({
      url: this.url,
      nickname: this.state.nickname,
      onEvent: (e) => this.handleEvent(e),
      onError: (err) => this.log(`WS error: ${err.message}`),
      onClose: () => this.onConnectionClosed(),
    });
    setTimeout(() => {
      void this.start();
    }, 1000);
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
        if (event.data.clientId === this.state.clientId) {
          // 自己出的牌:以 gateway 的广播为权威,从手牌移除,避免手牌与服务端不同步
          // (否则下回合可能重复出已打出的牌 → gateway checkPokerIndex 失败 → 静默丢弃 → 卡死)
          this.state.hand = removeCards(this.state.hand, event.data.pokers ?? []);
        } else {
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
        else if (this.state.phase === 'playing') this.recoverFromPlayError(event.data.code);
        break;
      case 'gameOver':
        this.state.phase = 'over';
        this.log(`game over, winner: ${event.data.winnerNickname} (${event.data.winnerType})`);
        // gateway 的 takeRobot 取走机器人后不会归还;重新 setNickname 让 gateway 再次
        // addRobot 把自己注册回池子,否则下一局凑不齐机器人。gateway 只关闭赢家连接,
        // 所以输家必须靠这里重新注册(setNickname 在 socket 关闭时会安全跳过,赢家改由
        // onConnectionClosed 重连兜底)。
        this.client.setNickname();
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
    this.playRetries = 0; // 新回合开始,清空 playError 重试计数
    try {
      const lastSell = this.state.lastPlay ? detectSell(this.state.lastPlay.cards) : null;
      const options = validSells(lastSell, this.state.hand);
      // 对手剩余牌数:供 Python play-agent 记牌(排除自己)。
      const otherCounts: Record<string, number> = {};
      for (const seat of this.state.seats) {
        if (seat.clientId === this.state.clientId) continue;
        otherCounts[seat.nickname || String(seat.clientId)] = seat.cardsLeft;
      }
      const decision = await decidePlay(
        {
          nickname: this.state.nickname,
          role: this.state.myRole,
          hand: this.state.hand,
          lastPlay: this.state.lastPlay,
          options,
          otherCounts,
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

  /**
   * gateway 拒绝出牌时的恢复:gateway 是合法性的唯一权威,且拒绝后不会推进回合。
   * 因此**绝不**重跑同一决策(否则与 gateway 形成无限循环,机器人卡死十几分钟),
   * 而是直接采取一个保证合法的动作让回合前进:
   *  - cantPass:不能过(我是首家)→ 出最小单张(领出单张必合法)
   *  - 其它(太小/不匹配/无效/乱序)→ 跟牌时过牌(必合法);领出时出最小单张
   *  - 超过重试上限 → 兜底过牌,彻底终止循环
   */
  private recoverFromPlayError(code: string): void {
    // order = 非我回合(与 gateway 回合不同步):绝不再发任何东西,否则形成 order 风暴。
    // 静待 gateway 的下一个正确回合信号(playRedirect/playPass)。
    if (code === 'order') {
      this.log('playError order(非我回合),不动作,等待正确回合信号');
      return;
    }
    this.playRetries += 1;
    if (this.playRetries > MAX_PLAY_RETRIES) {
      this.log(`playError 超过 ${MAX_PLAY_RETRIES} 次,放弃本回合并过牌`);
      this.client.sendPass();
      return;
    }
    if (code === 'cantPass') {
      this.state.lastPlay = null; // 我是首家
      this.playSmallestSingle();
      return;
    }
    if (this.state.lastPlay) {
      // 跟牌被拒 → 过牌(跟牌时过牌总是合法,立即让回合前进)
      this.log('出牌被拒,改为过牌');
      this.client.sendPass();
    } else {
      // 领出被拒 → 出最小单张(领出单张总是合法)
      this.log('领出被拒,改出最小单张');
      this.playSmallestSingle();
    }
  }

  /** 出手中最小的一张单牌——领出场景下保证合法,用于打破 playError 循环。 */
  private playSmallestSingle(): void {
    const sorted = sortHand(this.state.hand);
    if (sorted.length === 0) {
      this.client.sendPass();
      return;
    }
    this.client.sendPlay([sorted[0]]);
  }
}
