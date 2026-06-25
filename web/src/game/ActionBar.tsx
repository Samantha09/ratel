import { Button } from '../components/Button';

export interface ActionBarProps {
  myTurn: boolean;
  canPass: boolean;
  onPlay: () => void;
  onPass: () => void;
  selectedCount?: number;
}

export function ActionBar({ myTurn, canPass, onPlay, onPass, selectedCount = 0 }: ActionBarProps) {
  return (
    <div className="flex flex-col items-center gap-2.5">
      <p className="text-xs text-ink-subtle" role="status">
        {myTurn ? (
          selectedCount > 0 ? (
            <>
              已选 <span className="font-mono text-ink">{selectedCount}</span> 张 · 轮到你出牌
            </>
          ) : (
            <span className="text-primary-hover">轮到你出牌</span>
          )
        ) : (
          '等待其他玩家…'
        )}
      </p>
      <div className="flex items-center justify-center gap-3">
        <Button variant="secondary" size="lg" disabled={!myTurn || !canPass} onClick={onPass}>
          不出
        </Button>
        <Button size="lg" disabled={!myTurn} onClick={onPlay}>
          出牌
        </Button>
      </div>
    </div>
  );
}
