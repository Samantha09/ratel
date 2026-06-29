import { useState } from 'react';

export interface LobbyViewProps {
  connecting: boolean;
  lobbyError?: string | null;
  onClearLobbyError?: () => void;
  onCreate: (nickname: string) => void;
}

export function LobbyView({ connecting, lobbyError, onClearLobbyError, onCreate }: LobbyViewProps) {
  const [nickname, setNickname] = useState('');
  const ready = !connecting && nickname.trim().length > 0;

  const submit = () => {
    if (ready) onCreate(nickname.trim());
  };

  return (
    <div className="overlay">
      <div className="panel">
        <div className="eyebrow">PLAYSTATION × CARD GAME</div>
        <h2>斗 地 主</h2>
        <p>经典三人纸牌对战。叫分抢地主，斗智斗勇，最先出完手牌的一方获胜。</p>

        <div className="join-bar">
          <input
            className="room-input"
            style={{ letterSpacing: '1px' }}
            value={nickname}
            onChange={(e) => {
              setNickname(e.target.value);
              onClearLobbyError?.();
            }}
            onKeyDown={(e) => e.key === 'Enter' && submit()}
            placeholder="请输入昵称"
            autoFocus
          />
        </div>

        {lobbyError && (
          <p style={{ color: '#ff6b6b', marginTop: 14, fontSize: 14 }}>⚠ {lobbyError}</p>
        )}

        <div className="cta-row">
          <button type="button" className="btn btn-primary" disabled={!ready} onClick={submit}>
            {connecting ? '连接中…' : '开始游戏'}
          </button>
        </div>
      </div>
    </div>
  );
}
