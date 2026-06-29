export type PokerType = 0 | 1 | 2 | 3 | 4; // BLANK, DIAMOND, CLUB, SPADE, HEART
export type PokerLevel = number;             // 0..14 (see Global Constraints)
export interface Card { type: PokerType; level: PokerLevel; }

export const SellType = {
  ILLEGAL: 0, BOMB: 1, KING_BOMB: 2, SINGLE: 3, DOUBLE: 4, THREE: 5,
  THREE_ZONES_SINGLE: 6, THREE_ZONES_DOUBLE: 7,
  SINGLE_STRAIGHT: 10, DOUBLE_STRAIGHT: 11, VOID_SELL: 18,
} as const;
export type SellTypeValue = (typeof SellType)[keyof typeof SellType];

export type PlayErrorReason = 'mismatch' | 'less' | 'invalid' | 'order' | 'cantPass';

// One opponent/self entry inside a playRedirect (gateway `clientInfos`).
export interface ClientInfoMsg {
  clientId: number;
  nickname: string;
  type: number;        // 0 = LANDLORD, 1 = PEASANT
  cardsLeft: number;
  position: string;    // "up" | "down" | ""
}

export type ClientEvent =
  | { event: 'setNickname'; data: { nickname: string } }
  | { event: 'createRoomPve'; data: Record<string, never> }
  | { event: 'joinRoom'; data: { roomId: number } }
  | { event: 'landlordElect'; data: { grab: boolean } }
  | { event: 'play'; data: { pokers: Card[] } }
  | { event: 'pass'; data: Record<string, never> }
  | { event: 'exit'; data: Record<string, never> };

// Server -> frontend. Field names match the C++ gateway's JsonMapHelper egress.
export type ServerEvent =
  | { event: 'idSet'; data: { clientId: number } }
  | { event: 'connected'; data: Record<string, never> }
  | { event: 'showOptions'; data: Record<string, unknown> }
  | { event: 'gameStarting'; data: { pokers?: Card[]; clientId?: number; nextClientId?: number; nextClientNickname?: string; roomOwner?: string; roomClientCount?: number } }
  | { event: 'landlordElect'; data: { nextClientId: number; nextClientNickname?: string; preClientNickname?: string } }
  | { event: 'landlordConfirm'; data: { landlordId: number; landlordNickname?: string; additionalPokers?: Card[] } }
  | { event: 'showPokers'; data: { pokers: Card[]; clientId: number; clientNickname?: string; clientType?: number } }
  | { event: 'playRedirect'; data: { pokers?: Card[]; lastSellPokers?: Card[]; lastSellClientId?: number; sellClientId: number; sellClientNickname?: string; clientInfos?: ClientInfoMsg[] } }
  | { event: 'playPass'; data: { clientId: number; clientNickname?: string; nextClientId: number; nextClientNickname?: string } }
  | { event: 'playError'; data: { code: PlayErrorReason } }
  | { event: 'pveDifficultyNotSupport'; data: Record<string, never> }
  | { event: 'gameOver'; data: { winnerNickname: string; winnerType: string } }
  | { event: 'landlordCycle'; data: Record<string, unknown> };

export type Emitter = (e: ServerEvent) => void;
