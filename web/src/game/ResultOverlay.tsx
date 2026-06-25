import { Overlay } from '../components/Overlay';
import { Button } from '../components/Button';

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
    <Overlay>
      <div className="flex flex-col items-center text-center">
        <div
          className={[
            'flex h-16 w-16 items-center justify-center rounded-full text-3xl',
            iWon ? 'bg-success/15' : 'bg-danger/15',
          ].join(' ')}
        >
          {iWon ? '🏆' : '💔'}
        </div>
        <h2 className={`mt-4 font-display text-3xl font-semibold ${iWon ? 'text-success' : 'text-danger'}`}>
          {iWon ? '胜利' : '失败'}
        </h2>
        <p className="mt-1.5 text-sm text-ink-subtle">
          {landlordWon ? '地主获胜' : '农民获胜'}
          {winnerNickname ? ` · ${winnerNickname}` : ''}
        </p>
        <Button className="mt-6 w-full" size="lg" onClick={onAgain}>
          再来一局
        </Button>
      </div>
    </Overlay>
  );
}
