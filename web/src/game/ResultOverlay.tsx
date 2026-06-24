import { Overlay } from '../components/Overlay';
import { Button } from '../components/Button';

export interface ResultOverlayProps {
  winner: number;
  landlord: number;
  myClientId: number | null;
  onAgain: () => void;
}

export function ResultOverlay({ winner, landlord, myClientId, onAgain }: ResultOverlayProps) {
  const landlordWon = winner === landlord;
  const iAmLandlord = myClientId === landlord;
  const iWon = landlordWon ? iAmLandlord : !iAmLandlord;
  return (
    <Overlay>
      <h2 className={`font-display text-2xl font-semibold ${iWon ? 'text-success' : 'text-[#e5484d]'}`}>
        {iWon ? '胜利' : '失败'}
      </h2>
      <p className="mt-1 text-sm text-ink-subtle">
        {landlordWon ? '地主获胜' : '农民获胜'}
      </p>
      <Button className="mt-4 w-full" onClick={onAgain}>再来一局</Button>
    </Overlay>
  );
}
