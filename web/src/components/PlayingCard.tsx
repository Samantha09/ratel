import { Card } from '../types';

const RANK = ['3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A', '2'];
const SUIT = ['', '♦', '♣', '♠', '♥'];

function rankText(level: number): string {
  if (level === 13) return '小王';
  if (level === 14) return '大王';
  return RANK[level];
}

function suitText(card: Card): string {
  if (card.level === 13 || card.level === 14) return '';
  return SUIT[card.type];
}

function isRed(card: Card): boolean {
  if (card.level === 14) return true; // 大王 red
  if (card.level === 13) return false; // 小王 black
  return card.type === 1 || card.type === 4; // ♦ ♥
}

export interface PlayingCardProps {
  card: Card;
  selected?: boolean;
  onClick?: () => void;
}

export function PlayingCard({ card, selected = false, onClick }: PlayingCardProps) {
  const red = isRed(card);
  return (
    <button
      type="button"
      onClick={onClick}
      className={[
        'flex h-28 w-20 flex-col justify-between rounded-lg border p-2 transition-transform',
        'bg-surface-2 border-hairline',
        selected ? '-translate-y-2 border-primary' : '',
        red ? 'text-[#e5484d]' : 'text-ink',
      ].join(' ')}
    >
      <span className="font-display text-sm font-semibold leading-none">{rankText(card.level)}</span>
      <span className="text-center text-2xl leading-none">{suitText(card)}</span>
      <span className="self-end font-display text-sm font-semibold leading-none opacity-0">.</span>
    </button>
  );
}
