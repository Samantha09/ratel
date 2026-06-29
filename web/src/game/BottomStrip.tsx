import { Card } from '../types';
import { PlayingCard } from '../components/PlayingCard';

export interface BottomStripProps {
  cards: Card[];
}

/**
 * 底牌揭示条 — the 3 landlord bottom cards, revealed to everyone after bidding.
 * `landlordConfirm` broadcasts `additionalPokers` to all players, so this is real
 * data (not mock). Rendered only once the bottom cards are known.
 */
export function BottomStrip({ cards }: BottomStripProps) {
  if (!cards.length) return null;
  return (
    <div className="bottom-strip show">
      <span className="label">底牌</span>
      {cards.map((card, i) => (
        <PlayingCard key={`${card.level}:${card.type}:${i}`} card={card} variant="mini" />
      ))}
    </div>
  );
}
