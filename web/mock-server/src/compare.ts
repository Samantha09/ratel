import { Sell } from './cardtype.js';
import { SellType } from './types.js';

export function canBeat(candidate: Sell, last: Sell): boolean {
  if (candidate.type === SellType.KING_BOMB) return true;
  if (last.type === SellType.KING_BOMB) return false;
  if (candidate.type === SellType.BOMB && last.type !== SellType.BOMB) return true;
  if (last.type === SellType.BOMB && candidate.type !== SellType.BOMB) return false;
  if (candidate.type === SellType.BOMB && last.type === SellType.BOMB) {
    return candidate.coreLevel > last.coreLevel;
  }
  if (candidate.type === last.type && candidate.length === last.length) {
    return candidate.coreLevel > last.coreLevel;
  }
  return false;
}
