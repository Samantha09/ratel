import { Card } from '../types';
import { PlayingCard } from '../components/PlayingCard';

export interface HandProps {
  hand: Card[];
  selected: number[];
  onToggle: (index: number) => void;
}

export function Hand({ hand, selected, onToggle }: HandProps) {
  return (
    <div className="flex flex-wrap items-end justify-center gap-1.5">
      {hand.map((card, i) => (
        <PlayingCard
          key={`${card.level}:${card.type}:${i}`}
          card={card}
          selected={selected.includes(i)}
          onClick={() => onToggle(i)}
        />
      ))}
    </div>
  );
}
