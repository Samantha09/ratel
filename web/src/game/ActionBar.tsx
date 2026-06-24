import { Button } from '../components/Button';

export interface ActionBarProps {
  myTurn: boolean;
  canPass: boolean;
  onPlay: () => void;
  onPass: () => void;
}

export function ActionBar({ myTurn, canPass, onPlay, onPass }: ActionBarProps) {
  return (
    <div className="flex items-center justify-center gap-3">
      <Button variant="secondary" disabled={!myTurn || !canPass} onClick={onPass}>不出</Button>
      <Button disabled={!myTurn} onClick={onPlay}>出牌</Button>
    </div>
  );
}
