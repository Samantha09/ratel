import { Card, ClientInfoMsg, PlayErrorReason, ServerEvent } from '../types';

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
  promptBid: boolean;
}

export type Action =
  | { type: 'server'; event: ServerEvent }
  | { type: 'select'; index: number }
  | { type: 'clearError' }
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
  promptBid: false,
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

export function gameReducer(state: GameState, action: Action): GameState {
  switch (action.type) {
    case 'select':
      return { ...state, selected: toggle(state.selected, action.index) };
    case 'clearError':
      return { ...state, error: null };
    case 'reset':
      // Full reset to a fresh "connecting" state; useGame reconnects the socket,
      // and the incoming `connected` event moves us back to the lobby.
      return { ...initialState };
    case 'server':
      return applyEvent(state, action.event);
  }
}

function applyEvent(state: GameState, e: ServerEvent): GameState {
  switch (e.event) {
    case 'connected':
      return { ...state, connected: true, phase: 'lobby' };
    case 'idSet':
      return { ...state, clientId: e.data.clientId };
    case 'gameStarting':
      // The real gateway deals the player's 17 cards inside this event's `pokers`.
      // `nextClientId` is whoever bids for the landlord first.
      return {
        ...state,
        hand: e.data.pokers ? [...e.data.pokers] : state.hand,
        selected: [],
        phase: 'bidding',
        turnClientId: e.data.nextClientId ?? null,
      };

    case 'landlordElect': {
      // Prompt to grab only when it is addressed to me.
      const mine = e.data.nextClientId === state.clientId;
      return { ...state, promptBid: mine, turnClientId: e.data.nextClientId };
    }

    case 'landlordConfirm': {
      const landlord = e.data.landlordId;
      const iAmLandlord = landlord === state.clientId;
      // The landlord receives the 3 bottom cards.
      const hand = iAmLandlord && e.data.additionalPokers
        ? sortHand([...state.hand, ...e.data.additionalPokers])
        : state.hand;
      return {
        ...state,
        phase: 'playing',
        landlord,
        promptBid: false,
        hand,
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
      return { ...state, turnClientId: e.data.nextClientId, error: null };

    case 'playError':
      return { ...state, error: e.data.code };

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
