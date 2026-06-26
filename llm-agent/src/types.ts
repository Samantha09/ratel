export type PokerType = 0 | 1 | 2 | 3 | 4;
export type PokerLevel = number;

export interface Card {
  type: PokerType;
  level: PokerLevel;
}

export interface SeatInfo {
  clientId: number;
  nickname: string;
  type: number; // 0 = LANDLORD, 1 = PEASANT
  cardsLeft: number;
  position: string; // "up" | "down" | ""
}

export interface LastPlay {
  clientId: number;
  nickname: string;
  cards: Card[];
}

export interface AgentState {
  clientId: number;
  nickname: string;
  phase: 'lobby' | 'bidding' | 'playing' | 'over';
  hand: Card[];
  seats: SeatInfo[];
  turnClientId: number;
  lastPlay: LastPlay | null;
  consecutivePasses: number;
  landlordId: number | null;
  myRole: 'LANDLORD' | 'PEASANT' | null;
}

export type GamePhase = AgentState['phase'];

export interface Decision {
  action: 'play' | 'pass' | 'grab';
  cards?: Card[];
  reason: string;
}

export type ServerEvent =
  | { event: 'idSet'; data: { clientId: number } }
  | { event: 'connected'; data: Record<string, never> }
  | { event: 'showOptions'; data: Record<string, unknown> }
  | { event: 'gameStarting'; data: { pokers?: Card[]; clientId?: number; nextClientId?: number; nextClientNickname?: string; roomOwner?: string; roomClientCount?: number } }
  | { event: 'landlordElect'; data: { nextClientId: number; nextClientNickname?: string; preClientNickname?: string } }
  | { event: 'landlordConfirm'; data: { landlordId: number; landlordNickname?: string; additionalPokers?: Card[] } }
  | { event: 'showPokers'; data: { pokers: Card[]; clientId: number; clientNickname?: string; clientType?: number } }
  | { event: 'playRedirect'; data: { pokers?: Card[]; lastSellPokers?: Card[]; lastSellClientId?: number; sellClientId: number; sellClientNickname?: string; clientInfos?: SeatInfo[] } }
  | { event: 'playPass'; data: { clientId: number; clientNickname?: string; nextClientId: number; nextClientNickname?: string } }
  | { event: 'playError'; data: { code: string } }
  | { event: 'gameOver'; data: { winnerNickname: string; winnerType: string } }
  | { event: 'landlordCycle'; data: Record<string, unknown> };

export type ClientEvent =
  | { event: 'setNickname'; data: { nickname: string } }
  | { event: 'landlordElect'; data: { grab: boolean } }
  | { event: 'play'; data: { pokers: Card[] } }
  | { event: 'pass'; data: Record<string, never> };
