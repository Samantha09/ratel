export interface MainMenuProps {
  onPve: () => void;
  onPvp: () => void;
  lobbyError?: string | null;
}

/** 主菜单:人机对战 / 玩家对战。对应原型 ovStart。 */
export function MainMenu({ onPve, onPvp, lobbyError }: MainMenuProps) {
  return (
    <div className="overlay">
      <div className="panel">
        <div className="eyebrow">PLAYSTATION × CARD GAME</div>
        <h2>斗 地 主</h2>
        <p>经典三人纸牌对战。叫分抢地主，斗智斗勇，最先出完手牌的一方获胜。</p>
        {lobbyError && (
          <p style={{ color: '#ff6b6b', marginTop: 14, fontSize: 14 }}>⚠ {lobbyError}</p>
        )}
        <div className="cta-row">
          <button type="button" className="btn btn-primary" onClick={onPve}>
            🤖 人机对战
          </button>
          <button type="button" className="btn btn-ghost" onClick={onPvp}>
            👥 玩家对战
          </button>
        </div>
      </div>
    </div>
  );
}
