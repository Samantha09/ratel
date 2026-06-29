import { Card } from '../types';
import { PlayingCard } from '../components/PlayingCard';

export interface HandProps {
  hand: Card[];
  selected: number[];
  onToggle: (index: number) => void;
  /** Play the deal-in stagger animation (fresh deal). */
  dealing?: boolean;
}

export function Hand({ hand, selected, onToggle, dealing = false }: HandProps) {
  return (
    <div className="hand" aria-label="我的手牌">
      {hand.map((card, i) => (
        <PlayingCard
          key={`${card.level}:${card.type}:${i}`}
          card={card}
          selected={selected.includes(i)}
          dealing={dealing}
          onClick={() => onToggle(i)}
          style={dealing ? { animationDelay: `${i * 0.03}s` } : undefined}
        />
      ))}
    </div>
  );
}
