import { Overlay } from '../components/Overlay';
import { Button } from '../components/Button';

export interface ResultOverlayProps {
  winnerNickname: string;
  winnerType: string;            // "LANDLORD" | "PEASANT"
  myType: 'LANDLORD' | 'PEASANT' | null;
  onAgain: () => void;
}

export function ResultOverlay({ winnerNickname, winnerType, myType, onAgain }: ResultOverlayProps) {
  const landlordWon = winnerType === 'LANDLORD';
  const iWon = myType != null && myType === winnerType;
  return (
    <Overlay>
      <h2 className={`font-display text-2xl font-semibold ${iWon ? 'text-success' : 'text-[#e5484d]'}`}>
        {iWon ? '胜利' : '失败'}
      </h2>
      <p className="mt-1 text-sm text-ink-subtle">
        {landlordWon ? '地主获胜' : '农民获胜'}
        {winnerNickname ? ` · ${winnerNickname}` : ''}
      </p>
      <Button className="mt-4 w-full" onClick={onAgain}>再来一局</Button>
    </Overlay>
  );
}
