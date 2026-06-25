import { Card } from '../types';

const RANK = ['3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A', '2'];
const SUIT = ['', '♦', '♣', '♠', '♥'];

function rankText(level: number): string {
  if (level === 13) return '小王';
  if (level === 14) return '大王';
  return RANK[level] ?? '?';
}

function suitText(card: Card): string {
  if (card.level === 13 || card.level === 14) return '';
  return SUIT[card.type] ?? '';
}

function isJoker(card: Card): boolean {
  return card.level === 13 || card.level === 14;
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
  /** Compact face used inside the play area / opponent rows. */
  mini?: boolean;
}

export function PlayingCard({ card, selected = false, onClick, mini = false }: PlayingCardProps) {
  const red = isRed(card);
  const joker = isJoker(card);
  const rank = rankText(card.level);
  const suit = suitText(card);

  const size = mini ? 'h-12 w-9 rounded-md p-1' : 'h-[88px] w-16 rounded-lg p-1.5';
  const cornerRank = mini ? 'text-[11px]' : 'text-sm';
  const cornerSuit = mini ? 'text-[9px]' : 'text-[11px]';
  const centerSize = mini ? 'text-lg' : 'text-3xl';

  return (
    <button
      type="button"
      onClick={onClick}
      aria-pressed={selected}
      aria-label={joker ? rank : `${suit}${rank}`}
      className={[
        'card-face relative flex shrink-0 flex-col justify-between',
        red ? 'card-face--red' : '',
        mini ? 'card-face--mini' : '',
        'transition-all duration-150 ease-out',
        onClick
          ? 'hover:-translate-y-1 hover:shadow-card-lift focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary'
          : '',
        selected ? '-translate-y-2 ring-2 ring-primary shadow-card-lift' : '',
        size,
      ].join(' ')}
    >
      {joker ? (
        <span className="flex flex-1 flex-col items-center justify-center gap-0.5">
          <span className={`font-display font-bold leading-none ${mini ? 'text-[11px]' : 'text-base'}`}>{rank}</span>
          {!mini && <span className="text-[10px] tracking-[0.2em] text-ink-subtle">JOKER</span>}
        </span>
      ) : (
        <>
          <span className="flex flex-col items-center self-start leading-none">
            <span className={`font-display font-bold ${cornerRank}`}>{rank}</span>
            <span className={cornerSuit} aria-hidden="true">{suit}</span>
          </span>
          <span className={`text-center leading-none ${centerSize}`}>{suit}</span>
          <span
            className={`self-end font-display font-bold leading-none opacity-0 ${cornerRank}`}
            aria-hidden="true"
          >
            {rank}
          </span>
        </>
      )}
    </button>
  );
}
