import { Card } from '../types';
import { Overlay } from '../components/Overlay';
import { PlayingCard } from '../components/PlayingCard';

export interface ResultOverlayProps {
  winnerNickname: string;
  winnerType: string; // "LANDLORD" | "PEASANT"
  myType: 'LANDLORD' | 'PEASANT' | null;
  bottomCards: Card[];
  baseScore: number;
  multiplier: number;
  onAgain: () => void;
}

export function ResultOverlay({
  winnerNickname,
  winnerType,
  myType,
  bottomCards,
  baseScore,
  multiplier,
  onAgain,
}: ResultOverlayProps) {
  const landlordWon = winnerType === 'LANDLORD';
  const iWon = myType != null && myType === winnerType;
  const points = baseScore * multiplier;

  return (
    <Overlay panelClassName="result">
      <div className="eyebrow">{landlordWon ? '地 主 获 胜' : '农 民 获 胜'}</div>
      <h2 className={iWon ? 'win' : 'lose'}>{iWon ? '胜 利' : '失 败'}</h2>
      <p>
        {landlordWon ? '地主' : '农民'}阵营获胜{winnerNickname ? ` · ${winnerNickname}` : ''} · 本局{' '}
        <b style={{ color: '#fff' }}>{points}</b> 分（底分 {baseScore} × 倍数 {multiplier}）
      </p>
      {bottomCards.length > 0 && (
        <div className="bottom-cards">
          {bottomCards.map((card, i) => (
            <PlayingCard key={`${card.level}:${card.type}:${i}`} card={card} variant="mini" />
          ))}
        </div>
      )}
      <div className="cta-row">
        <button type="button" className="btn btn-primary" onClick={onAgain}>
          再来一局
        </button>
      </div>
    </Overlay>
  );
}
