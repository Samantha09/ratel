import { Overlay } from '../components/Overlay';

export interface ResultOverlayProps {
  winnerNickname: string;
  winnerType: string; // "LANDLORD" | "PEASANT"
  myType: 'LANDLORD' | 'PEASANT' | null;
  onAgain: () => void;
}

export function ResultOverlay({ winnerNickname, winnerType, myType, onAgain }: ResultOverlayProps) {
  const landlordWon = winnerType === 'LANDLORD';
  const iWon = myType != null && myType === winnerType;

  return (
    <Overlay panelClassName="result">
      <div className="eyebrow">{landlordWon ? '地 主 获 胜' : '农 民 获 胜'}</div>
      <h2 className={iWon ? 'win' : 'lose'}>{iWon ? '胜 利' : '失 败'}</h2>
      <p>
        {landlordWon ? '地主' : '农民'}阵营获胜{winnerNickname ? ` · ${winnerNickname}` : ''}
      </p>
      <div className="cta-row">
        <button type="button" className="btn btn-primary" onClick={onAgain}>
          再来一局
        </button>
      </div>
    </Overlay>
  );
}
