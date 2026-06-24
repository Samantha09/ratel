import { Card, ServerEvent, Emitter } from './types.js';
import { buildDeck, deal } from './deck.js';
import { Sell, detectSell } from './cardtype.js';
import { canBeat } from './compare.js';
import { botPlay, BotAction } from './bot.js';

interface Seat {
  id: number;
  nickname: string;
  isBot: boolean;
  hand: Card[];
}

interface LastPlay {
  sell: Sell;
  sellClientId: number;
  nickname: string;
}

type Phase = 'bidding' | 'playing' | 'over';

export interface GameOpts {
  humanNickname: string;
  emit: Emitter;
  rng?: () => number;
  autoHuman?: boolean;
}

const REDEAL_CAP = 3;

export class Game {
  private emit: Emitter;
  private rng: () => number;
  private autoHuman: boolean;
  private humanNickname: string;
  private seats: Seat[] = [];
  private bottom: Card[] = [];
  private landlord = -1;
  private turn = 0;
  private lastPlay: LastPlay | null = null;
  private consecutivePasses = 0;
  private phase: Phase = 'bidding';
  private biddingOrder: number[] = [0, 1, 2];
  private biddingIndex = 0;
  private redealCount = 0;
  private readonly humanSeat = 0;
  private roomId: number;

  constructor(opts: GameOpts) {
    this.emit = opts.emit;
    this.rng = opts.rng ?? Math.random;
    this.autoHuman = opts.autoHuman ?? false;
    this.humanNickname = opts.humanNickname;
    this.roomId = 1000 + Math.floor(this.rng() * 9000);
  }

  setNickname(name: string): void {
    this.humanNickname = name;
    if (this.seats[0]) this.seats[0] = { ...this.seats[0], nickname: name };
  }

  start(): void {
    this.seats = [
      { id: 0, nickname: this.humanNickname, isBot: false, hand: [] },
      { id: 1, nickname: 'Bot Alpha', isBot: true, hand: [] },
      { id: 2, nickname: 'Bot Beta', isBot: true, hand: [] },
    ];
    this.dealAndBeginBidding();
    this.emit({
      event: 'roomCreate',
      data: { roomId: this.roomId, owner: this.seats[0].nickname, clientCount: 3, type: 1 },
    });
    this.emit({ event: 'gameStarting', data: {} });
    this.advanceBidding();
  }

  private dealAndBeginBidding(): void {
    const { hands, bottom } = deal(buildDeck(), this.rng);
    this.seats.forEach((s, i) => (s.hand = hands[i]));
    this.bottom = bottom;
    this.phase = 'bidding';
    this.biddingOrder = [0, 1, 2]; // fixed order: human (seat 0) bids first — deterministic
    this.biddingIndex = 0;
    this.lastPlay = null;
    this.consecutivePasses = 0;
    this.landlord = -1;
  }

  private advanceBidding(): void {
    if (this.phase === 'over') return;
    while (this.biddingIndex < this.biddingOrder.length) {
      const seatId = this.biddingOrder[this.biddingIndex];
      const seat = this.seats[seatId];
      if (!seat.isBot && !this.autoHuman) {
        this.emit({ event: 'landlordElect', data: { client: seat.id, nickname: seat.nickname } });
        return; // wait for onHumanLandlordElect
      }
      // bot or autoHuman: decide instantly
      if (this.botWantsLandlord(seat)) {
        this.confirmLandlord(seatId);
        return;
      }
      this.biddingIndex++;
    }
    // nobody grabbed this round
    if (this.redealCount >= REDEAL_CAP) {
      this.confirmLandlord(this.biddingOrder[0]); // force someone to keep the game moving
      return;
    }
    this.redealCount++;
    this.dealAndBeginBidding();
    this.advanceBidding();
  }

  private botWantsLandlord(seat: Seat): boolean {
    const hasBomb = seat.hand.some((c) => seat.hand.filter((x) => x.level === c.level).length === 4);
    const hasJoker = seat.hand.some((c) => c.level === 13 || c.level === 14);
    return hasBomb || hasJoker;
  }

  onHumanLandlordElect(grab: boolean): void {
    if (this.phase !== 'bidding') return;
    if (this.biddingOrder[this.biddingIndex] !== this.humanSeat) return;
    const seatId = this.biddingOrder[this.biddingIndex];
    if (grab) {
      this.confirmLandlord(seatId);
    } else {
      this.biddingIndex++;
      this.advanceBidding();
    }
  }

  private confirmLandlord(seatId: number): void {
    this.landlord = seatId;
    this.seats[seatId].hand.push(...this.bottom);
    this.seats[seatId].hand.sort((a, b) => a.level - b.level || a.type - b.type);
    this.emit({ event: 'landlordConfirm', data: { landlord: seatId, nickname: this.seats[seatId].nickname } });
    this.emit({ event: 'showPokers', data: { pokers: this.seats[this.humanSeat].hand } });
    this.phase = 'playing';
    this.turn = seatId;
    this.lastPlay = null;
    this.consecutivePasses = 0;
    this.advancePlay();
  }

