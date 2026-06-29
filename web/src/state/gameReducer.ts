import { Card, ClientInfoMsg, PlayErrorReason, ServerEvent } from '../types';
import { evaluate, isBomb, isKingBomb } from '../game/pokerEngine';

export interface SeatInfo {
  id: number;
  nickname: string;
  cardsLeft: number;
  isLandlord: boolean;
}

export interface LastSell {
  client: number;
  nickname: string;
  pokers: Card[];
  type: number;
}

/** A seat's most recent action rendered in the 3-column play zone. */
export interface SeatPlay {
  pokers: Card[];
  passed: boolean;
}

export type LobbyScreen = 'nickname' | 'menu' | 'rooms' | 'waiting';

export interface GameState {
  phase: 'connecting' | 'lobby' | 'bidding' | 'playing' | 'over';
  connected: boolean;
  clientId: number | null;
  nickname: string;
  roomId: number | null;
  hand: Card[];
  selected: number[];
  seats: SeatInfo[];
  turnClientId: number | null;
  lastSell: LastSell | null;
  landlord: number | null;
  myType: 'LANDLORD' | 'PEASANT' | null;
  result: { winnerNickname: string; winnerType: string } | null;
  error: PlayErrorReason | null;
  lobbyError: string | null;
  promptBid: boolean;

  // --- prototype-parity additions ---
  bottomCards: Card[]; // 3 底牌(broadcast to everyone in landlordConfirm)
  multiplier: number; // 倍数,本局 1 起,炸弹/王炸 ×2(客户端计算)
  baseScore: number; // 底分(常量 3;gateway 合同无此字段)
  round: number; // 局数,首局 1,"再来一局" +1
  playsBySeat: Record<number, SeatPlay>; // 每座最近一手,驱动三列出牌区
  lobbyScreen: LobbyScreen; // 大厅子屏
}

export type Action =
  | { type: 'server'; event: ServerEvent }
  | { type: 'select'; index: number }
  | { type: 'setSelection'; indices: number[] }
  | { type: 'clearError' }
  | { type: 'clearLobbyError' }
  | { type: 'gotoLobby'; screen: LobbyScreen }
  | { type: 'bumpRound' }
  | { type: 'reset' };

export const initialState: GameState = {
  phase: 'connecting',
  connected: false,
  clientId: null,
  nickname: '',
  roomId: null,
  hand: [],
  selected: [],
  seats: [],
  turnClientId: null,
  lastSell: null,
  landlord: null,
  myType: null,
  result: null,
  error: null,
  lobbyError: null,
  promptBid: false,

  bottomCards: [],
  multiplier: 1,
  baseScore: 3,
  round: 1,
  playsBySeat: {},
  lobbyScreen: 'nickname',
};

function toggle(selected: number[], index: number): number[] {
  return selected.includes(index) ? selected.filter((i) => i !== index) : [...selected, index];
}

function removeCards(hand: Card[], toRemove: Card[]): Card[] {
  const remaining = [...toRemove];
  return hand.filter((card) => {
    const i = remaining.findIndex((r) => r.level === card.level && r.type === card.type);
    if (i >= 0) {
      remaining.splice(i, 1);
      return false;
    }
    return true;
  });
}

function sortHand(cards: Card[]): Card[] {
  return [...cards].sort((a, b) => a.level - b.level || a.type - b.type);
}

function seatsFromInfos(infos: ClientInfoMsg[]): SeatInfo[] {
  return infos.map((ci) => ({
    id: ci.clientId,
    nickname: ci.nickname,
    cardsLeft: ci.cardsLeft,
    isLandlord: ci.type === 0, // 0 = LANDLORD
  }));
}

/** True if the played cards form a bomb or rocket (doubles the multiplier). */
function isMultiplierBoost(pokers: Card[]): boolean {
  const s = evaluate(pokers);
  return !!s && (isBomb(s) || isKingBomb(s));
}

export function gameReducer(state: GameState, action: Action): GameState {
  switch (action.type) {
    case 'select':
      return { ...state, selected: toggle(state.selected, action.index) };
    case 'setSelection':
      return { ...state, selected: action.indices };
    case 'clearError':
      return { ...state, error: null };
    case 'clearLobbyError':
      return { ...state, lobbyError: null };
    case 'gotoLobby':
      return { ...state, lobbyScreen: action.screen };
    case 'bumpRound':
      return { ...state, round: state.round + 1 };
    case 'reset':
      // Replay a game: preserve nickname + round, clear everything else. The new
      // WS connection receives a fresh clientId via `idSet`; useGame reconnects.
      return { ...initialState, nickname: state.nickname, round: state.round };
    case 'server':
      return applyEvent(state, action.event);
  }
}

