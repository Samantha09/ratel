import { CSSProperties } from 'react';
import { Card } from '../types';

const RANK = ['3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A', '2'];
const SUIT = ['', '♦', '♣', '♠', '♥']; // index by Card.type

function isJoker(card: Card): boolean {
  return card.level === 13 || card.level === 14;
}
function isBigJoker(card: Card): boolean {
  return card.level === 14;
}
function isRed(card: Card): boolean {
  if (card.level === 14) return true; // 大王 red
  if (card.level === 13) return false; // 小王 black
  return card.type === 1 || card.type === 4; // ♦ ♥
}
function rankText(level: number): string {
  return RANK[level] ?? '?';
}
function suitText(card: Card): string {
  return SUIT[card.type] ?? '';
}

export interface PlayingCardProps {
  card: Card;
  /** `hand` = interactive full card; `mini` = compact display (played area / bottom strip). */
  variant?: 'hand' | 'mini';
  selected?: boolean;
  dealing?: boolean;
  onClick?: () => void;
  style?: CSSProperties;
}

export function PlayingCard({ card, variant = 'hand', selected = false, dealing = false, onClick, style }: PlayingCardProps) {
  const joker = isJoker(card);
  const big = isBigJoker(card);
  const red = isRed(card);
  const rank = rankText(card.level);
  const suit = suitText(card);
  const jokerColor = big ? 'var(--card-red)' : '#1a1a1a';

  if (variant === 'mini') {
    return (
      <div className={['mini', joker ? 'joker' : '', red ? 'red' : ''].filter(Boolean).join(' ')} style={style}>
        {joker ? (
          <div className="jk" style={{ color: jokerColor }}>
            JOKER
          </div>
        ) : (
          <>
            <span className="r">{rank}</span>
            <span className="s">{suit}</span>
          </>
        )}
      </div>
    );
  }

  return (
    <button
      type="button"
      onClick={onClick}
      aria-pressed={selected}
      aria-label={joker ? (big ? '大王' : '小王') : `${suit}${rank}`}
      className={['card', joker ? 'joker' : '', red ? 'red' : '', selected ? 'selected' : '', dealing ? 'dealing' : '']
        .filter(Boolean)
        .join(' ')}
      style={style}
    >
      {joker ? (
        <>
          <span className="jtag" style={{ color: jokerColor }}>
            {big ? '大' : '小'}
          </span>
          <div className="jtext" style={{ color: jokerColor }}>
            JOKER
          </div>
        </>
      ) : (
        <>
          <div className="corner">
            <span className="rk">{rank}</span>
            <span className="su">{suit}</span>
          </div>
          <div className="big">{suit}</div>
        </>
      )}
    </button>
  );
}