  // Reads phase as the full union so defensive guards after side-effecting calls
  // (recordPlay -> finishGame may set phase to 'over') are not flagged as dead code.
  private phaseNow(): Phase {
    return this.phase;
  }

  private advancePlay(): void {
    if (this.phaseNow() === 'over') return;
    const seat = this.seats[this.turn];
    const needsHumanWait = !seat.isBot && !this.autoHuman;
    if (needsHumanWait) {
      this.emit({ event: 'playTurn', data: {} });
      return;
    }
    const wasPass = this.resolveSeat(seat);
    if (this.phaseNow() === 'over') return;
    this.turn = this.afterAction(seat.id, wasPass);
    this.advancePlay();
  }

  private resolveSeat(seat: Seat): boolean {
    const action: BotAction = botPlay(seat.hand, this.lastPlay?.sell ?? null);
    if (action.kind === 'pass') {
      this.recordPass(seat);
      return true;
    }
    this.recordPlay(seat, action.cards);
    return false;
  }

  onHumanPlay(cards: Card[]): void {
    // Note: this mock only emits 'order'/'invalid'/'less' here (and 'cantPass' in
    // onHumanPass). The 'mismatch' playError reason is reserved for the real C++ gateway.
    if (this.phase !== 'playing' || this.turn !== this.humanSeat) {
      this.emit({ event: 'playError', data: { reason: 'order' } });
      return;
    }
    const seat = this.seats[this.humanSeat];
    const sell = detectSell(cards);
    if (!sell || !this.cardsInHand(seat.hand, cards)) {
      this.emit({ event: 'playError', data: { reason: 'invalid' } });
      return;
    }
    if (this.lastPlay && !canBeat(sell, this.lastPlay.sell)) {
      this.emit({ event: 'playError', data: { reason: 'less' } });
      return;
    }
    this.recordPlay(seat, cards);
    if (this.phaseNow() === 'over') return;
    this.turn = this.afterAction(seat.id, false);
    this.advancePlay();
  }

  onHumanPass(): void {
    if (this.phase !== 'playing' || this.turn !== this.humanSeat) {
      this.emit({ event: 'playError', data: { reason: 'order' } });
      return;
    }
    if (!this.lastPlay) {
      this.emit({ event: 'playError', data: { reason: 'cantPass' } });
      return;
    }
    this.recordPass(this.seats[this.humanSeat]);
    if (this.phaseNow() === 'over') return;
    // human seat is always 0; per brief, inline as this.afterAction(0, true)
    this.turn = this.afterAction(0, true);
    this.advancePlay();
  }

  /** Next-turn decision; clears lastPlay when both followers pass. Does not mutate turn itself. */
  private afterAction(seatId: number, wasPass: boolean): number {
    if (wasPass && this.consecutivePasses >= 2 && this.lastPlay) {
      const leader = this.lastPlay.sellClientId;
      this.lastPlay = null;
      this.consecutivePasses = 0;
      return leader;
    }
    return (seatId + 1) % 3;
  }

  private recordPlay(seat: Seat, cards: Card[]): void {
    const sell = detectSell(cards)!;
    for (const card of cards) {
      const idx = seat.hand.findIndex((h) => h.level === card.level && h.type === card.type);
      if (idx >= 0) seat.hand.splice(idx, 1);
    }
    this.lastPlay = { sell, sellClientId: seat.id, nickname: seat.nickname };
    this.consecutivePasses = 0;
    this.emit({
      event: 'playRedirect',
      data: {
        sellClient: seat.id,
        sellNickname: seat.nickname,
        sellPokers: cards,
        sellType: sell.type,
        nextClient: (seat.id + 1) % 3,
      },
    });
    if (seat.hand.length === 0) this.finishGame(seat.id);
  }

  private recordPass(seat: Seat): void {
    this.consecutivePasses++;
    this.emit({ event: 'playPass', data: { client: seat.id, nextClient: (seat.id + 1) % 3 } });
  }

  private finishGame(winnerId: number): void {
    this.phase = 'over';
    this.emit({
      event: 'gameOver',
      data: { winner: winnerId, landlord: this.landlord, nickname: this.seats[winnerId].nickname },
    });
  }

  private cardsInHand(hand: Card[], cards: Card[]): boolean {
    const pool = hand.map((c) => `${c.level}:${c.type}`);
    for (const card of cards) {
      const idx = pool.indexOf(`${card.level}:${card.type}`);
      if (idx < 0) return false;
      pool.splice(idx, 1);
    }
    return true;
  }

  // ---- idle-resolution hints for the server demo (human as bot) ----
  suggestBid(): boolean {
    return this.botWantsLandlord(this.seats[this.humanSeat]);
  }

  suggestPlay(): BotAction {
    return botPlay(this.seats[this.humanSeat].hand, this.lastPlay?.sell ?? null);
  }
}
