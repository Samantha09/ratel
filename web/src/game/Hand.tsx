import { useState } from 'react';
import { Card } from '../types';
import { PlayingCard } from '../components/PlayingCard';

export interface HandProps {
  hand: Card[];
  selected: number[];
  onToggle: (index: number) => void;
}

export function Hand({ hand, selected, onToggle }: HandProps) {
  const [hovered, setHovered] = useState<number | null>(null);

  return (
    <div className="flex items-end justify-center px-2">
      {hand.map((card, i) => {
        const isSelected = selected.includes(i);
        // Lift selected/hovered cards above their neighbours so the whole face
        // reads; otherwise only the top-left index column shows (covered body
        // stays hidden under the next card).
        const z = isSelected ? 100 + i : hovered === i ? 300 : i;
        return (
          <div
            key={`${card.level}:${card.type}:${i}`}
            className="relative transition-transform"
            style={{ marginLeft: i === 0 ? 0 : '-2.15rem', zIndex: z }}
            onMouseEnter={() => setHovered(i)}
            onMouseLeave={() => setHovered((h) => (h === i ? null : h))}
          >
            <PlayingCard card={card} selected={isSelected} onClick={() => onToggle(i)} />
          </div>
        );
      })}
    </div>
  );
}
