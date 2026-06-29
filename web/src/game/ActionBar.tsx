export interface ActionBarProps {
  myTurn: boolean;
  canPass: boolean;
  onPlay: () => void;
  onPass: () => void;
  /** 提示 is decorative (no client-side legal-play engine); GameView surfaces a toast. */
  onHint?: () => void;
  selectedCount?: number;
}

export function ActionBar({ myTurn, canPass, onPlay, onPass, onHint }: ActionBarProps) {
  return (
    <div className="actions">
      <button type="button" className="btn btn-ghost" disabled={!myTurn} onClick={onHint}>
        提示
      </button>
      <button type="button" className="btn btn-ghost" disabled={!myTurn || !canPass} onClick={onPass}>
        不出
      </button>
      <button type="button" className="btn btn-primary" disabled={!myTurn} onClick={onPlay}>
        出牌
      </button>
    </div>
  );
}
