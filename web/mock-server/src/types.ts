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

export type ClientEvent =
  | { event: 'setNickname'; data: { nickname: string } }
  | { event: 'createRoomPve'; data: Record<string, never> }
  | { event: 'joinRoom'; data: { roomId: number } }
  | { event: 'landlordElect'; data: { grab: boolean } }
  | { event: 'play'; data: { pokers: Card[] } }
  | { event: 'pass'; data: Record<string, never> }
  | { event: 'exit'; data: Record<string, never> };

export type ServerEvent =
  | { event: 'idSet'; data: { clientId: number } }
  | { event: 'connected'; data: Record<string, never> }
  | { event: 'roomCreate'; data: { roomId: number; owner: string; clientCount: number; type: number } }
  | { event: 'roomJoin'; data: { roomId: number; owner: string; clientCount: number; type: number } }
  | { event: 'roomJoinFail'; data: { reason: 'full' | 'inexist' } }
  | { event: 'gameStarting'; data: Record<string, never> }
  | { event: 'landlordElect'; data: { client: number; nickname: string } }
  | { event: 'landlordConfirm'; data: { landlord: number; nickname: string } }
  | { event: 'showPokers'; data: { pokers: Card[] } }
  | { event: 'playTurn'; data: Record<string, never> }
  | { event: 'playRedirect'; data: { sellClient: number; sellNickname: string; sellPokers: Card[]; sellType: number; nextClient: number } }
  | { event: 'playPass'; data: { client: number; nextClient: number } }
  | { event: 'playError'; data: { reason: PlayErrorReason } }
  | { event: 'gameOver'; data: { winner: number; landlord: number; nickname: string } };

export type Emitter = (e: ServerEvent) => void;