function applyEvent(state: GameState, e: ServerEvent): GameState {
  switch (e.event) {
    case 'connected':
      return {
        ...state,
        connected: true,
        phase: 'lobby',
        lobbyScreen: state.nickname ? 'menu' : 'nickname',
      };
    case 'idSet':
      return { ...state, clientId: e.data.clientId };
    case 'gameStarting':
      // The real gateway deals the player's 17 cards inside this event's `pokers`.
      // `nextClientId` is whoever bids for the landlord first. When that first
      // bidder is the human, the gateway sends NO separate `landlordElect`, so we
      // must raise the bid prompt straight from this event.
      return {
        ...state,
        hand: e.data.pokers ? [...e.data.pokers] : state.hand,
        selected: [],
        phase: 'bidding',
        turnClientId: e.data.nextClientId ?? null,
        promptBid: e.data.nextClientId != null && e.data.nextClientId === state.clientId,
        bottomCards: [],
        multiplier: 1,
        playsBySeat: {},
      };

    case 'landlordElect': {
      // Prompt to grab only when it is addressed to me.
      const mine = e.data.nextClientId === state.clientId;
      return { ...state, promptBid: mine, turnClientId: e.data.nextClientId };
    }

    case 'landlordConfirm': {
      const landlord = e.data.landlordId;
      const iAmLandlord = landlord === state.clientId;
      // The landlord receives the 3 bottom cards into their hand.
      const hand = iAmLandlord && e.data.additionalPokers
        ? sortHand([...state.hand, ...e.data.additionalPokers])
        : state.hand;
      // `additionalPokers` is broadcast to every player — capture it for the
      // bottom-strip reveal regardless of role.
      const bottomCards = e.data.additionalPokers ? [...e.data.additionalPokers] : state.bottomCards;
      return {
        ...state,
        phase: 'playing',
        landlord,
        promptBid: false,
        hand,
        bottomCards,
        multiplier: 1,
        playsBySeat: {},
        turnClientId: landlord, // landlord plays first
        myType: iAmLandlord ? 'LANDLORD' : 'PEASANT',
        seats: state.seats.map((s) => ({ ...s, isLandlord: s.id === landlord })),
      };
    }

    case 'showPokers': {
      // Broadcast: client `clientId` just played `pokers` (not "my hand").
      const who = e.data.clientId;
      const played = e.data.pokers ?? [];
      const isMine = who === state.clientId;
      const hand = isMine ? removeCards(state.hand, played) : state.hand;
      const seats = state.seats.map((s) =>
        s.id === who
          ? { ...s, cardsLeft: isMine ? hand.length : Math.max(0, s.cardsLeft - played.length) }
          : s,
      );
      return {
        ...state,
        hand,
        seats,
        selected: isMine ? [] : state.selected,
        turnClientId: isMine ? null : state.turnClientId,
        lastSell: played.length
          ? { client: who, nickname: e.data.clientNickname ?? '', pokers: played, type: 0 }
          : state.lastSell,
        playsBySeat: { ...state.playsBySeat, [who]: { pokers: played, passed: false } },
        multiplier: isMultiplierBoost(played) ? state.multiplier * 2 : state.multiplier,
        error: null,
      };
    }

    case 'playRedirect': {
      // Sent only to the player whose turn it is now (`sellClientId`).
      const infos = e.data.clientInfos ?? [];
      const seats = infos.length ? seatsFromInfos(infos) : state.seats;
      const lastPokers = e.data.lastSellPokers ?? [];
      const lastSell = lastPokers.length
        ? { client: e.data.lastSellClientId ?? 0, nickname: '', pokers: lastPokers, type: 0 }
        : state.lastSell;
      const myInfo = infos.find((i) => i.clientId === state.clientId);
      const myType = myInfo ? (myInfo.type === 0 ? 'LANDLORD' : 'PEASANT') : state.myType;
      return {
        ...state,
        phase: 'playing',
        hand: e.data.pokers ? [...e.data.pokers] : state.hand,
        seats,
        turnClientId: e.data.sellClientId,
        lastSell,
        myType,
        selected: [],
        error: null,
      };
    }

    case 'playPass':
      // A player (robot) passed. No playRedirect follows when the next turn is
      // the human, so advance the turn here from `nextClientId`; the standing
      // lead (lastSell) is unchanged.
      return {
        ...state,
        turnClientId: e.data.nextClientId,
        playsBySeat: { ...state.playsBySeat, [e.data.clientId]: { pokers: [], passed: true } },
        error: null,
      };

    case 'playError':
      return { ...state, error: e.data.code };

    case 'pveDifficultyNotSupport':
      return {
        ...state,
        lobbyError: '机器人资源不足，请重启 gateway 和 LLM agent 后再试',
      };

    case 'gameOver':
      return {
        ...state,
        phase: 'over',
        result: { winnerNickname: e.data.winnerNickname, winnerType: e.data.winnerType },
      };

    default:
      return state;
  }
}
