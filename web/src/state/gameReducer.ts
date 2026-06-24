import { Card, PlayErrorReason, ServerEvent } from '../types';

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
  result: { winner: number; landlord: number; nickname: string } | null;
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
  seats: [
    { id: 0, nickname: '', cardsLeft: 17, isLandlord: false },
    { id: 1, nickname: '', cardsLeft: 17, isLandlord: false },
    { id: 2, nickname: '', cardsLeft: 17, isLandlord: false },
  ],
  turnClientId: null,
  lastSell: null,
  landlord: null,
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

export function reducer(state: GameState, action: Action): GameState {
  switch (action.type) {
    case 'select':
      return { ...state, selected: toggle(state.selected, action.index) };
    case 'clearError':
      return { ...state, error: null };
    case 'reset':
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
    case 'roomCreate':
    case 'roomJoin':
      return { ...state, roomId: e.data.roomId };
    case 'roomJoinFail':
      return { ...state, error: null, roomId: null };
    case 'gameStarting':
      return { ...state, phase: 'bidding' };
    case 'landlordElect':
      return e.data.client === state.clientId ? { ...state, promptBid: true } : state;
    case 'landlordConfirm': {
      const seats = state.seats.map((s) =>
        s.id === e.data.landlord ? { ...s, isLandlord: true, nickname: e.data.nickname, cardsLeft: 20 } : s,
      );
      return { ...state, landlord: e.data.landlord, seats, phase: 'playing', promptBid: false };
    }
    case 'showPokers':
      return { ...state, hand: [...e.data.pokers], selected: [] };
    case 'playTurn': {
      const me = state.clientId;
      return { ...state, turnClientId: me, error: null };
    }
    case 'playRedirect': {
      const { sellClient, sellNickname, sellPokers, nextClient } = e.data;
      const isMine = sellClient === state.clientId;
      const hand = isMine ? removeCards(state.hand, sellPokers) : state.hand;
      const seats = state.seats.map((s) => {
        if (s.id === sellClient) {
          return {
            ...s,
            nickname: sellNickname || s.nickname,
            cardsLeft: isMine ? hand.length : Math.max(0, s.cardsLeft - sellPokers.length),
          };
        }
        return s;
      });
      return {
        ...state,
        hand,
        seats,
        selected: isMine ? [] : state.selected,
        lastSell: { client: sellClient, nickname: sellNickname, pokers: sellPokers, type: e.data.sellType },
        turnClientId: nextClient,
        error: null,
      };
    }
    case 'playPass':
      return { ...state, turnClientId: e.data.nextClient, error: null };
    case 'playError':
      return { ...state, error: e.data.reason };
    case 'gameOver':
      return { ...state, phase: 'over', result: e.data };
    default:
      return state;
  }
}
